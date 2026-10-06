#!/usr/bin/env python3
"""cvx2 code generator CLI.

Reads a JSON config listing one or more container instantiations, and for
each one expands the matching template (cvx2/templates/<template>.h/.c)
into a simple file containing no extra macros and outputs them to the
configured directories by `out_dir`.

Usage:
    python3 generator.py --config path/to/config.json --compiler gcc

This file should be accompanied by the /templates folder containing all the
cvx template files. Both .h/.c should be in the same folder.
"""

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

TEMPLATES_DIR = Path(__file__).resolve().parent.parent / "templates"
IDENTIFIER_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


class ConfigError(Exception):
    pass


class PreprocessError(Exception):
    pass


###
### CASE CONVERSION
###
# Only ever applied to library-provided name fragments (_init, _vtabv,
# ...) -- user-provided identifiers (struct_name, prefix) are always left
# exactly as given. E.g. with prefix "ga_" and suffix "_push_back":
#   snake_case -> "ga_push_back", camelCase -> "ga_pushBack" (fragment's
#   own first word stays lowercase), PascalCase -> "ga_PushBack".

CASE_SNAKE = "snake_case"
CASE_CAMEL = "camelCase"
CASE_PASCAL = "PascalCase"
VALID_CASES = (CASE_SNAKE, CASE_CAMEL, CASE_PASCAL)

UNDERSCORE_RUN_RE = re.compile(r"(_+)")


def identity(identifier):
    return identifier


def _capitalize_words(identifier, capitalize_first_word):
    out = []
    seen_word = False
    for word in UNDERSCORE_RUN_RE.split(identifier):
        if not word or word == "_":
            continue
        if word.startswith("_"):
            out.append(word)  # 2+ underscores: the private-helper marker, kept literally
            continue
        if not seen_word and not capitalize_first_word:
            out.append(word)
        else:
            out.append(word[:1].upper() + word[1:])
        seen_word = True
    return "".join(out)


def to_camel_case(fragment):
    """"_push_back" -> "pushBack": own first word stays lowercase."""
    return _capitalize_words(fragment, capitalize_first_word=False)


def to_pascal_case(fragment):
    """"_push_back" -> "PushBack": own first word is capitalized too."""
    return _capitalize_words(fragment, capitalize_first_word=True)


def case_fn_for(case_name):
    if case_name == CASE_SNAKE:
        return identity
    if case_name == CASE_CAMEL:
        return to_camel_case
    if case_name == CASE_PASCAL:
        return to_pascal_case
    raise ValueError(f"unknown case {case_name!r}, expected one of {VALID_CASES}")


###
### TEMPLATE EXPANSION
###
# A template's own macros (FUNC(X), VTAB_V, etc.) and placeholders
# (CVX_VAL, CVX_SNAME, ...) are real C preprocessor constructs -- so
# instead of re-implementing macro expansion by hand, we hand the file to
# the real preprocessor (-D for each placeholder/variant) and let it do
# that part. We deliberately never let it see cvx2/core.h, cvx2/flags.h,
# or any system header: every #include line is stripped before the
# preprocessor runs and spliced back in literally afterwards, so they stay
# real #includes in the output rather than being inlined -- and so e.g.
# "bool" is never touched (it only becomes "_Bool" if <stdbool.h> actually
# gets processed, which this never lets happen).
#
# Only CVX_(A, B) token-pasting is left for us to resolve ourselves: it's
# the one piece of case-conversion logic (CVX_SNAME's own text stays
# literal; only the pasted library suffix gets cased) that no C tool can
# do, so the templates' own paste macro is deliberately left undefined for
# the preprocessor and resolved here afterward instead.

# Required flags per compiler, in case one ever needs something different
# from -E -C (preprocess, keep comments).
COMPILER_FLAGS = {
    "gcc": ["-E", "-C"],
    "clang": ["-E", "-C"],
    "cc": ["-E", "-C"],
}

_INCLUDE_RE = re.compile(r"^[ \t]*#include[^\n]*\n?", re.MULTILINE)
_DEFINE_LINE_RE = re.compile(r"^[ \t]*#define\b[^\n]*\n?", re.MULTILINE)
_LINE_MARKER_RE = re.compile(r'^# \d+ "([^"]*)"')
_SYSTEM_PREFIXES = ("/usr", "/opt/homebrew", "/Library", "<built-in>", "<command-line>", "<command line>")
_EMPTY_CLANG_FORMAT_RE = re.compile(r"[ \t]*//[ \t]*clang-format off[ \t]*\n[ \t]*//[ \t]*clang-format on[ \t]*\n")
_BLANK_RUN_RE = re.compile(r"\n{3,}")
_PASTE_RE = re.compile(r"CVX_\(\s*(\w+)\s*,\s*(\w+)\s*\)")


def _strip_includes(text):
    """Pulls every #include line out of text (so the preprocessor never
    resolves them) and returns (includes_block, remaining_text). The
    fallback.h include is dropped outright rather than kept: it exists
    only so the template compiles standalone while editing, and must
    never appear in generated output."""
    includes = [inc for inc in _INCLUDE_RE.findall(text) if "cvx2/fallback.h" not in inc]
    return "".join(includes), _INCLUDE_RE.sub("", text)


def _local_macro_defines(header_text):
    """Raw #define lines (FUNC, VTAB_V, etc.) copied verbatim out of a
    template header's text, to prepend to the matching .c's text before
    preprocessing it: the .c relies on these via its own #include of the
    .h, which gets stripped along with every other #include."""
    return "".join(_DEFINE_LINE_RE.findall(header_text))


def _run_preprocessor(text, defines, compiler):
    if compiler not in COMPILER_FLAGS:
        raise PreprocessError(f"unknown compiler {compiler!r}, expected one of {sorted(COMPILER_FLAGS)}")

    cmd = [compiler, *COMPILER_FLAGS[compiler], "-x", "c", *(f"-D{d}" for d in defines), "-"]
    try:
        result = subprocess.run(cmd, input=text, capture_output=True, text=True)
    except FileNotFoundError:
        raise PreprocessError(f"compiler {compiler!r} not found on PATH")

    if result.returncode != 0:
        raise PreprocessError(result.stderr)

    # Keep only lines that trace back to our own input (stdin), not to any
    # implicitly preincluded system file (e.g. glibc's stdc-predef.h).
    out = []
    skip = False
    for line in result.stdout.splitlines():
        m = _LINE_MARKER_RE.match(line)
        if m:
            skip = m.group(1).startswith(_SYSTEM_PREFIXES)
            continue
        if not skip:
            out.append(line)
    return "\n".join(out)


def _resolve_pastes(text, case_fn):
    # By now CVX_(A, B)'s A and B are already literal text (the
    # preprocessor substituted CVX_SNAME/CVX_PFX/etc. for us) -- A is
    # always user text and stays as-is; B is always a library suffix and
    # gets cased.
    def repl(match):
        prefix, suffix = match.groups()
        return prefix + case_fn(suffix)

    for _ in range(4):  # in case a macro body itself contains another paste
        new_text = _PASTE_RE.sub(repl, text)
        if new_text == text:
            return new_text
        text = new_text
    return text


def expand(text, defines, compiler, case_fn):
    includes, stripped = _strip_includes(text)
    expanded = _run_preprocessor(stripped, defines, compiler)
    expanded = _resolve_pastes(expanded, case_fn)
    expanded = _EMPTY_CLANG_FORMAT_RE.sub("", expanded)
    expanded = _BLANK_RUN_RE.sub("\n\n", expanded)
    return includes + expanded


def rewrite_self_include(text, template_name, generated_header_filename):
    return re.sub(
        rf'#include\s+"{re.escape(template_name)}\.h"',
        f'#include "{generated_header_filename}"',
        text,
        count=1,
    )


###
### CONFIG VALIDATION
###
# Validates config files against config.schema.json's shape (kept in sync
# by hand -- no jsonschema dependency).

# Required string fields beyond struct_name/prefix, per template.
TEMPLATE_SCHEMAS = {
    "dynamic_array": {"scalar_fields": ["value_type"]},
    "hashtable": {"scalar_fields": ["key_type", "value_type"]},
}

# Per template, each @cvx2:variant axis and the macro to #define for each
# of its names (must match what the template's #ifdef actually checks).
TEMPLATE_VARIANTS = {
    "hashtable": {
        "collision": {
            "open_addressing": "CVX2_COLLISION_OPEN_ADDRESSING",
            "separate_chaining": "CVX2_COLLISION_SEPARATE_CHAINING",
        },
    },
}

_MISSING = object()


def _err(where, message):
    raise ConfigError(f"{where}: {message}")


def _field(instantiation, where, key, default=_MISSING, identifier=False, allowed=None):
    """Reads and validates a single string field: required unless `default`
    is given, optionally must be a valid C identifier, optionally must be
    one of `allowed`."""
    if key not in instantiation:
        if default is not _MISSING:
            return default
        _err(where, f"missing required field {key!r}")

    value = instantiation[key]
    valid = isinstance(value, str)
    if valid and identifier:
        valid = IDENTIFIER_RE.match(value) is not None
    if valid and allowed is not None:
        valid = value in allowed

    if not valid:
        kind = "a valid C identifier" if identifier else f"one of {allowed}" if allowed else "a string"
        _err(where, f"field {key!r} must be {kind}, got {value!r}")
    return value


def _check_type_expression(where, field, value):
    """Rejects raw array/function-pointer syntax in value_type/key_type --
    neither can be expressed as a plain declarator prefix; a typedef is
    needed instead (see the error message)."""
    if "[" in value or "(" in value:
        _err(
            where,
            f"field {field!r} = {value!r} looks like a raw array or function-pointer type, which "
            f"can't be spliced in as a plain declarator prefix (e.g. it would generate invalid C "
            f"like '{value} item;' instead of 'item[10];' or '(*item)(...)'). Define a typedef for "
            f"it instead (e.g. `typedef struct {{ int items[10]; }} my_array10_t;` or "
            f"`typedef int (*my_fn_t)(int, int);`) and set {field!r} to that typedef's name -- and "
            f"add the header that defines it to this instantiation's 'includes' list, since the "
            f"generated .c is its own translation unit and won't otherwise see it.",
        )


def validate_instantiation(instantiation, index):
    where = f"instantiations[{index}]"

    if not isinstance(instantiation, dict):
        _err(where, f"must be an object, got {instantiation!r}")

    template = _field(instantiation, where, "template", allowed=tuple(TEMPLATE_SCHEMAS))
    schema = TEMPLATE_SCHEMAS[template]

    for field in schema["scalar_fields"]:
        value = _field(instantiation, where, field)
        _check_type_expression(where, field, value)

    struct_name = _field(instantiation, where, "struct_name", identifier=True)
    prefix = _field(instantiation, where, "prefix", identifier=True)
    file_name = _field(instantiation, where, "file_name", default=None, identifier=True)
    case_name = _field(instantiation, where, "case", default=CASE_SNAKE, allowed=VALID_CASES)
    out_dir = _field(instantiation, where, "out_dir", default=".")

    axes = TEMPLATE_VARIANTS.get(template, {})
    variants = instantiation.get("variants", {})
    if not isinstance(variants, dict):
        _err(where, "field 'variants' must be an object mapping axis name -> variant name")
    for axis, names in axes.items():
        if axis not in variants:
            _err(where, f"template {template!r} requires variants[{axis!r}] to be set (choices: {sorted(names)})")
    for axis, name in variants.items():
        if axis not in axes:
            _err(
                where,
                f"config specifies variants[{axis!r}] but template {template!r} has no such axis "
                f"(it uses: {sorted(axes)})",
            )
        elif name not in axes[axis]:
            _err(where, f"variants[{axis!r}] = {name!r} is not a known variant (choices: {sorted(axes[axis])})")

    # Headers needed for value_type/key_type, if not a builtin.
    includes = instantiation.get("includes", [])
    if not isinstance(includes, list) or not all(isinstance(h, str) for h in includes):
        _err(where, "field 'includes' must be an array of strings (header paths)")

    return {
        "template": template,
        "value_type": instantiation.get("value_type"),
        "key_type": instantiation.get("key_type"),
        "struct_name": struct_name,
        "prefix": prefix,
        "file_name": file_name,
        "case": case_name,
        "variants": variants,
        "out_dir": out_dir,
        "includes": includes,
    }


def validate_config(raw_config):
    if not isinstance(raw_config, dict) or "instantiations" not in raw_config:
        raise ConfigError("config must be an object with an 'instantiations' array")

    instantiations = raw_config["instantiations"]
    if not isinstance(instantiations, list) or not instantiations:
        raise ConfigError("'instantiations' must be a non-empty array")

    return [validate_instantiation(inst, i) for i, inst in enumerate(instantiations)]


###
### GENERATION
###


def _include_directive(header):
    """Formats one entry of an instantiation's "includes" list as a real
    #include line. "<foo.h>" is passed through as a system include;
    anything else is treated as a local path and quoted."""
    header = header.strip()
    if header.startswith("<") and header.endswith(">"):
        return f"#include {header}"
    return f'#include "{header}"'


def _defines_for(inst):
    defines = [f"CVX_VAL={inst['value_type']}", f"CVX_SNAME={inst['struct_name']}", f"CVX_PFX={inst['prefix']}"]
    if inst["key_type"] is not None:
        defines.append(f"CVX_KEY={inst['key_type']}")

    axes = TEMPLATE_VARIANTS.get(inst["template"], {})
    for axis, name in inst["variants"].items():
        defines.append(axes[axis][name])
    return defines


def generate_instantiation(inst, compiler, templates_dir=TEMPLATES_DIR):
    template_name = inst["template"]
    case_fn = case_fn_for(inst["case"])
    defines = _defines_for(inst)

    header_text = (templates_dir / f"{template_name}.h").read_text()
    source_text = (templates_dir / f"{template_name}.c").read_text()

    header_out = expand(header_text, defines, compiler, case_fn)
    source_out = expand(_local_macro_defines(header_text) + source_text, defines, compiler, case_fn)

    file_stem = inst["file_name"] or inst["struct_name"]
    header_filename = f"{file_stem}.h"
    source_filename = f"{file_stem}.c"

    guard = f"{file_stem.upper()}_H"
    includes_block = "".join(f"{_include_directive(h)}\n" for h in inst["includes"])
    header_out = f"#ifndef {guard}\n#define {guard}\n\n{includes_block}{header_out.strip()}\n\n#endif /* {guard} */\n"

    source_out = rewrite_self_include(source_out, template_name, header_filename)
    source_out = source_out.strip() + "\n"

    return header_filename, header_out, source_filename, source_out


def run(config_path, compiler):
    raw_config = json.loads(Path(config_path).read_text())
    instantiations = validate_config(raw_config)

    written = []
    for inst in instantiations:
        header_filename, header_out, source_filename, source_out = generate_instantiation(inst, compiler)

        out_dir = Path(config_path).parent / inst["out_dir"]
        out_dir.mkdir(parents=True, exist_ok=True)

        (out_dir / header_filename).write_text(header_out)
        (out_dir / source_filename).write_text(source_out)
        written.append(str(out_dir / header_filename))
        written.append(str(out_dir / source_filename))

    return written


def main(argv=None):
    parser = argparse.ArgumentParser(description="Generate cvx2 container types from a JSON config.")
    parser.add_argument("--config", required=True, help="path to the JSON config file")
    parser.add_argument(
        "--compiler", required=True, choices=sorted(COMPILER_FLAGS), help="C compiler to preprocess templates with"
    )
    args = parser.parse_args(argv)

    try:
        written = run(args.config, args.compiler)
    except (ConfigError, PreprocessError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    for path in written:
        print(f"wrote {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

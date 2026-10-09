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
NAMES_H_PATH = Path(__file__).resolve().parent.parent / "names.h"
IDENTIFIER_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


class ConfigError(Exception):
    pass


class PreprocessError(Exception):
    pass


###
### CASE CONVERSION
###

# based on names.h
CASE_SNAKE = "snake_case"
CASE_CAMEL = "camelCase"
CASE_PASCAL = "PascalCase"
VALID_CASES = (CASE_SNAKE, CASE_CAMEL, CASE_PASCAL)

CASE_DEFINES = {
    CASE_CAMEL: "CVX_NAMES_CAMELCASE",
    CASE_PASCAL: "CVX_NAMES_PASCALCASE",
}

LINE_COMMENT_RE = re.compile(r"^[ \t]*//.*\n?", re.MULTILINE)
# Only the #ifdef/#define/#endif skeleton is needed by the preprocessor --
# names.h's own explanatory comments aren't for consumers of generated code
# and would otherwise leak into it (comments are normally preserved, see
# TEMPLATE EXPANSION below), so they're stripped once, up front.
NAMES_H_DEFS = LINE_COMMENT_RE.sub("", NAMES_H_PATH.read_text())

# cvx2/core.h's CVX_(A,B)/CVX__(A,B) token-paste helpers, defined here
# directly (rather than letting the preprocessor see core.h for real) so
# the rest of core.h (CVX_VTAB_DEFINITION, etc.) stays unresolved/literal
# in generated output. Safe to let the real preprocessor fully resolve the
# paste now that names.h has already supplied the correctly-cased suffix.
PASTE_HELPERS = "#define CVX__(A, B) A##B\n#define CVX_(A, B) CVX__(A, B)\n"


###
### TEMPLATE EXPANSION
###

COMPILER_FLAGS = {
    "gcc": ["-E", "-C"],
    "clang": ["-E", "-C"],
    "cc": ["-E", "-C"],
}

DEFINE_LINE_RE = re.compile(r"^[ \t]*#define\b[^\n]*\n?", re.MULTILINE)
LINE_MARKER_RE = re.compile(r'^# \d+ "([^"]*)"')
SYSTEM_PREFIXES = (
    "/usr",
    "/opt/homebrew",
    "/Library",
    "<built-in>",
    "<command-line>",
    "<command line>",
)
EMPTY_CLANG_FORMAT_RE = re.compile(
    r"[ \t]*//[ \t]*clang-format off[ \t]*\n[ \t]*//[ \t]*clang-format on[ \t]*\n"
)
BLANK_RUN_RE = re.compile(r"\n{3,}")


INCLUDE_RE = re.compile(r"^[ \t]*#include[^\n]*\n?", re.MULTILINE)
GENERATION_ONLY_INCLUDES = ("cvx2/fallback.h", "cvx2/names.h")


def _strip_includes(text):
    """Pulls every #include line out of text (so the preprocessor never
    resolves them) and returns (includes_block, remaining_text)."""
    includes = [
        inc
        for inc in INCLUDE_RE.findall(text)
        if not any(g in inc for g in GENERATION_ONLY_INCLUDES)
    ]
    return "".join(includes), INCLUDE_RE.sub("", text)


def _local_macro_defines(header_text):
    """Raw #define lines (FUNC, VTAB_V, etc.) copied verbatim out of a
    template header's text, to prepend to the matching .c's text before
    preprocessing it: the .c relies on these via its own #include of the
    .h, which gets stripped along with every other #include."""
    return "".join(DEFINE_LINE_RE.findall(header_text))


def _run_preprocessor(text, defines, compiler):
    if compiler not in COMPILER_FLAGS:
        raise PreprocessError(
            f"unknown compiler {compiler!r}, expected one of {sorted(COMPILER_FLAGS)}"
        )

    cmd = [
        compiler,
        *COMPILER_FLAGS[compiler],
        "-x",
        "c",
        *(f"-D{d}" for d in defines),
        "-",
    ]
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
        m = LINE_MARKER_RE.match(line)
        if m:
            skip = m.group(1).startswith(SYSTEM_PREFIXES)
            continue
        if not skip:
            out.append(line)
    return "\n".join(out)


def expand(text, defines, compiler):
    includes, stripped = _strip_includes(text)
    expanded = _run_preprocessor(
        PASTE_HELPERS + NAMES_H_DEFS + stripped, defines, compiler
    )
    expanded = EMPTY_CLANG_FORMAT_RE.sub("", expanded)
    expanded = BLANK_RUN_RE.sub("\n\n", expanded)
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

# Required string fields beyond struct_name/prefix, per template.
TEMPLATE_SCHEMAS = {
    "dynamic_array": {"scalar_fields": ["value_type"]},
    "hashtable": {"scalar_fields": ["key_type", "value_type"]},
}

TEMPLATE_VARIANTS = {
    "hashtable": {
        "collision": {
            "open_addressing": "CVX2_COLLISION_OPEN_ADDRESSING",
            "separate_chaining": "CVX2_COLLISION_SEPARATE_CHAINING",
        },
    },
}

MISSING = object()


def _err(where, message):
    raise ConfigError(f"{where}: {message}")


def _field(instantiation, where, key, default=MISSING, identifier=False, allowed=None):
    """Reads and validates a single string field: required unless `default`
    is given, optionally must be a valid C identifier, optionally must be
    one of `allowed`."""
    if key not in instantiation:
        if default is not MISSING:
            return default
        _err(where, f"missing required field {key!r}")

    value = instantiation[key]
    valid = isinstance(value, str)
    if valid and identifier:
        valid = IDENTIFIER_RE.match(value) is not None
    if valid and allowed is not None:
        valid = value in allowed

    if not valid:
        kind = (
            "a valid C identifier"
            if identifier
            else f"one of {allowed}"
            if allowed
            else "a string"
        )
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
    case_name = _field(
        instantiation, where, "case", default=CASE_SNAKE, allowed=VALID_CASES
    )
    out_dir = _field(instantiation, where, "out_dir", default=".")

    axes = TEMPLATE_VARIANTS.get(template, {})
    variants = instantiation.get("variants", {})
    if not isinstance(variants, dict):
        _err(
            where,
            "field 'variants' must be an object mapping axis name -> variant name",
        )
    for axis, names in axes.items():
        if axis not in variants:
            _err(
                where,
                f"template {template!r} requires variants[{axis!r}] to be set (choices: {sorted(names)})",
            )
    for axis, name in variants.items():
        if axis not in axes:
            _err(
                where,
                f"config specifies variants[{axis!r}] but template {template!r} has no such axis "
                f"(it uses: {sorted(axes)})",
            )
        elif name not in axes[axis]:
            _err(
                where,
                f"variants[{axis!r}] = {name!r} is not a known variant (choices: {sorted(axes[axis])})",
            )

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
    header = header.strip()
    if header.startswith("<") and header.endswith(">"):
        return f"#include {header}"
    return f'#include "{header}"'


def _defines_for(inst):
    defines = [
        f"CVX_VAL={inst['value_type']}",
        f"CVX_SNAME={inst['struct_name']}",
        f"CVX_PFX={inst['prefix']}",
    ]
    if inst["key_type"] is not None:
        defines.append(f"CVX_KEY={inst['key_type']}")
    if inst["case"] in CASE_DEFINES:
        defines.append(CASE_DEFINES[inst["case"]])

    axes = TEMPLATE_VARIANTS.get(inst["template"], {})
    for axis, name in inst["variants"].items():
        defines.append(axes[axis][name])
    return defines


def generate_instantiation(inst, compiler, templates_dir=TEMPLATES_DIR):
    template_name = inst["template"]
    defines = _defines_for(inst)

    header_text = (templates_dir / f"{template_name}.h").read_text()
    source_text = (templates_dir / f"{template_name}.c").read_text()

    header_out = expand(header_text, defines, compiler)
    source_out = expand(
        _local_macro_defines(header_text) + source_text, defines, compiler
    )

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
        header_filename, header_out, source_filename, source_out = (
            generate_instantiation(inst, compiler)
        )

        out_dir = Path(config_path).parent / inst["out_dir"]
        out_dir.mkdir(parents=True, exist_ok=True)

        (out_dir / header_filename).write_text(header_out)
        (out_dir / source_filename).write_text(source_out)
        written.append(str(out_dir / header_filename))
        written.append(str(out_dir / source_filename))

    return written


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Generate cvx2 container types from a JSON config."
    )
    parser.add_argument("--config", required=True, help="path to the JSON config file")
    parser.add_argument(
        "--compiler",
        required=True,
        choices=sorted(COMPILER_FLAGS),
        help="C compiler to preprocess templates with",
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

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
import dataclasses
import json
import re
import subprocess
import sys
from pathlib import Path

# 1. Config Validation
# 2. Macro Expansion
# 3. File Generation

TEMPLATES_DIR = Path(__file__).resolve().parent.parent / "templates"
NAMES_H_PATH = Path(__file__).resolve().parent.parent / "names.h"
IDENTIFIER_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")

# based on names.h
CASE_SNAKE = "snake_case"
CASE_CAMEL = "camelCase"
CASE_PASCAL = "PascalCase"
VALID_CASES = (CASE_SNAKE, CASE_CAMEL, CASE_PASCAL)
# Things "case" can be set for independently, each defaulting to snake_case
CASE_TARGETS = ("function", "struct")


class ConfigError(Exception):
    pass


class PreprocessError(Exception):
    pass


###
### CONFIG VALIDATION
###


def field_spec(
    default=dataclasses.MISSING, identifier=False, allowed=None, type_expr=False
):
    """A dataclass field carrying cvx2 config validation rules as metadata:
    required unless `default` is given, optionally must be a valid C
    identifier, optionally one of `allowed`, optionally rejected if it
    looks like a raw array/function-pointer type (see _validate_field)."""
    kwargs = {} if default is dataclasses.MISSING else {"default": default}
    return dataclasses.field(
        **kwargs,
        metadata={"identifier": identifier, "allowed": allowed, "type_expr": type_expr},
    )


@dataclasses.dataclass(kw_only=True)
class _InstantiationBase:
    struct_name: str = field_spec(identifier=True)
    prefix: str = field_spec(identifier=True)
    out_dir: str = field_spec(default=".")
    file_name: str = field_spec(default=None, identifier=True)


@dataclasses.dataclass(kw_only=True)
class DynamicArrayConfig(_InstantiationBase):
    value_type: str = field_spec(type_expr=True)


@dataclasses.dataclass(kw_only=True)
class CircularBufferConfig(_InstantiationBase):
    value_type: str = field_spec(type_expr=True)


@dataclasses.dataclass(kw_only=True)
class LinkedListConfig(_InstantiationBase):
    value_type: str = field_spec(type_expr=True)


@dataclasses.dataclass(kw_only=True)
class HashtableConfig(_InstantiationBase):
    key_type: str = field_spec(type_expr=True)
    # Required when variants.kind == "map" (the default), forbidden when
    # == "set" -- see _validate_hashtable_kind.
    value_type: str = field_spec(default=None, type_expr=True)


@dataclasses.dataclass(kw_only=True)
class HamtConfig(_InstantiationBase):
    key_type: str = field_spec(type_expr=True)
    value_type: str = field_spec(type_expr=True)


# Adding a template means adding one small dataclass like the two above and
# registering it here -- no other validation code site needs to change.
TEMPLATE_SCHEMAS = {
    "dynamic_array": DynamicArrayConfig,
    "circular_buffer": CircularBufferConfig,
    "linked_list": LinkedListConfig,
    "hashtable": HashtableConfig,
    "hamt": HamtConfig,
}

TEMPLATE_VARIANTS = {
    "linked_list": {
        "node": {
            "single": "CVX2_NODE_SINGLE",
            "double": "CVX2_NODE_DOUBLE",
        },
    },
    "hashtable": {
        "collision": {
            "open_addressing": "CVX2_COLLISION_OPEN_ADDRESSING",
            "separate_chaining": "CVX2_COLLISION_SEPARATE_CHAINING",
        },
        "kind": {
            "map": "CVX2_KIND_MAP",
            "set": "CVX2_KIND_SET",
        },
    },
}

TEMPLATE_VARIANT_DEFAULTS = {
    "hashtable": {"kind": "map"},
}


def _err(where, message):
    raise ConfigError(f"{where}: {message}")


def _validate_field(where, name, value, meta):
    """Applies one dataclass field's metadata-declared rules (see
    field_spec) to a raw JSON value."""
    valid = isinstance(value, str)
    if valid and meta["identifier"]:
        valid = IDENTIFIER_RE.match(value) is not None
    if valid and meta["allowed"] is not None:
        valid = value in meta["allowed"]

    if not valid:
        kind = (
            "a valid C identifier"
            if meta["identifier"]
            else f"one of {meta['allowed']}"
            if meta["allowed"]
            else "a string"
        )
        _err(where, f"field {name!r} must be {kind}, got {value!r}")

    # Rejects raw array/function-pointer syntax in value_type/key_type --
    # neither can be expressed as a plain declarator prefix; a typedef is
    # needed instead.
    if meta["type_expr"] and ("[" in value or "(" in value):
        _err(
            where,
            f"field {name!r} = {value!r} looks like a raw array or function-pointer type, which "
            f"can't be spliced in as a plain declarator prefix (e.g. it would generate invalid C "
            f"like '{value} item;' instead of 'item[10];' or '(*item)(...)'). Define a typedef for "
            f"it instead (e.g. `typedef struct {{ int items[10]; }} my_array10_t;` or "
            f"`typedef int (*my_fn_t)(int, int);`) and set {name!r} to that typedef's name -- and "
            f"add the header that defines it to this instantiation's 'includes' list, since the "
            f"generated .c is its own translation unit and won't otherwise see it.",
        )


def _build(cls, instantiation, where):
    """Validates and constructs one template's config dataclass by walking
    its declared fields -- each field's required/default/identifier/
    allowed/type_expr rules live on the field itself (see field_spec), so
    adding a field to a template means adding one line to its dataclass,
    not a new call site here."""
    kwargs = {}
    for f in dataclasses.fields(cls):
        if f.name not in instantiation:
            if f.default is dataclasses.MISSING:
                _err(where, f"missing required field {f.name!r}")
            continue
        value = instantiation[f.name]
        _validate_field(where, f.name, value, f.metadata)
        kwargs[f.name] = value
    return cls(**kwargs)


def _validate_variants(instantiation, where, template):
    axes = TEMPLATE_VARIANTS.get(template, {})
    defaults = TEMPLATE_VARIANT_DEFAULTS.get(template, {})
    variants = instantiation.get("variants", {})
    if not isinstance(variants, dict):
        _err(
            where,
            "field 'variants' must be an object mapping axis name -> variant name",
        )
    for axis, names in axes.items():
        if axis not in variants:
            if axis in defaults:
                variants[axis] = defaults[axis]
                continue
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
    return variants


def _validate_includes(instantiation, where):
    # Headers needed for value_type/key_type, if not a builtin.
    includes = instantiation.get("includes", [])
    if not isinstance(includes, list) or not all(isinstance(h, str) for h in includes):
        _err(where, "field 'includes' must be an array of strings (header paths)")
    return includes


def _validate_case(instantiation, where):
    case = instantiation.get("case", {})
    if not isinstance(case, dict):
        _err(
            where,
            f"field 'case' must be an object with keys {CASE_TARGETS}, got {case!r}",
        )
    for target in case:
        if target not in CASE_TARGETS:
            _err(
                where, f"case[{target!r}] is not a known key (choices: {CASE_TARGETS})"
            )
    result = {}
    for target in CASE_TARGETS:
        value = case.get(target, CASE_SNAKE)
        if value not in VALID_CASES:
            _err(where, f"case[{target!r}] must be one of {VALID_CASES}, got {value!r}")
        result[target] = value
    return result


def _validate_hashtable_kind(result, where):
    """A hashtable with variants.kind == "set" stores keys only (no
    value_type); == "map" (the default) stores key -> value_type, like
    before this axis existed."""
    kind = result["variants"]["kind"]
    if kind == "map" and result["value_type"] is None:
        _err(
            where,
            'hashtable with variants["kind"] = "map" requires \'value_type\' to be set',
        )
    if kind == "set" and result["value_type"] is not None:
        _err(
            where,
            'hashtable with variants["kind"] = "set" must not set \'value_type\' '
            "-- a set has no values, only keys",
        )


def validate_instantiation(instantiation, index):
    where = f"instantiations[{index}]"

    if not isinstance(instantiation, dict):
        _err(where, f"must be an object, got {instantiation!r}")

    template = instantiation.get("template")
    if template not in TEMPLATE_SCHEMAS:
        _err(
            where,
            f"field 'template' must be one of {tuple(TEMPLATE_SCHEMAS)}, got {template!r}",
        )

    config = _build(TEMPLATE_SCHEMAS[template], instantiation, where)

    result = dataclasses.asdict(config)
    result.setdefault(
        "key_type", None
    )  # templates without a key_type (e.g. dynamic_array)
    result["template"] = template
    result["variants"] = _validate_variants(instantiation, where, template)
    result["includes"] = _validate_includes(instantiation, where)
    result["case"] = _validate_case(instantiation, where)

    if template == "hashtable":
        _validate_hashtable_kind(result, where)

    return result


def validate_config(raw_config):
    if not isinstance(raw_config, dict) or "instantiations" not in raw_config:
        raise ConfigError("config must be an object with an 'instantiations' array")

    instantiations = raw_config["instantiations"]
    if not isinstance(instantiations, list) or not instantiations:
        raise ConfigError("'instantiations' must be a non-empty array")

    return [validate_instantiation(inst, i) for i, inst in enumerate(instantiations)]


###
### CASE CONVERSION
###

CASE_DEFINES = {
    "function": {
        CASE_CAMEL: "CVX_FUNC_NAMES_CAMELCASE",
        CASE_PASCAL: "CVX_FUNC_NAMES_PASCALCASE",
    },
    "struct": {
        CASE_CAMEL: "CVX_STRUCT_NAMES_CAMELCASE",
        CASE_PASCAL: "CVX_STRUCT_NAMES_PASCALCASE",
    },
}

###
### TEMPLATE EXPANSION
###

NAMES_H_DEFS = NAMES_H_PATH.read_text()
PASTE_HELPERS = "#define CVX__(A, B) A##B\n#define CVX_(A, B) CVX__(A, B)\n"

COMPILER_FLAGS = {
    "gcc": ["-E", "-C"],
    "clang": ["-E", "-C"],
    "cc": ["-E", "-C"],
}

DEFINE_LINE_RE = re.compile(r"^[ \t]*#define\b[^\n]*\n?", re.MULTILINE)
# Prepended as the first line of text handed to the preprocessor, and
# located again in its output, to cut away whatever compiler-injected
# preamble (built-in macros, glibc's stdc-predef.h, etc.) precedes our own
# content -- see _run_preprocessor.
STDIN_MARKER = "/* CVX_GENERATOR_STDIN_MARKER */"
LINE_MARKER_RE = re.compile(r'^# \d+ "[^"]*"[^\n]*\n?', re.MULTILINE)
EMPTY_CLANG_FORMAT_RE = re.compile(
    r"[ \t]*//[ \t]*clang-format off[ \t]*\n(?:[ \t]*\n)*[ \t]*//[ \t]*clang-format on[ \t]*\n"
)
BLANK_RUN_RE = re.compile(r"\n{3,}")
# Stands in for a template's own blank lines while preprocessing, so they can
# be told apart from the blank lines cpp pads in for dropped directives and
# #if branches -- see _mark_blank_lines. A // comment, so it stays inert even
# when the blank line sits inside a /* */ block.
BLANK_MARKER = "// CVX_GENERATOR_BLANK_LINE"
BLANK_LINE_RE = re.compile(r"^[ \t]*$\n?", re.MULTILINE)
BLANK_MARKER_LINE_RE = re.compile(rf"^[ \t]*{re.escape(BLANK_MARKER)}[ \t]*$", re.MULTILINE)


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
        result = subprocess.run(
            cmd, input=f"{STDIN_MARKER}\n{text}", capture_output=True, text=True
        )
    except FileNotFoundError:
        raise PreprocessError(f"compiler {compiler!r} not found on PATH")

    if result.returncode != 0:
        raise PreprocessError(result.stderr)

    # Our own text starts right after STDIN_MARKER, which cuts away whatever
    # compiler-injected preamble (built-in macros, glibc's stdc-predef.h,
    # etc.) precedes it -- no need to inspect every "# N \"file\"" marker to
    # tell our content apart from the compiler's.
    marker_pos = result.stdout.find(STDIN_MARKER)
    if marker_pos == -1:
        raise PreprocessError(
            "internal error: stdin marker missing from preprocessor output"
        )
    out = result.stdout[marker_pos + len(STDIN_MARKER) :]
    return LINE_MARKER_RE.sub("", out)


def _strip_unresolved_names(text, defines):
    """A NAME##X paste with no matching names.h entry stays as the literal
    token NAME_x glued to the prefix/struct name (e.g. hamtNAME__make_leaf).
    Drops that NAME so X renders as written (hamt__make_leaf), uncased --
    names.h only needs entries for names whose case should change."""
    owners = [
        d.split("=", 1)[1]
        for d in defines
        if d.startswith(("CVX_PFX=", "CVX_SNAME="))
    ]
    if not owners:
        return text
    alternation = "|".join(re.escape(o) for o in sorted(owners, key=len, reverse=True))
    return re.sub(rf"\b({alternation})NAME(?=_)", r"\1", text)


def _mark_blank_lines(text):
    return re.sub(r"^[ \t]*$(?=\n)", BLANK_MARKER, text, flags=re.MULTILINE)


def _restore_blank_lines(text):
    """Drops every blank line cpp produced (dropped directives, untaken #if
    branches, macros expanding to nothing), then turns the template's own
    marked blank lines back into real ones."""
    text = BLANK_LINE_RE.sub("", text)
    return BLANK_MARKER_LINE_RE.sub("", text)


def expand(text, defines, compiler):
    includes, stripped = _strip_includes(text)
    expanded = _run_preprocessor(
        PASTE_HELPERS + NAMES_H_DEFS + _mark_blank_lines(stripped), defines, compiler
    )
    expanded = _restore_blank_lines(expanded)
    expanded = _strip_unresolved_names(expanded, defines)
    expanded = EMPTY_CLANG_FORMAT_RE.sub("", expanded)
    if includes:
        expanded = "\n" + expanded
    return BLANK_RUN_RE.sub("\n\n", includes + expanded)


def rewrite_self_include(text, template_name, generated_header_filename):
    return re.sub(
        rf'#include\s+"{re.escape(template_name)}\.h"',
        f'#include "{generated_header_filename}"',
        text,
        count=1,
    )


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
        f"CVX_SNAME={inst['struct_name']}",
        f"CVX_PFX={inst['prefix']}",
    ]
    if inst["value_type"] is not None:
        defines.append(f"CVX_VAL={inst['value_type']}")
    if inst["key_type"] is not None:
        defines.append(f"CVX_KEY={inst['key_type']}")
    for target, case in inst["case"].items():
        if case in CASE_DEFINES[target]:
            defines.append(CASE_DEFINES[target][case])

    variant = TEMPLATE_VARIANTS.get(inst["template"], {})
    for choice, macro in inst["variants"].items():
        defines.append(variant[choice][macro])
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

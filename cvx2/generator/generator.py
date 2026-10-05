#!/usr/bin/env python3
"""cvx2 code generator CLI.

Reads a JSON config listing one or more container instantiations, and for
each one expands the matching template (cvx2/templates/<template>.h/.c)
into a simple file containing no extra macros and outputs them to the
configured directories by `out_dir`.

Usage:
    python3 generator.py --config path/to/config.json

This file should be accompanied by the /templates folder containing all the
cvx template files. Both .h/.c should be in the same folder.
"""

import argparse
import json
import re
import sys
from pathlib import Path

TEMPLATES_DIR = Path(__file__).resolve().parent.parent / "templates"
IDENTIFIER_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


class VariantError(Exception):
    pass


class ConfigError(Exception):
    pass


###
### CASE CONVERSION
###

CASE_SNAKE = "snake_case"
CASE_CAMEL = "camelCase"
CASE_UPPER_CAMEL = "UpperCamelCase"
VALID_CASES = (CASE_SNAKE, CASE_CAMEL, CASE_UPPER_CAMEL)

UNDERSCORE_RUN_RE = re.compile(r"(_+)")


def identity(identifier):
    return identifier


def _convert_case(identifier, capitalize_first_word):
    parts = [p for p in UNDERSCORE_RUN_RE.split(identifier) if p]

    out = []
    seen_word = False
    for token in parts:
        if token[0] == "_":
            if len(token) >= 2:
                out.append(token)
            continue
        if not seen_word:
            out.append(
                token[:1].upper() + token[1:]
                if capitalize_first_word
                else token.lower()
            )
            seen_word = True
        else:
            out.append(token[:1].upper() + token[1:])

    return "".join(out) if seen_word else identifier


def to_camel_case(identifier):
    """camelCase"""
    return _convert_case(identifier, capitalize_first_word=False)


def to_upper_camel_case(identifier):
    """UpperCamelCase"""
    return _convert_case(identifier, capitalize_first_word=True)


def case_fn_for(case_name):
    if case_name == CASE_CAMEL:
        return to_camel_case
    if case_name == CASE_UPPER_CAMEL:
        return to_upper_camel_case
    if case_name == CASE_SNAKE:
        return identity
    raise ValueError(f"unknown case {case_name!r}, expected one of {VALID_CASES}")


###
### VARIANT SELECTION
###
# A variant block looks like:
#
#     // @cvx2:variant axis="collision" name="open_addressing"
#     #ifdef CVX2_COLLISION_OPEN_ADDRESSING
#     ... code ...
#     #endif
#     // @cvx2:endvariant
#
# select_variants() picks the block matching the config and discards the
# rest, based on the marker comments alone (the #ifdef/#endif just let the
# template compile standalone while editing).

_VARIANT_START_RE = re.compile(
    r'^\s*//\s*@cvx2:variant\s+axis="([^"]+)"\s+name="([^"]+)"\s*$'
)
_VARIANT_END_RE = re.compile(r"^\s*//\s*@cvx2:endvariant\s*$")
_IF_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef)\b")
_ENDIF_RE = re.compile(r"^\s*#\s*endif\b")


class _VariantBlock:
    __slots__ = ("axis", "body", "end", "name", "start")

    def __init__(self, axis, name, start, end, body):
        self.axis = axis
        self.name = name
        self.start = start  # index of the "// @cvx2:variant ..." line
        self.end = end  # index of the "// @cvx2:endvariant" line
        self.body = body  # lines strictly between the inner #ifdef and #endif


def _find_variant_blocks(lines, template_name):
    blocks = []
    i = 0
    while i < len(lines):
        m = _VARIANT_START_RE.match(lines[i])
        if not m:
            i += 1
            continue

        axis, name = m.group(1), m.group(2)
        start = i

        if i + 1 >= len(lines) or not _IF_RE.match(lines[i + 1]):
            raise VariantError(
                f"{template_name}: line {i + 1}: @cvx2:variant must be immediately followed "
                f"by a #if/#ifdef/#ifndef line"
            )

        # depth starts at 1 for the #ifdef right after the marker; body is
        # everything after it, up to (not including) the matching #endif.
        depth = 1
        j = i + 2
        body_start = j
        while j < len(lines) and depth > 0:
            if _IF_RE.match(lines[j]):
                depth += 1
            elif _ENDIF_RE.match(lines[j]):
                depth -= 1
                if depth == 0:
                    break
            j += 1
        else:
            raise VariantError(
                f"{template_name}: line {start + 1}: unterminated #if inside @cvx2:variant block"
            )

        if j >= len(lines):
            raise VariantError(
                f"{template_name}: line {start + 1}: missing #endif for @cvx2:variant block"
            )

        body = lines[body_start:j]
        endif_line = j

        if endif_line + 1 >= len(lines) or not _VARIANT_END_RE.match(
            lines[endif_line + 1]
        ):
            raise VariantError(
                f"{template_name}: line {endif_line + 1}: expected // @cvx2:endvariant "
                f"immediately after the block's #endif"
            )

        end = endif_line + 1
        blocks.append(_VariantBlock(axis, name, start, end, body))
        i = end + 1

    return blocks


def select_variants(text, variants_config, template_name):
    """Strips every @cvx2:variant block down to the one instance selected by
    variants_config (an {axis: name} dict), deleting all others. Raises
    VariantError if the template and config disagree about which axes/names
    exist."""
    lines = text.split("\n")
    blocks = _find_variant_blocks(lines, template_name)

    if not blocks:
        return text

    axes_in_template = {}
    for b in blocks:
        axes_in_template.setdefault(b.axis, set()).add(b.name)

    for axis, names in axes_in_template.items():
        if axis not in variants_config:
            raise VariantError(
                f"{template_name}: requires variants[{axis!r}] to be set in config (choices: {sorted(names)})"
            )
        selected = variants_config[axis]
        if selected not in names:
            raise VariantError(
                f"{template_name}: variants[{axis!r}] = {selected!r} is not a known variant "
                f"(choices: {sorted(names)})"
            )

    # Rebuild the file: kept blocks become just their body; dropped blocks
    # (and all their scaffolding lines) are removed entirely. `blocks` is
    # already in document order from _find_variant_blocks.
    out = []
    i = 0
    bi = 0
    while i < len(lines):
        if bi < len(blocks) and blocks[bi].start == i:
            b = blocks[bi]
            if variants_config[b.axis] == b.name:
                out.extend(b.body)
            i = b.end + 1
            bi += 1
        else:
            out.append(lines[i])
            i += 1

    return "\n".join(out)


###
### TEMPLATE EXPANSION
###
# Resolves a template's local macros (FUNC(X), VTAB_V, etc., #define'd in
# the .h) and CVX_(A, B) token-pasting into literal generated identifiers.
# cvx2/core.h and cvx2/flags.h stay as real #includes in the output.

_FUNC_DEF_RE = re.compile(
    r"^[ \t]*#define[ \t]+FUNC\((\w+)\)[ \t]+(.+?)[ \t]*$", re.MULTILINE
)
_OBJECT_DEF_RE = re.compile(
    r"^[ \t]*#define[ \t]+(VTAB_K|VTAB_V|ENTRY|NODE)[ \t]+(.+?)[ \t]*$", re.MULTILINE
)
_PASTE_RE = re.compile(r"CVX_\(\s*(\w+)\s*,\s*(\w+)\s*\)")
_GUARD_BLOCK_RE = re.compile(
    r"^[ \t]*//[ \t]*clang-format off[ \t]*\n"
    r"(?:[ \t]*#ifndef[ \t]+\w+\n[ \t]*#error[^\n]*\n[ \t]*#endif\n)+"
    r"[ \t]*//[ \t]*clang-format on[ \t]*\n",
    re.MULTILINE,
)
_FALLBACK_INCLUDE_RE = re.compile(
    r'^[ \t]*#include[ \t]+"cvx2/fallback\.h"[ \t]*\n', re.MULTILINE
)
_BLANK_RUN_RE = re.compile(r"\n{3,}")

_IDENTIFIER_PLACEHOLDERS = ("CVX_SNAME", "CVX_PFX")
_LITERAL_PLACEHOLDERS = ("CVX_VAL", "CVX_KEY")


class LocalMacros:
    def __init__(self, func_param, func_body, object_defs):
        self.func_param = func_param
        self.func_body = func_body
        self.object_defs = object_defs  # {NAME: body}


class ExpandContext:
    def __init__(self, value_type, key_type, struct_name, prefix, case_fn):
        self.raw = {
            "CVX_VAL": value_type,
            "CVX_KEY": key_type,
            "CVX_SNAME": struct_name,
            "CVX_PFX": prefix,
        }
        self.case_fn = case_fn


def parse_local_macros(header_text):
    """Reads FUNC(X)=... and the object-like macro definitions out of a
    template header's text and strips all template-only scaffolding
    (the fallback.h include, the required-macro guard block, and the macro
    #define lines themselves) from it. Returns (macros, stripped_text)."""
    m = _FUNC_DEF_RE.search(header_text)
    func_param, func_body = (m.group(1), m.group(2)) if m else (None, None)
    object_defs = {
        om.group(1): om.group(2) for om in _OBJECT_DEF_RE.finditer(header_text)
    }

    stripped = _GUARD_BLOCK_RE.sub("", header_text)
    stripped = _FALLBACK_INCLUDE_RE.sub("", stripped)
    stripped = _FUNC_DEF_RE.sub("", stripped)
    stripped = _OBJECT_DEF_RE.sub("", stripped)

    return LocalMacros(func_param, func_body, object_defs), stripped


def _apply_func_calls(text, macros):
    if macros.func_param is None:
        return text

    def repl(match):
        arg = match.group(1).strip()
        return re.sub(rf"\b{re.escape(macros.func_param)}\b", arg, macros.func_body)

    return re.sub(r"\bFUNC\(([^()]*)\)", repl, text)


def _apply_object_macros(text, macros):
    if not macros.object_defs:
        return text

    name_alt_re = re.compile(
        r"\b(" + "|".join(re.escape(n) for n in macros.object_defs) + r")\b"
    )
    return name_alt_re.sub(lambda m: macros.object_defs[m.group(1)], text)


def _resolve_pastes(text, ctx):
    def repl(match):
        a, b = match.group(1), match.group(2)
        raw_a = ctx.raw.get(a, a)
        raw_b = ctx.raw.get(b, b)
        return ctx.case_fn(raw_a + raw_b)

    # Loop in case a macro body itself contains another paste.
    for _ in range(4):
        new_text = _PASTE_RE.sub(repl, text)
        if new_text == text:
            return new_text
        text = new_text
    return text


def _resolve_bare_placeholders(text, ctx):
    for name in _IDENTIFIER_PLACEHOLDERS:
        text = re.sub(rf"\b{name}\b", lambda _m, n=name: ctx.case_fn(ctx.raw[n]), text)
    for name in _LITERAL_PLACEHOLDERS:
        value = ctx.raw[name]
        if value is None:
            continue
        text = re.sub(rf"\b{name}\b", lambda _m, v=value: v, text)
    return text


def expand(text, macros, ctx):
    text = _apply_func_calls(text, macros)
    text = _apply_object_macros(text, macros)
    text = _resolve_pastes(text, ctx)
    text = _resolve_bare_placeholders(text, ctx)
    text = _BLANK_RUN_RE.sub("\n\n", text)
    return text


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

# variant_axes: the @cvx2:variant axes this template's blocks use.
TEMPLATE_SCHEMAS = {
    "dynamic_array": {
        "scalar_fields": {"value_type": (str, "string")},
        "variant_axes": [],
    },
    "hashtable": {
        "scalar_fields": {"key_type": (str, "string"), "value_type": (str, "string")},
        "variant_axes": ["collision"],
    },
}


def _err(where, message):
    raise ConfigError(f"{where}: {message}")


def _require(instantiation, where, key, expected_type, type_name):
    if key not in instantiation:
        _err(where, f"missing required field {key!r}")
    value = instantiation[key]
    valid = isinstance(value, expected_type)
    if expected_type is int and isinstance(value, bool):
        valid = False  # JSON true/false must not satisfy an integer field
    if not valid:
        _err(where, f"field {key!r} must be a {type_name}, got {value!r}")
    return value


def _require_identifier(instantiation, where, key):
    value = _require(instantiation, where, key, str, "string")
    if not IDENTIFIER_RE.match(value):
        _err(where, f"field {key!r} must be a valid C identifier, got {value!r}")
    return value


def _optional(instantiation, key, default):
    return instantiation.get(key, default)


def _optional_identifier(instantiation, where, key):
    if key not in instantiation:
        return None
    value = instantiation[key]
    if not isinstance(value, str) or not IDENTIFIER_RE.match(value):
        _err(
            where,
            f"field {key!r}, if present, must be a valid C identifier, got {value!r}",
        )
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

    template = _require(instantiation, where, "template", str, "string")
    if template not in TEMPLATE_SCHEMAS:
        _err(
            where,
            f"unknown template {template!r}, expected one of {sorted(TEMPLATE_SCHEMAS)}",
        )
    schema = TEMPLATE_SCHEMAS[template]

    for field, (expected_type, type_name) in schema["scalar_fields"].items():
        value = _require(instantiation, where, field, expected_type, type_name)
        if field in ("value_type", "key_type"):
            _check_type_expression(where, field, value)

    _require_identifier(instantiation, where, "struct_name")
    _require_identifier(instantiation, where, "prefix")

    # Optional: the generated .h/.c basename, when it needs to differ from
    # struct_name (e.g. struct_name "CharMap" but files char_map.h/.c).
    # Defaults to struct_name.
    file_name = _optional_identifier(instantiation, where, "file_name")

    case_name = _optional(instantiation, "case", CASE_SNAKE)
    if case_name not in VALID_CASES:
        _err(where, f"field 'case' must be one of {VALID_CASES}, got {case_name!r}")

    variants = _optional(instantiation, "variants", {})
    if not isinstance(variants, dict):
        _err(
            where,
            "field 'variants' must be an object mapping axis name -> variant name",
        )
    for axis in schema["variant_axes"]:
        if axis not in variants:
            _err(
                where,
                f"template {template!r} requires config['variants'][{axis!r}] to be set",
            )
    for axis in variants:
        if axis not in schema["variant_axes"]:
            _err(
                where,
                f"config specifies variants[{axis!r}] but template {template!r} has no such axis "
                f"(it uses: {schema['variant_axes']})",
            )
        if not isinstance(variants[axis], str):
            _err(where, f"variants[{axis!r}] must be a string")

    out_dir = _optional(instantiation, "out_dir", ".")
    if not isinstance(out_dir, str):
        _err(where, "field 'out_dir' must be a string")

    # Headers needed for value_type/key_type, if not a builtin.
    includes = _optional(instantiation, "includes", [])
    if not isinstance(includes, list) or not all(isinstance(h, str) for h in includes):
        _err(where, "field 'includes' must be an array of strings (header paths)")

    return {
        "template": template,
        "value_type": instantiation.get("value_type"),
        "key_type": instantiation.get("key_type"),
        "struct_name": instantiation["struct_name"],
        "prefix": instantiation["prefix"],
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


def generate_instantiation(inst, templates_dir=TEMPLATES_DIR):
    template_name = inst["template"]
    case_fn = case_fn_for(inst["case"])

    header_text = (templates_dir / f"{template_name}.h").read_text()
    source_text = (templates_dir / f"{template_name}.c").read_text()

    header_text = select_variants(header_text, inst["variants"], f"{template_name}.h")
    source_text = select_variants(source_text, inst["variants"], f"{template_name}.c")

    macros, header_stripped = parse_local_macros(header_text)

    ctx = ExpandContext(
        value_type=inst["value_type"],
        key_type=inst["key_type"],
        struct_name=inst["struct_name"],
        prefix=inst["prefix"],
        case_fn=case_fn,
    )

    header_out = expand(header_stripped, macros, ctx)
    source_out = expand(source_text, macros, ctx)

    file_stem = inst["file_name"] or inst["struct_name"]
    header_filename = f"{file_stem}.h"
    source_filename = f"{file_stem}.c"

    guard = f"{file_stem.upper()}_H"
    includes_block = "".join(f"{_include_directive(h)}\n" for h in inst["includes"])
    header_out = f"#ifndef {guard}\n#define {guard}\n\n{includes_block}{header_out.strip()}\n\n#endif /* {guard} */\n"

    source_out = rewrite_self_include(source_out, template_name, header_filename)
    source_out = source_out.strip() + "\n"

    return header_filename, header_out, source_filename, source_out


def run(config_path):
    raw_config = json.loads(Path(config_path).read_text())
    instantiations = validate_config(raw_config)

    written = []
    for inst in instantiations:
        header_filename, header_out, source_filename, source_out = (
            generate_instantiation(inst)
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
    args = parser.parse_args(argv)

    try:
        written = run(args.config)
    except (ConfigError, VariantError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    for path in written:
        print(f"wrote {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

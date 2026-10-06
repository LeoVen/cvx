"""Tests for generate_instantiation() and its helpers. generate_instantiation
itself now shells out to a real C compiler to preprocess templates, so
these need one on PATH (skipped, not failed, if none is found). See
test_generate_e2e.py for the full compile-and-run version."""
import shutil
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))
from cvx2.generator import generator

COMPILER = next((c for c in generator.COMPILER_FLAGS if shutil.which(c)), None)


def da(**overrides):
    base = {
        "template": "dynamic_array",
        "value_type": "int",
        "key_type": None,
        "struct_name": "my_array",
        "prefix": "ma",
        "file_name": None,
        "case": "snake_case",
        "variants": {},
        "out_dir": ".",
        "includes": [],
    }
    base.update(overrides)
    return base


class TestIncludeDirective(unittest.TestCase):
    def test_local_path_is_quoted(self):
        self.assertEqual(generator._include_directive("types.h"), '#include "types.h"')

    def test_angle_bracket_passed_through(self):
        self.assertEqual(generator._include_directive("<stdint.h>"), "#include <stdint.h>")

    def test_strips_whitespace(self):
        self.assertEqual(generator._include_directive("  types.h  "), '#include "types.h"')


@unittest.skipUnless(COMPILER, "no C compiler found on PATH")
class TestGenerateInstantiationIncludes(unittest.TestCase):
    def test_no_includes_by_default(self):
        # Only the template's own includes (cvx2/core.h, <stdbool.h>, ...)
        # should appear -- no custom "includes" entry was configured.
        header_filename, header_out, _, _ = generator.generate_instantiation(da(), COMPILER)
        self.assertEqual(header_filename, "my_array.h")
        self.assertIn('#include "cvx2/core.h"', header_out)
        self.assertEqual(header_out.count("#include"), 4)  # core.h, flags.h, stdbool.h, stddef.h

    def test_custom_type_header_is_included_in_generated_header(self):
        header_filename, header_out, source_filename, source_out = generator.generate_instantiation(
            da(value_type="my_point_t", includes=["my_point_t.h"]), COMPILER
        )
        self.assertIn('#include "my_point_t.h"', header_out)
        # It must appear before the struct uses the type, i.e. early in the file.
        self.assertLess(header_out.index('#include "my_point_t.h"'), header_out.index("my_point_t"))
        # The .c only #includes its own header, so the type is visible there
        # transitively -- not re-included directly in the .c.
        self.assertNotIn('#include "my_point_t.h"', source_out)
        self.assertIn(f'#include "{header_filename}"', source_out)

    def test_multiple_includes_preserve_order(self):
        _, header_out, _, _ = generator.generate_instantiation(da(includes=["a.h", "<stdint.h>", "b.h"]), COMPILER)
        ia, ib, ic = (header_out.index(s) for s in ('#include "a.h"', "#include <stdint.h>", '#include "b.h"'))
        self.assertTrue(ia < ib < ic)


@unittest.skipUnless(COMPILER, "no C compiler found on PATH")
class TestFileName(unittest.TestCase):
    def test_defaults_to_struct_name(self):
        header_filename, _, source_filename, _ = generator.generate_instantiation(da(struct_name="my_array"), COMPILER)
        self.assertEqual(header_filename, "my_array.h")
        self.assertEqual(source_filename, "my_array.c")

    def test_decouples_filename_from_struct_name(self):
        header_filename, header_out, source_filename, source_out = generator.generate_instantiation(
            da(struct_name="CharMap", file_name="char_map"), COMPILER
        )
        self.assertEqual(header_filename, "char_map.h")
        self.assertEqual(source_filename, "char_map.c")
        # The C struct itself still uses struct_name, unaffected by file_name.
        self.assertIn("struct CharMap", header_out)
        self.assertNotIn("struct char_map", header_out)
        # The .c's self-include follows the (decoupled) header filename.
        self.assertIn('#include "char_map.h"', source_out)

    def test_include_guard_follows_file_name_not_struct_name(self):
        _, header_out, _, _ = generator.generate_instantiation(da(struct_name="CharMap", file_name="char_map"), COMPILER)
        self.assertIn("#ifndef CHAR_MAP_H", header_out)
        self.assertIn("#define CHAR_MAP_H", header_out)
        self.assertNotIn("CHARMAP_H", header_out)


if __name__ == "__main__":
    unittest.main()

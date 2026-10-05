"""Unit-level tests for generator.py that don't need a C toolchain (see
test_generate_e2e.py for the compile-and-run version)."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))
from cvx2.generator import generator


def da(**overrides):
    base = {
        "template": "dynamic_array",
        "value_type": "int",
        "key_type": None,
        "struct_name": "my_array",
        "prefix": "ma",
        "tag": 1,
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


class TestGenerateInstantiationIncludes(unittest.TestCase):
    def test_no_includes_by_default(self):
        header_filename, header_out, _, _ = generator.generate_instantiation(da())
        self.assertEqual(header_filename, "my_array.h")
        self.assertNotIn("#include", header_out.split("\n")[3])  # nothing injected right after the guard

    def test_custom_type_header_is_included_in_generated_header(self):
        header_filename, header_out, source_filename, source_out = generator.generate_instantiation(
            da(value_type="my_point_t", includes=["my_point_t.h"])
        )
        self.assertIn('#include "my_point_t.h"', header_out)
        # It must appear before the struct uses the type, i.e. early in the file.
        self.assertLess(header_out.index('#include "my_point_t.h"'), header_out.index("my_point_t"))
        # The .c only #includes its own header, so the type is visible there
        # transitively -- not re-included directly in the .c.
        self.assertNotIn('#include "my_point_t.h"', source_out)
        self.assertIn(f'#include "{header_filename}"', source_out)

    def test_multiple_includes_preserve_order(self):
        _, header_out, _, _ = generator.generate_instantiation(da(includes=["a.h", "<stdint.h>", "b.h"]))
        ia, ib, ic = (header_out.index(s) for s in ('#include "a.h"', "#include <stdint.h>", '#include "b.h"'))
        self.assertTrue(ia < ib < ic)


class TestFileName(unittest.TestCase):
    def test_defaults_to_struct_name(self):
        header_filename, _, source_filename, _ = generator.generate_instantiation(da(struct_name="my_array"))
        self.assertEqual(header_filename, "my_array.h")
        self.assertEqual(source_filename, "my_array.c")

    def test_decouples_filename_from_struct_name(self):
        header_filename, header_out, source_filename, source_out = generator.generate_instantiation(
            da(struct_name="CharMap", file_name="char_map")
        )
        self.assertEqual(header_filename, "char_map.h")
        self.assertEqual(source_filename, "char_map.c")
        # The C struct itself still uses struct_name, unaffected by file_name.
        self.assertIn("struct CharMap", header_out)
        self.assertNotIn("struct char_map", header_out)
        # The .c's self-include follows the (decoupled) header filename.
        self.assertIn('#include "char_map.h"', source_out)

    def test_include_guard_follows_file_name_not_struct_name(self):
        _, header_out, _, _ = generator.generate_instantiation(da(struct_name="CharMap", file_name="char_map"))
        self.assertIn("#ifndef CHAR_MAP_H", header_out)
        self.assertIn("#define CHAR_MAP_H", header_out)
        self.assertNotIn("CHARMAP_H", header_out)


if __name__ == "__main__":
    unittest.main()

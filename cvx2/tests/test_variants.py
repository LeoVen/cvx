import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))
from cvx2.generator import generator

TEMPLATE = """\
before

// @cvx2:variant axis="collision" name="open_addressing"
#ifdef CVX2_COLLISION_OPEN_ADDRESSING
struct ENTRY oa_body;
#endif
// @cvx2:endvariant

// @cvx2:variant axis="collision" name="separate_chaining"
#ifdef CVX2_COLLISION_SEPARATE_CHAINING
struct NODE sc_body;
#endif
// @cvx2:endvariant

after
"""


class TestSelectVariants(unittest.TestCase):
    def test_keeps_selected_block_strips_others(self):
        out = generator.select_variants(TEMPLATE, {"collision": "open_addressing"}, "t.h")
        self.assertIn("struct ENTRY oa_body;", out)
        self.assertNotIn("struct NODE sc_body;", out)
        self.assertNotIn("@cvx2:variant", out)
        self.assertNotIn("#ifdef", out)
        self.assertIn("before", out)
        self.assertIn("after", out)

    def test_selects_the_other_block(self):
        out = generator.select_variants(TEMPLATE, {"collision": "separate_chaining"}, "t.h")
        self.assertIn("struct NODE sc_body;", out)
        self.assertNotIn("struct ENTRY oa_body;", out)

    def test_missing_axis_in_config_errors(self):
        with self.assertRaises(generator.VariantError):
            generator.select_variants(TEMPLATE, {}, "t.h")

    def test_unknown_variant_name_errors(self):
        with self.assertRaises(generator.VariantError):
            generator.select_variants(TEMPLATE, {"collision": "quantum_probing"}, "t.h")

    def test_no_blocks_is_a_no_op(self):
        text = "just plain C code\nwith no variant blocks\n"
        self.assertEqual(generator.select_variants(text, {}, "t.h"), text)

    def test_malformed_missing_ifdef_errors(self):
        bad = '// @cvx2:variant axis="x" name="y"\nnot an ifdef\n#endif\n// @cvx2:endvariant\n'
        with self.assertRaises(generator.VariantError):
            generator.select_variants(bad, {"x": "y"}, "t.h")

    def test_malformed_missing_endvariant_errors(self):
        bad = '// @cvx2:variant axis="x" name="y"\n#ifdef X\nbody\n#endif\nnot endvariant\n'
        with self.assertRaises(generator.VariantError):
            generator.select_variants(bad, {"x": "y"}, "t.h")

    def test_nested_ifdef_inside_block_is_handled(self):
        text = (
            '// @cvx2:variant axis="x" name="y"\n'
            "#ifdef CVX2_X_Y\n"
            "#ifdef SOME_OTHER_FLAG\n"
            "nested\n"
            "#endif\n"
            "outer_body\n"
            "#endif\n"
            "// @cvx2:endvariant\n"
        )
        out = generator.select_variants(text, {"x": "y"}, "t.h")
        self.assertIn("nested", out)
        self.assertIn("outer_body", out)
        self.assertNotIn("@cvx2:", out)


if __name__ == "__main__":
    unittest.main()

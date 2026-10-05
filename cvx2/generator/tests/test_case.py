"""Case conversion itself is now done by the real preprocessor via
cvx2/names.h (see test_expand.py) -- this only covers the small bit of
Python-side mapping from a config's "case" object to the -D flags that
select the right names.h branches."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))
from cvx2.generator import generator


class TestCaseDefines(unittest.TestCase):
    def test_snake_case_has_no_define(self):
        for target in generator.CASE_TARGETS:
            self.assertNotIn(generator.CASE_SNAKE, generator.CASE_DEFINES[target])

    def test_camel_case_maps_to_names_h_macro(self):
        self.assertEqual(generator.CASE_DEFINES["function"][generator.CASE_CAMEL], "CVX_FUNC_NAMES_CAMELCASE")
        self.assertEqual(generator.CASE_DEFINES["struct"][generator.CASE_CAMEL], "CVX_STRUCT_NAMES_CAMELCASE")

    def test_pascal_case_maps_to_names_h_macro(self):
        self.assertEqual(generator.CASE_DEFINES["function"][generator.CASE_PASCAL], "CVX_FUNC_NAMES_PASCALCASE")
        self.assertEqual(generator.CASE_DEFINES["struct"][generator.CASE_PASCAL], "CVX_STRUCT_NAMES_PASCALCASE")


if __name__ == "__main__":
    unittest.main()

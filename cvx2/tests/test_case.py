import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))
from cvx2.generator import generator


class TestToCamelCase(unittest.TestCase):
    def test_own_first_word_stays_lowercase(self):
        self.assertEqual(generator.to_camel_case("_push_back"), "pushBack")
        self.assertEqual(generator.to_camel_case("_vtabv"), "vtabv")

    def test_double_underscore_marker_preserved(self):
        public = generator.to_camel_case("_get_ref")
        private = generator.to_camel_case("__get_ref")
        self.assertNotEqual(public, private)
        self.assertEqual(public, "getRef")
        self.assertEqual(private, "__getRef")

    def test_double_underscore_mid_string(self):
        self.assertEqual(generator.to_camel_case("__primes"), "__primes")
        self.assertEqual(generator.to_camel_case("__primes_count"), "__primesCount")


class TestToPascalCase(unittest.TestCase):
    def test_own_first_word_also_capitalized(self):
        self.assertEqual(generator.to_pascal_case("_push_back"), "PushBack")
        self.assertEqual(generator.to_pascal_case("_vtabv"), "Vtabv")

    def test_double_underscore_marker_preserved(self):
        public = generator.to_pascal_case("_get_ref")
        private = generator.to_pascal_case("__get_ref")
        self.assertNotEqual(public, private)
        self.assertEqual(public, "GetRef")
        self.assertEqual(private, "__GetRef")

    def test_double_underscore_mid_string(self):
        self.assertEqual(generator.to_pascal_case("__primes"), "__Primes")
        self.assertEqual(generator.to_pascal_case("__primes_count"), "__PrimesCount")


class TestCaseFnFor(unittest.TestCase):
    def test_mapping(self):
        self.assertIs(generator.case_fn_for(generator.CASE_SNAKE), generator.identity)
        self.assertIs(generator.case_fn_for(generator.CASE_CAMEL), generator.to_camel_case)
        self.assertIs(generator.case_fn_for(generator.CASE_PASCAL), generator.to_pascal_case)

    def test_unknown_case_raises(self):
        with self.assertRaises(ValueError):
            generator.case_fn_for("kebab-case")


if __name__ == "__main__":
    unittest.main()

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))
from cvx2.generator import generator


class TestToCamelCase(unittest.TestCase):
    def test_simple_words(self):
        self.assertEqual(generator.to_camel_case("my_map"), "myMap")
        self.assertEqual(generator.to_camel_case("my_map_entry"), "myMapEntry")
        self.assertEqual(generator.to_camel_case("my_map_vtabv"), "myMapVtabv")

    def test_single_word_unchanged(self):
        self.assertEqual(generator.to_camel_case("capacity"), "capacity")

    def test_prefix_plus_function(self):
        self.assertEqual(generator.to_camel_case("mm_insert"), "mmInsert")
        self.assertEqual(generator.to_camel_case("moa_get_ref"), "moaGetRef")

    def test_double_underscore_marker_preserved(self):
        # The public _get_ref and the private __get_ref must NOT collide
        # once both are prefixed and case-converted.
        public = generator.to_camel_case("moa_get_ref")
        private = generator.to_camel_case("moa__get_ref")
        self.assertNotEqual(public, private)
        self.assertEqual(public, "moaGetRef")
        self.assertEqual(private, "moa__GetRef")

    def test_double_underscore_mid_string(self):
        self.assertEqual(generator.to_camel_case("mm__primes"), "mm__Primes")
        self.assertEqual(generator.to_camel_case("mm__primes_count"), "mm__PrimesCount")

    def test_idempotent_on_no_underscores(self):
        self.assertEqual(generator.to_camel_case("x"), "x")

    def test_case_fn_for(self):
        self.assertIs(generator.case_fn_for(generator.CASE_SNAKE), generator.identity)
        self.assertIs(
            generator.case_fn_for(generator.CASE_CAMEL), generator.to_camel_case
        )
        self.assertIs(
            generator.case_fn_for(generator.CASE_UPPER_CAMEL),
            generator.to_upper_camel_case,
        )
        with self.assertRaises(ValueError):
            generator.case_fn_for("PascalCase")


class TestToUpperCamelCase(unittest.TestCase):
    def test_simple_words(self):
        self.assertEqual(generator.to_upper_camel_case("my_map"), "MyMap")
        self.assertEqual(generator.to_upper_camel_case("my_map_entry"), "MyMapEntry")
        self.assertEqual(generator.to_upper_camel_case("my_map_vtabv"), "MyMapVtabv")

    def test_single_word_gets_capitalized(self):
        # Unlike to_camel_case, the first word is capitalized too.
        self.assertEqual(generator.to_upper_camel_case("capacity"), "Capacity")

    def test_prefix_plus_function(self):
        self.assertEqual(generator.to_upper_camel_case("mm_insert"), "MmInsert")
        self.assertEqual(generator.to_upper_camel_case("moa_get_ref"), "MoaGetRef")

    def test_double_underscore_marker_preserved(self):
        public = generator.to_upper_camel_case("moa_get_ref")
        private = generator.to_upper_camel_case("moa__get_ref")
        self.assertNotEqual(public, private)
        self.assertEqual(public, "MoaGetRef")
        self.assertEqual(private, "Moa__GetRef")

    def test_double_underscore_mid_string(self):
        self.assertEqual(generator.to_upper_camel_case("mm__primes"), "Mm__Primes")
        self.assertEqual(
            generator.to_upper_camel_case("mm__primes_count"), "Mm__PrimesCount"
        )

    def test_idempotent_on_already_capitalized_single_word(self):
        self.assertEqual(generator.to_upper_camel_case("X"), "X")

    def test_leading_underscore_preserved(self):
        self.assertEqual(generator.to_upper_camel_case("__private"), "__Private")


if __name__ == "__main__":
    unittest.main()

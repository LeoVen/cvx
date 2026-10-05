import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))
from cvx2.generator import generator


def da(**overrides):
    base = {
        "template": "dynamic_array",
        "value_type": "int",
        "struct_name": "my_array",
        "prefix": "ma",
    }
    base.update(overrides)
    return base


def ht(**overrides):
    base = {
        "template": "hashtable",
        "key_type": "int",
        "value_type": "int",
        "struct_name": "my_map",
        "prefix": "mm",
        "variants": {"collision": "open_addressing"},
    }
    base.update(overrides)
    return base


class TestValidateConfig(unittest.TestCase):
    def test_valid_minimal_dynamic_array(self):
        result = generator.validate_config({"instantiations": [da()]})
        self.assertEqual(result[0]["struct_name"], "my_array")
        self.assertEqual(result[0]["case"], "snake_case")
        self.assertEqual(result[0]["out_dir"], ".")

    def test_valid_hashtable_requires_variants(self):
        result = generator.validate_config({"instantiations": [ht()]})
        self.assertEqual(result[0]["variants"], {"collision": "open_addressing"})

    def test_upper_camel_case_is_a_valid_case(self):
        result = generator.validate_config({"instantiations": [da(case="UpperCamelCase")]})
        self.assertEqual(result[0]["case"], "UpperCamelCase")

    def test_hashtable_missing_variants_axis_errors(self):
        bad = ht()
        del bad["variants"]
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [bad]})

    def test_hashtable_unknown_axis_errors(self):
        bad = ht(variants={"collision": "open_addressing", "nonexistent_axis": "x"})
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [bad]})

    def test_unknown_template_errors(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(template="avl_tree")]})

    def test_missing_required_field_errors(self):
        bad = da()
        del bad["value_type"]
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [bad]})

    def test_bad_struct_name_not_an_identifier_errors(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(struct_name="not an identifier")]})

    def test_invalid_case_errors(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(case="PascalCase")]})

    def test_empty_instantiations_errors(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": []})

    def test_missing_instantiations_key_errors(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({})

    def test_batch_multiple_instantiations(self):
        result = generator.validate_config({"instantiations": [da(), ht()]})
        self.assertEqual(len(result), 2)

    def test_includes_defaults_to_empty_list(self):
        result = generator.validate_config({"instantiations": [da()]})
        self.assertEqual(result[0]["includes"], [])

    def test_includes_accepts_a_list_of_strings(self):
        result = generator.validate_config({"instantiations": [da(includes=["types.h", "<stdint.h>"])]})
        self.assertEqual(result[0]["includes"], ["types.h", "<stdint.h>"])

    def test_includes_rejects_non_list(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(includes="types.h")]})

    def test_includes_rejects_non_string_entries(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(includes=[123])]})

    def test_file_name_defaults_to_none(self):
        result = generator.validate_config({"instantiations": [da()]})
        self.assertIsNone(result[0]["file_name"])

    def test_file_name_accepted_independently_of_struct_name(self):
        result = generator.validate_config(
            {"instantiations": [da(struct_name="CharMap", file_name="char_map")]}
        )
        self.assertEqual(result[0]["struct_name"], "CharMap")
        self.assertEqual(result[0]["file_name"], "char_map")

    def test_file_name_must_be_a_valid_identifier(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(file_name="not a valid name")]})

    def test_file_name_must_be_a_string(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(file_name=123)]})


if __name__ == "__main__":
    unittest.main()

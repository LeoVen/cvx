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
        self.assertEqual(result[0]["case"], {"function": "snake_case", "struct": "snake_case"})
        self.assertEqual(result[0]["out_dir"], ".")

    def test_valid_hashtable_requires_variants(self):
        result = generator.validate_config({"instantiations": [ht()]})
        self.assertEqual(
            result[0]["variants"], {"collision": "open_addressing", "kind": "map"}
        )

    def test_upper_camel_case_is_a_valid_case(self):
        result = generator.validate_config(
            {"instantiations": [da(case={"function": "PascalCase", "struct": "PascalCase"})]}
        )
        self.assertEqual(result[0]["case"], {"function": "PascalCase", "struct": "PascalCase"})

    def test_case_keys_can_differ(self):
        result = generator.validate_config(
            {"instantiations": [da(case={"function": "snake_case", "struct": "PascalCase"})]}
        )
        self.assertEqual(result[0]["case"], {"function": "snake_case", "struct": "PascalCase"})

    def test_missing_case_key_defaults_to_snake_case(self):
        result = generator.validate_config({"instantiations": [da(case={"struct": "camelCase"})]})
        self.assertEqual(result[0]["case"], {"function": "snake_case", "struct": "camelCase"})

    def test_case_as_string_errors(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(case="snake_case")]})

    def test_unknown_case_key_errors(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(case={"enum": "PascalCase"})]})

    def test_valid_circular_buffer(self):
        result = generator.validate_config(
            {"instantiations": [da(template="circular_buffer", struct_name="my_ring")]}
        )
        self.assertEqual(result[0]["template"], "circular_buffer")
        self.assertEqual(result[0]["value_type"], "int")

    def test_circular_buffer_requires_value_type(self):
        bad = da(template="circular_buffer")
        del bad["value_type"]
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [bad]})

    def test_circular_buffer_rejects_variants(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config(
                {"instantiations": [da(template="circular_buffer", variants={"kind": "map"})]}
            )

    def test_valid_linked_list(self):
        for node in ("single", "double"):
            result = generator.validate_config(
                {"instantiations": [da(template="linked_list", variants={"node": node})]}
            )
            self.assertEqual(result[0]["variants"], {"node": node})

    def test_linked_list_requires_node_variant(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [da(template="linked_list")]})

    def test_linked_list_unknown_node_variant_errors(self):
        with self.assertRaises(generator.ConfigError):
            generator.validate_config(
                {"instantiations": [da(template="linked_list", variants={"node": "triple"})]}
            )

    def test_hashtable_missing_variants_axis_errors(self):
        bad = ht()
        del bad["variants"]
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [bad]})

    def test_hashtable_unknown_axis_errors(self):
        bad = ht(variants={"collision": "open_addressing", "nonexistent_axis": "x"})
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [bad]})

    def test_hashtable_unknown_variant_name_errors(self):
        bad = ht(variants={"collision": "quantum_probing"})
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [bad]})

    def test_hashtable_separate_chaining_is_a_valid_variant(self):
        result = generator.validate_config(
            {"instantiations": [ht(variants={"collision": "separate_chaining"})]}
        )
        self.assertEqual(
            result[0]["variants"], {"collision": "separate_chaining", "kind": "map"}
        )

    def test_hashtable_kind_defaults_to_map(self):
        result = generator.validate_config({"instantiations": [ht()]})
        self.assertEqual(result[0]["variants"]["kind"], "map")

    def test_hashtable_kind_set_requires_no_value_type(self):
        bad = ht(variants={"collision": "open_addressing", "kind": "set"})
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [bad]})

    def test_hashtable_kind_set_without_value_type_is_valid(self):
        config = ht(variants={"collision": "open_addressing", "kind": "set"})
        del config["value_type"]
        result = generator.validate_config({"instantiations": [config]})
        self.assertIsNone(result[0]["value_type"])
        self.assertEqual(result[0]["variants"]["kind"], "set")

    def test_hashtable_kind_map_requires_value_type(self):
        config = ht(variants={"collision": "open_addressing", "kind": "map"})
        del config["value_type"]
        with self.assertRaises(generator.ConfigError):
            generator.validate_config({"instantiations": [config]})

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
            generator.validate_config({"instantiations": [da(case={"function": "kebab-case"})]})

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

"""Tests for generate_instantiation() and its helpers. generate_instantiation
itself now shells out to a real C compiler to preprocess templates, so
these need one on PATH (skipped, not failed, if none is found). See
test_generate_e2e.py for the full compile-and-run version."""
import contextlib
import io
import json
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent.parent))
import cvx_generator as generator

COMPILER = next((c for c in generator.COMPILER_FLAGS if shutil.which(c)), None)


def da(**overrides):
    base = {
        "template": "dynamic_array",
        "value_type": "int",
        "key_type": None,
        "struct_name": "my_array",
        "prefix": "ma",
        "file_name": None,
        "case": {"function": "snake_case", "struct": "snake_case"},
        "options": {},
        "out_dir": {"headers": ".", "sources": "."},
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
        # Only the template's own includes (cvx/core.h, <stdbool.h>, ...)
        # should appear -- no custom "includes" entry was configured.
        header_filename, header_out, _, _ = generator.generate_instantiation(da(), COMPILER)
        self.assertEqual(header_filename, "my_array.h")
        self.assertIn('#include "cvx/core.h"', header_out)
        self.assertEqual(header_out.count("#include"), 6)  # core.h, flags.h, stdbool.h, stddef.h, stdlib.h, string.h

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


class TestOutputPaths(unittest.TestCase):
    def test_resolves_each_kind_against_output_root(self):
        header_path, source_path = generator.output_paths(
            da(out_dir={"headers": "include", "sources": "src"}), Path("out")
        )
        self.assertEqual(header_path, Path("out/include/my_array.h"))
        self.assertEqual(source_path, Path("out/src/my_array.c"))

    def test_uses_file_name(self):
        header_path, source_path = generator.output_paths(da(file_name="arr"), Path("out"))
        self.assertEqual(header_path, Path("out/arr.h"))
        self.assertEqual(source_path, Path("out/arr.c"))


class TestRun(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())
        self.addCleanup(shutil.rmtree, self.tmp)
        self.config_path = self.tmp / "config" / "config.json"
        self.config_path.parent.mkdir()
        self.config_path.write_text(
            json.dumps(
                {
                    "out_dir": {"headers": "include", "sources": "src"},
                    "instantiations": [
                        {"template": "dynamic_array", "value_type": "int", "struct_name": "my_array", "prefix": "ma"},
                        {
                            "template": "dynamic_array",
                            "value_type": "char",
                            "struct_name": "my_chars",
                            "prefix": "mc",
                            "out_dir": "gen",
                        },
                    ],
                }
            )
        )

    def list_outputs(self, output_root=None):
        stdout = io.StringIO()
        with contextlib.redirect_stdout(stdout):
            generator.run(self.config_path, COMPILER, True, output_root)
        return stdout.getvalue().splitlines()

    def test_list_outputs_defaults_to_config_folder(self):
        root = self.config_path.parent
        self.assertEqual(
            self.list_outputs(),
            [
                str(root / "include/my_array.h"),
                str(root / "src/my_array.c"),
                str(root / "gen/my_chars.h"),
                str(root / "gen/my_chars.c"),
            ],
        )

    def test_list_outputs_resolves_against_output_flag(self):
        root = self.tmp / "elsewhere"
        self.assertEqual(self.list_outputs(root)[:2], [str(root / "include/my_array.h"), str(root / "src/my_array.c")])

    def test_list_outputs_does_not_create_folders(self):
        self.list_outputs()
        self.assertEqual(list(self.config_path.parent.iterdir()), [self.config_path])

    @unittest.skipUnless(COMPILER, "no C compiler found on PATH")
    def test_generation_writes_where_list_outputs_points(self):
        root = self.tmp / "elsewhere"
        written = generator.run(self.config_path, COMPILER, False, root)
        self.assertEqual(written, self.list_outputs(root))
        for path in written:
            self.assertTrue(Path(path).is_file(), path)

    @unittest.skipUnless(COMPILER, "no C compiler found on PATH")
    def test_main_accepts_output_flag(self):
        root = self.tmp / "elsewhere"
        with contextlib.redirect_stdout(io.StringIO()):
            status = generator.main(["--config", str(self.config_path), "--compiler", COMPILER, "--output", str(root)])
        self.assertEqual(status, 0)
        self.assertTrue((root / "include/my_array.h").is_file())
        self.assertTrue((root / "src/my_array.c").is_file())


if __name__ == "__main__":
    unittest.main()

"""End-to-end: run the real CLI against e2e/config.json, then
compile the generated output plus e2e/smoke.c with gcc and run it.
Skips (rather than fails) if gcc isn't on PATH -- unit tests above already
cover the generator's own logic without needing a C toolchain."""

import shutil
import subprocess
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
E2E_DIR = Path(__file__).resolve().parent / "e2e"


@unittest.skipUnless(shutil.which("gcc"), "gcc not found on PATH")
class TestGenerateEndToEnd(unittest.TestCase):
    def test_generate_and_compile_and_run(self):
        generated_dir = E2E_DIR / "generated"
        if generated_dir.exists():
            shutil.rmtree(generated_dir)

        gen = subprocess.run(
            [
                sys.executable,
                str(REPO_ROOT / "cvx_generator.py"),
                "--config",
                str(E2E_DIR / "config.json"),
                "--compiler",
                "gcc",
            ],
            capture_output=True,
            text=True,
        )
        self.assertEqual(gen.returncode, 0, gen.stderr)

        binary = Path("/tmp") / "cvx_e2e_smoke"
        sources = [
            str(E2E_DIR / "smoke.c"),
            str(generated_dir / "int_array.c"),
            str(generated_dir / "int_map_oa.c"),
            str(generated_dir / "int_map_sc.c"),
            str(generated_dir / "int10_array.c"),
            str(generated_dir / "binop_list.c"),
            str(generated_dir / "int_ring.c"),
            str(generated_dir / "str_ring.c"),
            str(generated_dir / "int_slist.c"),
            str(generated_dir / "int_dlist.c"),
            str(generated_dir / "str_list.c"),
            str(generated_dir / "int_heap.c"),
            str(generated_dir / "str_heap.c"),
            str(generated_dir / "int_segtree.c"),
            str(generated_dir / "uint_segtree.c"),
        ]
        compile_result = subprocess.run(
            [
                "gcc",
                "-I",
                str(REPO_ROOT),
                "-I",
                str(E2E_DIR),
                "-Wall",
                "-Wextra",
                "-Wpedantic",
                "-fsanitize=address,undefined",
                "-g",
                *sources,
                "-o",
                str(binary),
            ],
            capture_output=True,
            text=True,
        )
        self.assertEqual(compile_result.returncode, 0, compile_result.stderr)

        run_result = subprocess.run([str(binary)], capture_output=True, text=True)
        self.assertEqual(
            run_result.returncode, 0, run_result.stdout + run_result.stderr
        )
        self.assertIn("all cvx smoke tests passed", run_result.stdout)


if __name__ == "__main__":
    unittest.main()

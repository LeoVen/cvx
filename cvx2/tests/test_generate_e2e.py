"""End-to-end: run the real CLI against cvx2/examples/config.json, then
compile the generated output plus cvx2/examples/smoke.c with gcc and run it.
Skips (rather than fails) if gcc isn't on PATH -- unit tests above already
cover the generator's own logic without needing a C toolchain."""
import shutil
import subprocess
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
CVX2_DIR = REPO_ROOT / "cvx2"
EXAMPLES_DIR = CVX2_DIR / "examples"


@unittest.skipUnless(shutil.which("gcc"), "gcc not found on PATH")
class TestGenerateEndToEnd(unittest.TestCase):
    def test_generate_and_compile_and_run(self):
        generated_dir = EXAMPLES_DIR / "generated"
        if generated_dir.exists():
            shutil.rmtree(generated_dir)

        gen = subprocess.run(
            [sys.executable, str(CVX2_DIR / "generator" / "generator.py"), "--config", str(EXAMPLES_DIR / "config.json")],
            capture_output=True,
            text=True,
        )
        self.assertEqual(gen.returncode, 0, gen.stderr)

        binary = Path("/tmp") / "cvx2_e2e_smoke"
        sources = [
            str(EXAMPLES_DIR / "smoke.c"),
            str(generated_dir / "int_array.c"),
            str(generated_dir / "int_map_oa.c"),
            str(generated_dir / "int_map_sc.c"),
            str(generated_dir / "int10_array.c"),
            str(generated_dir / "binop_list.c"),
        ]
        compile_result = subprocess.run(
            [
                "gcc",
                "-I", str(REPO_ROOT),
                "-I", str(EXAMPLES_DIR),
                "-Wall", "-Wextra", "-Wpedantic",
                "-fsanitize=address,undefined",
                "-g",
                *sources,
                "-o", str(binary),
            ],
            capture_output=True,
            text=True,
        )
        self.assertEqual(compile_result.returncode, 0, compile_result.stderr)

        run_result = subprocess.run([str(binary)], capture_output=True, text=True)
        self.assertEqual(run_result.returncode, 0, run_result.stdout + run_result.stderr)
        self.assertIn("all cvx2 smoke tests passed", run_result.stdout)


if __name__ == "__main__":
    unittest.main()

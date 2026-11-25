"""Exercise fixed capture completion without game assets or a running game."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


class FixedFinishTests(unittest.TestCase):
    def run_fixture(self, lines, code=0, fixed=True):
        with tempfile.TemporaryDirectory(prefix="pe-route-finish-") as tmp:
            root = Path(tmp)
            fake = root / "port"
            fake.write_text(f"#!{sys.executable}\nimport sys\n"
                            + "\n".join(f"print({line!r}, file=sys.stderr, flush=True)" for line in lines)
                            + f"\nsys.exit({code})\n")
            fake.chmod(0o755)
            args = [sys.executable, str(Path(__file__).with_name("day1_route_regress.py")),
                    "--bin", str(fake), "--until", "field_start", "--max-frames", "2",
                    "--skip-normal-start-check", "--log", str(root / "route.log")]
            if fixed:
                args.append("--finish-at-frame-limit")
            return subprocess.run(args, capture_output=True, text=True, timeout=5)

    def test_clean_frame_limit(self):
        result = self.run_fixture(["[ROUTE] frame=1 token=A8000000 story=00000009",
                                   "[HOST] stop_reason=frame-limit"])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("REGRESS PASS", result.stdout)

    def test_reject_incomplete_or_failed_finish(self):
        checkpoint = "[ROUTE] frame=1 token=A8000000 story=00000009"
        for lines, code in [([checkpoint], 0),
                            ([checkpoint, "[HOST] stop_reason=frame-limit"], 7),
                            (["[HOST] stop_reason=frame-limit"], 0),
                            (["[ROUTE] frame=1 token=A8000000 story=00000010",
                              "[ROUTE] frame=2 token=A8000000 story=00000000"], 0)]:
            with self.subTest(lines=lines, code=code):
                result = self.run_fixture(lines, code)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("REGRESS FAIL", result.stdout)
                self.assertNotIn("Traceback", result.stderr)

    def test_legacy_checkpoint_only_mode(self):
        result = self.run_fixture(["[ROUTE] frame=1 token=A8000000 story=00000009"], fixed=False)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()

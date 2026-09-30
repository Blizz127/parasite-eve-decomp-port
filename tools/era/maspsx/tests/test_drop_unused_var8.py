import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


FRAME = [
    "subu\t$sp,$sp,32",
    "sw\t$16,24($sp)",
    "sw\t$31,28($sp)",
    "jal\tfunc_80052F70",
    "lw\t$31,28($sp)",
    "lw\t$16,24($sp)",
    "addu\t$sp,$sp,32",
    "jr\t$31",
]


class TestDropUnusedVar8(unittest.TestCase):
    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_DROP_UNUSED_VAR8": "0"}):
            out = MaspsxProcessor(lines, drop_unused_var8=on).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_shrinks_unused_slot(self):
        self.assertEqual(
            [
                "subu\t$sp,$sp,24",
                "sw\t$16,16($sp)",
                "sw\t$31,20($sp)",
                "jal\tfunc_80052F70",
                "nop",
                "lw\t$31,20($sp)",
                "lw\t$16,16($sp)",
                "addu\t$sp,$sp,24",
                "jr\t$31",
            ],
            self.process(FRAME),
        )

    def test_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_DROP_UNUSED_VAR8": "1"}):
            out = MaspsxProcessor(FRAME).process_lines()
        got = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual(self.process(FRAME)[:3], got[:3])

    def test_flag_off_keeps_32(self):
        got = self.process(FRAME, on=False)
        self.assertIn("subu\t$sp,$sp,32", got)

    def test_refuses_a_real_local(self):
        lines = FRAME[:]
        lines.insert(3, "sw\t$2,16($sp)")
        self.assertIn("subu\t$sp,$sp,32", self.process(lines))

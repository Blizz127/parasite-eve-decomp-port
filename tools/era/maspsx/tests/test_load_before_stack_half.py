import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


PAIR = [
    "mult\t$2,$7",
    "#.set\tvolatile",
    "sh\t$3,0($sp)",
    "#.set\tnovolatile",
    "lhu\t$2,0($5)",
    "lhu\t$3,2($5)",
    "mflo\t$9",
]
ALREADY = [
    "mult\t$2,$7",
    "lhu\t$2,2($5)",
    "sh\t$3,0($sp)",
    "mflo\t$9",
]


class TestLoadBeforeStackHalf(unittest.TestCase):
    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_LOAD_BEFORE_STACK_HALF": "0"}):
            out = MaspsxProcessor(lines, load_before_stack_half=on).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_swaps_sh_then_lhu(self):
        got = self.process(PAIR)
        self.assertEqual(got[0], "mult\t$2,$7")
        self.assertEqual(got[1], "lhu\t$2,0($5)")
        self.assertEqual(got[2], "sh\t$3,0($sp)")
        self.assertEqual(got[3], "lhu\t$3,2($5)")

    def test_leaves_lhu_then_sh(self):
        self.assertEqual(
            ["mult\t$2,$7", "lhu\t$2,2($5)", "sh\t$3,0($sp)", "mflo\t$9"],
            self.process(ALREADY),
        )

    def test_flag_off(self):
        got = self.process(PAIR, on=False)
        self.assertLess(got.index("sh\t$3,0($sp)"), got.index("lhu\t$2,0($5)"))

    def test_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_LOAD_BEFORE_STACK_HALF": "1"}):
            out = MaspsxProcessor(PAIR).process_lines()
        got = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual(self.process(PAIR)[:4], got[:4])

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

LINES = [
    "lw\t$2,D_800C0DBC",
    "sw\t$0,D_800BE9EC",
    "beq\t$2,$0,$L3",
    "sh\t$0,0($6)",
    "$L3:",
    "jr\t$31",
]


class TestSinkZeroStoreIntoBeqz(unittest.TestCase):
    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_SINK_ZERO_STORE_INTO_BEQZ": "0"}):
            out = MaspsxProcessor(lines, sink_zero_store_into_beqz=on).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_positive_store_fills_beqz_delay(self):
        got = self.process(LINES)
        self.assertEqual(
            [
                "lw\t$2,D_800C0DBC",
                "lui\t$at,%hi(D_800BE9EC)",
                "beq\t$2,$0,$L3",
                "sw\t$0,%lo(D_800BE9EC)($at)",
                "sh\t$0,0($6)",
            ],
            got[:5],
        )

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_SINK_ZERO_STORE_INTO_BEQZ": "1"}):
            out = MaspsxProcessor(LINES).process_lines()
        got = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual(self.process(LINES)[:5], got[:5])

    def test_negative_flag_off_keeps_store_before_branch(self):
        got = self.process(LINES, on=False)
        self.assertLess(got.index("sw\t$0,D_800BE9EC"), got.index("beq\t$2,$0,$L3"))

    def test_negative_nonzero_store(self):
        lines = ["sw\t$4,D_800BE9EC", "beq\t$2,$0,$L3", "nop"]
        self.assertEqual(self.process(lines, on=False)[:1], self.process(lines)[:1])

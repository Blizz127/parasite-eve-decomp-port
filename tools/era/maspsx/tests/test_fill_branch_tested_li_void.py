import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_8008486C (void): retail fills `beq $2,$0,<epilogue>` with the
# following `li $2,1` even though the branch tests $2.
LINES = [
    "slt\t$2,$2,61",
    "beq\t$2,$0,$L1",
    "li\t$2,1\t\t\t# 0x00000001",
    "sb\t$2,88($16)",
    "$L1:",
    "lw\t$31,20($sp)",
    ".set\tnoreorder",
    ".set\tnomacro",
    "j\t$31",
    "addu\t$sp,$sp,24",
    ".set\tmacro",
    ".set\treorder",
]


def insns(out):
    return [
        x for x in strip_comments(out)
        if x and not x.startswith(".") and not x.endswith(":")
    ]


class TestFillBranchTestedLiVoid(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_FILL_BRANCH_TESTED_LI_VOID": "1" if on else "0"}
        with patch.dict(os.environ, env):
            return insns(MaspsxProcessor(lines).process_lines())

    def test_fills(self):
        got = self.process(LINES, True)
        self.assertEqual(["beq\t$2,$0,$L1", "li\t$2,1", "sb\t$2,88($16)"], got[1:4])

    def test_flag_off(self):
        got = self.process(LINES, False)
        self.assertEqual(["beq\t$2,$0,$L1", "nop", "li\t$2,1"], got[1:4])

    def test_untested_register_not_filled(self):
        lines = list(LINES)
        lines[2] = "li\t$3,1"
        got = self.process(lines, True)
        self.assertEqual("nop", got[2])

    def test_live_at_target_not_filled(self):
        lines = list(LINES)
        lines[5] = "sw\t$2,20($sp)"
        got = self.process(lines, True)
        self.assertEqual("nop", got[2])

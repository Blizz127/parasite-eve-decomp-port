import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_80083944 (frameless, with patch 12): the `return 0` branch steals the
# `move $2,$0` of the `j $31` block at its target and goes straight to the
# final return.
LINES = [
    ".ent\tf",
    "f:",
    ".frame\t$sp,0,$31",
    ".mask\t0x00000000,0",
    ".fmask\t0x00000000,0",
    "sltu\t$2,$2,$3",
    "bne\t$2,$0,$L29",
    ".set\tnoreorder",
    ".set\tnomacro",
    "j\t$L5",
    "sb\t$0,71($5)",
    ".set\tmacro",
    ".set\treorder",
    "$L29:",
    ".set\tnoreorder",
    ".set\tnomacro",
    "j\t$31",
    "move\t$2,$0",
    ".set\tmacro",
    ".set\treorder",
    "$L5:",
    "li\t$2,1",
    "j\t$31",
    ".end\tf",
]


def insns(out):
    return [
        x for x in strip_comments(out)
        if x and not x.startswith(".") and not x.endswith(":")
    ]


class TestBranchStealJumpSlot(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {
            "MASPSX_RETURN_VIA_EPILOGUE_JUMP": "1",
            "MASPSX_BRANCH_STEAL_JUMP_SLOT": "1" if on else "0",
        }
        with patch.dict(os.environ, env):
            return insns(MaspsxProcessor(lines).process_lines())

    def test_steals(self):
        got = self.process(LINES, True)
        self.assertEqual("sltu\t$2,$2,$3", got[0])
        self.assertTrue(got[1].startswith("bne\t$2,$0,$L9"), got[1])
        self.assertEqual("addu\t$2,$0,$zero", got[2])

    def test_flag_off(self):
        got = self.process(LINES, False)
        self.assertEqual("bne\t$2,$0,$L29", got[1])
        self.assertEqual("nop", got[2])

    def test_live_on_fall_through_blocks(self):
        # the fall-through reads $2 before writing it: no steal
        lines = list(LINES)
        lines[10] = "sb\t$2,71($5)"
        got = self.process(lines, True)
        self.assertEqual("bne\t$2,$0,$L29", got[1])

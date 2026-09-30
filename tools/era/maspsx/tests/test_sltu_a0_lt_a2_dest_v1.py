import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


# The low-half tail of func_80073244 after maspsx. The compare of $a0 < $a2
# must land in $v1; the earlier $a2 < $a0 compare stays in $v0.
WINDOW = [
    ".set\tnoreorder",
    "sltu\t$2,$6,$4",
    "bne\t$2,$0,$L9",
    "li\t$2,0x00000001",
    "sltu\t$2,$4,$6",
    "bne\t$2,$0,$L5",
    "li\t$2,-1",
    "j\t$L8",
    "addu\t$2,$0,$zero",
]


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".")]


class TestSltuA0LtA2DestV1(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_SLTU_A0_LT_A2_DEST_V1": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(
                lines, sltu_a0_lt_a2_dest_v1=on
            ).process_lines()
        return kept(out)

    def test_renames_only_the_a0_lt_a2_pair(self):
        got = self.process(WINDOW, True)
        self.assertIn("sltu\t$2,$6,$4", got)
        self.assertIn("sltu\t$3,$4,$6", got)
        self.assertNotIn("sltu\t$2,$4,$6", got)
        self.assertIn("bne\t$3,$0,$L5", got)
        self.assertIn("bne\t$2,$0,$L9", got)
        self.assertIn("li\t$2,0x00000001", got)
        self.assertIn("li\t$2,-1", got)

    def test_flag_off_keeps_v0(self):
        off = self.process(WINDOW, False)
        self.assertIn("sltu\t$2,$4,$6", off)
        self.assertNotIn("sltu\t$3,$4,$6", off)
        self.assertEqual(self.process(WINDOW, False), off)

    def test_leaves_a_different_operand_order(self):
        changed = [
            "sltu\t$2,$6,$4" if x == "sltu\t$2,$4,$6" else x for x in WINDOW
        ]
        got = self.process(changed, True)
        self.assertEqual(got.count("sltu\t$2,$6,$4"), 2)
        self.assertNotIn("sltu\t$3,$4,$6", got)

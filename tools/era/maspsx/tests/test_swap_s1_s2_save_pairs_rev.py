import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_80076664 / func_800768A0: retail saves the $s1=$a0 pair first.
PAIRS = [
    "subu\t$sp,$sp,32",
    "sw\t$18,24($sp)",
    "move\t$18,$5",
    "sw\t$17,20($sp)",
    "move\t$17,$4",
    "li\t$4,-1",
    "jr\t$31",
]


class TestSwapS1S2SavePairsRev(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_SWAP_S1_S2_SAVE_PAIRS_REV": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_swaps_the_pairs(self):
        self.assertEqual(
            [
                "sw\t$17,20($sp)",
                "addu\t$17,$4,$zero",
                "sw\t$18,24($sp)",
                "addu\t$18,$5,$zero",
            ],
            self.process(PAIRS, True)[1:5],
        )

    def test_flag_off(self):
        self.assertEqual("sw\t$18,24($sp)", self.process(PAIRS, False)[1])

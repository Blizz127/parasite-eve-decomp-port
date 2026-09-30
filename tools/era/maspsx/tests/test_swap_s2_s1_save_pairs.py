import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


PAIRS = [
    "subu\t$sp,$sp,32",
    "sw\t$17,20($sp)",
    "move\t$17,$4",
    "sw\t$18,24($sp)",
    "move\t$18,$5",
    "li\t$4,-1",
    "jr\t$31",
]


class TestSwapS2S1SavePairs(unittest.TestCase):
    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_SWAP_S2_S1_SAVE_PAIRS": "0"}):
            out = MaspsxProcessor(lines, swap_s2_s1_save_pairs=on).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_swaps_the_pairs(self):
        got = self.process(PAIRS)
        self.assertEqual(
            [
                "sw\t$18,24($sp)",
                "addu\t$18,$5,$zero",
                "sw\t$17,20($sp)",
                "addu\t$17,$4,$zero",
            ],
            got[1:5],
        )

    def test_flag_off(self):
        got = self.process(PAIRS, on=False)
        self.assertEqual("sw\t$17,20($sp)", got[1])

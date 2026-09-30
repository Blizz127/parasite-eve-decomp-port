import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


TRIPLE = [
    "li\t$2,-1",
    ".set\tnoreorder",
    ".set\tnomacro",
    "beq\t$3,$2,$L2",
    "addu\t$2,$3,$2",
    ".set\tmacro",
    ".set\treorder",
    "j\t$L2",
    "sw\t$2,20($16)",
]

BNE = [
    "li\t$2,-1",
    "bne\t$3,$2,$L13",
    "addu\t$2,$3,$2",
]


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".")]


class TestAdduNeg1BeqDelay(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_ADDU_NEG1_BEQ_DELAY": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines, addu_neg1_beq_delay=on).process_lines()
        return kept(out)

    def test_rewrites_beq_delay(self):
        got = self.process(TRIPLE, True)
        self.assertIn("addiu\t$2,$3,-1", got)
        self.assertNotIn("addu\t$2,$3,$2", got)
        self.assertIn("li\t$2,-1", got)
        self.assertIn("beq\t$3,$2,$L2", got)

    def test_flag_off_is_identical(self):
        self.assertEqual(self.process(TRIPLE, False), self.process(TRIPLE, False))
        off = self.process(TRIPLE, False)
        self.assertIn("addu\t$2,$3,$2", off)
        self.assertNotIn("addiu\t$2,$3,-1", off)

    def test_leaves_bne_alone(self):
        got = self.process(BNE, True)
        self.assertIn("addu\t$2,$3,$2", got)
        self.assertNotIn("addiu\t$2,$3,-1", got)

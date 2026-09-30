import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


WINDOW = [
    ".set\tnoreorder",
    "mult\t$9,$11",
    "mflo\t$9",
    "sra\t$9,$9,12",
    "mult\t$10,$12",
    "mfhi\t$3",
    "mflo\t$10",
]


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".")]


class TestMultMfloToMultu(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_MULT_MFLO_TO_MULTU": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines, mult_mflo_to_multu=on).process_lines()
        return kept(out)

    def test_rewrites_mult_whose_next_use_is_mflo(self):
        got = self.process(WINDOW, True)
        self.assertIn("multu\t$9,$11", got)
        self.assertNotIn("mult\t$9,$11", got)
        self.assertIn("mflo\t$9", got)
        # mfhi means the low half is not the only result.
        self.assertIn("mult\t$10,$12", got)
        self.assertNotIn("multu\t$10,$12", got)

    def test_flag_off_keeps_mult(self):
        off = self.process(WINDOW, False)
        self.assertIn("mult\t$9,$11", off)
        self.assertNotIn("multu\t$9,$11", off)
        self.assertEqual(self.process(WINDOW, False), off)

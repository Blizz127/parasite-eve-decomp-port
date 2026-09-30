import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


WINDOW = [
    ".set\tnoreorder",
    "mult\t$4,$4",
    "mfhi\t$3",
    "mflo\t$2",
    "move\t$7,$2",
    "move\t$8,$3",
    "mult\t$5,$5",
    "mfhi\t$3",
    "mflo\t$2",
    "move\t$9,$2",
    "move\t$10,$3",
    "mult\t$6,$6",
    "mfhi\t$3",
    "mflo\t$2",
    "move\t$4,$2",
    "move\t$5,$3",
    "addu\t$2,$7,$9",
]


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".")]


class TestMfloBeforeMfhi(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_MFLO_BEFORE_MFHI": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines, mflo_before_mfhi=on).process_lines()
        return kept(out)

    def test_retargets_copies_and_keeps_nops_only_before_mult(self):
        got = self.process(WINDOW, True)
        self.assertEqual(
            got,
            [
                "mult\t$4,$4",
                "mflo\t$7",
                "mfhi\t$8",
                "nop",
                "nop",
                "mult\t$5,$5",
                "mflo\t$9",
                "mfhi\t$10",
                "nop",
                "nop",
                "mult\t$6,$6",
                "mflo\t$4",
                "mfhi\t$5",
                "addu\t$2,$7,$9",
            ],
        )

    def test_flag_off_keeps_mfhi_first(self):
        got = self.process(WINDOW, False)
        self.assertIn("mfhi\t$3", got)
        self.assertNotIn("mflo\t$7", got)

    def test_folds_shift_copies_into_the_half_reads(self):
        lines = [
            ".set\tnoreorder",
            "mult\t$4,$5",
            "mfhi\t$5",
            "mflo\t$4",
            "srl\t$2,$4,16",
            "sll\t$3,$5,16",
            "j\t$31",
            "or\t$2,$3,$2",
        ]
        got = self.process(lines, True)
        self.assertEqual(
            got,
            [
                "mult\t$4,$5",
                "mflo\t$2",
                "mfhi\t$3",
                "srl\t$2,$2,16",
                "sll\t$3,$3,16",
                "j\t$31",
                "or\t$2,$3,$2",
            ],
        )
        off = self.process(lines, False)
        self.assertIn("mfhi\t$5", off)
        self.assertIn("srl\t$2,$4,16", off)

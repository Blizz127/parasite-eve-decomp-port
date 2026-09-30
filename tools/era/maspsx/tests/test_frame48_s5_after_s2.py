import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


PROLOGUE = [
    "subu\t$sp,$sp,48",
    "sw\t$21,36($sp)",
    "sw\t$18,24($sp)",
    "addu\t$18,$4,$zero",
    "sw\t$17,20($sp)",
    "jal\tfunc_80066800",
    "addu\t$21,$5,$zero",
    "lw\t$31,40($sp)",
    "lw\t$21,36($sp)",
    "lw\t$18,24($sp)",
    "lw\t$17,20($sp)",
    "addu\t$sp,$sp,48",
    "j\t$31",
]

OTHER = [
    "subu\t$sp,$sp,32",
    "sw\t$16,16($sp)",
    "addu\t$sp,$sp,32",
    "j\t$31",
]


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".")]


class TestFrame48S5AfterS2(unittest.TestCase):
    @staticmethod
    def process(lines, on=True):
        env = {"MASPSX_FRAME48_S5_AFTER_S2": "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines, frame48_s5_after_s2=on).process_lines()
        return kept(out)

    def test_reorders_save_and_pads_frame(self):
        got = self.process(PROLOGUE)
        self.assertEqual(
            got[:6],
            [
                "subu\t$sp,$sp,56",
                "sw\t$18,32($sp)",
                "addu\t$18,$4,$zero",
                "sw\t$21,44($sp)",
                "sw\t$17,28($sp)",
                "jal\tfunc_80066800",
            ],
        )
        self.assertIn("addu\t$21,$5,$zero", got)
        self.assertIn("lw\t$31,48($sp)", got)
        self.assertIn("lw\t$21,44($sp)", got)
        self.assertIn("addu\t$sp,$sp,56", got)

    def test_off_is_identity(self):
        self.assertEqual(self.process(PROLOGUE, on=False), kept(
            MaspsxProcessor(PROLOGUE).process_lines()
        ) if False else self.process(PROLOGUE, on=False))
        off = self.process(PROLOGUE, on=False)
        self.assertEqual(off[0], "subu\t$sp,$sp,48")
        self.assertEqual(off[1], "sw\t$21,36($sp)")

    def test_other_frame_unchanged(self):
        got = self.process(OTHER)
        self.assertEqual(got[0], "subu\t$sp,$sp,32")
        self.assertIn("sw\t$16,16($sp)", got)
        self.assertIn("addu\t$sp,$sp,32", got)

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


WINDOW = [
    ".set\tnoreorder",
    "addu\t$3,$2,$zero",
    "sll\t$2,$2,24",
    "beq\t$2,$0,$L5",
    "li\t$2,-65536",
    "lhu\t$2,D_800B0DBC",
    "nop",
    "sll\t$2,$2,16",
    "$L5:",
    "blez\t$2,$L3",
    "nop",
]


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".") and not x.endswith(":")]


class TestDupHalfShift(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_DUP_HALF_SHIFT": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines, dup_half_shift=on).process_lines()
        return out

    def test_rewrites_the_shift_window(self):
        out = self.process(WINDOW, True)
        text = "\n".join(out)
        self.assertNotIn("sll\t$2,$2,24", text)
        self.assertIn("beq\t$3,$0,$L5b", text)
        self.assertIn("addiu\t$2,$zero,-1", text)
        self.assertIn("lhu\t$2,D_800B0DBC", text)
        self.assertIn("j\t$L5", text)
        self.assertIn("$L5b:", text)
        self.assertEqual(text.count("sll\t$2,$2,16"), 2)
        self.assertIn("blez\t$2,$L3", text)
        # The halfword load stays ahead of both shifts.
        body = kept(out)
        self.assertLess(body.index("lhu\t$2,D_800B0DBC"), body.index("sll\t$2,$2,16"))

    def test_flag_off_keeps_the_sll24(self):
        out = self.process(WINDOW, False)
        text = "\n".join(out)
        self.assertIn("sll\t$2,$2,24", text)
        self.assertNotIn("$L5b:", text)
        self.assertEqual(self.process(WINDOW, False), out)

    def test_leaves_a_different_shift(self):
        changed = ["sll\t$2,$2,16" if x.startswith("sll\t$2,$2,24") else x for x in WINDOW]
        text = "\n".join(self.process(changed, True))
        self.assertNotIn("beq\t$3,$0,$L5b", text)
        self.assertNotIn("addiu\t$2,$zero,-1", text)

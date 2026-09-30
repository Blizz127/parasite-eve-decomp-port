import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


# cc1 -O2 -G0 for a `return 1` hidden behind an empty asm barrier.
# The `j $L4` falls through into `$L4: j $31` once deleted; its delay is the zero.
WINDOW = [
    ".set\tnoreorder",
    "bne\t$2,$0,$L3",
    "li\t$2,0x00000001",
    "j\t$L4",
    "addu\t$2,$0,$zero",
    "$L3:",
    "$L4:",
    "$L6:",
    "j\t$31",
    "nop",
]

KEPT_JUMP = [
    ".set\tnoreorder",
    "j\t$L9",
    "addu\t$2,$0,$zero",
    "lw\t$2,0($3)",
    "$L9:",
    "j\t$31",
    "nop",
]


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".")]


class TestDropJumpToEpilogue(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_DROP_JUMP_TO_EPILOGUE": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines, drop_jump_to_epilogue=on).process_lines()
        return kept(out)

    def test_drops_jump_whose_target_is_the_epilogue(self):
        got = self.process(WINDOW, True)
        self.assertNotIn("j\t$L4", got)
        self.assertIn("bne\t$2,$0,$L3", got)
        self.assertIn("li\t$2,0x00000001", got)
        self.assertIn("addu\t$2,$0,$zero", got)
        self.assertIn("j\t$31", got)
        # The zeroing stays ahead of the epilogue, as fallthrough.
        self.assertLess(got.index("addu\t$2,$0,$zero"), got.index("j\t$31"))
        self.assertLess(got.index("li\t$2,0x00000001"), got.index("addu\t$2,$0,$zero"))

    def test_flag_off_keeps_the_jump(self):
        off = self.process(WINDOW, False)
        self.assertIn("j\t$L4", off)
        self.assertEqual(self.process(WINDOW, False), off)

    def test_leaves_a_jump_over_real_code(self):
        got = self.process(KEPT_JUMP, True)
        self.assertIn("j\t$L9", got)

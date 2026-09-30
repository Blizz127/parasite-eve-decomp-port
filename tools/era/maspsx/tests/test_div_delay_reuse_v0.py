import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


# The cc1 -O2 -G8 window for func_800133E8, before expand-div. The pass runs
# after expand_div, on the same process_lines result the link check uses.
CC1 = """
.set	noreorder
.set	nomacro
beq	$2,$0,$L5
addu	$7,$5,1
.set	macro
.set	reorder
rem	$3,$3,$7
sll	$4,$3,16
.set	noreorder
.set	nomacro
j	$L14
slt	$4,$6,$4
.set	macro
.set	reorder
bgez	$4,$L10
sll	$5,$5,16
.set	macro
.set	reorder
rem	$2,$3,$7
addu	$3,$7,$2
sll	$4,$3,16
slt	$5,$5,$6
.set	noreorder
.set	nomacro
bne	$5,$0,$L10
slt	$4,$4,$6
.set	macro
.set	reorder
beq	$4,$0,$L10
lw	$3,D_8009CE00
""".strip("\n").splitlines()


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".")]


class TestDivDelayReuseV0(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_DIV_DELAY_REUSE_V0": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(
                lines, expand_div=True, div_delay_reuse_v0=on
            ).process_lines()
        return kept(out)

    def test_rewrites_divisor_and_second_remainder(self):
        got = self.process(CC1, True)
        self.assertIn("addu\t$2,$5,1", got)
        self.assertNotIn("addu\t$7,$5,1", got)
        self.assertIn("div\t$zero,$3,$2", got)
        self.assertNotIn("div\t$zero,$3,$7", got)
        self.assertIn("break\t0x7", got)
        self.assertEqual(got.count("sll\t$4,$3,16"), 1)
        self.assertIn("sll\t$4,$2,16", got)
        self.assertIn("slt\t$2,$6,$4", got)
        self.assertNotIn("slt\t$4,$6,$4", got)
        self.assertIn("addu\t$2,$2,$3", got)
        self.assertNotIn("addu\t$3,$7,$2", got)
        self.assertNotIn("mfhi\t$2", got)
        self.assertEqual(got.count("mfhi\t$3"), 2)
        self.assertIn("bne\t$2,$0,$L10", got)
        self.assertIn("beq\t$2,$0,$L10", got)
        self.assertIn("slt\t$2,$4,$6", got)
        self.assertIn("lw\t$3,D_8009CE00", got)

    def test_flag_off_keeps_a3_divisor(self):
        off = self.process(CC1, False)
        self.assertIn("addu\t$7,$5,1", off)
        self.assertIn("div\t$zero,$3,$7", off)
        self.assertIn("mfhi\t$2", off)
        self.assertIn("break\t0x7", off)
        self.assertNotIn("addu\t$2,$5,1", off)
        self.assertNotIn("sll\t$4,$2,16", off)
        self.assertEqual(self.process(CC1, False), off)

    def test_leaves_a_different_branch_alone(self):
        changed = [
            "bltz\t$4,$L10" if x.startswith("bgez") else x for x in CC1
        ]
        got = self.process(changed, True)
        self.assertIn("addu\t$7,$5,1", got)
        self.assertNotIn("addu\t$2,$5,1", got)
        self.assertIn("mfhi\t$2", got)

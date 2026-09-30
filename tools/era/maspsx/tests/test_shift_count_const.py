import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_8007F0C8: cc1 2.8.1 reload_cse put `$3` (known == 1) in a shift count.
LINES = [
    "li\t$3,1\t\t\t# 0x00000001",
    "bne\t$2,$3,$L57",
    "lw\t$2,0($4)",
    "sll\t$3,$2,$3",
    "$L57:",
    "sll\t$3,$2,$3",
    "jr\t$31",
]


def insns(out):
    return [x for x in strip_comments(out) if x and not x.startswith(".")]


class TestShiftCountConst(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_SHIFT_COUNT_CONST": "1" if on else "0"}
        with patch.dict(os.environ, env):
            return insns(MaspsxProcessor(lines).process_lines())

    def test_folds_in_block_only(self):
        got = self.process(LINES, True)
        self.assertIn("sll\t$3,$2,1", got)
        # after the label the count is unknown and stays a register
        self.assertIn("sll\t$3,$2,$3", got)

    def test_redefinition_blocks(self):
        lines = ["li\t$3,1", "addu\t$3,$3,$4", "sll\t$3,$2,$3", "jr\t$31"]
        self.assertIn("sll\t$3,$2,$3", self.process(lines, True))

    def test_out_of_range(self):
        lines = ["li\t$3,40", "sll\t$3,$2,$3", "jr\t$31"]
        self.assertIn("sll\t$3,$2,$3", self.process(lines, True))

    def test_flag_off(self):
        self.assertNotIn("sll\t$3,$2,1", self.process(LINES, False))


class TestShiftCountConstJumpSlot(unittest.TestCase):
    """func_80083944: the count sits in a noreorder `j` delay slot."""

    def test_noreorder_jump_slot_folds(self):
        lines = [
            "li\t$4,3\t\t\t# 0x00000003",
            "lbu\t$2,234($5)",
            ".set\tnoreorder",
            ".set\tnomacro",
            "j\t$L26",
            "sll\t$2,$2,$4",
            ".set\tmacro",
            ".set\treorder",
            "sll\t$3,$3,$4",
            "$L26:",
            "jr\t$31",
        ]
        with patch.dict(os.environ, {"MASPSX_SHIFT_COUNT_CONST": "1"}):
            got = insns(MaspsxProcessor(lines).process_lines())
        self.assertIn("sll\t$2,$2,3", got)
        # dead code after the jump: the block is over
        self.assertIn("sll\t$3,$3,$4", got)


class TestMoveConstLi(unittest.TestCase):
    """func_80083944: `move $8,$2` with $2 == -1 across a branch."""

    LINES = [
        "li\t$2,-1\t\t\t# 0xffffffff",
        "beq\t$4,$2,$L12",
        "move\t$8,$2",
        "$L12:",
        "move\t$9,$2",
        "jr\t$31",
    ]

    def process(self, on):
        env = {"MASPSX_RELOAD_CSE_CONST": "1" if on else "0"}
        with patch.dict(os.environ, env):
            return insns(MaspsxProcessor(self.LINES).process_lines())

    def test_folds(self):
        got = self.process(True)
        self.assertIn("li\t$8,-1", got)
        self.assertIn("addu\t$9,$2,$zero", got)

    def test_flag_off(self):
        self.assertIn("addu\t$8,$2,$zero", self.process(False))


class TestReloadCseConstAdduAndZero(unittest.TestCase):
    """func_8008486C: `move $10,$8` with $8 = 0 from `move $8,$0` in a
    branch slot, and `addu $4,$16,$3` with $3 == 5."""

    def process(self, lines, on=True):
        env = {"MASPSX_RELOAD_CSE_CONST": "1" if on else "0"}
        with patch.dict(os.environ, env):
            return insns(MaspsxProcessor(lines).process_lines())

    def test_zero_copy(self):
        lines = [
            ".set\tnoreorder",
            "beq\t$2,$0,$L1",
            "move\t$8,$0",
            ".set\treorder",
            "li\t$11,1",
            "move\t$10,$8",
            "$L1:",
            "jr\t$31",
        ]
        got = self.process(lines)
        self.assertIn("addu\t$10,$0,$zero", got)

    def test_addu_const(self):
        lines = ["li\t$3,5", "addu\t$4,$16,$3", "jr\t$31"]
        got = self.process(lines)
        self.assertIn("addu\t$4,$16,5", got)
        self.assertIn("addu\t$4,$16,$3", self.process(lines, on=False))

    def test_addu_both_unknown(self):
        lines = ["addu\t$4,$16,$3", "jr\t$31"]
        self.assertIn("addu\t$4,$16,$3", self.process(lines))

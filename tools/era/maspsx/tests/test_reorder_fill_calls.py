import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestReorderFillCalls(unittest.TestCase):
    """Patch 22: MASPSX_REORDER_FILL_CALLS (cc1 -fno-delayed-branch output).

    Retail func_80084B78: `jal func_80083E50` / `li $a1,1` (slot) and
    `jal func_80083E50` / `move $a1,$zero`; the epilogue is
    `lw $ra,16($sp)` / `move $v0,$zero` / `jr $ra` / `addiu $sp,$sp,24`.
    cc1 (-fno-delayed-branch) emits `li $5,1` / `jal` and
    `move $2,$0` / `lw $31` / `addu $sp` / `j $31` in reorder mode.
    """

    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_REORDER_FILL_CALLS": "0"}):
            out = MaspsxProcessor(lines, reorder_fill_calls=on).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_positive_call_slot(self):
        res = self.process(["li\t$5,0x00000001\t\t# 1", "jal\tfunc_80083E50"])
        self.assertEqual(["jal\tfunc_80083E50", "li\t$5,0x00000001"], res[:2])

    def test_positive_move_call_slot(self):
        res = self.process(["move\t$5,$0", "jal\tfoo"])
        self.assertEqual("jal\tfoo", res[0])
        self.assertTrue(res[1].startswith("addu\t$5,$0") or res[1] == "move\t$5,$0")

    def test_positive_epilogue(self):
        res = self.process(
            ["move\t$2,$0", "lw\t$31,16($sp)", "addu\t$sp,$sp,24", "j\t$31"]
        )
        self.assertEqual("lw\t$31,16($sp)", res[0])
        self.assertTrue(res[1].startswith("addu\t$2,$0"))
        self.assertEqual(["j\t$31", "addu\t$sp,$sp,24"], res[2:4])

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_REORDER_FILL_CALLS": "1"}):
            out = MaspsxProcessor(["move\t$5,$0", "jal\tfoo"]).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual("jal\tfoo", res[0])

    def test_negative_flag_off(self):
        lines = ["move\t$5,$0", "jal\tfoo"]
        res = self.process(lines, on=False)
        self.assertEqual("jal\tfoo", res[1])
        self.assertEqual("nop", res[2])

    def test_negative_label_between(self):
        # the call is a branch target: X did not run on that path
        lines = ["move\t$5,$0", "$L9:", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_load(self):
        # HF1: loads are not moved (the callee's first insn could read it)
        lines = ["lw\t$5,0($4)", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_branch_before_call(self):
        lines = ["beq\t$2,$0,$L9", "jal\tfoo", "$L9:"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_ra_and_sp(self):
        # HF2: never touch $ra / $sp / $at
        for x in ["move\t$31,$4", "addu\t$4,$31,$0", "addu\t$sp,$sp,8", "move\t$1,$4"]:
            lines = [x, "jal\tfoo"]
            self.assertEqual(self.process(lines, on=False), self.process(lines), x)

    def test_negative_jalr_target_written(self):
        # HF3: jalr reads its target before the slot runs
        lines = ["move\t$2,$4", "jal\t$2"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_positive_jalr_other_register(self):
        res = self.process(["move\t$5,$0", "jal\t$2"])
        self.assertEqual("jal\t$2", res[0])

    def test_negative_large_li(self):
        # a two-word li is not a single instruction
        lines = ["li\t$5,0x12345678", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_symbol_operand(self):
        lines = ["la\t$5,D_80010000", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_noreorder_region(self):
        lines = [".set\tnoreorder", "move\t$5,$0", "jal\tfoo", "nop",
                 ".set\treorder"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_epilogue_reads_ra(self):
        lines = ["move\t$2,$31", "lw\t$31,16($sp)", "addu\t$sp,$sp,24", "j\t$31"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))


if __name__ == "__main__":
    unittest.main()

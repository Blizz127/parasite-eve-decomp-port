import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestFillJumpStackStore(unittest.TestCase):
    """Patch 11: MASPSX_FILL_JUMP_STACK_STORE.

    cc1 enters a `for` loop over volatile locals with `j <test>` and leaves
    the volatile `i = 0` store in front of it (reorg never moves a volatile
    insn into a delay slot). Retail's ASPSX reorder pass moved the store into
    the jump's slot: func_8007DCAC 0x8007DCB8 and func_800862F4 0x80086410,
    both `j .Ltest` / `sw $zero,0x0($sp)`.

    The negatives are as load-bearing as the positives: every near-miss must
    keep the stock `nop` slot.
    """

    NOP = "nop"

    @staticmethod
    def process(lines, fill=True):
        with patch.dict(os.environ, {"MASPSX_FILL_JUMP_STACK_STORE": "0"}):
            return [
                x
                for x in strip_comments(
                    MaspsxProcessor(lines, fill_jump_stack_store=fill).process_lines()
                )
                if x
            ]

    # -- positives ---------------------------------------------------------

    def test_positive_volatile_loop_entry(self):
        # Exact cc1 2.8.1 shape from func_8007DCAC (volatile markers kept).
        lines = [
            "li\t$2,13",
            "#.set\tvolatile",
            "sw\t$2,4($sp)",
            "#.set\tnovolatile",
            "#.set\tvolatile",
            "sw\t$0,0($sp)",
            "#.set\tnovolatile",
            "j\t$L4",
        ]
        self.assertEqual(
            ["li\t$2,13", "sw\t$2,4($sp)", "j\t$L4", "sw\t$0,0($sp)"],
            self.process(lines),
        )

    def test_positive_register_value_and_widths(self):
        for op in ("sw", "sh", "sb"):
            with self.subTest(op=op):
                self.assertEqual(
                    ["j\t$L9", f"{op}\t$3,-8($29)"],
                    self.process([f"{op}\t$3,-8($29)", "j\t$L9"]),
                )

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_FILL_JUMP_STACK_STORE": "1"}):
            res = [
                x
                for x in strip_comments(
                    MaspsxProcessor(["sw\t$0,0($sp)", "j\t$L22"]).process_lines()
                )
                if x
            ]
        self.assertEqual(["j\t$L22", "sw\t$0,0($sp)"], res)

    # -- negatives ---------------------------------------------------------

    def test_negative_flag_off_keeps_nop(self):
        self.assertEqual(
            ["sw\t$0,0($sp)", "j\t$L22", self.NOP],
            self.process(["sw\t$0,0($sp)", "j\t$L22"], fill=False),
        )

    def test_negative_non_sp_base(self):
        self.assertEqual(
            ["sw\t$0,0($4)", "j\t$L22", self.NOP],
            self.process(["sw\t$0,0($4)", "j\t$L22"]),
        )

    def test_negative_register_and_return_jumps(self):
        for jump in ("j\t$31", "j\t$2", "jal\tfunc_80012345", "jal\t$2"):
            with self.subTest(jump=jump):
                res = self.process(["sw\t$0,0($sp)", jump])
                self.assertEqual("sw\t$0,0($sp)", res[0])

    def test_negative_conditional_branch(self):
        res = self.process(["sw\t$0,0($sp)", "bne\t$2,$0,$L3"])
        self.assertEqual("sw\t$0,0($sp)", res[0])

    def test_negative_label_between(self):
        # The jump is a branch target: the store has not necessarily run.
        self.assertEqual(
            ["sw\t$0,0($sp)", "$L7:", "j\t$L22", self.NOP],
            self.process(["sw\t$0,0($sp)", "$L7:", "j\t$L22"]),
        )

    def test_negative_set_between(self):
        res = self.process(
            ["sw\t$0,0($sp)", ".set\tnoreorder", "j\t$L22", "nop", ".set\treorder"]
        )
        self.assertEqual("sw\t$0,0($sp)", res[0])

    def test_negative_symbolic_and_wide_offsets(self):
        # A symbol store is a macro (patches 1/8 own it); a >16-bit offset
        # expands to an $at macro. Neither is a single-word stack store.
        res = self.process(["sw\t$2,40000($sp)", "j\t$L22"])
        self.assertNotIn("# FILL_JUMP_STACK_STORE START", res)
        self.assertEqual(self.NOP, res[-1])
        res = self.process(["sw\t$2,D_80010000", "j\t$L22"])
        self.assertEqual(self.NOP, res[-1])

    def test_negative_load_is_not_moved(self):
        self.assertEqual(
            ["lw\t$2,0($sp)", "j\t$L22", self.NOP],
            self.process(["lw\t$2,0($sp)", "j\t$L22"]),
        )


if __name__ == "__main__":
    unittest.main()

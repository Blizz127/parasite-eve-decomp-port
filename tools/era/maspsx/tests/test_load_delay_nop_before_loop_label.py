import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestLoadDelayNopBeforeLoopLabel(unittest.TestCase):
    """Patch 21: MASPSX_LOAD_DELAY_NOP_BEFORE_LOOP_LABEL.

    Retail func_8007D1D4 0x8007D37C: `lw $v1,D_8009B3FC` / nop /
    0x8007D384 loop head (`sh $zero,0($v1)` ...) / ... /
    `bnez $v0,0x8007D384` -- the load-delay nop sits BEFORE the loop-head
    label, so the back-edge (0x8007D3A4 `1440fff7`) skips it. Stock maspsx
    puts the nop after the label (inside the loop).
    """

    LOOP = [
        "lw\t$3,0($7)",
        "$L16:",
        "sh\t$0,0($3)",
        "addu\t$4,$4,1",
        "slt\t$2,$4,24",
        ".set\tnoreorder",
        ".set\tnomacro",
        "bne\t$2,$0,$L16",
        "addu\t$3,$3,16",
        ".set\tmacro",
        ".set\treorder",
    ]

    @staticmethod
    def process(lines, on=True):
        with patch.dict(
            os.environ, {"MASPSX_LOAD_DELAY_NOP_BEFORE_LOOP_LABEL": "0"}
        ):
            out = MaspsxProcessor(
                lines, load_delay_nop_before_loop_label=on
            ).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def head(self, res):
        i = res.index("$L16:")
        return res[i - 1 : i + 2]

    def test_positive_nop_before_loop_head(self):
        res = self.process(self.LOOP)
        self.assertEqual(["nop", "$L16:", "sh\t$0,0($3)"], self.head(res))

    def test_positive_environment_flag(self):
        with patch.dict(
            os.environ, {"MASPSX_LOAD_DELAY_NOP_BEFORE_LOOP_LABEL": "1"}
        ):
            out = MaspsxProcessor(self.LOOP).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual("nop", self.head(res)[0])

    def test_negative_flag_off(self):
        res = self.process(self.LOOP, on=False)
        self.assertEqual(["lw\t$3,0($7)", "$L16:", "nop"], self.head(res))

    def test_negative_forward_entry(self):
        # HL21-2: a branch BEFORE the label also reaches it -> not a pure
        # loop head; that entry would arrive with no delay covering.
        lines = ["beq\t$5,$0,$L16"] + self.LOOP
        res = self.process(lines)
        self.assertEqual("nop", self.head(res)[2])

    def test_negative_not_a_branch_target(self):
        # an unreferenced label is not a loop head
        lines = [x.replace("$L16", "$L99") if x.startswith("bne") else x
                 for x in self.LOOP]
        res = self.process(lines)
        self.assertEqual("nop", self.head(res)[2])

    def test_negative_load_in_back_edge_slot(self):
        # HL21-3: a noreorder back-edge whose slot is a load would reach the
        # first use with no delay once the nop moves before the label.
        lines = [x if x != "addu\t$3,$3,16" else "lw\t$3,4($3)"
                 for x in self.LOOP]
        res = self.process(lines)
        self.assertEqual("nop", self.head(res)[2])

    def test_negative_label_not_next(self):
        # HL21-1: another instruction between the load and the label keeps
        # the stock path (and here no nop is needed at all).
        lines = [self.LOOP[0], "addu\t$9,$9,1"] + self.LOOP[1:]
        self.assertEqual(self.process(lines, on=False), self.process(lines))


if __name__ == "__main__":
    unittest.main()

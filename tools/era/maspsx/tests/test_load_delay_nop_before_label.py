import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestLoadDelayNopBeforeLabel(unittest.TestCase):
    """Patch 14: MASPSX_LOAD_DELAY_NOP_BEFORE_LABEL.

    Positives (retail has the nop BEFORE the label): func_80083578 /
    func_8007BF44 (`lw $3,SYM` / nop / $L2: volatile poll load) and
    func_80081414 (`lb $2` / nop / $L8: cc1 noreorder `beq`).
    Regression set (retail keeps label-then-nop, label followed by a plain
    instruction): func_800292EC $L5, func_8006F224 $L14/$L7, func_8006F39C
    $L24/$L31.
    """

    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_LOAD_DELAY_NOP_BEFORE_LABEL": "0"}):
            out = MaspsxProcessor(
                lines, load_delay_nop_before_label=on
            ).process_lines()
        return [x for x in strip_comments(out) if x]

    # func_80083578, cc1 output (2.7.2 and 2.8.1 identical here)
    VOLATILE_POLL = [
        "lw\t$3,0($4)",
        "$L2:",
        "#.set\tvolatile",
        "lhu\t$2,4($3)",
        "#.set\tnovolatile",
        "#nop",
        "andi\t$2,$2,0x0002",
        "beq\t$2,$0,$L2",
    ]

    # func_80081414 $L8 shape
    NOREORDER_JOIN = [
        "lb\t$2,0($16)",
        "$L8:",
        ".set\tnoreorder",
        ".set\tnomacro",
        "beq\t$2,$0,$L33",
        "slt\t$2,$18,8",
        ".set\tmacro",
        ".set\treorder",
        "$L33:",
        "j\t$31",
    ]

    # func_8006F224 $L14 shape (regression: keep label-then-nop)
    PLAIN_LOOP = [
        "lw\t$4,0($5)",
        "$L14:",
        "lbu\t$2,0($4)",
        "#nop",
        "beq\t$2,$0,$L19",
        "addu\t$3,$3,1",
        "$L19:",
    ]

    def test_positive_volatile_poll_label(self):
        res = self.process(self.VOLATILE_POLL)
        self.assertEqual(["lw\t$3,0($4)", "nop", "$L2:", "lhu\t$2,4($3)"], res[:4])

    def test_positive_noreorder_join_label(self):
        res = self.process(self.NOREORDER_JOIN)
        self.assertEqual(["lb\t$2,0($16)", "nop", "$L8:"], res[:3])

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_LOAD_DELAY_NOP_BEFORE_LABEL": "1"}):
            out = MaspsxProcessor(self.VOLATILE_POLL).process_lines()
        res = [x for x in strip_comments(out) if x]
        self.assertEqual(["lw\t$3,0($4)", "nop", "$L2:"], res[:3])

    def test_negative_flag_off_keeps_label_then_nop(self):
        res = self.process(self.VOLATILE_POLL, on=False)
        self.assertEqual(["lw\t$3,0($4)", "$L2:", "nop", "lhu\t$2,4($3)"], res[:4])

    def test_negative_plain_instruction_after_label(self):
        # HN2: the regression shape is untouched with the knob on.
        self.assertEqual(
            self.process(self.PLAIN_LOOP, on=False), self.process(self.PLAIN_LOOP)
        )
        self.assertEqual(["lw\t$4,0($5)", "$L14:", "nop"], self.process(self.PLAIN_LOOP)[:3])

    def test_negative_reorder_directive_after_label(self):
        lines = ["lw\t$3,0($4)", "$L2:", ".set\treorder", "lhu\t$2,4($3)"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_branch_in_with_load_in_noreorder_slot(self):
        # HN3: a cc1-filled delay slot that loads would lose its load delay
        # once the nop moves before the label.
        lines = self.VOLATILE_POLL[:-1] + [
            ".set\tnoreorder",
            ".set\tnomacro",
            "beq\t$2,$0,$L2",
            "lw\t$3,0($5)",
            ".set\tmacro",
            ".set\treorder",
        ]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_positive_branch_in_with_alu_noreorder_slot(self):
        lines = self.VOLATILE_POLL[:-1] + [
            ".set\tnoreorder",
            ".set\tnomacro",
            "beq\t$2,$0,$L2",
            "addu\t$6,$6,1",
            ".set\tmacro",
            ".set\treorder",
        ]
        self.assertEqual(["lw\t$3,0($4)", "nop", "$L2:"], self.process(lines)[:3])

    def test_negative_label_not_adjacent_to_load(self):
        # HN1: another instruction between the load and the label.
        lines = ["lw\t$3,0($4)", "addu\t$5,$5,1", "$L2:", "#.set\tvolatile", "lhu\t$2,0($3)"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))


if __name__ == "__main__":
    unittest.main()

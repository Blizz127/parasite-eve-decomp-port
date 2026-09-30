import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


def call(slot, jump="jal\tfunc_80072724"):
    return [
        ".set\tnoreorder",
        ".set\tnomacro",
        jump,
        slot,
        ".set\tmacro",
        ".set\treorder",
    ]


class TestLoadDelayCallSlotStore(unittest.TestCase):
    """Patch 13: MASPSX_LOAD_DELAY_CALL_SLOT_STORE.

    Retail keeps a load-delay nop before a jal whose slot store reads the
    loaded register: func_800742B8 0x80074304 (`lw $v0` / nop / jal /
    `sw $v0,0($v1)`) and func_8007A2A4 0x8007A304 (`lw $v0` / nop / jal /
    `sb $zero,0($v0)`). An ALU slot keeps no nop (func_80089328 0x800895F8).
    """

    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_LOAD_DELAY_CALL_SLOT_STORE": "0"}):
            out = MaspsxProcessor(
                lines, load_delay_call_slot_store=on
            ).process_lines()
        return [x for x in strip_comments(out) if x]

    def nop_before_jal(self, res):
        return res[res.index("jal\tfunc_80072724") - 1] == "nop"

    def test_positive_store_value_register(self):
        res = self.process(["lw\t$2,52($16)"] + call("sw\t$2,0($3)"))
        self.assertEqual(
            ["lw\t$2,52($16)", "nop", "jal\tfunc_80072724", "sw\t$2,0($3)"], res
        )

    def test_positive_store_base_register(self):
        res = self.process(["lw\t$2,0($4)"] + call("sb\t$0,0($2)"))
        self.assertTrue(self.nop_before_jal(res))

    def test_positive_named_register_spelling(self):
        res = self.process(["lw\t$v0,0($a0)"] + call("sh\t$2,4($a1)"))
        self.assertTrue(self.nop_before_jal(res))

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_LOAD_DELAY_CALL_SLOT_STORE": "1"}):
            out = MaspsxProcessor(["lw\t$2,52($16)"] + call("sw\t$2,0($3)")).process_lines()
        res = [x for x in strip_comments(out) if x]
        self.assertTrue(self.nop_before_jal(res))

    def test_negative_flag_off(self):
        res = self.process(["lw\t$2,52($16)"] + call("sw\t$2,0($3)"), on=False)
        self.assertFalse(self.nop_before_jal(res))

    def test_negative_alu_slot(self):
        # func_80089328: `lh $a0` / jal / `addu $a1,$a0,$zero`, no nop.
        res = self.process(["lh\t$4,66($2)"] + call("addu\t$5,$4,$0"))
        self.assertFalse(self.nop_before_jal(res))

    def test_negative_store_not_reading_load(self):
        res = self.process(["lw\t$2,52($16)"] + call("sw\t$3,0($4)"))
        self.assertFalse(self.nop_before_jal(res))

    def test_negative_reorder_mode_jal(self):
        # No cc1 noreorder block: the store after the jal is not its slot.
        res = self.process(["lw\t$2,52($16)", "jal\tfunc_80072724", "sw\t$2,0($3)"])
        self.assertEqual("lw\t$2,52($16)", res[0])
        self.assertEqual("jal\tfunc_80072724", res[1])

    def test_negative_register_call_and_branch(self):
        res = self.process(["lw\t$2,52($16)"] + call("sw\t$2,0($3)", jump="jal\t$25"))
        self.assertNotEqual("nop", res[1])
        res = self.process(
            ["lw\t$2,52($16)"] + call("sw\t$2,0($3)", jump="bne\t$4,$0,$L3")
        )
        self.assertNotEqual("nop", res[1])

    def test_negative_label_between(self):
        res = self.process(["lw\t$2,52($16)", "$L4:"] + call("sw\t$2,0($3)"))
        self.assertFalse(self.nop_before_jal(res))


if __name__ == "__main__":
    unittest.main()

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


def _block(branch, slot):
    return [".set\tnoreorder", ".set\tnomacro", branch, slot, ".set\tmacro", ".set\treorder"]


class TestDropTargetDupLi(unittest.TestCase):
    """LOCAL PATCH 35 (MASPSX_DROP_TARGET_DUP_LI)."""

    @staticmethod
    def process(lines, enabled):
        with patch.dict(os.environ, {"MASPSX_DROP_TARGET_DUP_LI": "0"}):
            return MaspsxProcessor(list(lines), drop_target_dup_li=enabled).process_lines()

    @staticmethod
    def count_li255(out):
        return sum(1 for x in out if x.split("#")[0].strip() in ("li\t$2,255", "addiu\t$2,$0,255", "ori\t$2,$0,255"))

    def base(self):
        # shape of cc1 2.8.1 output for func_80084C4C's switch dispatch
        return (
            _block("beq\t$3,$2,$L29", "li\t$2,255\t\t\t# 0x000000ff")
            + ["", "beq\t$3,$2,$L1", "j\t$L24"]
            + _block("j\t$L1", "sb\t$2,70($16)")
            + ["", "$L29:", "li\t$2,255\t\t\t# 0x000000ff"]
            + _block("j\t$L1", "sb\t$2,70($16)")
            + ["", "$L24:", "nop", "$L1:", "j\t$31"]
        )

    def test_target_copy_dropped(self):
        off = self.process(self.base(), enabled=False)
        on = self.process(self.base(), enabled=True)
        self.assertEqual(self.count_li255(off) - self.count_li255(on), 1)

    def test_flag_off_identical_to_stock(self):
        with patch.dict(os.environ, {"MASPSX_DROP_TARGET_DUP_LI": "0"}):
            stock = MaspsxProcessor(self.base()).process_lines()
        self.assertEqual(stock, self.process(self.base(), enabled=False))

    def test_fallthrough_into_label_refused(self):
        lines = self.base()
        i = lines.index("$L29:")
        lines.insert(i, "addu\t$4,$4,1")  # H2: falls through
        self.assertEqual(self.process(lines, True), self.process(lines, False))

    def test_second_reference_refused(self):
        lines = self.base() + ["beq\t$5,$0,$L29"]  # H1
        self.assertEqual(self.process(lines, True), self.process(lines, False))

    def test_different_slot_value_refused(self):
        lines = self.base()
        k = lines.index("li\t$2,255\t\t\t# 0x000000ff")
        lines[k] = "li\t$2,254"
        self.assertEqual(self.process(lines, True), self.process(lines, False))

    def test_reorder_mode_branch_refused(self):
        lines = ["beq\t$3,$2,$L29", "li\t$2,255", "j\t$L24", "$L29:", "li\t$2,255", "j\t$L1", "$L24:", "nop", "$L1:", "j\t$31"]
        self.assertEqual(self.process(lines, True), self.process(lines, False))


if __name__ == "__main__":
    unittest.main()

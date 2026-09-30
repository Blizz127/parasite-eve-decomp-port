import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestDispatchFoldLa(unittest.TestCase):
    """MASPSX_DISPATCH_FOLD, `la` form.

    func_80051CC4: cc1 hoists the switch table's address out of the loop as
    `la $14,$L20` and indexes it with `addu $2,$2,$14` / `lw $2,0($2)`, so
    the indexed-load fold never fires. Retail 0x80051D2C/30:
    `lui $t6,0x8001` / `addiu $t6,$t6,0x11F8` = &jtbl_800111F8.
    """

    SOURCE = [
        "func:",
        "la\t$14,$L20",
        "addu\t$2,$2,$14",
        "lw\t$2,0($2)",
        "j\t$2",
        ".rdata",
        ".align\t3",
        "$L20:",
        ".word\t$L15",
        ".text",
        "$L15:",
        "la\t$4,$L99",
    ]

    @staticmethod
    def process(lines, fold):
        env = {"MASPSX_DISPATCH_FOLD": ""}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines, dispatch_fold_symbol=fold).process_lines()
        return [x for x in strip_comments(out) if x]

    def test_positive_la_of_rdata_table_folded(self):
        res = self.process(self.SOURCE, "jtbl_800111F8")
        self.assertIn("la\t$14,jtbl_800111F8", res)
        self.assertNotIn("la\t$14,$L20", res)

    def test_positive_multi_table_mapping(self):
        lines = [
            "la\t$14,$L20",
            "la\t$15,$L30",
            ".rdata",
            "$L20:",
            ".word\t$L1",
            ".text",
            ".rdata",
            "$L30:",
            ".word\t$L2",
            ".text",
        ]
        res = self.process(lines, "jtbl_A,jtbl_B")
        self.assertIn("la\t$14,jtbl_A", res)
        self.assertIn("la\t$15,jtbl_B", res)

    def test_negative_non_table_label_untouched(self):
        # HD1: $L99 is not defined in .rdata (not a switch table).
        res = self.process(self.SOURCE, "jtbl_800111F8")
        self.assertIn("la\t$4,$L99", res)

    def test_negative_fold_off(self):
        res = self.process(self.SOURCE, None)
        self.assertIn("la\t$14,$L20", res)

    def test_negative_offset_or_compound_untouched(self):
        # HD2: only a plain two-operand `la $r,$L<n>`.
        lines = ["la\t$14,$L20+4", ".rdata", "$L20:", ".word\t$L1", ".text"]
        res = self.process(lines, "jtbl_800111F8")
        self.assertIn("la\t$14,$L20+4", res)


if __name__ == "__main__":
    unittest.main()

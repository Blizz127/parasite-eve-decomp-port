import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestDivNoReuseNop(unittest.TestCase):
    """Patch 16: MASPSX_DIV_NO_REUSE_NOP.

    Retail func_80077E64 0x80077F20 (2.8.1 band): the expanded div ends
    `mflo $a0` / `slti $v0,$a0,-0x8000` with no nop; stock maspsx emits a
    "Reuse of $4" nop between them. The mult/div-after-mfxx hazard nop must
    survive.
    """

    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_DIV_NO_REUSE_NOP": "0"}):
            out = MaspsxProcessor(
                lines, expand_div=True, div_no_reuse_nop=on
            ).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def tail(self, res):
        i = max(k for k, x in enumerate(res) if x.startswith(("mflo", "mfhi")))
        return res[i:]

    def test_positive_div_result_read_next(self):
        res = self.process(["div\t$4,$2,$6", "slt\t$2,$4,-32768"])
        self.assertEqual(["mflo\t$4", "slt\t$2,$4,-32768"], self.tail(res))

    def test_positive_remu_result_read_next(self):
        res = self.process(["remu\t$2,$5,$4", "beq\t$2,$0,$L3"])
        self.assertEqual("mfhi\t$2", self.tail(res)[0])
        self.assertEqual("beq\t$2,$0,$L3", self.tail(res)[1])

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_DIV_NO_REUSE_NOP": "1"}):
            out = MaspsxProcessor(
                ["div\t$4,$2,$6", "slt\t$2,$4,-32768"], expand_div=True
            ).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual(["mflo\t$4", "slt\t$2,$4,-32768"], self.tail(res))

    def test_negative_flag_off_keeps_reuse_nop(self):
        res = self.process(["div\t$4,$2,$6", "slt\t$2,$4,-32768"], on=False)
        self.assertEqual(["mflo\t$4", "nop", "slt\t$2,$4,-32768"], self.tail(res))

    def test_negative_mult_hazard_nop_kept(self):
        # mflo followed by a div within 2 instructions keeps its hazard nops.
        lines = ["div\t$4,$2,$6", "div\t$3,$4,$7"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_plain_mult_mflo_untouched(self):
        # cc1's own mult/mflo lines never go through the div expansion.
        lines = ["mult\t$2,$5", "mflo\t$2", "addu\t$3,$2,$4"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))


if __name__ == "__main__":
    unittest.main()

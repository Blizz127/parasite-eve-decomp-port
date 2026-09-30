import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestRodataFold(unittest.TestCase):
    """Patch 20: MASPSX_RODATA_FOLD=s0,s1,...

    func_800323C8 copies three initialized 18-byte local arrays from retail's
    shared .rodata pool (0x800323D0 `lui $a2,0x8001` / `addiu $a2,0xDFC` =
    D_80010DFC, then D_80010E10, D_80010E24). cc1 emits its own `$LC0..2`
    copies addressed by `la $6,$LC<n>`; the fold retargets them to the pool
    symbols (the build strips the byte-verified duplicate .rodata).
    """

    SOURCE = [
        ".rdata",
        ".align\t2",
        "$LC0:",
        ".byte\t0",
        ".align\t2",
        "$LC1:",
        ".byte\t1",
        ".text",
        "func:",
        "la\t$6,$LC0",
        "lwl\t$2,3($6)",
        "la\t$6,$LC1",
        "lw\t$3,$LC1+4",
        "la\t$7,$LC10",
    ]

    @staticmethod
    def process(lines, fold):
        with patch.dict(os.environ, {"MASPSX_RODATA_FOLD": ""}):
            out = MaspsxProcessor(lines, rodata_fold_symbols=fold).process_lines()
        return [x for x in strip_comments(out) if x]

    def test_positive_la_and_load_retargeted(self):
        src = self.SOURCE[:-1]
        res = self.process(src, "D_80010DFC,D_80010E10")
        self.assertIn("la\t$6,D_80010DFC", res)
        self.assertIn("la\t$6,D_80010E10", res)
        self.assertTrue(any("D_80010E10+4" in x for x in res))
        self.assertFalse(any("$LC0" in x and not x.endswith(":") for x in res))

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_RODATA_FOLD": "A,B"}):
            out = MaspsxProcessor(self.SOURCE[:-1]).process_lines()
        self.assertIn("la\t$6,A", [x for x in strip_comments(out) if x])

    def test_negative_definitions_untouched(self):
        # HR2: the .rdata labels themselves stay (the build strips them).
        res = self.process(self.SOURCE[:-1], "A,B")
        self.assertIn("$LC0:", res)
        self.assertIn("$LC1:", res)

    def test_negative_count_mismatch_raises(self):
        # HR1: a wrong list length must never silently mis-map.
        with self.assertRaises(ValueError):
            self.process(self.SOURCE[:-1], "A")

    def test_negative_token_boundary(self):
        # `$LC1` must not match inside `$LC10` (an undefined label here, so
        # left alone).
        lines = self.SOURCE[:-2] + ["la\t$7,$LC10"]
        res = self.process(lines, "A,B")
        self.assertIn("la\t$7,$LC10", res)

    def test_negative_fold_off(self):
        res = self.process(self.SOURCE[:-1], None)
        self.assertIn("la\t$6,$LC0", res)


if __name__ == "__main__":
    unittest.main()

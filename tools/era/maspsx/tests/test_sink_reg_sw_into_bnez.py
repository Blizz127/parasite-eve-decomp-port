import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


BUMP = [
    "addu\t$2,$3,1",
    "sw\t$2,D_8009B53C",
    "bne\t$2,$0,$L6",
    "addu\t$2,$3,2",
    "sw\t$2,D_8009B53C",
    "$L6:",
    "j\t$31",
]


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".")]


class TestSinkRegSwIntoBnez(unittest.TestCase):
    @staticmethod
    def process(lines, on=True):
        env = {"MASPSX_SINK_REG_SW_INTO_BNEZ": "0"}
        with patch.dict(os.environ, env):
            return kept(MaspsxProcessor(
                lines, sink_reg_sw_into_bnez=on).process_lines())

    def test_store_moves_into_delay(self):
        got = self.process(BUMP)
        self.assertEqual(got[0], "addu\t$2,$3,1")
        self.assertIn("lui\t$at,%hi(D_8009B53C)", got)
        self.assertIn("bne\t$2,$0,$L6", got)
        self.assertIn("sw\t$2,%lo(D_8009B53C)($at)", got)
        bne_at = got.index("bne\t$2,$0,$L6")
        self.assertEqual(got[bne_at + 1], "sw\t$2,%lo(D_8009B53C)($at)")
        self.assertLess(got.index("lui\t$at,%hi(D_8009B53C)"), bne_at)
        self.assertIn("addu\t$2,$3,2", got)

    def test_off_leaves_store_first(self):
        got = self.process(BUMP, on=False)
        self.assertEqual(got[1], "sw\t$2,D_8009B53C")
        self.assertEqual(got[2], "bne\t$2,$0,$L6")

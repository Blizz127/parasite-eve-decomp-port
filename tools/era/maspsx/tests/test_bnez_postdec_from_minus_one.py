import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


LOOP = [
    "li\t$8,-2147021300",
    "li\t$11,0x00000001",
    "sw\t$11,64($8)",
    "li\t$11,0x00000002",
    "sw\t$11,60($8)",
    "li\t$13,0x0000000e",
    "li\t$2,-1",
    "$L2:",
    "lw\t$11,64($8)",
    "lw\t$12,60($8)",
    "addu\t$8,$8,-4",
    "addu\t$11,$11,$12",
    "addu\t$13,$13,-1",
    ".set\tnoreorder",
    ".set\tnomacro",
    "bne\t$13,$2,$L2",
    "sw\t$11,60($8)",
    ".set\tmacro",
    ".set\treorder",
    "li\t$11,0x00000040",
    "j\t$31",
]

LIVE = [
    "li\t$2,-1",
    "addu\t$13,$13,-1",
    ".set\tnoreorder",
    ".set\tnomacro",
    "bne\t$13,$2,$L2",
    "sw\t$11,60($8)",
    ".set\tmacro",
    ".set\treorder",
    "addu\t$3,$2,$zero",
]


def kept(lines):
    return [x for x in strip_comments(lines) if x and not x.startswith(".")]


class TestBnezPostdecFromMinusOne(unittest.TestCase):
    @staticmethod
    def process(lines, on=True):
        env = {
            "MASPSX_BNEZ_POSTDEC_FROM_MINUS_ONE": "0",
            "MASPSX_ORI_SMALL_LI": "0",
        }
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(
                lines, bnez_postdec_from_minus_one=on, ori_small_li=on
            ).process_lines()
        return kept(out)

    def test_rewrites_loop_and_small_li(self):
        got = self.process(LOOP)
        self.assertNotIn("li\t$2,-1", got)
        self.assertIn("ori\t$11,$zero,1", got)
        self.assertIn("ori\t$11,$zero,2", got)
        self.assertIn("ori\t$13,$zero,14", got)
        self.assertIn("ori\t$11,$zero,64", got)
        self.assertIn("li\t$8,-2147021300", got)
        sw_at = got.index("sw\t$11,60($8)")
        bne_at = got.index("bne\t$13,$zero,$L2")
        dec_at = got.index("addu\t$13,$13,-1")
        self.assertLess(sw_at, bne_at)
        self.assertEqual(dec_at, bne_at + 1)
        self.assertEqual(got[got.index("addu\t$11,$11,$12") + 1], "sw\t$11,60($8)")

    def test_leaves_minus_one_when_register_stays_live(self):
        got = self.process(LIVE)
        self.assertIn("li\t$2,-1", got)
        self.assertIn("bne\t$13,$2,$L2", got)

    def test_flag_off(self):
        got = self.process(LOOP, on=False)
        self.assertIn("li\t$2,-1", got)
        self.assertIn("bne\t$13,$2,$L2", got)
        self.assertIn("li\t$11,0x00000001", got)

    def test_environment_flag(self):
        with patch.dict(
            os.environ,
            {
                "MASPSX_BNEZ_POSTDEC_FROM_MINUS_ONE": "1",
                "MASPSX_ORI_SMALL_LI": "1",
            },
        ):
            got = kept(MaspsxProcessor(LOOP).process_lines())
        self.assertEqual(self.process(LOOP), got)

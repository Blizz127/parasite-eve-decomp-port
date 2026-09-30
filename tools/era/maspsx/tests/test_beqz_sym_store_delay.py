import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


BODY = [
    "sw\t$2,D_800A347C",
    "beq\t$3,$0,$L8",
    "$L6:",
    "jal\tfunc_80073C5C",
    "$L8:",
]


def kept(lines):
    return [
        x
        for x in strip_comments(lines)
        if x and not x.startswith(".") and not x.endswith(":")
    ]


class TestBeqzSymStoreDelay(unittest.TestCase):
    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_BEQZ_SYM_STORE_DELAY": "0"}):
            out = MaspsxProcessor(lines, beqz_sym_store_delay=on).process_lines()
        return kept(out)

    def test_sinks_sw_into_empty_beq_delay(self):
        got = self.process(BODY)
        self.assertEqual(
            [
                "lui\t$at,%hi(D_800A347C)",
                "beq\t$3,$0,$L8",
                "sw\t$2,%lo(D_800A347C)($at)",
                "jal\tfunc_80073C5C",
                "nop",
            ],
            got,
        )

    def test_flag_off(self):
        got = self.process(BODY, on=False)
        self.assertLess(got.index("sw\t$2,D_800A347C"), got.index("beq\t$3,$0,$L8"))

    def test_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_BEQZ_SYM_STORE_DELAY": "1"}):
            got = kept(MaspsxProcessor(BODY).process_lines())
        self.assertEqual(self.process(BODY), got)

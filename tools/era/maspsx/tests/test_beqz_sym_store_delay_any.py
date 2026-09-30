import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_8007C564 shape: the instruction after the beq is NOT a label.
BODY = [
    "sw\t$4,D_800B0CD0",
    "beq\t$2,$0,$L4",
    "lw\t$2,D_800BCD7C",
    "$L4:",
    "jal\tfunc_80073C5C",
]


def kept(lines):
    return [
        x
        for x in strip_comments(lines)
        if x and not x.startswith(".") and not x.endswith(":")
    ]


def run(env):
    with patch.dict(os.environ, env):
        return kept(MaspsxProcessor(BODY, beqz_sym_store_delay=True).process_lines())


class TestBeqzSymStoreDelayAny(unittest.TestCase):
    def test_without_any_keeps_store_before_branch(self):
        got = run({"MASPSX_BEQZ_SYM_STORE_DELAY_ANY": "0"})
        self.assertLess(got.index("sw\t$4,D_800B0CD0"), got.index("beq\t$2,$0,$L4"))

    def test_any_sinks_store_into_delay_slot(self):
        got = run({"MASPSX_BEQZ_SYM_STORE_DELAY_ANY": "1"})
        i = got.index("beq\t$2,$0,$L4")
        self.assertEqual("lui\t$at,%hi(D_800B0CD0)", got[i - 1])
        self.assertEqual("sw\t$4,%lo(D_800B0CD0)($at)", got[i + 1])

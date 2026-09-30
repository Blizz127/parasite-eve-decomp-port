import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_80072A74 (__adddf3): `bne $2,...` steals the target's `sra $2,...`
# (writes the tested register) and the copy at the target is deleted because
# the fall-through already ran the slot.
MOVE = [
    "and\t$2,$17,$2",
    "bne\t$2,$0,$L3",
    "beq\t$16,$0,$L26",
    "$L3:",
    "sra\t$2,$19,20",
    "andi\t$20,$2,0x07ff",
    "jr\t$31",
    "$L26:",
    "move\t$2,$18",
    "jr\t$31",
]

# func_80072F64 (__divdf3): an untested destination (mode "any"); the
# target label has another referrer, so the insn is copied and the branch
# goes to a new label after it.
COPY = [
    "bne\t$3,$0,$L2",
    "sw\t$16,72($sp)",
    "bne\t$10,$0,$L2",
    ".set\tnoreorder",
    "bne\t$20,$0,$L4",
    "li\t$19,-1",
    ".set\treorder",
    "$L4:",
    "move\t$2,$18",
    "jr\t$31",
    "$L2:",
    "li\t$2,0x7fff0000",
    "ori\t$2,$2,0xffff",
    "jr\t$31",
]


def insns(out):
    return [
        x for x in strip_comments(out)
        if x and not x.startswith(".") and not x.endswith(":")
    ]


class TestBranchStealTargetInsn(unittest.TestCase):
    @staticmethod
    def process(lines, mode):
        with patch.dict(os.environ, {"MASPSX_BRANCH_STEAL_TARGET_INSN": mode}):
            return insns(MaspsxProcessor(lines).process_lines())

    def test_tested_register_moves(self):
        got = self.process(MOVE, "1")
        self.assertEqual(["bne\t$2,$0,$L3", "sra\t$2,$19,20"], got[1:3])
        self.assertEqual(1, got.count("sra\t$2,$19,20"))

    def test_flag_off(self):
        got = self.process(MOVE, "0")
        self.assertEqual(["bne\t$2,$0,$L3", "nop"], got[1:3])

    def test_any_mode_copies_lui(self):
        got = self.process(COPY, "any")
        i = got.index("bne\t$10,$0,$L910001")
        # one word in the noreorder slot (GNU as emits a lone lui)
        self.assertEqual("li\t$2,0x7fff0000", got[i + 1])
        # the target keeps its own copy for the other referrer
        self.assertEqual(2, sum(1 for x in got if x.startswith(("li\t$2,0x7fff", "lui\t$2,0x7fff"))))

    def test_mode_1_ignores_untested(self):
        got = self.process(COPY, "1")
        self.assertIn("bne\t$10,$0,$L2", got)

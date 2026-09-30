import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestFillBranchSplitLi(unittest.TestCase):
    """Patch 19: MASPSX_FILL_BRANCH_SPLIT_LI.

    Retail func_80038D74 0x80038E28: `bnez $v0,.L80038E70` /
    `lui $s0,0x8001` (slot) / `ori $s0,$s0,0x3` -- the high word of the
    split constant 0x80010003 sits in the branch's delay slot; $s0 is dead
    at the target (next written before any read). cc1 emits
    `bne` / `li $16,0x80010000` / `ori $16,$16,0x0003` with an empty slot.
    """

    @staticmethod
    def body(after_target, before=()):
        return [
            "func:",
            *before,
            "bne\t$2,$0,$L26",
            "li\t$16,-2147418112",
            "ori\t$16,$16,0x0003",
            "$L27:",
            "jal\tfoo",
            "mult\t$2,$16",
            "mflo\t$3",
            "$L26:",
            *after_target,
            "j\t$31",
            ".end\tfunc",
        ]

    DEAD = [
        "lw\t$4,0($5)",
        "jal\tbar",
        "mult\t$4,$2",
        "mfhi\t$8",
        "beq\t$8,$0,$L40",
        "addu\t$3,$3,1",
        "$L40:",
        "li\t$16,0x00000068",
        "addu\t$2,$16,$0",
    ]

    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_FILL_BRANCH_SPLIT_LI": "0"}):
            out = MaspsxProcessor(lines, fill_branch_split_li=on).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def slot(self, res):
        i = next(k for k, x in enumerate(res) if x.startswith("bne"))
        return res[i + 1 : i + 3]

    def test_positive_lui_into_slot(self):
        res = self.process(self.body(self.DEAD))
        self.assertEqual(["lui\t$16,0x8001", "ori\t$16,$16,0x0003"], self.slot(res))

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_FILL_BRANCH_SPLIT_LI": "1"}):
            out = MaspsxProcessor(self.body(self.DEAD)).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual("lui\t$16,0x8001", self.slot(res)[0])

    def test_negative_flag_off(self):
        res = self.process(self.body(self.DEAD), on=False)
        self.assertEqual("nop", self.slot(res)[0])

    def test_negative_live_at_target(self):
        # HS4: $16 read at the target before any write.
        res = self.process(self.body(["addu\t$2,$16,$0", "li\t$16,5"]))
        self.assertEqual("nop", self.slot(res)[0])

    def test_negative_live_on_taken_path_of_inner_branch(self):
        # HS5: a reorder branch's target is walked too.
        after = ["beq\t$2,$0,$L41", "li\t$16,1", "$L41:", "addu\t$2,$16,$0"]
        res = self.process(self.body(after))
        self.assertEqual("nop", self.slot(res)[0])

    def test_negative_caller_saved_across_call(self):
        # HS6: only callee-saved registers survive a call in the walk.
        lines = [
            "func:",
            "bne\t$2,$0,$L26",
            "li\t$9,-2147418112",
            "ori\t$9,$9,0x0003",
            "$L26:",
            "jal\tbar",
            "li\t$9,1",
            "j\t$31",
            ".end\tfunc",
        ]
        res = self.process(lines)
        self.assertEqual("nop", self.slot(res)[0])

    def test_negative_jalr_through_reg(self):
        res = self.process(self.body(["jal\t$16", "li\t$16,1"]))
        self.assertEqual("nop", self.slot(res)[0])

    def test_negative_low_half_nonzero(self):
        # HS1: not a pure high word.
        lines = self.body(self.DEAD)
        lines[lines.index("li\t$16,-2147418112")] = "li\t$16,-2147418109"
        res = self.process(lines)
        self.assertEqual("nop", self.slot(res)[0])

    def test_negative_no_completing_ori(self):
        # HS2: the li must be the first half of a split constant.
        lines = self.body(self.DEAD)
        lines[lines.index("ori\t$16,$16,0x0003")] = "addu\t$16,$16,3"
        res = self.process(lines)
        self.assertEqual("nop", self.slot(res)[0])

    def test_negative_ori_other_register(self):
        lines = self.body(self.DEAD)
        lines[lines.index("ori\t$16,$16,0x0003")] = "ori\t$17,$16,0x0003"
        res = self.process(lines)
        self.assertEqual("nop", self.slot(res)[0])

    def test_patch6_unchanged_without_knob(self):
        # Patch 6 alone never fills a non-compare, so the li stays out.
        with patch.dict(os.environ, {"MASPSX_FILL_BRANCH_SPLIT_LI": "0"}):
            out = MaspsxProcessor(
                self.body(self.DEAD), fill_branch_delay_slot=True
            ).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual("nop", self.slot(res)[0])


if __name__ == "__main__":
    unittest.main()

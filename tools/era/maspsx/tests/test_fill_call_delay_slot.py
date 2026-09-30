"""LOCAL PATCH 8 (fill_call_delay_slot) — durable tests.

The knob schedules the SECOND word of an absolute store macro
(`sw/sh/sb $r,SYM`) or of an address macro (`la $r,SYM`) into the delay slot
of a following plain `jal <symbol>` / `j <label>` / `jalr` / `jr`:

    lui $at,%hi(SYM) / jal T / sw $r,%lo(SYM)($at)
    lui $r,%hi(SYM)  / jal T / addiu $r,$r,%lo(SYM)

Every hazard guard in the patch has a negative test here. Flag-off must be
byte-identical, which each negative test asserts directly by comparing the
disabled and enabled outputs.
"""

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestFillCallDelaySlotGuards(unittest.TestCase):
    @staticmethod
    def process(lines, enabled, **kwargs):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variable set.
        with patch.dict(os.environ, {"MASPSX_FILL_CALL_DELAY_SLOT": "0"}):
            return MaspsxProcessor(
                lines, fill_call_delay_slot=enabled, **kwargs
            ).process_lines()

    def assert_unchanged(self, lines, **kwargs):
        self.assertEqual(
            self.process(lines, enabled=False, **kwargs),
            self.process(lines, enabled=True, **kwargs),
        )

    # ------------------------------------------------------------------
    # absolute-store arm
    # ------------------------------------------------------------------
    def test_sw_before_jal_is_filled(self):
        lines = [
            "sw\t$4,D_8009B438",
            "jal\tfunc_80085DC4",
        ]

        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)

        self.assertNotEqual(disabled, enabled)
        self.assertIn("# FILL_CALL_DELAY_SLOT START", enabled)
        start = enabled.index("# FILL_CALL_DELAY_SLOT START")
        self.assertEqual(
            enabled[start:start + 6],
            [
                "# FILL_CALL_DELAY_SLOT START",
                ".set\tnoat",
                "lui\t$at,%hi(D_8009B438)",
                "jal\tfunc_80085DC4",
                "sw\t$4,%lo(D_8009B438)($at)",
                ".set\tat",
            ],
        )
        # the jump must not also be emitted with a nop slot of its own
        self.assertNotIn("nop  # DEBUG: branch/jump", enabled)

    def test_sh_and_sb_before_jal_are_filled(self):
        # ROM has a jal+sh site (func_80073F00 at 0x800740a4), so unlike
        # patch 1 this arm is not sw-only.
        for op in ("sh", "sb"):
            lines = [f"{op}\t$0,D_800945E6", "jal\tfunc_80074100"]
            enabled = self.process(lines, enabled=True)
            self.assertIn(f"{op}\t$0,%lo(D_800945E6)($at)", enabled, op)

    def test_sw_before_plain_j_is_filled(self):
        lines = [
            "sw\t$4,D_8009B374",
            "j\t$L12",
        ]

        enabled = self.process(lines, enabled=True)
        self.assertIn("j\t$L12", enabled)
        self.assertIn("sw\t$4,%lo(D_8009B374)($at)", enabled)

    def test_sw_before_jalr_is_filled(self):
        # cc1 spells an indirect call `jal $2`; the moved store writes no
        # register so the target register is undisturbed.
        lines = [
            "sw\t$4,D_8009B374",
            "jal\t$2",
        ]

        enabled = self.process(lines, enabled=True)
        self.assertIn("jal\t$2", enabled)
        self.assertIn("sw\t$4,%lo(D_8009B374)($at)", enabled)

    # H1 ---------------------------------------------------------------
    def test_h1_ra_source_register_blocks_fill(self):
        # `jal` overwrites $31, so a moved store of $31 would write the NEW
        # return address.
        for reg in ("$31", "$ra"):
            self.assert_unchanged([f"sw\t{reg},D_8009B438", "jal\tfunc_80085DC4"])

    # H4 ---------------------------------------------------------------
    def test_h4_at_source_register_blocks_fill(self):
        for reg in ("$at", "$1"):
            self.assert_unchanged([f"sw\t{reg},D_8009B438", "jal\tfunc_80085DC4"])

    def test_h3_jalr_through_at_blocks_fill(self):
        # The surviving `lui $at` would destroy the jump target.
        for reg in ("$at", "$1"):
            self.assert_unchanged([f"sw\t$4,D_8009B438", f"jal\t{reg}"])

    # ------------------------------------------------------------------
    # `la` arm
    # ------------------------------------------------------------------
    def test_index_pair_schedule_matches_retail_order(self):
        lines = [
            "sll\t$4,$4,2",
            "sll\t$2,$2,2",
            "addu\t$2,$2,$16",
            "lw\t$3,0($2)",
            "lbu\t$2,D_8009AFD5",
            "nop",
            "sll\t$2,$2,2",
            "addu\t$2,$2,$19",
            "addu\t$4,$4,$16",
            "lw\t$6,0($2)",
            "lw\t$7,0($4)",
            "la\t$4,D_80011B28",
            ".set\tnoreorder",
            "jal\tfunc_80071A74",
            "sw\t$3,16($sp)",
            ".set\treorder",
            "jal\tfunc_8007B9EC",
            "li\t$3,-1",
            "$L8:",
            "bne\t$3,$0,$L12",
            "li\t$2,-1",
        ]
        with patch.dict(os.environ, {"MASPSX_INDEX_PAIR_JAL_SCHEDULE": "1"}):
            got = MaspsxProcessor(lines).process_lines()
        text = "\n".join(got)
        flat = text.replace("\t", "").replace(" ", "")
        self.assertLess(flat.index("sll$2,$2,2"), flat.index("sll$4,$4,2"))
        self.assertLess(flat.index("sw$3,16($sp)"), flat.index("lw$6,0($2)"))
        self.assertIn("addiu$4,$4,%lo(D_80011B28)", flat)
        self.assertIn("j$L12_join", flat)
        self.assertIn("addu$2,$zero,$zero", flat)
        self.assertIn("bne$2,$zero,$L12", flat)
        self.assertNotIn("li$3,-1", flat)

    def test_la_before_jal_is_filled(self):
        lines = [
            "la\t$4,func_80081E70",
            "jal\tfunc_800824C8",
        ]

        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)

        self.assertNotEqual(disabled, enabled)
        start = enabled.index("# FILL_CALL_DELAY_SLOT LA START")
        self.assertEqual(
            enabled[start:start + 4],
            [
                "# FILL_CALL_DELAY_SLOT LA START",
                "lui\t$4,%hi(func_80081E70)",
                "jal\tfunc_800824C8",
                "addiu\t$4,$4,%lo(func_80081E70)",
            ],
        )
        self.assertNotIn("nop  # DEBUG: branch/jump", enabled)

    def test_la_before_jalr_with_other_target_is_filled(self):
        # ROM: func_80074BB8 — lui $a0 / jalr $v0 / addiu $a0,$a0,%lo.
        lines = [
            "la\t$4,D_80011814",
            "jal\t$2",
        ]

        enabled = self.process(lines, enabled=True)
        self.assertIn("lui\t$4,%hi(D_80011814)", enabled)
        self.assertIn("addiu\t$4,$4,%lo(D_80011814)", enabled)

    # H3 ---------------------------------------------------------------
    def test_h3_la_into_the_jalr_target_register_blocks_fill(self):
        # The jump would read a half-formed address (only the `lui` applied).
        self.assert_unchanged(["la\t$2,func_80081E70", "jal\t$2"])

    def test_h3_register_alias_cannot_fail_open(self):
        # $4 and $a0 are the same register; the guard compares canonically.
        self.assert_unchanged(["la\t$a0,func_80081E70", "jal\t$4"])
        self.assert_unchanged(["la\t$4,func_80081E70", "jal\t$a0"])

    # H2 ---------------------------------------------------------------
    def test_h2_la_into_ra_blocks_fill(self):
        for reg in ("$31", "$ra"):
            self.assert_unchanged([f"la\t{reg},func_80081E70", "jal\tfunc_800824C8"])

    # H9 ---------------------------------------------------------------
    def test_h9_numeric_operand_blocks_fill(self):
        self.assert_unchanged(["la\t$4,-4", "jal\tfunc_800824C8"])
        self.assert_unchanged(["la\t$4,0x10", "jal\tfunc_800824C8"])
        self.assert_unchanged(["sw\t$4,56200($4)", "jal\tfunc_800824C8"])

    def test_indexed_la_blocks_fill(self):
        self.assert_unchanged(["la\t$4,SYM($2)", "jal\tfunc_800824C8"])

    # H8 ---------------------------------------------------------------
    def test_h8_gp_allow_la_blocks_the_la_arm(self):
        # ASPSX >= 2.80 turns `la` of a small-data symbol into a single
        # gp-relative instruction, so there is no second word to move.
        self.assert_unchanged(
            ["la\t$4,func_80081E70", "jal\tfunc_800824C8"], gp_allow_la=True
        )

    def test_h8_gp_relative_store_is_not_split(self):
        lines = [
            ".sdata",
            "D_8009B438:",
            "\t.word\t0",
            ".text",
            "sw\t$4,D_8009B438",
            "jal\tfunc_80085DC4",
        ]

        enabled = self.process(lines, enabled=True, sdata_limit=8)
        self.assertIn("sw\t$4,%gp_rel(D_8009B438)($gp)", enabled)
        self.assertNotIn("# FILL_CALL_DELAY_SLOT START", enabled)

    # ------------------------------------------------------------------
    # shared lookahead guards
    # ------------------------------------------------------------------
    # H5 ---------------------------------------------------------------
    def test_h5_label_between_blocks_fill(self):
        self.assert_unchanged(["sw\t$4,D_8009B438", "$L7:", "jal\tfunc_80085DC4"])
        self.assert_unchanged(["la\t$4,func_80081E70", "$L7:", "jal\tfunc_800824C8"])

    def test_h5_set_directive_between_blocks_fill(self):
        lines = [
            "sw\t$4,D_8009B438",
            ".set\tnoreorder",
            ".set\tnomacro",
            "jal\tfunc_80085DC4",
            "nop",
            ".set\tmacro",
            ".set\treorder",
        ]
        self.assert_unchanged(lines)

    # H6 ---------------------------------------------------------------
    def test_h6_conditional_branch_is_not_touched(self):
        for branch in ("beq\t$4,$2,$L3", "bne\t$4,$2,$L3", "blez\t$4,$L3"):
            self.assert_unchanged(["sw\t$4,D_8009B438", branch])
            self.assert_unchanged(["la\t$4,func_80081E70", branch])

    def test_return_jump_slot_is_left_to_patches_1_and_3(self):
        # `j $31` must stay untouched so patch 1 / patch 3 keep their ROM
        # behaviour when several knobs are on.
        self.assert_unchanged(["sw\t$4,D_8009B438", "j\t$31"])
        self.assert_unchanged(["sh\t$4,D_8009B438", "j\t$31"])
        self.assert_unchanged(["la\t$4,func_80081E70", "j\t$31"])

    def test_patch_1_still_owns_the_return_slot_when_both_knobs_are_on(self):
        lines = ["sw\t$4,D_800A36A0", "j\t$31"]
        with patch.dict(os.environ, {"MASPSX_FILL_CALL_DELAY_SLOT": "0"}):
            only_1 = MaspsxProcessor(
                lines, fill_store_delay_slot=True
            ).process_lines()
            both = MaspsxProcessor(
                lines, fill_store_delay_slot=True, fill_call_delay_slot=True
            ).process_lines()
        self.assertEqual(only_1, both)
        self.assertIn("# FILL_STORE_DELAY_SLOT START", both)

    # H7 ---------------------------------------------------------------
    def test_h7_compound_macro_lines_block_fill(self):
        self.assert_unchanged(
            ["sw\t$4,D_8009B438", "jal\tfunc_80085DC4;nop"]
        )
        self.assert_unchanged(
            ["lw\t$3,0($4);sw\t$4,D_8009B438", "jal\tfunc_80085DC4"]
        )

    def test_nonadjacent_call_blocks_fill(self):
        self.assert_unchanged(
            ["sw\t$4,D_8009B438", "addiu\t$5,$0,1", "jal\tfunc_80085DC4"]
        )

    def test_blank_and_comment_lines_are_skipped(self):
        lines = [
            "sw\t$4,D_8009B438",
            "",
            "jal\tfunc_80085DC4",
        ]
        enabled = self.process(lines, enabled=True)
        self.assertIn("sw\t$4,%lo(D_8009B438)($at)", enabled)

    # ------------------------------------------------------------------
    # env gate
    # ------------------------------------------------------------------
    def test_env_var_enables_the_knob(self):
        lines = ["sw\t$4,D_8009B438", "jal\tfunc_80085DC4"]
        with patch.dict(os.environ, {"MASPSX_FILL_CALL_DELAY_SLOT": "1"}):
            enabled = MaspsxProcessor(lines).process_lines()
        self.assertIn("# FILL_CALL_DELAY_SLOT START", enabled)

    def test_default_is_off(self):
        lines = ["sw\t$4,D_8009B438", "jal\tfunc_80085DC4"]
        with patch.dict(os.environ, {}, clear=True):
            default = MaspsxProcessor(lines).process_lines()
        self.assertNotIn("# FILL_CALL_DELAY_SLOT START", default)
        self.assertIn("nop  # DEBUG: branch/jump", default)


if __name__ == "__main__":
    unittest.main()

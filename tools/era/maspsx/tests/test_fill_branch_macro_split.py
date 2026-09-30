"""LOCAL PATCH 10 (fill_branch_macro_split) — durable tests.

The knob schedules the SECOND word of an absolute store macro
(`sw/sh/sb $r,SYM`) or of an address macro (`la $r,SYM`) into the delay slot
of an immediately following reorder-mode CONDITIONAL branch:

    lui $at,%hi(SYM) / beq $a,$b,L / sw $r,%lo(SYM)($at)
    lui $r,%hi(SYM)  / beq $v,$0,L / addiu $r,$r,%lo(SYM)

Every hazard guard (HB1..HB6 in the patch log) has a negative test here, and
every negative test asserts flag-off/flag-on byte identity directly.
"""

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

CLEAN_ENV = {
    "MASPSX_FILL_BRANCH_MACRO_SPLIT": "0",
    "MASPSX_FILL_CALL_DELAY_SLOT": "0",
    "MASPSX_FILL_BRANCH_DELAY_SLOT": "0",
}


class TestFillBranchMacroSplit(unittest.TestCase):
    @staticmethod
    def process(lines, enabled, **kwargs):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variables set.
        with patch.dict(os.environ, CLEAN_ENV):
            return MaspsxProcessor(
                lines, fill_branch_macro_split=enabled, **kwargs
            ).process_lines()

    def assert_unchanged(self, lines, **kwargs):
        self.assertEqual(
            self.process(lines, enabled=False, **kwargs),
            self.process(lines, enabled=True, **kwargs),
        )

    # ------------------------------------------------------------------
    # absolute-store arm
    # ------------------------------------------------------------------
    def test_sw_before_beq_is_filled(self):
        # ROM: func_800739C4 0x800739D8 lui $at / beq $a0,$v0 / sw $a1,%lo($at)
        lines = ["sw\t$5,D_80094568", "beq\t$4,$2,$L3"]
        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)
        self.assertNotEqual(disabled, enabled)
        start = enabled.index("# FILL_BRANCH_MACRO_SPLIT START")
        self.assertEqual(
            enabled[start:start + 6],
            [
                "# FILL_BRANCH_MACRO_SPLIT START",
                ".set\tnoat",
                "lui\t$at,%hi(D_80094568)",
                "beq\t$4,$2,$L3",
                "sw\t$5,%lo(D_80094568)($at)",
                ".set\tat",
            ],
        )
        self.assertNotIn("nop  # DEBUG: branch/jump", enabled)

    def test_store_of_a_branch_operand_is_still_filled(self):
        # The moved store writes no GPR, so storing the register the branch
        # tests is harmless (5 such sites in ROM, e.g. func_8007C214's
        # `beqz $a0` with `sw $v1` -- here the stored reg IS the tested one).
        lines = ["sw\t$4,D_800BE9E4", "beq\t$4,$0,$L2"]
        enabled = self.process(lines, enabled=True)
        self.assertIn("sw\t$4,%lo(D_800BE9E4)($at)", enabled)

    def test_all_six_branch_spellings_and_sh_sb(self):
        for br in ("bne\t$4,$2,$L1", "blez\t$4,$L1", "bgtz\t$4,$L1",
                   "bltz\t$4,$L1", "bgez\t$4,$L1"):
            enabled = self.process(["sw\t$5,D_1", br], enabled=True)
            self.assertIn(br, enabled, br)
            self.assertIn("sw\t$5,%lo(D_1)($at)", enabled, br)
        for op in ("sh", "sb"):
            enabled = self.process([f"{op}\t$5,D_1", "beq\t$4,$0,$L1"], enabled=True)
            self.assertIn(f"{op}\t$5,%lo(D_1)($at)", enabled, op)

    # ------------------------------------------------------------------
    # `la` arm
    # ------------------------------------------------------------------
    def test_la_before_beqz_is_filled(self):
        # ROM: func_800809E0 lui $a1 / beqz $v0 / addiu $a1,$a1,%lo
        lines = ["la\t$5,D_80011E44", "beq\t$2,$0,$L7"]
        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)
        self.assertNotEqual(disabled, enabled)
        start = enabled.index("# FILL_BRANCH_MACRO_SPLIT LA START")
        self.assertEqual(
            enabled[start:start + 4],
            [
                "# FILL_BRANCH_MACRO_SPLIT LA START",
                "lui\t$5,%hi(D_80011E44)",
                "beq\t$2,$0,$L7",
                "addiu\t$5,$5,%lo(D_80011E44)",
            ],
        )
        self.assertNotIn("nop  # DEBUG: branch/jump", enabled)

    def test_la_before_bltz_is_filled(self):
        # ROM: func_80082E00 lui $v1 / bltz $v0 / addiu $v1,$v1,%lo
        enabled = self.process(["la\t$3,D_8009B77C", "bltz\t$2,$L9"], enabled=True)
        self.assertIn("addiu\t$3,$3,%lo(D_8009B77C)", enabled)

    # HB1 --------------------------------------------------------------
    def test_hb1_la_into_a_branch_operand_blocks_fill(self):
        # The branch would test a half-formed address (only the lui applied).
        self.assert_unchanged(["la\t$2,D_1", "beq\t$2,$0,$L1"])
        self.assert_unchanged(["la\t$2,D_1", "bne\t$4,$2,$L1"])
        self.assert_unchanged(["la\t$2,D_1", "bltz\t$2,$L1"])

    def test_hb1_register_alias_cannot_fail_open(self):
        self.assert_unchanged(["la\t$a0,D_1", "beq\t$4,$0,$L1"])
        self.assert_unchanged(["la\t$4,D_1", "beq\t$a0,$0,$L1"])

    def test_hb1_at_as_branch_operand_blocks_fill(self):
        self.assert_unchanged(["sw\t$5,D_1", "beq\t$at,$0,$L1"])
        self.assert_unchanged(["sw\t$5,D_1", "beq\t$1,$0,$L1"])

    # HB2 / HB3 --------------------------------------------------------
    def test_hb2_at_value_register_blocks_fill(self):
        for reg in ("$at", "$1"):
            self.assert_unchanged([f"sw\t{reg},D_1", "beq\t$4,$0,$L1"])
            self.assert_unchanged([f"la\t{reg},D_1", "beq\t$4,$0,$L1"])

    def test_hb3_ra_blocks_fill_and_link_branches_are_refused(self):
        for reg in ("$31", "$ra"):
            self.assert_unchanged([f"sw\t{reg},D_1", "beq\t$4,$0,$L1"])
            self.assert_unchanged([f"la\t{reg},D_1", "beq\t$4,$0,$L1"])
        self.assert_unchanged(["sw\t$5,D_1", "bltzal\t$4,$L1"])
        self.assert_unchanged(["la\t$5,D_1", "bgezal\t$4,$L1"])

    # HB4 --------------------------------------------------------------
    def test_hb4_label_between_blocks_fill(self):
        self.assert_unchanged(["sw\t$5,D_1", "$L4:", "beq\t$4,$0,$L1"])
        self.assert_unchanged(["la\t$5,D_1", "$L4:", "beq\t$4,$0,$L1"])

    def test_hb4_set_noreorder_between_blocks_fill(self):
        lines = [
            "sw\t$5,D_1",
            ".set\tnoreorder",
            ".set\tnomacro",
            "beq\t$4,$0,$L1",
            "addiu\t$2,$0,1",
            ".set\tmacro",
            ".set\treorder",
        ]
        self.assert_unchanged(lines)

    def test_hb4_branch_inside_noreorder_region_blocks_fill(self):
        lines = [
            ".set\tnoreorder",
            "sw\t$5,D_1",
            "beq\t$4,$0,$L1",
            "nop",
            ".set\treorder",
        ]
        self.assert_unchanged(lines)

    # HB5 --------------------------------------------------------------
    def test_hb5_compound_numeric_indexed_and_gp_block_fill(self):
        self.assert_unchanged(["sw\t$5,D_1", "beq\t$4,$0,$L1;nop"])
        self.assert_unchanged(["lw\t$3,0($4);sw\t$5,D_1", "beq\t$4,$0,$L1"])
        self.assert_unchanged(["sw\t$5,56200($4)", "beq\t$4,$0,$L1"])
        self.assert_unchanged(["la\t$5,-4", "beq\t$4,$0,$L1"])
        self.assert_unchanged(["la\t$5,D_1($2)", "beq\t$4,$0,$L1"])
        self.assert_unchanged(["la\t$5,D_1", "beq\t$4,$0,$L1"], gp_allow_la=True)
        lines = [".sdata", "D_1:", "\t.word\t0", ".text",
                 "sw\t$5,D_1", "beq\t$4,$0,$L1"]
        enabled = self.process(lines, enabled=True, sdata_limit=8)
        self.assertIn("sw\t$5,%gp_rel(D_1)($gp)", enabled)
        self.assertNotIn("# FILL_BRANCH_MACRO_SPLIT START", enabled)

    # HB6 --------------------------------------------------------------
    def test_hb6_only_conditional_branches_with_label_targets(self):
        # jumps/calls belong to patch 8 (off here), the return to patch 1/3
        self.assert_unchanged(["sw\t$5,D_1", "jal\tfunc_1"])
        self.assert_unchanged(["sw\t$5,D_1", "j\t$L1"])
        self.assert_unchanged(["sw\t$5,D_1", "j\t$31"])
        self.assert_unchanged(["la\t$5,D_1", "jal\tfunc_1"])
        self.assert_unchanged(["sw\t$5,D_1", "beq\t$4,$0,$2"])

    def test_nonadjacent_branch_blocks_fill(self):
        self.assert_unchanged(["sw\t$5,D_1", "addiu\t$2,$0,1", "beq\t$4,$0,$L1"])

    # ------------------------------------------------------------------
    # composition with the other slot patches
    # ------------------------------------------------------------------
    def test_patch_8_and_patch_10_do_not_interfere(self):
        lines = [
            "sw\t$5,D_1", "beq\t$4,$0,$L1",
            "sw\t$6,D_2", "jal\tfunc_1",
        ]
        with patch.dict(os.environ, CLEAN_ENV):
            both = MaspsxProcessor(
                lines, fill_call_delay_slot=True, fill_branch_macro_split=True
            ).process_lines()
            only_8 = MaspsxProcessor(lines, fill_call_delay_slot=True).process_lines()
            only_10 = MaspsxProcessor(lines, fill_branch_macro_split=True).process_lines()
        self.assertIn("# FILL_BRANCH_MACRO_SPLIT START", both)
        self.assertIn("# FILL_CALL_DELAY_SLOT START", both)
        self.assertNotIn("# FILL_BRANCH_MACRO_SPLIT START", only_8)
        self.assertNotIn("# FILL_CALL_DELAY_SLOT START", only_10)

    def test_patch_6_never_sees_a_branch_consumed_by_patch_10(self):
        # patch 6 keys on the instruction AFTER the branch; once patch 10 has
        # consumed the branch line the slot is taken and the compare stays.
        lines = ["sw\t$5,D_1", "beq\t$4,$0,$L1", "slti\t$2,$4,10", "bne\t$2,$0,$L2", "$L1:", "$L2:"]
        with patch.dict(os.environ, CLEAN_ENV):
            out = MaspsxProcessor(
                lines, fill_branch_macro_split=True, fill_branch_delay_slot=True
            ).process_lines()
        self.assertIn("sw\t$5,%lo(D_1)($at)", out)
        self.assertIn("slti\t$2,$4,10", out)
        self.assertNotIn("# FILL_BRANCH_DELAY_SLOT START", out[:out.index("slti\t$2,$4,10")])

    # ------------------------------------------------------------------
    # band1's patch8b_fixture.s: both arms inside a realistic cc1 function
    # body (ROM: func_8007BDDC 0x8007BE80 store arm, func_800809E0 la arm)
    # ------------------------------------------------------------------
    def test_fixture_store_arm_in_function_body(self):
        lines = [
            ".text", ".globl\tt_store", ".ent\tt_store", "t_store:",
            "lw\t$3,D_800A347C",
            "addiu\t$2,$3,1",
            "slt\t$3,$20,$3",
            "sw\t$2,D_800A347C",
            "beq\t$3,$0,$L9",
            "$L9:",
            "j\t$31",
            ".end\tt_store",
        ]
        out = self.process(lines, enabled=True)
        i = out.index("lui\t$at,%hi(D_800A347C)")
        self.assertEqual(out[i + 1], "beq\t$3,$0,$L9")
        self.assertEqual(out[i + 2], "sw\t$2,%lo(D_800A347C)($at)")
        # the preceding lines are untouched (the load stays a macro for gas)
        m = out.index("# FILL_BRANCH_MACRO_SPLIT START")
        self.assertEqual(out[m - 2], "addiu\t$2,$3,1")
        self.assertEqual(out[m - 1], "slt\t$3,$20,$3")
        self.assertEqual(out[m + 1], ".set\tnoat")
        self.assertIn("lw\t$3,D_800A347C", out)
        # exactly one slot consumed: the return `j $31` keeps its own nop
        self.assertEqual(out.count("nop  # DEBUG: branch/jump"), 1)

    def test_fixture_la_arm_in_function_body(self):
        lines = [
            ".text", ".globl\tt_la", ".ent\tt_la", "t_la:",
            "la\t$5,D_80011E44",
            "beq\t$2,$0,$L8",
            "$L8:",
            "j\t$31",
            ".end\tt_la",
        ]
        out = self.process(lines, enabled=True)
        i = out.index("lui\t$5,%hi(D_80011E44)")
        self.assertEqual(out[i + 1], "beq\t$2,$0,$L8")
        self.assertEqual(out[i + 2], "addiu\t$5,$5,%lo(D_80011E44)")
        self.assertEqual(out.count("nop  # DEBUG: branch/jump"), 1)

    # ------------------------------------------------------------------
    # env gate
    # ------------------------------------------------------------------
    def test_env_var_enables_the_knob(self):
        with patch.dict(os.environ, {"MASPSX_FILL_BRANCH_MACRO_SPLIT": "1"}):
            enabled = MaspsxProcessor(["sw\t$5,D_1", "beq\t$4,$0,$L1"]).process_lines()
        self.assertIn("# FILL_BRANCH_MACRO_SPLIT START", enabled)

    def test_default_is_off(self):
        with patch.dict(os.environ, {}, clear=True):
            default = MaspsxProcessor(["sw\t$5,D_1", "beq\t$4,$0,$L1"]).process_lines()
        self.assertNotIn("# FILL_BRANCH_MACRO_SPLIT START", default)
        self.assertIn("nop  # DEBUG: branch/jump", default)


if __name__ == "__main__":
    unittest.main()

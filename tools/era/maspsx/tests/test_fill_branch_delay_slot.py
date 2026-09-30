import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

NOP = "nop  # DEBUG: branch/jump"
START = "# FILL_BRANCH_DELAY_SLOT START"


def _wrap(body):
    """Wrap a body in the minimal cc1 function scaffolding maspsx expects."""
    return [
        "\t.text",
        "\t.ent\tfunc",
        "func:",
        *body,
        "\t.end\tfunc",
    ]


# The retail func_80043474 shape: a compare chain whose third branch is left
# in reorder mode by cc1 (its fall-through insn redefines the tested register,
# which gcc's reorg.c refuses to schedule), and whose $2 is killed by the
# epilogue `move $2,$3` on the taken path.
BAND_CHAIN = _wrap(
    [
        "slt\t$2,$4,536",
        "bne\t$2,$0,$L8",
        "slt\t$2,$4,723",
        ".set\tnoreorder",
        ".set\tnomacro",
        "beq\t$2,$0,$L3",
        "li\t$3,0x00000006",
        ".set\tmacro",
        ".set\treorder",
        "",
        ".set\tnoreorder",
        ".set\tnomacro",
        "j\t$L3",
        "li\t$3,0x00000005",
        ".set\tmacro",
        ".set\treorder",
        "",
        "$L8:",
        "li\t$3,0x00000004",
        "$L3:",
        ".set\tnoreorder",
        ".set\tnomacro",
        "j\t$31",
        "move\t$2,$3",
        ".set\tmacro",
        ".set\treorder",
    ]
)


class TestFillBranchDelaySlotGuards(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variable set.
        with patch.dict(os.environ, {"MASPSX_FILL_BRANCH_DELAY_SLOT": "0"}):
            return MaspsxProcessor(
                lines, fill_branch_delay_slot=enabled
            ).process_lines()

    def assert_unchanged(self, lines):
        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    # --- the ROM-evidenced shape -------------------------------------------

    def test_flag_off_keeps_the_nop(self):
        disabled = self.process(BAND_CHAIN, enabled=False)
        self.assertIn(NOP, disabled)
        self.assertNotIn(START, disabled)

    def test_compare_chain_is_filled(self):
        enabled = self.process(BAND_CHAIN, enabled=True)
        self.assertIn(START, enabled)
        self.assertIn("slt\t$2,$4,723", enabled)
        # the moved instruction is consumed, not duplicated
        self.assertEqual(
            len([x for x in enabled if x == "slt\t$2,$4,723"]), 1
        )
        # and no nop is left behind for that branch
        self.assertEqual(
            len([x for x in enabled if x == NOP]),
            len([x for x in self.process(BAND_CHAIN, enabled=False)
                 if x == NOP]) - 1,
        )

    def test_env_var_enables_the_patch(self):
        with patch.dict(os.environ, {"MASPSX_FILL_BRANCH_DELAY_SLOT": "1"}):
            enabled = MaspsxProcessor(BAND_CHAIN).process_lines()
        self.assertIn(START, enabled)

    # --- safety gate: liveness at the branch target -------------------------

    def test_live_at_target_blocks_fill(self):
        # abs(): `bgez $4,$L2` / `subu $4,$0,$4`.  $4 is LIVE at $L2, so the
        # fill would miscompile (retail keeps the nop -- func_80077DC4).
        lines = _wrap(
            [
                "bgez\t$4,$L2",
                "subu\t$4,$0,$4",
                "$L2:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$4",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_return_without_kill_blocks_fill(self):
        # $2 reaches the return unwritten, so it is live out of the function.
        lines = _wrap(
            [
                "slt\t$3,$4,8",
                "bne\t$3,$0,$L2",
                "slt\t$3,$4,16",
                ".set\tnoreorder",
                ".set\tnomacro",
                "beq\t$3,$0,$L2",
                "li\t$2,0x00000002",
                ".set\tmacro",
                ".set\treorder",
                "$L2:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "nop",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_call_on_taken_path_blocks_fill(self):
        lines = _wrap(
            [
                "slt\t$2,$4,8",
                "bne\t$2,$0,$L2",
                "slt\t$2,$4,16",
                ".set\tnoreorder",
                ".set\tnomacro",
                "beq\t$2,$0,$L2",
                "li\t$3,0x00000002",
                ".set\tmacro",
                ".set\treorder",
                "$L2:",
                "jal\tsome_function",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_computed_jump_on_taken_path_blocks_fill(self):
        lines = _wrap(
            [
                "slt\t$2,$4,8",
                "bne\t$2,$0,$L2",
                "slt\t$2,$4,16",
                ".set\tnoreorder",
                ".set\tnomacro",
                "beq\t$2,$0,$L2",
                "li\t$3,0x00000002",
                ".set\tmacro",
                ".set\treorder",
                "$L2:",
                "j\t$3",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_unknown_target_label_blocks_fill(self):
        lines = _wrap(
            [
                "slt\t$2,$4,8",
                "bne\t$2,$0,$LNOWHERE",
                "slt\t$2,$4,16",
                ".set\tnoreorder",
                ".set\tnomacro",
                "beq\t$2,$0,$L3",
                "li\t$3,0x00000002",
                ".set\tmacro",
                ".set\treorder",
                "$L3:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    # --- fidelity gate: the compare-chain shape -----------------------------

    def test_non_compare_candidate_blocks_fill(self):
        # func_8006A2E8: `beq $2,$0,$L2` / `move $2,$5`.  $2 IS dead at $L2,
        # so the fill would be legal -- but retail keeps the nop, so the
        # fidelity gate must refuse a non-compare candidate.
        lines = _wrap(
            [
                "sltu\t$2,$5,16",
                "beq\t$2,$0,$L2",
                "move\t$2,$5",
                "sh\t$2,D_800BCE9E",
                "$L2:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$0",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_compare_not_consumed_by_a_branch_blocks_fill(self):
        # func_80012E7C: `beq $2,$0,$L13` / `sll $2,$3,2` feeding a jump
        # table load.  Dead at the target, still a nop in retail.
        lines = _wrap(
            [
                "sltu\t$2,$3,7",
                "beq\t$2,$0,$L13",
                "sll\t$2,$3,2",
                "lw\t$2,$L10($2)",
                "$L13:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "li\t$2,0x00000001",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_load_candidate_blocks_fill(self):
        lines = _wrap(
            [
                "slt\t$2,$4,8",
                "bne\t$2,$0,$L2",
                "lw\t$2,0($4)",
                ".set\tnoreorder",
                ".set\tnomacro",
                "beq\t$2,$0,$L2",
                "li\t$3,0x00000002",
                ".set\tmacro",
                ".set\treorder",
                "$L2:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    # --- structural guards --------------------------------------------------

    def test_label_between_branch_and_candidate_blocks_fill(self):
        lines = _wrap(
            [
                "slt\t$2,$4,536",
                "bne\t$2,$0,$L8",
                "$L9:",
                "slt\t$2,$4,723",
                ".set\tnoreorder",
                ".set\tnomacro",
                "beq\t$2,$0,$L3",
                "li\t$3,0x00000006",
                ".set\tmacro",
                ".set\treorder",
                "$L8:",
                "li\t$3,0x00000004",
                "$L3:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_noreorder_branch_is_untouched(self):
        # cc1 already filled this slot itself; the patch must never touch a
        # branch inside a `.set noreorder` block.
        lines = _wrap(
            [
                ".set\tnoreorder",
                ".set\tnomacro",
                "bne\t$2,$0,$L3",
                "li\t$3,0x00000001",
                ".set\tmacro",
                ".set\treorder",
                "$L3:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_unconditional_jump_is_untouched(self):
        lines = _wrap(
            [
                "j\t$L3",
                "slt\t$2,$4,723",
                "$L3:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_forbidden_destination_blocks_fill(self):
        # $sp is never a legal fill destination for this patch
        lines = _wrap(
            [
                "slt\t$2,$4,8",
                "bne\t$2,$0,$L2",
                "addiu\t$sp,$sp,8",
                "$L2:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_out_of_range_immediate_blocks_fill(self):
        # an `sltu` with a negative literal can reach for $at, so it is never
        # a single-word candidate
        lines = _wrap(
            [
                "slt\t$2,$4,8",
                "bne\t$2,$0,$L8",
                "sltu\t$2,$4,-1",
                ".set\tnoreorder",
                ".set\tnomacro",
                "beq\t$2,$0,$L3",
                "li\t$3,0x00000006",
                ".set\tmacro",
                ".set\treorder",
                "$L8:",
                "li\t$3,0x00000004",
                "$L3:",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)

    def test_inline_asm_on_taken_path_blocks_fill(self):
        lines = _wrap(
            [
                "slt\t$2,$4,8",
                "bne\t$2,$0,$L2",
                "slt\t$2,$4,16",
                ".set\tnoreorder",
                ".set\tnomacro",
                "beq\t$2,$0,$L2",
                "li\t$3,0x00000002",
                ".set\tmacro",
                ".set\treorder",
                "$L2:",
                "#APP",
                "\tnop",
                "#NO_APP",
                ".set\tnoreorder",
                ".set\tnomacro",
                "j\t$31",
                "move\t$2,$3",
                ".set\tmacro",
                ".set\treorder",
            ]
        )
        self.assert_unchanged(lines)


if __name__ == "__main__":
    unittest.main()

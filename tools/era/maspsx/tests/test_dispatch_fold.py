import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestDispatchFold(unittest.TestCase):
    """Switch dispatch retarget (MASPSX_DISPATCH_FOLD).

    cc1 lowers a switch into a compound indexed load against its own
    local rodata table label (`lw $2,$L30($2)`). Retail references the
    shared pool table with the 3-word indexed-symbol form instead. With
    THREE_WORD + FOLD gates set, the local $L<digits> operand is
    substituted with the pool symbol so GNU as emits retail's exact
    3-word dispatch; the dead local table is stripped build-side.

    The negatives here are as load-bearing as the positives: every
    near-miss must pass through unchanged.
    """

    DISPATCH = "lw\t$2,$L30($2)"
    FOLDED = [
        ".set\tnoat",
        "lui\t$at,%hi(jtbl_80010000)",
        "addu\t$at,$at,$2",
        "lw\t$2,%lo(jtbl_80010000)($at)",
        ".set\tat",
    ]
    UNFOLDED_THREE_WORD = [
        ".set\tnoat",
        "lui\t$at,%hi($L30)",
        "addu\t$at,$at,$2",
        "lw\t$2,%lo($L30)($at)",
        ".set\tat",
    ]
    LEGACY_FOUR_WORD = [
        ".set\tnoat",
        "lui\t$at,%hi($L30)",
        "addiu\t$at,$at,%lo($L30)",
        "addu\t$at,$at,$2",
        "lw\t$2,0x0($at)",
        ".set\tat",
    ]

    @staticmethod
    def process(
        lines,
        *,
        fold=None,
        three_word=True,
        env_fold=False,
        env_three_word=False,
    ):
        env = {
            "MASPSX_DISPATCH_FOLD": fold or "",
            "MASPSX_THREE_WORD_SYMBOL_STORE": "1" if env_three_word else "0",
        }
        if env_fold and fold:
            env["MASPSX_DISPATCH_FOLD"] = fold
        with patch.dict(os.environ, env):
            return strip_comments(
                MaspsxProcessor(
                    lines,
                    addiu_at=True,
                    three_word_symbol_store=three_word,
                    dispatch_fold_symbol=fold,
                ).process_lines()
            )

    def test_positive_load_fold_retargets_local_label(self):
        self.assertEqual(self.FOLDED, self.process([self.DISPATCH], fold="jtbl_80010000"))

    def test_environment_flag_selects_fold(self):
        with patch.dict(
            os.environ, {"MASPSX_DISPATCH_FOLD": "jtbl_80010000"}
        ):
            res = strip_comments(
                MaspsxProcessor(
                    [self.DISPATCH],
                    addiu_at=True,
                    three_word_symbol_store=True,
                ).process_lines()
            )
        self.assertEqual(self.FOLDED, res)

    def test_negative_fold_off_keeps_three_word_with_local_label(self):
        # THREE_WORD alone: 3-word shape, but no retarget.
        self.assertEqual(
            self.UNFOLDED_THREE_WORD, self.process([self.DISPATCH])
        )

    def test_negative_all_gates_off_keeps_legacy_four_word(self):
        self.assertEqual(
            self.LEGACY_FOUR_WORD,
            self.process([self.DISPATCH], three_word=False),
        )

    def test_negative_named_symbol_is_never_substituted(self):
        line = "lw\t$2,test_sym($4)"
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(test_sym)",
            "addu\t$at,$at,$4",
            "lw\t$2,%lo(test_sym)($at)",
            ".set\tat",
        ]
        self.assertEqual(expected, self.process([line], fold="jtbl_80010000"))

    def test_negative_non_numeric_local_label_is_never_substituted(self):
        line = "lw\t$2,$Lmain($3)"
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi($Lmain)",
            "addu\t$at,$at,$3",
            "lw\t$2,%lo($Lmain)($at)",
            ".set\tat",
        ]
        self.assertEqual(expected, self.process([line], fold="jtbl_80010000"))

    def test_negative_plain_register_offset_load_passes_through(self):
        # Not an addend at all — numeric offset loads never enter the fold.
        self.assertEqual(
            ["lw\t$2,0x10($6)"],
            self.process(["lw\t$2,0x10($6)"], fold="jtbl_80010000"),
        )

    def test_negative_lwc2_passes_through_untouched(self):
        # lwc2 precedent from the load gate: coprocessor loads must not be
        # disturbed by any gate combination.
        line = "lwc2\t$4,0x28($6)"
        for kwargs in (
            {"fold": None},
            {"fold": "jtbl_80010000"},
        ):
            with self.subTest(**kwargs):
                self.assertEqual([line], self.process([line], **kwargs))

    def test_positive_store_compound_line_unaffected_by_fold_gate(self):
        # Stores route through their own branch; the fold gate must not
        # alter store behavior in either state.
        line = "sw\t$2,$L30($4)"
        with_fold = self.process([line], fold="jtbl_80010000")
        without_fold = self.process([line])
        self.assertEqual(with_fold, without_fold)

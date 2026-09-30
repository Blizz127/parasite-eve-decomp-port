import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


def folded(dest, index, symbol):
    return [
        ".set\tnoat",
        f"lui\t$at,%hi({symbol})",
        f"addu\t$at,$at,{index}",
        f"lw\t{dest},%lo({symbol})($at)",
        ".set\tat",
    ]


class TestDispatchFoldMulti(unittest.TestCase):
    """Multi-table switch dispatch retarget (MASPSX_DISPATCH_FOLD=a,b,...).

    A leaf with several switches owns several cc1-local tables. cc1 emits
    each table in `.rdata` right after its tablejump, so the i-th `$L<n>`
    label defined in `.rdata` is the i-th table of the object's .rodata; it
    is retargeted to the i-th pool symbol. func_80085F74 (SpuSetCommonAttr,
    jtbl_800120BC + jtbl_800120DC) is the motivating leaf: swapping the list
    order swaps both dispatch loads and fails the link check.
    """

    # Shape of cc1 2.8.1 output for two switches (func_80085F74), trimmed.
    # The file-scope .rdata block with a non-$L label must not count.
    SOURCE = [
        ".rdata",
        ".align\t2",
        "rkeep.2:",
        ".word\t$L90",
        ".text",
        "func:",
        "lw\t$2,$L17($2)",
        "j\t$2",
        ".rdata",
        ".align\t2",
        "$L17:",
        ".word\t$L8",
        ".text",
        "$L8:",
        "lw\t$3,$L40($3)",
        "j\t$3",
        ".rdata",
        ".align\t2",
        "$L40:",
        ".word\t$L31",
        ".text",
        "$L31:",
        "lw\t$4,$L55($5)",
    ]

    @staticmethod
    def process(lines, fold, three_word=True):
        with patch.dict(
            os.environ,
            {"MASPSX_DISPATCH_FOLD": "", "MASPSX_THREE_WORD_SYMBOL_STORE": "0"},
        ):
            return strip_comments(
                MaspsxProcessor(
                    lines,
                    addiu_at=True,
                    three_word_symbol_store=three_word,
                    dispatch_fold_symbol=fold,
                ).process_lines()
            )

    def dispatch_block(self, res, dest, index):
        # Return the 5-line expansion of the load that writes `dest`.
        for i, line in enumerate(res):
            if line.startswith(f"lw\t{dest},%lo(") and line.endswith("($at)"):
                return res[i - 3 : i + 2]
        self.fail(f"no folded load of {dest} in {res}")

    def test_positive_tables_map_in_rdata_order(self):
        lines = self.SOURCE[:-1]
        res = self.process(lines, "jtbl_800120BC,jtbl_800120DC")
        self.assertEqual(
            folded("$2", "$2", "jtbl_800120BC"), self.dispatch_block(res, "$2", "$2")
        )
        self.assertEqual(
            folded("$3", "$3", "jtbl_800120DC"), self.dispatch_block(res, "$3", "$3")
        )

    def test_positive_order_follows_rdata_not_label_number(self):
        # Table labels numbered against their emission order: the mapping
        # follows the .rdata definition order, not the numeric label.
        lines = [
            "lw\t$2,$L50($2)",
            ".rdata",
            "$L50:",
            ".word\t$L1",
            ".text",
            "lw\t$3,$L12($3)",
            ".rdata",
            "$L12:",
            ".word\t$L2",
            ".text",
        ]
        res = self.process(lines, "jtbl_80010000,jtbl_80010020")
        self.assertEqual(folded("$2", "$2", "jtbl_80010000"), res[0:5])
        self.assertEqual(folded("$3", "$3", "jtbl_80010020"), res[9:14])

    def test_positive_whitespace_in_list_is_ignored(self):
        lines = self.SOURCE[:-1]
        spaced = self.process(lines, " jtbl_800120BC , jtbl_800120DC ")
        plain = self.process(lines, "jtbl_800120BC,jtbl_800120DC")
        self.assertEqual(plain, spaced)

    def test_positive_environment_list_selects_fold(self):
        lines = self.SOURCE[:-1]
        with patch.dict(
            os.environ, {"MASPSX_DISPATCH_FOLD": "jtbl_800120BC,jtbl_800120DC"}
        ):
            res = strip_comments(
                MaspsxProcessor(
                    lines, addiu_at=True, three_word_symbol_store=True
                ).process_lines()
            )
        self.assertEqual(res, self.process(lines, "jtbl_800120BC,jtbl_800120DC"))

    def test_negative_count_mismatch_fails_loudly(self):
        lines = self.SOURCE[:-1]
        with self.assertRaisesRegex(ValueError, "names 3 tables but cc1 emitted 2"):
            self.process(lines, "jtbl_A,jtbl_B,jtbl_C")

    def test_negative_non_table_local_label_fails_loudly(self):
        # $L55 is not defined in .rdata: never guess a mapping for it.
        with self.assertRaisesRegex(ValueError, r"\$L55 is not a .rdata switch table"):
            self.process(self.SOURCE, "jtbl_800120BC,jtbl_800120DC")

    def test_negative_single_symbol_keeps_legacy_every_label_fold(self):
        # One symbol: every $L<digits> operand folds, even one not defined
        # in .rdata, exactly as before the multi-table form existed.
        res = self.process(self.SOURCE, "jtbl_80010000")
        for dest, index in (("$2", "$2"), ("$3", "$3")):
            self.assertEqual(
                folded(dest, index, "jtbl_80010000"),
                self.dispatch_block(res, dest, index),
            )
        self.assertIn("lw\t$4,%lo(jtbl_80010000)($at)", res)

    def test_negative_three_word_off_ignores_list(self):
        lines = self.SOURCE[:-1]
        off = self.process(lines, "jtbl_800120BC,jtbl_800120DC", three_word=False)
        self.assertEqual(off, self.process(lines, None, three_word=False))

    def test_negative_named_symbol_never_substituted(self):
        lines = self.SOURCE[:-1] + ["lw\t$2,test_sym($4)"]
        res = self.process(lines, "jtbl_800120BC,jtbl_800120DC")
        self.assertIn("lw\t$2,%lo(test_sym)($at)", res)


if __name__ == "__main__":
    unittest.main()

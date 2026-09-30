import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestThreeWordSymbolStore(unittest.TestCase):
    STORE = "sh\t$0,D_8009448C($4)"
    LEGACY_STORE = [
        ".set\tnoat",
        "lui\t$at,%hi(D_8009448C)",
        "addiu\t$at,$at,%lo(D_8009448C)",
        "addu\t$at,$at,$4",
        "sh\t$0,0x0($at)",
        ".set\tat",
    ]

    @staticmethod
    def process(lines, *, constructor_enabled=False, environment_enabled=False):
        value = "1" if environment_enabled else "0"
        with patch.dict(os.environ, {"MASPSX_THREE_WORD_SYMBOL_STORE": value}):
            return strip_comments(
                MaspsxProcessor(
                    lines,
                    addiu_at=True,
                    three_word_symbol_store=constructor_enabled,
                ).process_lines()
            )

    def test_disabled_keeps_legacy_four_word_expansion(self):
        self.assertEqual(
            self.LEGACY_STORE,
            self.process([self.STORE]),
        )

    def test_environment_flag_selects_three_word_assembler_path(self):
        # This is the ASPSX 2.30 path: GNU as expands the retained pseudo to
        # lui %hi / addu index / sh %lo, as covered by the linked probe.
        self.assertEqual(
            [self.STORE],
            self.process([self.STORE], environment_enabled=True),
        )

    def test_constructor_flag_selects_three_word_assembler_path(self):
        self.assertEqual(
            [self.STORE],
            self.process([self.STORE], constructor_enabled=True),
        )

    def test_compound_line_keeps_exact_legacy_expansion(self):
        compound = f"{self.STORE};nop"

        self.assertEqual(self.LEGACY_STORE, self.process([compound]))
        self.assertEqual(
            self.LEGACY_STORE,
            self.process([compound], environment_enabled=True),
        )

    def test_indexed_symbolic_load_disabled_keeps_four_word_expansion(self):
        # Flag OFF: loads get the 4-word expansion (lui/addiu/addu/load-0x0)
        load = "lh\t$2,D_8009448C($4)"
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(D_8009448C)",
            "addiu\t$at,$at,%lo(D_8009448C)",
            "addu\t$at,$at,$4",
            "lh\t$2,0x0($at)",
            ".set\tat",
        ]

        self.assertEqual(expected, self.process([load]))

    def test_indexed_symbolic_load_enabled_emits_three_word_at_form(self):
        # Flag ON: loads emit the explicit 3-word ASPSX 2.30 form with $at
        # as the address temp (lui $at / addu $at,$at,$b / lw %lo($at)).
        # NOT a bare pass-through: GNU as expands lw $r,SYM($b) with the
        # DESTINATION register as temp (lui $r / addu $r,$r,$b / lw $r,0($r))
        # — ROM-proven divergence (func_800363F4).
        load = "lw\t$3,D_800BCEA8+12($2)"
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(D_800BCEA8+12)",
            "addu\t$at,$at,$2",
            "lw\t$3,%lo(D_800BCEA8+12)($at)",
            ".set\tat",
        ]
        self.assertEqual(expected, self.process([load], environment_enabled=True))

    def test_indexed_symbolic_load_with_addend_emits_three_word_at_form(self):
        # Flag ON: lh with addend emits the explicit 3-word $at form
        # (ROM-proven shape for func_80037548/800363F4)
        load = "lh\t$2,D_800BCEA8+16($4)"
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(D_800BCEA8+16)",
            "addu\t$at,$at,$4",
            "lh\t$2,%lo(D_800BCEA8+16)($at)",
            ".set\tat",
        ]
        self.assertEqual(expected, self.process([load], environment_enabled=True))

    def test_indexed_symbolic_load_compound_keeps_four_word_expansion(self):
        # Compound lines retain 4-word expansion even with flag ON
        load = "lw\t$3,D_800BCEA8+12($2)"
        compound = f"{load};nop"
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(D_800BCEA8+12)",
            "addiu\t$at,$at,%lo(D_800BCEA8+12)",
            "addu\t$at,$at,$2",
            "lw\t$3,0x0($at)",
            ".set\tat",
        ]

        self.assertEqual(expected, self.process([compound]))
        self.assertEqual(expected, self.process([compound], environment_enabled=True))

    def test_lwc2_stays_outside_gate_flag_on_and_off_identical(self):
        # lwc2 is not in load_mnemonics, so it never routes through the
        # three-word gate: output must be byte-identical with flag OFF and ON.
        load = "lwc2\t$5,D_800BCEA8+12($2)"
        off = self.process([load])
        on = self.process([load], environment_enabled=True)
        self.assertEqual([load], off)
        self.assertEqual(off, on)

    def test_all_load_widths_emit_three_word_at_form_when_enabled(self):
        # All load mnemonics (lb, lbu, lh, lhu, lw, lwl, lwr) emit the
        # explicit 3-word $at form when the flag is enabled.
        for op in ["lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr"]:
            load = f"{op}\t$3,D_800BCEA8($2)"
            expected = [
                ".set\tnoat",
                "lui\t$at,%hi(D_800BCEA8)",
                "addu\t$at,$at,$2",
                f"{op}\t$3,%lo(D_800BCEA8)($at)",
                ".set\tat",
            ]
            self.assertEqual(
                expected,
                self.process([load], environment_enabled=True),
                f"{op} should emit the 3-word $at form when flag enabled",
            )


if __name__ == "__main__":
    unittest.main()

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestFillJumpSymbolStore(unittest.TestCase):
    """LOCAL PATCH 34 (MASPSX_FILL_JUMP_SYMBOL_STORE)."""

    @staticmethod
    def process(lines, enabled):
        with patch.dict(os.environ, {"MASPSX_FILL_JUMP_SYMBOL_STORE": "0"}):
            return MaspsxProcessor(
                lines, fill_jump_symbol_store=enabled
            ).process_lines()

    def test_indexed_store_moves_into_label_jump_slot(self):
        # ROM func_80076C34 0x80076E04:
        #   lui $at,0x800C / addu $at,$at,$a0 / j .L80076E38 / sw $v0,%lo(..)($at)
        lines = ["sw\t$2,D_800BD030+4($4)", "j\t$L18", "$L13:", "nop"]
        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)
        self.assertNotIn("# FILL_JUMP_SYMBOL_STORE START", disabled)
        start = enabled.index("# FILL_JUMP_SYMBOL_STORE START")
        body = enabled[start + 1:enabled.index("# FILL_JUMP_SYMBOL_STORE END")]
        self.assertEqual(
            body,
            [
                ".set\tnoat",
                "lui\t$at,%hi(D_800BD030+4)",
                "addu\t$at,$at,$4",
                "j\t$L18",
                "sw\t$2,%lo(D_800BD030+4)($at)",
                ".set\tat",
            ],
        )
        # the jump is consumed, not emitted a second time with a nop slot
        self.assertEqual(sum(1 for x in enabled if x.startswith("j\t")), 1)

    def test_flag_off_identical_to_stock(self):
        lines = ["sw\t$2,D_800BD030+4($4)", "j\t$L18", "$L13:", "nop"]
        with patch.dict(os.environ, {"MASPSX_FILL_JUMP_SYMBOL_STORE": "0"}):
            stock = MaspsxProcessor(lines).process_lines()
        self.assertEqual(stock, self.process(lines, enabled=False))

    def test_return_jump_left_to_other_patches(self):
        lines = ["sw\t$2,D_800BD030+4($4)", "j\t$31"]
        self.assertEqual(
            self.process(lines, enabled=True), self.process(lines, enabled=False)
        )

    def test_label_between_store_and_jump_refused(self):
        lines = ["sw\t$2,D_800BD030($4)", "$L5:", "j\t$L18"]
        self.assertEqual(
            self.process(lines, enabled=True), self.process(lines, enabled=False)
        )

    def test_call_refused(self):
        lines = ["sw\t$2,D_800BD030($4)", "jal\tfunc_80001234"]
        self.assertEqual(
            self.process(lines, enabled=True), self.process(lines, enabled=False)
        )

    def test_numeric_offset_refused(self):
        lines = ["sw\t$2,16($4)", "j\t$L18"]
        self.assertEqual(
            self.process(lines, enabled=True), self.process(lines, enabled=False)
        )

    def test_load_refused(self):
        lines = ["lw\t$2,D_800BD030($4)", "j\t$L18"]
        self.assertEqual(
            self.process(lines, enabled=True), self.process(lines, enabled=False)
        )

    def test_noreorder_block_refused(self):
        lines = [
            ".set\tnoreorder",
            "sw\t$2,D_800BD030($4)",
            "j\t$L18",
            "nop",
            ".set\treorder",
        ]
        self.assertEqual(
            self.process(lines, enabled=True), self.process(lines, enabled=False)
        )


if __name__ == "__main__":
    unittest.main()

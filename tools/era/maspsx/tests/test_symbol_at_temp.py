import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestSymbolAtTemp(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        # Keep the environment from leaking the production opt-in into tests.
        with patch.dict(
            os.environ,
            {"MASPSX_SYMBOL_AT_TEMP": "0", "MASPSX_SYMBOL_LOAD_DEST_TEMP": "0"},
        ):
            return MaspsxProcessor(
                lines, symbol_at_temp=enabled, addiu_at=True
            ).process_lines()

    def test_indexed_load_uses_at_temp_with_lo_displacement(self):
        lines = ["lbu\t$2,D_800A0ED4($2)"]

        enabled = self.process(lines, enabled=True)

        self.assertIn("lui\t$at,%hi(D_800A0ED4)", enabled)
        self.assertIn("addu\t$at,$at,$2", enabled)
        self.assertIn("lbu\t$2,%lo(D_800A0ED4)($at)", enabled)
        self.assertIn("# SYMBOL_AT_TEMP START", enabled)

    def test_gate_off_keeps_legacy_four_word_expansion(self):
        lines = ["lbu\t$2,D_800A0ED4($2)"]

        disabled = self.process(lines, enabled=False)

        self.assertIn("addiu\t$at,$at,%lo(D_800A0ED4)", disabled)
        self.assertIn("lbu\t$2,0x0($at)", disabled)
        self.assertNotIn("# SYMBOL_AT_TEMP START", disabled)

    def test_indexed_store_uses_at_temp(self):
        lines = ["sb\t$4,D_800A3348($2)"]

        enabled = self.process(lines, enabled=True)

        self.assertIn("lui\t$at,%hi(D_800A3348)", enabled)
        self.assertIn("sb\t$4,%lo(D_800A3348)($at)", enabled)
        self.assertIn("# SYMBOL_AT_TEMP STORE START", enabled)

    def test_numeric_operand_is_not_rewritten(self):
        lines = ["lw\t$2,0x8000($4)"]

        enabled = self.process(lines, enabled=True)

        self.assertNotIn("# SYMBOL_AT_TEMP START", enabled)

    def test_compound_macro_keeps_legacy_path(self):
        lines = ["lbu\t$2,0($4); nop"]

        enabled = self.process(lines, enabled=True)

        self.assertNotIn("# SYMBOL_AT_TEMP START", enabled)

    def test_comma_list_rewrites_only_named_symbols(self):
        lines = [
            "lw\t$2,jtbl_80011388($2)",
            "lw\t$4,D_800B0E38($2)",
        ]
        with patch.dict(
            os.environ,
            {
                "MASPSX_SYMBOL_AT_TEMP": "jtbl_80011388",
                "MASPSX_SYMBOL_LOAD_DEST_TEMP": "0",
            },
        ):
            out = MaspsxProcessor(lines, addiu_at=True).process_lines()
        self.assertIn("# SYMBOL_AT_TEMP START", out)
        self.assertIn("lw\t$2,%lo(jtbl_80011388)($at)", out)
        self.assertIn("addiu\t$at,$at,%lo(D_800B0E38)", out)
        self.assertIn("lw\t$4,0x0($at)", out)


if __name__ == "__main__":
    unittest.main()

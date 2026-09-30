import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestSymbolLoadDestTemp(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variable set.
        with patch.dict(os.environ, {"MASPSX_SYMBOL_LOAD_DEST_TEMP": "0"}):
            return MaspsxProcessor(
                lines, symbol_load_dest_temp=enabled, addiu_at=True
            ).process_lines()

    def test_compound_load_uses_dest_register_as_temp(self):
        lines = ["lbu\t$2,D_800A3348($4)"]

        enabled = self.process(lines, enabled=True)

        self.assertIn("lui\t$2,%hi(D_800A3348)", enabled)
        self.assertIn("addu\t$2,$2,$4", enabled)
        self.assertIn("lbu\t$2,%lo(D_800A3348)($2)", enabled)
        self.assertIn("# DEST_TEMP START", enabled)

    def test_gate_off_keeps_legacy_at_expansion(self):
        lines = ["lbu\t$2,D_800A3348($4)"]

        disabled = self.process(lines, enabled=False)

        self.assertIn("lui\t$at,%hi(D_800A3348)", disabled)
        self.assertIn("addu\t$at,$at,$4", disabled)
        self.assertNotIn("$2,$2,$4", disabled)
        self.assertNotIn("# DEST_TEMP START", disabled)

    def test_dest_equal_to_index_falls_back_to_at(self):
        # func_80074A44: `lhu $v0,D_800957D8($v0)` -- the destination IS the
        # index, so it cannot be the address temp (the `lui $2` would clobber
        # the index before the `addu` read it).  GNU as and retail use $at.
        lines = ["lhu\t$2,D_800957D8($2)"]

        enabled = self.process(lines, enabled=True)

        self.assertNotIn("# DEST_TEMP START", enabled)
        self.assertNotIn("lui\t$2,%hi(D_800957D8)", enabled)
        self.assertNotIn("addu\t$2,$2,$2", enabled)
        self.assertIn("lui\t$at,%hi(D_800957D8)", enabled)
        self.assertTrue(any(line.startswith("lhu\t$2,") and "($at)" in line
                            for line in enabled), enabled)

    def test_dest_equal_to_index_alias_spelling_falls_back_to_at(self):
        # Canonical register compare: `$v0` and `$2` are the same register.
        lines = ["lhu\t$v0,D_800957D8($2)"]

        enabled = self.process(lines, enabled=True)

        self.assertNotIn("# DEST_TEMP START", enabled)

    def test_dest_distinct_from_index_still_uses_dest_temp(self):
        # Same function, the other load: `lhu $v1,D_800957CC($v0)` keeps the
        # dest-temp form.
        lines = ["lhu\t$3,D_800957CC($2)"]

        enabled = self.process(lines, enabled=True)

        self.assertIn("# DEST_TEMP START", enabled)
        self.assertIn("lui\t$3,%hi(D_800957CC)", enabled)
        self.assertIn("addu\t$3,$3,$2", enabled)
        self.assertIn("lhu\t$3,%lo(D_800957CC)($3)", enabled)

    def test_numeric_operand_is_not_rewritten(self):
        lines = ["lw\t$2,0x8000($4)"]

        enabled = self.process(lines, enabled=True)

        self.assertNotIn("# DEST_TEMP START", enabled)

    def test_compound_macro_keeps_legacy_path(self):
        lines = ["lbu\t$2,0($4); nop"]

        enabled = self.process(lines, enabled=True)

        self.assertNotIn("$2,$2,$4", enabled)

    def test_indexed_store_before_return_fills_delay_slot(self):
        lines = [
            "sb\t$4,D_800A3348($2)",
            "j\t$31",
        ]

        enabled = self.process(lines, enabled=True)

        self.assertIn("lui\t$at,%hi(D_800A3348)", enabled)
        self.assertIn("addu\t$at,$at,$2", enabled)
        self.assertIn("j\t$31", enabled)
        self.assertIn("sb\t$4,%lo(D_800A3348)($at)", enabled)
        self.assertIn("# DEST_TEMP STORE_FILL START", enabled)

    def test_store_not_before_return_keeps_legacy(self):
        lines = ["sb\t$4,D_800A3348($2)"]

        enabled = self.process(lines, enabled=True)

        self.assertNotIn("# DEST_TEMP STORE_FILL START", enabled)
        self.assertIn("addiu\t$at,$at,%lo(D_800A3348)", enabled)


if __name__ == "__main__":
    unittest.main()

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

# K5 / MASPSX_NUMERIC_LOAD_ASPSX: cc1 2.8.1 -mno-split-addresses emits an
# indexed load from a numeric hardware-register address outside int16
# (`*(volatile u32 *)(0x1F801088 + (ch << 4))`). Retail ASPSX (func_8007CEAC)
# uses the destination register as the address temp when it differs from the
# index (0x8007CEE4, 0x8007CF08) and `$at` in ASPSX operand order when the
# destination is the index (0x8007CF4C).


class TestNumericLoadAspsx(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        env = {"MASPSX_NUMERIC_LOAD_ASPSX": "1" if enabled else "0"}
        with patch.dict(os.environ, env):
            return MaspsxProcessor(list(lines)).process_lines()

    def test_dest_register_is_temp(self):
        out = self.process(["lw\t$2,528486536($5)"], True)
        self.assertIn("lui\t$2,%hi(528486536)", out)
        self.assertIn("addu\t$2,$2,$5", out)
        self.assertIn("lw\t$2,%lo(528486536)($2)", out)
        self.assertFalse(any("$at" in l and not l.startswith(".set") for l in out), out)

    def test_dest_equal_index_uses_at_aspsx_order(self):
        out = self.process(["lw\t$5,528486536($5)"], True)
        self.assertIn("lui\t$at,%hi(528486536)", out)
        self.assertIn("addu\t$at,$at,$5", out)
        self.assertIn("lw\t$5,%lo(528486536)($at)", out)

    def test_gate_off_keeps_gnu_as_order(self):
        out = self.process(["lw\t$2,528486536($5)"], False)
        self.assertIn("lui\t$at,%hi(528486536)", out)
        self.assertIn("addu\t$at,$5,$at", out)
        self.assertIn("lw\t$2,%lo(528486536)($at)", out)
        self.assertNotIn("# NUMERIC_LOAD_ASPSX START", out)

    def test_gate_off_is_identical_to_unset(self):
        lines = ["lw\t$2,528486536($5)", "lbu\t$3,-40000($4)", "lw\t$5,528486536($5)"]
        off = self.process(lines, False)
        with patch.dict(os.environ, {}, clear=False):
            os.environ.pop("MASPSX_NUMERIC_LOAD_ASPSX", None)
            unset = MaspsxProcessor(list(lines)).process_lines()
        self.assertEqual(off, unset)

    def test_small_offset_untouched(self):
        on = self.process(["lw\t$2,16($5)"], True)
        off = self.process(["lw\t$2,16($5)"], False)
        self.assertEqual(on, off)


if __name__ == "__main__":
    unittest.main()

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestSpillBeforeSymbolLoad(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        with patch.dict(os.environ, {"MASPSX_SPILL_BEFORE_SYMBOL_LOAD": "0"}):
            return MaspsxProcessor(
                lines, spill_before_symbol_load=enabled
            ).process_lines()

    def test_swaps_independent_spill_ahead_of_symbolic_load(self):
        lines = [
            "lw\t$2,D_8009B740",
            "sw\t$19,28($sp)",
            "sw\t$31,32($sp)",
        ]
        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)
        self.assertEqual(disabled[0].split()[0], "lw")
        # The save is now the first instruction; the load follows it.
        enabled_ops = [ln.split()[0] for ln in enabled if ln.split() and ln.split()[0] in ("lw", "sw", "lui")]
        self.assertEqual(enabled_ops[0], "sw")
        self.assertIn("sw\t$19,28($sp)", enabled)

    def test_does_not_swap_when_load_uses_the_spilled_register(self):
        lines = [
            "lw\t$19,D_8009B740",
            "sw\t$19,28($sp)",
        ]
        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_does_not_swap_an_indexed_load(self):
        lines = [
            "lw\t$2,0($3)",
            "sw\t$19,28($sp)",
        ]
        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_flag_off_is_identity(self):
        lines = [
            "lw\t$2,D_8009B740",
            "sw\t$19,28($sp)",
            "j\t$31",
        ]
        self.assertEqual(
            self.process(lines, enabled=False),
            MaspsxProcessor(lines).process_lines(),
        )

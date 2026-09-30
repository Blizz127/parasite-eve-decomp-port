import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestFillStoreDelaySlotGuards(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variable set.
        with patch.dict(os.environ, {"MASPSX_FILL_STORE_DELAY_SLOT": "0"}):
            return MaspsxProcessor(
                lines, fill_store_delay_slot=enabled
            ).process_lines()

    def test_immediate_return_jump_is_filled(self):
        lines = [
            "sw\t$4,D_800A36A0",
            "j\t$31",
            "#nop",
        ]

        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)

        self.assertNotEqual(disabled, enabled)
        self.assertIn("# FILL_STORE_DELAY_SLOT START", enabled)

    def test_label_blocks_fill(self):
        lines = [
            "sw\t$4,D_800A36A0",
            "$L1:",
            "j\t$31",
            "#nop",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_set_directive_blocks_fill(self):
        lines = [
            "sw\t$4,D_800A36A0",
            ".set\tnoreorder",
            "j\t$31",
            "sw\t$4,D_800A36A0",
            ".set\treorder",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )


if __name__ == "__main__":
    unittest.main()

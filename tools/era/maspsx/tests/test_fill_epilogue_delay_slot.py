import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestFillEpilogueDelaySlotGuards(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variable set.
        with patch.dict(os.environ, {"MASPSX_FILL_EPILOGUE_DELAY_SLOT": "0"}):
            return MaspsxProcessor(
                lines, fill_epilogue_delay_slot=enabled
            ).process_lines()

    def test_immediate_return_jump_is_filled(self):
        lines = [
            "addu\t$sp,$sp,32",
            "j\t$31",
        ]

        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)

        self.assertNotEqual(disabled, enabled)
        self.assertIn("# FILL_EPILOGUE_DELAY_SLOT START", enabled)
        self.assertIn("addiu\t$sp,$sp,32", enabled)

    def test_addiu_form_is_filled(self):
        lines = [
            "addiu\t$sp,$sp,24",
            "j\t$31",
        ]

        enabled = self.process(lines, enabled=True)
        self.assertIn("# FILL_EPILOGUE_DELAY_SLOT START", enabled)
        self.assertIn("addiu\t$sp,$sp,24", enabled)

    def test_nonadjacent_return_blocks_fill(self):
        lines = [
            "addu\t$sp,$sp,32",
            "addiu\t$2,$2,1",
            "j\t$31",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_label_blocks_fill(self):
        lines = [
            "addu\t$sp,$sp,32",
            "$L1:",
            "j\t$31",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_set_directive_blocks_fill(self):
        lines = [
            "addu\t$sp,$sp,32",
            ".set\tnoreorder",
            "j\t$31",
            "addu\t$sp,$sp,32",
            ".set\treorder",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )


if __name__ == "__main__":
    unittest.main()

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_80076664 / func_800768A0: GPU status poll through *D_80095854.
LINES = [
    ".extern\tD_80095854",
    "move\t$20,$3",
    "lw\t$2,D_80095854",
    "li\t$3,67108864\t\t\t# 0x4000000",
    "lw\t$2,0($2)",
    "and\t$2,$2,$3",
    "jr\t$31",
]


class TestSymbolPointerLoadLiAfter(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_SYMBOL_POINTER_LOAD_LI_AFTER": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_reorders(self):
        got = [x for x in self.process(LINES, True) if x != "nop"]
        self.assertEqual(
            [
                "lw\t$2,D_80095854",
                "addu\t$20,$3,$zero",
                "lw\t$2,0($2)",
                "li\t$3,67108864",
            ],
            got[:4],
        )

    def test_without_move(self):
        got = [x for x in self.process(LINES[:1] + LINES[2:], True) if x != "nop"]
        self.assertEqual(
            ["lw\t$2,D_80095854", "lw\t$2,0($2)", "li\t$3,67108864"], got[:3]
        )

    def test_flag_off(self):
        got = [x for x in self.process(LINES, False) if x != "nop"]
        self.assertEqual(
            ["addu\t$20,$3,$zero", "lw\t$2,D_80095854", "li\t$3,67108864", "lw\t$2,0($2)"],
            got[:4],
        )

import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_80072A74 (__adddf3): `return ub.d` -> move $2 / j / move $3 in slot.
LINES = ["move\t$2,$16", "move\t$3,$17", "j\t$L25", "$L25:", "jr\t$31"]


def insns(out):
    return [x for x in strip_comments(out) if x and not x.startswith(".")]


class TestFillJumpPrecedingMove(unittest.TestCase):
    @staticmethod
    def process(on):
        env = {"MASPSX_FILL_JUMP_PRECEDING_MOVE": "1" if on else "0"}
        with patch.dict(os.environ, env):
            return insns(MaspsxProcessor(LINES).process_lines())

    def test_fills(self):
        self.assertEqual(
            ["addu\t$2,$16,$zero", "j\t$L25", "addu\t$3,$17,$zero"],
            self.process(True)[:3],
        )

    def test_flag_off(self):
        self.assertEqual("nop", self.process(False)[3])

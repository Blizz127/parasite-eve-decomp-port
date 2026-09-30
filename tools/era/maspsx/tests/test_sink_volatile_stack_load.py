import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_800735C4 (__muldf3): the volatile exponent local stands in for a
# spilled pseudo; retail reloads it between the two argument-half loads.
LINES = [
    "li\t$5,0x00000001",
    "#.set\tvolatile",
    "lw\t$11,80($sp)",
    "#.set\tnovolatile",
    "li\t$2,0x0000000a",
    "sw\t$2,16($sp)",
    "lw\t$6,24($sp)",
    "lw\t$7,28($sp)",
    "addu\t$23,$11,-1023",
    "jal\tf",
]


def insns(out):
    return [x for x in strip_comments(out) if x and not x.startswith(".")]


class TestSinkVolatileStackLoad(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_SINK_VOLATILE_STACK_LOAD": "1" if on else "0"}
        with patch.dict(os.environ, env):
            return insns(MaspsxProcessor(lines).process_lines())

    def test_sinks(self):
        got = self.process(LINES, True)
        i = got.index("lw\t$6,24($sp)")
        self.assertEqual(
            ["lw\t$6,24($sp)", "lw\t$11,80($sp)", "lw\t$7,28($sp)", "addu\t$23,$11,-1023"],
            got[i:i + 4],
        )

    def test_flag_off(self):
        got = self.process(LINES, False)
        self.assertEqual("lw\t$11,80($sp)", got[1])

    def test_store_to_same_slot_blocks(self):
        lines = list(LINES)
        lines[5] = "sw\t$2,80($sp)"
        got = self.process(lines, True)
        self.assertEqual("lw\t$11,80($sp)", got[1])

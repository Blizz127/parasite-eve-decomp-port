import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_8007F0C8: packed-int copy; retail (ASPSX) keeps the load-delay nop
# between the lwl/lwr pair and the swl/swr pair.
LINES = [
    "ulw\t$2,0($18)",
    "#nop",
    "usw\t$2,49($sp)",
    "jr\t$31",
]


class TestUlwLoadDelayNop(unittest.TestCase):
    @staticmethod
    def process(lines, on):
        env = {"MASPSX_ULW_LOAD_DELAY_NOP": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(lines).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_keeps_real_nop(self):
        got = self.process(LINES, True)
        self.assertEqual(["ulw\t$2,0($18)", "nop", "usw\t$2,49($sp)"], got[:3])

    def test_flag_off(self):
        got = self.process(LINES, False)
        self.assertEqual(["ulw\t$2,0($18)", "usw\t$2,49($sp)"], got[:2])

    def test_needs_nop_comment(self):
        got = self.process(["ulw\t$2,0($18)", "usw\t$2,49($sp)", "jr\t$31"], True)
        self.assertNotIn("nop", got)

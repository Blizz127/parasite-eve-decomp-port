import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# func_80072A74 / func_80072F64: the load-delay nop of `lw $18` belongs
# before the join labels, so branches to them skip it.
LINES = [
    "or\t$19,$2,$3",
    "lw\t$18,24($sp)",
    "$L24:",
    "$L26:",
    "#nop",
    "move\t$2,$18",
    "jr\t$31",
]


class TestLoadDelayNopBeforeAnyLabel(unittest.TestCase):
    @staticmethod
    def process(on):
        env = {"MASPSX_LOAD_DELAY_NOP_BEFORE_ANY_LABEL": "1" if on else "0"}
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(LINES).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_nop_before_labels(self):
        got = self.process(True)
        i = got.index("lw\t$18,24($sp)")
        self.assertEqual("nop", got[i + 1])
        self.assertEqual("$L24:", got[i + 2])

    def test_flag_off(self):
        got = self.process(False)
        i = got.index("lw\t$18,24($sp)")
        self.assertEqual("$L24:", got[i + 1])

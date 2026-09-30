import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# cc1 2.8.1 shape of func_8006AD40's entry and epilogue: the flag test's
# delay is the fall-through symbolic load, and `move $2,$0` sits in front
# of the noreorder return.
ENTRY = [
    "lw\t$2,0($21)",
    "andi\t$2,$2,0x0001",
    "beq\t$2,$0,$L2",
    "lw\t$22,D_800B0DD8",
    "$L3:",
    "lw\t$31,44($sp)",
    "lw\t$16,16($sp)",
    "$L2:",
    "move\t$2,$0",
    ".set\tnoreorder",
    ".set\tnomacro",
    "j\t$31",
    "addu\t$sp,$sp,48",
    ".set\tmacro",
    ".set\treorder",
]


class TestSinkReturnZeroIntoFlagDelay(unittest.TestCase):
    @staticmethod
    def process(lines, sink=True, unfill=False):
        env = {
            "MASPSX_SINK_RETURN_ZERO_INTO_FLAG_DELAY": "0",
            "MASPSX_UNFILL_EPILOGUE_DELAY_SLOT": "0",
        }
        with patch.dict(os.environ, env):
            out = MaspsxProcessor(
                lines,
                sink_return_zero_into_flag_delay=sink,
                unfill_epilogue_delay_slot=unfill,
            ).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_positive_moves_zero_into_delay_and_keeps_load(self):
        got = self.process(ENTRY)
        self.assertEqual(
            [
                "beq\t$2,$0,$L2",
                "addu\t$2,$0,$zero",
                "lw\t$22,D_800B0DD8",
            ],
            got[got.index("beq\t$2,$0,$L2"):got.index("beq\t$2,$0,$L2") + 3],
        )
        self.assertLess(got.index("addu\t$2,$0,$zero"), got.index("$L3:"))
        self.assertEqual(got[-2:], ["j\t$31", "addu\t$sp,$sp,48"])

    def test_positive_with_unfill_pops_stack_before_return(self):
        got = self.process(ENTRY, unfill=True)
        self.assertEqual(
            ["beq\t$2,$0,$L2", "addu\t$2,$0,$zero", "lw\t$22,D_800B0DD8"],
            got[got.index("beq\t$2,$0,$L2"):got.index("beq\t$2,$0,$L2") + 3],
        )
        self.assertEqual(got[-3:], ["addu\t$sp,$sp,48", "j\t$31", "nop"])
        self.assertEqual(got.count("addu\t$2,$0,$zero"), 1)

    def test_positive_environment_flag(self):
        with patch.dict(
            os.environ,
            {
                "MASPSX_SINK_RETURN_ZERO_INTO_FLAG_DELAY": "1",
                "MASPSX_UNFILL_EPILOGUE_DELAY_SLOT": "0",
            },
        ):
            out = MaspsxProcessor(ENTRY).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual(self.process(ENTRY), res)

    def test_negative_flag_off(self):
        off = self.process(ENTRY, sink=False)
        beq = off.index("beq\t$2,$0,$L2")
        self.assertNotEqual(off[beq + 1], "addu\t$2,$0,$zero")
        self.assertIn("addu\t$2,$0,$zero", off[-4:])

    def test_negative_two_branches_to_the_epilogue(self):
        lines = ["beq\t$2,$0,$L2"] + ENTRY
        self.assertEqual(self.process(lines, sink=False), self.process(lines))

    def test_negative_idempotent_zero_stays_single(self):
        with patch.dict(
            os.environ,
            {
                "MASPSX_SINK_RETURN_ZERO_INTO_FLAG_DELAY": "0",
                "MASPSX_UNFILL_EPILOGUE_DELAY_SLOT": "0",
            },
        ):
            proc = MaspsxProcessor(ENTRY, sink_return_zero_into_flag_delay=True)
            proc.process_lines()
            text = "\n".join(proc.lines)
            self.assertEqual(text.count("move\t$2,$0") + text.count("addu\t$2,$0,$zero"), 1)

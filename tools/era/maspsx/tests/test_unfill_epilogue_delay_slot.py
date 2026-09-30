import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# cc1 2.8.1 epilogue of func_8007D614 (ra-only frame)
EPILOGUE = [
    "$L10:",
    "lw\t$31,16($sp)",
    "#nop",
    ".set\tnoreorder",
    ".set\tnomacro",
    "j\t$31",
    "addu\t$sp,$sp,24",
    ".set\tmacro",
    ".set\treorder",
    "",
    ".end\tfunc_8007D614",
]


class TestUnfillEpilogueDelaySlot(unittest.TestCase):
    """Patch 15: MASPSX_UNFILL_EPILOGUE_DELAY_SLOT.

    Retail func_8007D614 / func_80077404 (ra-only frames, 2.8.1 band):
    `lw $ra` / `addiu $sp` / `jr $ra` / nop; gcc-2.8.1-psx emits
    `lw $ra` / nop / `jr $ra` / `addiu $sp`.
    """

    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_UNFILL_EPILOGUE_DELAY_SLOT": "0"}):
            out = MaspsxProcessor(lines, unfill_epilogue_delay_slot=on).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_positive_ra_only_epilogue(self):
        self.assertEqual(
            ["$L10:", "lw\t$31,16($sp)", "addu\t$sp,$sp,24", "j\t$31", "nop"],
            self.process(EPILOGUE),
        )

    def test_positive_addiu_spelling(self):
        lines = [x.replace("addu\t$sp,$sp,24", "addiu\t$sp,$sp,24") for x in EPILOGUE]
        self.assertEqual(
            ["$L10:", "lw\t$31,16($sp)", "addiu\t$sp,$sp,24", "j\t$31", "nop"],
            self.process(lines),
        )

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_UNFILL_EPILOGUE_DELAY_SLOT": "1"}):
            out = MaspsxProcessor(EPILOGUE).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual(["$L10:", "lw\t$31,16($sp)", "addu\t$sp,$sp,24", "j\t$31", "nop"], res)

    def test_positive_idempotent_on_reprocess(self):
        proc = MaspsxProcessor(EPILOGUE, unfill_epilogue_delay_slot=True)
        self.assertEqual(proc.process_lines(), proc.process_lines())

    def test_negative_flag_off_keeps_cc1_fill(self):
        self.assertEqual(
            ["$L10:", "lw\t$31,16($sp)", "nop", "j\t$31", "addu\t$sp,$sp,24"],
            self.process(EPILOGUE, on=False),
        )

    def test_negative_other_slot_instruction(self):
        lines = [x.replace("addu\t$sp,$sp,24", "move\t$2,$16") for x in EPILOGUE]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_non_return_jump(self):
        lines = [x.replace("j\t$31", "j\t$L3") for x in EPILOGUE]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_label_inside_block(self):
        lines = EPILOGUE[:5] + ["$L11:"] + EPILOGUE[5:]
        self.assertEqual(self.process(lines, on=False), self.process(lines))


if __name__ == "__main__":
    unittest.main()

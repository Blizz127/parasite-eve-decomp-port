import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


def function(body, frame="0", mask="0x00000000"):
    return [
        ".ent\tf",
        "f:",
        f".frame\t$sp,{frame},$31",
        f".mask\t{mask},0",
        *body,
        ".end\tf",
    ]


# cc1 2.8.1 shape of func_80072334 (memmove), trimmed to the two exits.
MID_RETURN = [
    "bgtz\t$3,$L5",
    ".set\tnoreorder",
    ".set\tnomacro",
    "j\t$31",
    "move\t$2,$7",
    ".set\tmacro",
    ".set\treorder",
    "$L7:",
    "move\t$2,$7",
    "j\t$31",
]


class TestReturnViaEpilogueJump(unittest.TestCase):
    """Patch 12: MASPSX_RETURN_VIA_EPILOGUE_JUMP.

    Retail func_80072334 0x80072368: `j .L80072398` / `addu $v0,$a3,$zero`
    where .L80072398 is the final `jr $ra`; gcc-2.8.1-psx emits a second
    return insn there instead.
    """

    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_RETURN_VIA_EPILOGUE_JUMP": "0"}):
            out = MaspsxProcessor(lines, return_via_epilogue_jump=on).process_lines()
        return [x for x in strip_comments(out) if x]

    def jumps(self, res):
        return [x for x in res if x.startswith("j\t")]

    def test_positive_mid_return_retargeted_to_final_return(self):
        res = self.process(function(MID_RETURN))
        self.assertEqual(["j\t$L900000", "j\t$31"], self.jumps(res))
        final = res.index("j\t$31")
        self.assertEqual("$L900000:", res[final - 1])
        # the stolen slot insn stays in the retargeted jump's slot
        mid = res.index("j\t$L900000")
        self.assertEqual("addu\t$2,$7,$zero", res[mid + 1])

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_RETURN_VIA_EPILOGUE_JUMP": "1"}):
            out = MaspsxProcessor(function(MID_RETURN)).process_lines()
        self.assertIn("j\t$L900000", strip_comments(out))

    def test_positive_idempotent_on_reprocess(self):
        proc = MaspsxProcessor(function(MID_RETURN), return_via_epilogue_jump=True)
        first = proc.process_lines()
        second = proc.process_lines()
        self.assertEqual(first, second)

    def test_negative_flag_off_is_unchanged(self):
        res = self.process(function(MID_RETURN), on=False)
        self.assertEqual(["j\t$31", "j\t$31"], self.jumps(res))
        self.assertNotIn("$L900000:", res)

    def test_negative_function_with_frame(self):
        # With a frame, an earlier `j $31` is a duplicated epilogue; jumping
        # to the final one would skip its restores.
        res = self.process(function(MID_RETURN, frame="24", mask="0x80000000"))
        self.assertEqual(["j\t$31", "j\t$31"], self.jumps(res))

    def test_negative_single_return_untouched(self):
        body = ["move\t$2,$4", "j\t$31"]
        self.assertEqual(
            self.process(function(body), on=False), self.process(function(body))
        )

    def test_negative_reorder_mode_mid_return_untouched(self):
        # Not opened by a cc1 noreorder block: not the reorg-produced shape.
        body = ["move\t$2,$4", "j\t$31", "$L3:", "move\t$2,$0", "j\t$31"]
        self.assertEqual(["j\t$31", "j\t$31"], self.jumps(self.process(function(body))))


if __name__ == "__main__":
    unittest.main()

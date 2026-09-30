"""LOCAL PATCH K3: MASPSX_NARROW_SHIFTED_WORD_LOAD.

`lw $d,SYM($b)` ... `sra $d,$d,N` (16 <= N <= 31) becomes
`lh $d,SYM+2($b)` ... `sra $d,$d,N-16`. Test window: room_m0141i
func_801911F0's sin read of the Psy-Q table D_800966EC (retail
`lh %lo(D_800966EE)($at)` / `sra $v0,$v0,5`).
"""
import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


def window(shift="21", mid="sw\t$6,36($sp)"):
    return [
        ".set\treorder",
        "sll\t$2,$16,9",
        "andi\t$2,$2,0x3e00",
        "lw\t$3,D_800966EC($2)",
        "la\t$2,D_80192120",
        mid,
        f"sra\t$3,$3,{shift}",
        "jal\tfunc_800CEE20",
        "sw\t$3,28($sp)",
    ]


def kept(lines):
    return [x for x in strip_comments(lines)
            if x and not x.startswith(".") and not x.endswith(":")]


class TestNarrowShiftedWordLoad(unittest.TestCase):
    @staticmethod
    def process(lines, on, **kw):
        env = {"MASPSX_NARROW_SHIFTED_WORD_LOAD": "1" if on else "0"}
        env.update(kw)
        with patch.dict(os.environ, env):
            return MaspsxProcessor(list(lines)).process_lines()

    def test_narrows_load_and_shift(self):
        text = "\n".join(self.process(window(), True))
        self.assertIn("%hi(D_800966EC+2)", text)
        self.assertIn("lh\t$3,", text)
        self.assertNotIn("lw\t$3,", text)
        self.assertIn("sra\t$3,$3,5", text)
        self.assertNotIn("sra\t$3,$3,21", text)

    def test_retail_three_word_form_with_symbol_at_temp(self):
        body = kept(self.process(window(), True, MASPSX_SYMBOL_AT_TEMP="1"))
        i = body.index("lui\t$at,%hi(D_800966EC+2)")
        self.assertEqual(body[i + 1], "addu\t$at,$at,$2")
        self.assertEqual(body[i + 2], "lh\t$3,%lo(D_800966EC+2)($at)")

    def test_shift_of_16_is_dropped(self):
        body = kept(self.process(window("16"), True))
        self.assertFalse(any(x.startswith("sra\t$3") for x in body))
        self.assertTrue(any(x.startswith("lh\t$3,") for x in body))

    def test_existing_offset_is_bumped(self):
        lines = [x.replace("D_800966EC($2)", "D_800966EC+8($2)") for x in window()]
        text = "\n".join(self.process(lines, True))
        self.assertIn("D_800966EC+10", text)

    def test_flag_off_is_identity_with_default(self):
        off = self.process(window(), False)
        with patch.dict(os.environ, {}, clear=False):
            os.environ.pop("MASPSX_NARROW_SHIFTED_WORD_LOAD", None)
            default = MaspsxProcessor(list(window())).process_lines()
        self.assertEqual(off, default)
        self.assertIn("sra\t$3,$3,21", "\n".join(off))

    def test_leaves_small_shift(self):
        text = "\n".join(self.process(window("15"), True))
        self.assertNotIn("lh\t$3,", text)
        self.assertIn("sra\t$3,$3,15", text)

    def test_blocked_when_dest_read_between(self):
        text = "\n".join(self.process(window(mid="sw\t$3,36($sp)"), True))
        self.assertNotIn("lh\t$3,", text)
        self.assertIn("sra\t$3,$3,21", text)

    def test_register_prefix_is_not_a_read(self):
        # $30 mentions "$3" as a prefix but is a different register.
        text = "\n".join(self.process(window(mid="addu\t$30,$30,1"), True))
        self.assertIn("sra\t$3,$3,5", text)

    def test_blocked_by_label_or_branch(self):
        for mid in ("$L9:", "beq\t$4,$0,$L9"):
            text = "\n".join(self.process(window(mid=mid), True))
            self.assertIn("sra\t$3,$3,21", text, mid)

    def test_full_word_use_is_left_alone(self):
        # The cos half of the same table is read as a full word (addiu).
        lines = ["lw\t$2,D_800966EC($2)", "addu\t$18,$2,512"]
        text = "\n".join(self.process(lines, True))
        self.assertNotIn("lh\t$2,", text)


if __name__ == "__main__":
    unittest.main()

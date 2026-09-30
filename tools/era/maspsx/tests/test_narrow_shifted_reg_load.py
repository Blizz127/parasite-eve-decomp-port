"""LOCAL PATCH K3b: MASPSX_NARROW_SHIFTED_REG_LOAD.

`lw $d,N($b)` ... `sra $d,$d,M` (16 <= M <= 31) becomes
`lh $d,N+2($b)` ... `sra $d,$d,M-16`. Test window: room_m0273i
func_8019A290's `*(int *)p >> 21` (retail `lh $v0,2($a2)` / `sra 5`).
The window may overwrite the base register after the load.
"""
import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


def window(shift="21", off="0", mid="addu\t$6,$0,192"):
    return [
        ".set\treorder",
        f"lw\t$2,{off}($6)",
        mid,
        "sw\t$8,40($sp)",
        f"sra\t$2,$2,{shift}",
        "jal\tfunc_800CEE20",
        "sw\t$2,28($sp)",
    ]


def kept(lines):
    return [x for x in strip_comments(lines)
            if x and not x.startswith(".") and not x.endswith(":")]


class TestNarrowShiftedRegLoad(unittest.TestCase):
    @staticmethod
    def process(lines, **env):
        base = {"MASPSX_NARROW_SHIFTED_REG_LOAD": "0",
                "MASPSX_NARROW_SHIFTED_WORD_LOAD": "0"}
        base.update(env)
        with patch.dict(os.environ, base):
            return MaspsxProcessor(list(lines)).process_lines()

    def test_narrows_register_base_load(self):
        body = kept(self.process(window(), MASPSX_NARROW_SHIFTED_REG_LOAD="1"))
        self.assertIn("lh\t$2,2($6)", body)
        self.assertIn("sra\t$2,$2,5", body)
        self.assertNotIn("lw\t$2,0($6)", body)

    def test_negative_and_positive_offsets(self):
        body = kept(self.process(window(off="-8"), MASPSX_NARROW_SHIFTED_REG_LOAD="1"))
        self.assertIn("lh\t$2,-6($6)", body)
        body = kept(self.process(window(off="12"), MASPSX_NARROW_SHIFTED_REG_LOAD="1"))
        self.assertIn("lh\t$2,14($6)", body)

    def test_shift_of_16_is_dropped(self):
        body = kept(self.process(window("16"), MASPSX_NARROW_SHIFTED_REG_LOAD="1"))
        self.assertIn("lh\t$2,2($6)", body)
        self.assertFalse(any(x.startswith("sra\t$2") for x in body))

    def test_small_shift_and_dest_use_are_left(self):
        body = kept(self.process(window("15"), MASPSX_NARROW_SHIFTED_REG_LOAD="1"))
        self.assertIn("lw\t$2,0($6)", body)
        body = kept(self.process(window(mid="addu\t$3,$2,1"),
                                 MASPSX_NARROW_SHIFTED_REG_LOAD="1"))
        self.assertIn("lw\t$2,0($6)", body)

    def test_k3_alone_does_not_touch_register_base(self):
        self.assertEqual(
            self.process(window(), MASPSX_NARROW_SHIFTED_WORD_LOAD="1"),
            self.process(window()),
        )

    def test_k3b_alone_does_not_touch_symbol_loads(self):
        lines = [x.replace("0($6)", "D_800966EC($6)") for x in window()]
        self.assertEqual(
            self.process(lines, MASPSX_NARROW_SHIFTED_REG_LOAD="1"),
            self.process(lines),
        )

    def test_flag_off_is_identity_with_default(self):
        off = self.process(window())
        saved = {k: os.environ.pop(k, None) for k in
                 ("MASPSX_NARROW_SHIFTED_REG_LOAD", "MASPSX_NARROW_SHIFTED_WORD_LOAD")}
        try:
            default = MaspsxProcessor(list(window())).process_lines()
        finally:
            for k, v in saved.items():
                if v is not None:
                    os.environ[k] = v
        self.assertEqual(off, default)
        self.assertIn("sra\t$2,$2,21", "\n".join(off))


if __name__ == "__main__":
    unittest.main()

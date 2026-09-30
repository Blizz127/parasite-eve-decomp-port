"""LOCAL PATCH K6: MASPSX_FILL_JUMP_REG_STORE.

Patch 11 (MASPSX_FILL_JUMP_STACK_STORE) for any register base: the
single-word store that immediately precedes a reorder-mode `j <label>`
moves into the jump's delay slot. cc1's `movstrsi` block move (strcpy of a
3-byte literal: lh/lb/sh/sb) is one insn, so reorg cannot split its last
store into the slot. ROM: func_80081A7C 0x80081BE8 `j` / `sb $v1,34($s5)`.
"""
import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


def window(store="sb\t$3,34($21)", between=""):
    lines = [
        ".set\treorder",
        "lh\t$2,$LC1",
        "lb\t$3,$LC1+2",
        "sh\t$2,32($21)",
        store,
    ]
    if between:
        lines.append(between)
    lines += ["j\t$L10", "$L12:", "lbu\t$6,32($16)", "$L10:", "j\t$31"]
    return lines


def kept(lines):
    return [x for x in strip_comments(lines)
            if x and not x.startswith(".") and not x.endswith(":")]


class TestFillJumpRegStore(unittest.TestCase):
    @staticmethod
    def process(lines, **env):
        base = {"MASPSX_FILL_JUMP_REG_STORE": "0",
                "MASPSX_FILL_JUMP_STACK_STORE": "0"}
        base.update(env)
        with patch.dict(os.environ, base):
            return MaspsxProcessor(list(lines)).process_lines()

    def test_register_base_store_fills_jump_slot(self):
        body = kept(self.process(window(), MASPSX_FILL_JUMP_REG_STORE="1"))
        i = body.index("j\t$L10")
        self.assertEqual(body[i + 1], "sb\t$3,34($21)")
        self.assertEqual(body[i - 1], "sh\t$2,32($21)")

    def test_flag_off_leaves_nop(self):
        body = kept(self.process(window()))
        i = body.index("j\t$L10")
        self.assertEqual(body[i - 1], "sb\t$3,34($21)")
        self.assertEqual(body[i + 1], "nop")

    def test_patch11_alone_still_refuses_register_base(self):
        self.assertEqual(
            self.process(window(), MASPSX_FILL_JUMP_STACK_STORE="1"),
            self.process(window()),
        )

    def test_stack_store_still_fills(self):
        body = kept(self.process(window("sw\t$0,16($sp)"),
                                 MASPSX_FILL_JUMP_REG_STORE="1"))
        i = body.index("j\t$L10")
        self.assertEqual(body[i + 1], "sw\t$0,16($sp)")

    def test_at_base_refused(self):
        body = kept(self.process(window("sw\t$2,0($at)"),
                                 MASPSX_FILL_JUMP_REG_STORE="1"))
        i = body.index("j\t$L10")
        self.assertEqual(body[i + 1], "nop")

    def test_label_between_refused(self):
        body = kept(self.process(window(between="$L99:"),
                                 MASPSX_FILL_JUMP_REG_STORE="1"))
        i = body.index("j\t$L10")
        self.assertEqual(body[i + 1], "nop")

    def test_flag_off_is_identity_with_default(self):
        off = self.process(window())
        saved = {k: os.environ.pop(k, None) for k in
                 ("MASPSX_FILL_JUMP_REG_STORE", "MASPSX_FILL_JUMP_STACK_STORE")}
        try:
            default = MaspsxProcessor(list(window())).process_lines()
        finally:
            for k, v in saved.items():
                if v is not None:
                    os.environ[k] = v
        self.assertEqual(off, default)


if __name__ == "__main__":
    unittest.main()

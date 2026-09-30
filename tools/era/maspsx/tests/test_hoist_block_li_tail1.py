"""LOCAL PATCH K4 tail-1: MASPSX_HOIST_BLOCK_LI_TAIL1=<sym,...>.

Inside a K4 region naming a listed symbol, constants 0..n-2 move before
reload 1 and constant n-1 directly after reload 1; reload 2 keeps its nop.
Retail func_8002BC90 0x8002C724 (D_800B0130 gauge block): five `li`,
`lw D_8009CDDC`, `li $4,1`, ..., `lw D_8009CDDC`, `nop`.
"""
import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

LOAD = "lw\t$3,D_8009CDDC"


def group(val, off, dst):
    return [
        "sll\t$2,$3,3",
        "addu\t$2,$2,$3",
        "sll\t$2,$2,3",
        f"sb\t{val},{dst}+{off}($2)",
    ]


def window(dst="D_800B0130"):
    lines = [".set\treorder", LOAD, "#nop"] + group("$0", 4, dst)
    for reg, k, off in (("$9", 255, 5), ("$8", 61, 6), ("$7", 129, 7),
                        ("$6", 131, 8), ("$5", 19, 9), ("$4", 1, 10)):
        lines += [LOAD, f"li\t{reg},{k}"] + group(reg, off, dst)
    lines += [LOAD, "#nop"] + group("$0", 20, dst)
    lines += ["j\t$31"]
    return lines


def kept(lines):
    return [x for x in strip_comments(lines)
            if x and not x.startswith(".") and not x.endswith(":")]


class TestHoistBlockLiTail1(unittest.TestCase):
    @staticmethod
    def process(lines, **env):
        base = {"MASPSX_HOIST_BLOCK_LI": "D_8009CDDC",
                "MASPSX_HOIST_BLOCK_LI_TAIL1": ""}
        base.update(env)
        with patch.dict(os.environ, base):
            return kept(MaspsxProcessor(list(lines)).process_lines())

    def test_tail1_order(self):
        body = self.process(window(), MASPSX_HOIST_BLOCK_LI_TAIL1="D_800B0130")
        self.assertEqual(body[:5], ["li\t$9,255", "li\t$8,61", "li\t$7,129",
                                    "li\t$6,131", "li\t$5,19"])
        loads = [i for i, x in enumerate(body) if x == LOAD]
        self.assertEqual(body[loads[0] + 1], "li\t$4,1")
        self.assertEqual(body[loads[1] + 1], "nop")
        self.assertEqual(sum(x.startswith("li\t") for x in body), 6)

    def test_default_k4_order_without_tail1(self):
        body = self.process(window())
        loads = [i for i, x in enumerate(body) if x == LOAD]
        self.assertEqual(body[loads[0] + 1], "li\t$5,19")
        self.assertEqual(body[loads[1] + 1], "li\t$4,1")

    def test_unlisted_symbol_keeps_default_order(self):
        self.assertEqual(
            self.process(window("D_800B00E8"), MASPSX_HOIST_BLOCK_LI_TAIL1="D_800B0130"),
            self.process(window("D_800B00E8")),
        )

    def test_symbol_prefix_does_not_match(self):
        self.assertEqual(
            self.process(window("D_800B01300"), MASPSX_HOIST_BLOCK_LI_TAIL1="D_800B0130"),
            self.process(window("D_800B01300")),
        )

    def test_tail1_without_k4_is_identity(self):
        self.assertEqual(
            self.process(window(), MASPSX_HOIST_BLOCK_LI="",
                         MASPSX_HOIST_BLOCK_LI_TAIL1="D_800B0130"),
            self.process(window(), MASPSX_HOIST_BLOCK_LI=""),
        )


if __name__ == "__main__":
    unittest.main()

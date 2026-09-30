"""LOCAL PATCH K4: MASPSX_HOIST_BLOCK_LI.

A straight-line run of store groups that each reload `lw $3,SYM` gets
its `li` constants re-placed: the first n-2 before reload 1, the last
two directly after reloads 1 and 2. Test window: colour block 1 of
func_800299CC (retail 0x80029B3C: li $8,70 / li $7,130 / li $6,159 /
lui+lw D_8009CDDC / li $5,255 / ... / lw / li $4,249).
"""
import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


def group(val, off):
    return [
        "sll\t$2,$3,3",
        "addu\t$2,$2,$3",
        "sll\t$2,$2,2",
        f"sb\t{val},D_800B00E8+{off}($2)",
    ]


def window(sym="D_8009CDDC"):
    lines = [".set\treorder", f"lw\t$3,{sym}", "#nop"] + group("$0", 4)
    for reg, k, off in (("$8", 70, 5), ("$7", 130, 6), ("$6", 159, 12),
                        ("$5", 255, 13), ("$4", 249, 14)):
        lines += [f"lw\t$3,{sym}", f"li\t{reg},{k}"] + group(reg, off)
    lines += [f"lw\t$3,{sym}", "#nop"] + group("$0", 20)
    lines += ["j\t$31"]
    return lines


LOAD = "lw\t$3,D_8009CDDC"


def kept(lines):
    return [x for x in strip_comments(lines)
            if x and not x.startswith(".") and not x.endswith(":")]


class TestHoistBlockLi(unittest.TestCase):
    @staticmethod
    def process(lines, value):
        with patch.dict(os.environ, {"MASPSX_HOIST_BLOCK_LI": value}):
            return kept(MaspsxProcessor(list(lines)).process_lines())

    def _check_retail_order(self, body):
        head = body[:5]
        self.assertEqual(head[0], "li\t$8,70")
        self.assertEqual(head[1], "li\t$7,130")
        self.assertEqual(head[2], "li\t$6,159")
        self.assertEqual(head[3], LOAD)
        self.assertEqual(head[4], "li\t$5,255")
        second = [i for i, x in enumerate(body) if x == LOAD][1]
        self.assertEqual(body[second + 1], "li\t$4,249")
        self.assertEqual(sum(x.startswith("li\t") for x in body), 5)

    def test_hoists_on(self):
        self._check_retail_order(self.process(window(), "1"))

    def test_symbol_list_selects(self):
        self._check_retail_order(self.process(window(), "D_8009CDDC"))

    def test_symbol_list_other_symbol_untouched(self):
        self.assertEqual(self.process(window(), "D_80000000"),
                         self.process(window(), "0"))

    def test_off_keeps_cc1_order(self):
        body = self.process(window(), "0")
        self.assertEqual(body[0], LOAD)
        i = body.index("li\t$8,70")
        self.assertTrue(body[i - 1].startswith("lw\t$3,"))

    def test_register_conflict_leaves_region(self):
        lines = window()
        # a use of $8 between the head and its li blocks the move
        lines.insert(3, "addu\t$9,$8,$0")
        self.assertEqual(self.process(lines, "1"), self.process(lines, "0"))

    def test_branch_splits_region(self):
        lines = window()
        cut = lines.index("li\t$7,130")
        lines.insert(cut - 1, "bne\t$9,$0,$L5")
        lines.insert(cut, "$L5:")
        on = self.process(lines, "1")
        self.assertNotEqual(on, self.process(lines, "0"))
        self.assertEqual(on.count("li\t$8,70"), 1)


if __name__ == "__main__":
    unittest.main()

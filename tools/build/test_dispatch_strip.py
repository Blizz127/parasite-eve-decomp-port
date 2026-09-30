#!/usr/bin/env python3
"""Regression tests for the multi-table dispatch-fold layout check.

`disc1_build.dispatch_multi_layout` decides where the .rel.rodata relocations
of a leaf with several folded switch tables must sit. No game data needed:
the inputs are relocation offsets and raw .rodata bytes.

Layouts pinned here come from real objects:
  * func_80085F74 (cc1 2.8.1, `.align 2`): two 8-entry tables, no pad, 0x40.
  * func_800862F4 (cc1 2.8.1, `.align 3`): two 7-entry tables, each followed
    by one zero word -- retail's pool shows that word as the
    `.word .L00000000_main` null entry of jtbl_800120FC / jtbl_8001211C.

Run: python3 tools/build/test_dispatch_strip.py
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from disc1_build import (  # noqa: E402
    BuildError,
    dispatch_multi_layout,
    dispatch_rodata_pad_ok,
    pool_block_literals,
)

SYM = "jtbl_A,jtbl_B"


def words(offsets: list[int], size: int, zero_at: list[int] | None = None) -> bytes:
    data = bytearray(b"\xAA" * size)
    for offset in zero_at or []:
        data[offset : offset + 4] = bytes(4)
    return bytes(data)


class TestDispatchMultiLayout(unittest.TestCase):
    def test_back_to_back_tables_no_pad(self):
        offsets = [4 * i for i in range(16)]
        self.assertEqual(
            offsets, dispatch_multi_layout(offsets, [8, 8], words(offsets, 0x40), SYM)
        )

    def test_align3_zero_pad_between_and_after(self):
        offsets = [4 * i for i in range(7)] + [0x20 + 4 * i for i in range(7)]
        rodata = words(offsets, 0x40, zero_at=[0x1C, 0x3C])
        self.assertEqual(offsets, dispatch_multi_layout(offsets, [7, 7], rodata, SYM))

    def test_three_tables_mixed_padding(self):
        # 5 entries (+pad) then 2 entries (ends 8-aligned, no pad) then 3.
        offsets = [0, 4, 8, 12, 16, 0x18, 0x1C, 0x20, 0x24, 0x28]
        rodata = words(offsets, 0x30, zero_at=[0x14, 0x2C])
        self.assertEqual(
            offsets, dispatch_multi_layout(offsets, [5, 2, 3], rodata, "a,b,c")
        )

    def test_refuses_non_zero_pad(self):
        offsets = [4 * i for i in range(7)] + [0x20 + 4 * i for i in range(7)]
        rodata = words(offsets, 0x40, zero_at=[0x3C])  # 0x1C left non-zero
        with self.assertRaisesRegex(BuildError, "non-zero pad"):
            dispatch_multi_layout(offsets, [7, 7], rodata, SYM)

    def test_refuses_pad_when_table_already_8_aligned(self):
        # Table 0 ends at 0x20 (8-aligned): a 4-byte gap is not .align 3 pad.
        offsets = [4 * i for i in range(8)] + [0x24 + 4 * i for i in range(7)]
        rodata = words(offsets, 0x40, zero_at=[0x20])
        with self.assertRaisesRegex(BuildError, "does not follow"):
            dispatch_multi_layout(offsets, [8, 7], rodata, SYM)

    def test_refuses_gap_wider_than_one_word(self):
        offsets = [4 * i for i in range(7)] + [0x28 + 4 * i for i in range(6)]
        rodata = words(offsets, 0x40, zero_at=[0x1C, 0x20, 0x24])
        with self.assertRaisesRegex(BuildError, "does not follow"):
            dispatch_multi_layout(offsets, [7, 6], rodata, SYM)

    def test_unpadded_split_is_not_visible_in_rodata(self):
        # Documented limit: back-to-back tables with no pad carry no boundary
        # in .rodata, so a 7+9 claim over an 8+8 object is accepted HERE. The
        # wrong split cannot survive the build's link/SHA-1 check: each
        # dispatch's `sltiu` bound and its folded pool symbol are fixed in
        # .text, so a table-size disagreement is a .text mismatch.
        offsets = [4 * i for i in range(16)]
        self.assertEqual(
            offsets, dispatch_multi_layout(offsets, [7, 9], words(offsets, 0x40), SYM)
        )

    def test_refuses_first_table_not_at_zero(self):
        offsets = [4 + 4 * i for i in range(7)] + [0x20 + 4 * i for i in range(8)]
        rodata = words(offsets, 0x40, zero_at=[0])
        with self.assertRaisesRegex(BuildError, "not at .rodata 0"):
            dispatch_multi_layout(offsets, [7, 8], rodata, SYM)

    def test_refuses_trailing_extra_data(self):
        offsets = [4 * i for i in range(16)]
        with self.assertRaisesRegex(BuildError, "refusing to strip"):
            dispatch_multi_layout(offsets, [8, 8], words(offsets, 0x48), SYM)



def pool(name: str, words: list[str]) -> str:
    body = "".join(f"    /* 0 00000000 00000000 */ .word {w}\n" for w in words)
    return f".align 3\nnonmatching {name}\n\ndlabel {name}\n{body}enddlabel {name}\n"


class TestPoolBlockLiterals(unittest.TestCase):
    """Overlay pools (PE.IMG ovl_0457 / room headers): `.L<vram>` entries and
    the `.align 3` zero word splat folds into the preceding dlabel."""

    def test_accepts_local_label_entries(self):
        src = pool("jtbl_8018F0FC", [".L80190BA0", ".L80190CC4", ".L80190AF4"])
        self.assertEqual(
            [".L80190BA0", ".L80190CC4", ".L80190AF4"],
            pool_block_literals(src, "jtbl_8018F0FC"),
        )

    def test_mixed_hex_and_label_entries(self):
        src = pool("jtbl_80122FA4", ["0x8012374C", ".L8012377C"])
        self.assertEqual(2, len(pool_block_literals(src, "jtbl_80122FA4")))

    def test_trailing_zero_pad_is_dropped(self):
        src = pool("jtbl_80122FA4", [".L8012374C"] * 5 + ["0x00000000"])
        self.assertEqual(5, len(pool_block_literals(src, "jtbl_80122FA4")))

    def test_interior_zero_is_kept(self):
        src = pool("jtbl_A0000000", ["0x80001000", "0x00000000", "0x80001008"])
        self.assertEqual(3, len(pool_block_literals(src, "jtbl_A0000000")))

    def test_missing_block_and_single_entry_refuse(self):
        src = pool("jtbl_80122FA4", ["0x8012374C", "0x00000000"])
        with self.assertRaisesRegex(BuildError, "no jtbl_80122FBC block"):
            pool_block_literals(src, "jtbl_80122FBC")
        with self.assertRaisesRegex(BuildError, "no literal words"):
            pool_block_literals(src, "jtbl_80122FA4")

    def test_pool_count_still_refuses_long_or_short_object_table(self):
        # 5 real entries + pad -> pool of 0x14; cc1 table must be 0x14 or 0x18
        # (one zero `.align 3` word). A 6- or 4-entry object table refuses.
        src = pool("jtbl_80122FA4", [".L8012374C"] * 5 + ["0x00000000"])
        expected = 4 * len(pool_block_literals(src, "jtbl_80122FA4"))
        self.assertTrue(dispatch_rodata_pad_ok(0x18, expected, bytes(4)))
        self.assertFalse(dispatch_rodata_pad_ok(0x18, expected, b"\xAA" * 4))
        self.assertFalse(dispatch_rodata_pad_ok(0x20, expected, bytes(12)))
        self.assertFalse(dispatch_rodata_pad_ok(0x10, expected, b""))

if __name__ == "__main__":
    unittest.main()

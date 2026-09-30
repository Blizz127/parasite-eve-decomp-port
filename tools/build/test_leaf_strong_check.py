#!/usr/bin/env python3
"""Hermetic regression tests for the shared strong leaf-check primitives.

These run without any toolchain or retail image: they exercise the boundary /
terminator / interior-return logic on synthetic word streams, which is the part
that makes an oversized or mis-carved span detectable.
"""

from __future__ import annotations

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "analysis"))

from leaf_strong_check import (  # noqa: E402
    JR_RA,
    boundary_check,
    sym_address,
)

LOAD_VRAM = 0x80010000
LOAD_FILE_START = 0x800


def make_retail(vram: int, words: list[int], total_file_size: int = 0x2000) -> bytes:
    """Place `words` at `vram` in a synthetic retail image."""
    image = bytearray(total_file_size)
    base = vram - LOAD_VRAM + LOAD_FILE_START
    for index, word in enumerate(words):
        struct.pack_into("<I", image, base + index * 4, word)
    return bytes(image)


class BoundaryCheckTest(unittest.TestCase):
    def test_bare_jr_ra_terminator_is_ok(self) -> None:
        vram = 0x80010000
        # body, addiu sp, jr $ra, nop
        words = [0x27BDFFE8, 0x03E00008, 0x00000000]
        retail = make_retail(vram, words)
        label, ok, total, interior, interior_ok = boundary_check(retail, vram, 0xC)
        self.assertEqual(label, "jr_ra")
        self.assertTrue(ok)
        self.assertEqual(total, 1)
        self.assertEqual(interior, [])
        self.assertTrue(interior_ok)

    def test_tail_j_terminator_is_ok(self) -> None:
        vram = 0x80010000
        words = [0x24020001, 0x08000000]  # li v0,1; j 0
        retail = make_retail(vram, words)
        label, ok, _total, interior, interior_ok = boundary_check(retail, vram, 0x8)
        self.assertEqual(label, "tail_j")
        self.assertTrue(ok)
        self.assertEqual(interior, [])
        self.assertTrue(interior_ok)

    def test_indirect_tail_jr_is_ok(self) -> None:
        vram = 0x80010000
        # lui / lw / nop / jr $t1 / nop  (func_8007E204 shape, shortened)
        words = [0x3C09000A, 0x8D2934C8, 0x00000000, 0x01200008, 0x00000000]
        retail = make_retail(vram, words)
        label, ok, total, interior, interior_ok = boundary_check(retail, vram, 0x14)
        self.assertEqual(label, "tail_jr")
        self.assertTrue(ok)
        self.assertEqual(total, 0)
        self.assertEqual(interior, [])
        self.assertTrue(interior_ok)

    def test_non_boundary_terminator_fails(self) -> None:
        vram = 0x80010000
        words = [0x24020001, 0x24630001]  # li v0,1; addiu v1,v1,1
        retail = make_retail(vram, words)
        label, ok, _total, _interior, _interior_ok = boundary_check(retail, vram, 0x8)
        self.assertTrue(label.startswith("other:"))
        self.assertFalse(ok)

    def test_interior_jr_ra_is_detected(self) -> None:
        vram = 0x80010000
        # func A: [prologue, jr $ra, nop] then func B: [prologue, jr $ra, nop]
        # A span covering both has one interior jr $ra (0x18) and the terminal one.
        words = [
            0x27BDFFE8, 0x03E00008, 0x00000000,   # A: ..., jr $ra, nop
            0x27BDFFE8, 0x03E00008, 0x00000000,   # B: ..., jr $ra, nop
        ]
        retail = make_retail(vram, words)
        label, ok, total, interior, interior_ok = boundary_check(retail, vram, 0x18)
        self.assertEqual(label, "jr_ra")
        self.assertTrue(ok)
        self.assertEqual(interior, [0x4])
        self.assertEqual(total, 2)
        self.assertFalse(interior_ok)

    def test_shrunk_span_without_terminator_fails(self) -> None:
        # A span that stops before the return lands on a non-return word; a
        # two-word tail with no `jr $ra` must fail the boundary check.
        vram = 0x80010000
        words = [0x27BDFFE8, 0x24630001, 0x03E00008, 0x00000000]
        retail = make_retail(vram, words)
        label, ok, _total, _interior, _interior_ok = boundary_check(retail, vram, 0x8)
        self.assertTrue(label.startswith("other:"))
        self.assertFalse(ok)

    def test_tiny_span_is_trivially_ok(self) -> None:
        retail = make_retail(0x80010000, [0x00000000])
        label, ok, total, interior, interior_ok = boundary_check(retail, 0x80010000, 4)
        self.assertEqual(label, "tiny")
        self.assertTrue(ok)
        self.assertEqual(total, 0)
        self.assertTrue(interior_ok)


class SymAddressTest(unittest.TestCase):
    def test_address_named_symbol_resolves(self) -> None:
        self.assertEqual(sym_address("D_800BCD80"), 0x800BCD80)
        self.assertEqual(sym_address("func_800906B4"), 0x800906B4)

    def test_non_address_symbol_is_none(self) -> None:
        self.assertIsNone(sym_address("printf"))
        self.assertIsNone(sym_address("__udivsi3"))

    def test_libcall_alias_resolves(self) -> None:
        # configs/USA/libcall_aliases.ld: cc1's soft-float helpers and memset.
        self.assertEqual(sym_address("__divdf3"), 0x80072F64)
        self.assertEqual(sym_address("__floatsidf"), 0x80073454)
        self.assertEqual(sym_address("memset"), 0x80071A44)


if __name__ == "__main__":  # pragma: no cover
    unittest.main()

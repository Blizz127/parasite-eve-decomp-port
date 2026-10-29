#!/usr/bin/env python3
"""Unit tests for overlay_callback_census.py on synthetic overlays (no retail bytes).

Run: python3 tools/analysis/test_overlay_callback_census.py
"""
from __future__ import annotations

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import overlay_callback_census as c  # noqa: E402

VRAM = 0x80300000


def lui(rt: int, hi: int) -> int:
    return (0x0F << 26) | (rt << 16) | hi


def addiu(rt: int, rs: int, imm: int) -> int:
    return (0x09 << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


def ori(rt: int, rs: int, imm: int) -> int:
    return (0x0D << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


def blob_of(words: list[int], size: int = 0x200) -> bytes:
    b = bytearray(size)
    for i, w in enumerate(words):
        struct.pack_into("<I", b, i * 4, w)
    return bytes(b)


class LuiPairs(unittest.TestCase):
    def test_addiu_negative_ori_and_rewrite_stop(self):
        text = blob_of([
            lui(7, 0x8030), 0, addiu(7, 7, 0x40),           # a3 = 0x80300040
            lui(4, 0x8031), addiu(4, 4, -0x10),              # a0 = 0x8030FFF0
            lui(5, 0x8030), ori(5, 5, 0x80),                 # a1 = 0x80300080
            lui(6, 0x8030), lui(6, 0x9000), addiu(6, 6, 4),  # rewritten: only the second pairs
        ], 0x28)
        got = [a for a, _ in c.lui_pairs(text, VRAM)]
        self.assertEqual(got, [0x80300040, 0x8030FFF0, 0x80300080, 0x90000004])


class Classify(unittest.TestCase):
    def setUp(self):
        # Text window [0x20, 0x100).  Function A at 0x40 (size 0x20), function B
        # at 0x80 (size 0x20) with a deliberately wrong fingerprint.
        words = [0] * 0x80
        words[0x40 // 4:0x60 // 4] = [0x11111111] * 8
        words[0x80 // 4:0xA0 // 4] = [0x22222222] * 8
        words[0x20 // 4] = lui(7, 0x8030)
        words[0x24 // 4] = addiu(7, 7, 0xC0)       # lui pair -> 0x803000C0 (no row)
        words[0x100 // 4] = VRAM + 0x40              # data word -> row start (resolved)
        words[0x104 // 4] = VRAM + 0x48              # data word inside row A (internal)
        words[0x108 // 4] = VRAM + 0x80              # row B start, but B's FNV is wrong
        words[0x10C // 4] = VRAM + 0x1F0             # outside the text window: ignored
        words[0x110 // 4] = VRAM + 0xE0              # hand-dispatch arm (resolved)
        self.blob = blob_of(words)
        fp_a = c.fnv1a(self.blob[0x40:0x60])
        self.rows = [(VRAM + 0x40, 0x20, fp_a), (VRAM + 0x80, 0x20, 0xDEADBEEF), (0x80010000, 0x20, 0)]
        self.ovl = {"id": "ovl_TEST", "vram": VRAM, "text_start": 0x20, "text_end": 0x100, "exe_scan": True}

    def test_classes(self):
        exe = blob_of([VRAM + 0x80, VRAM + 0x44], 0x10)
        res = c.census_overlay(self.ovl, self.blob, self.rows, {VRAM + 0xE0}, exe)
        self.assertEqual(res["attributed_rows"], 1)
        self.assertEqual(res["resolved"], 2)                     # 0x40 row start + 0xE0 arm
        self.assertEqual(res["internal"], 2)                     # 0x48 word + 0x44 exe word
        self.assertEqual([u["addr"] for u in res["uncovered"]], [VRAM + 0x80, VRAM + 0xC0])
        via = {u["addr"]: u["via"] for u in res["uncovered"]}
        self.assertIn("exe+0x0", via[VRAM + 0x80])
        self.assertTrue(any(v.startswith("lui ") for v in via[VRAM + 0xC0]))

    def test_shared_arena_skips_exe(self):
        exe = blob_of([VRAM + 0xC8], 0x10)
        ovl = dict(self.ovl, exe_scan=False)
        res = c.census_overlay(ovl, self.blob, self.rows, set(), exe)
        self.assertNotIn(VRAM + 0xC8, [u["addr"] for u in res["uncovered"]])

    def test_registry_rows_parse(self):
        text = ('    {0x801EDC44u, 0x950u, 0xC7367DCEu, pe_gct_2229, "func_801EDC44", 0x00000000u, 0, 0u},\n'
                '    {0x8001220Cu, 0x2ECu, 0x00000000u, pe_gct_0, "func_8001220C", 0x00000000u, 0, 0u},\n')
        self.assertEqual(c.registry_rows(text), [(0x801EDC44, 0x950, 0xC7367DCE), (0x8001220C, 0x2EC, 0)])


if __name__ == "__main__":
    unittest.main()

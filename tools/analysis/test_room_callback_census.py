#!/usr/bin/env python3
"""Unit tests for room_callback_census.py on a synthetic room chunk (no retail bytes).

Run: python3 tools/analysis/test_room_callback_census.py
"""
from __future__ import annotations

import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import room_callback_census as c  # noqa: E402

VB = c.ROOM_VMA
JR, RET0 = c.JR_RA, c.MOVE_V0_ZERO


def lui(rt: int, hi: int) -> int:
    return (0x0F << 26) | (rt << 16) | hi


def addiu(rt: int, rs: int, imm: int) -> int:
    return (0x09 << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


def j(target: int) -> int:
    return (0x02 << 26) | ((target >> 2) & 0x03FFFFFF)


def li(v: int) -> int:                 # addiu $v0,$zero,v: a plain non-terminating instruction
    return addiu(2, 0, v)


def pair(addr: int, rt: int = 1) -> list[int]:
    hi, lo = (addr + 0x8000) >> 16 & 0xFFFF, addr & 0xFFFF
    return [lui(rt, hi), addiu(rt, rt, lo - 0x10000 if lo & 0x8000 else lo)]


# Byte offsets in the synthetic chunk.
D, S, H, U, T = 0x0C, 0x10, 0x18, 0x24, 0x30     # data word, slot-0 stub, hand, uncovered, tail call
R1, R2, R3, V, S2 = 0x40, 0x60, 0x70, 0x80, 0x98  # row, ambiguous row, unresolved row, internal-j fn, stub
DESC = 0xA0                                      # 7-pointer descriptor
EXE_TARGET = 0x80020000


def build_chunk() -> bytes:
    w = [0] * (0x100 // 4)

    def put(off: int, words: list[int]) -> None:
        w[off // 4:off // 4 + len(words)] = words

    put(0x00, [0x00012345, 0x00012000, 0x10, 0x400])    # header words, then data word D
    put(S, [JR, RET0])                                   # bare `return 0`
    put(H, [li(1), JR, 0])
    put(U, [li(2), JR, 0])
    put(T, [li(3), j(EXE_TARGET), 0])                    # tail call into the EXE text
    put(R1, pair(VB + D) + [li(5), li(6), li(7), li(8), JR, 0])
    put(R2, pair(VB + S2) + [JR, 0])
    put(R3, [li(11), li(12), JR, 0])
    put(V, [li(13), j(VB + V + 0xC), 0, li(14), JR, 0])  # jump to a label in the same run
    put(S2, [JR, RET0])
    put(DESC, [VB + S, VB + H, VB + R1, VB + R2, VB + R3, VB + V, VB + U])
    put(0xC4, [VB + R1 + 8, VB + T, VB + V + 0xC])
    return struct.pack("<%dI" % len(w), *w)


def tables_for(chunk: bytes) -> list[dict]:
    def fp(off: int, size: int) -> int:
        return c.occ.fnv1a(chunk[off:off + size])

    def ent(off: int, size: int, f: int, sym: str = "", clo: int = 0) -> dict:
        return {"vma": VB + off, "size": size, "fp": f, "symbol": sym, "closure": clo}

    return [
        {"id": "exe", "lo": 0x80010000, "hi": 0x800E0808, "entries": [ent(-0x10000000, 4, 0)]},
        {"id": "room_A", "lo": VB, "hi": VB + 0x30000,
         "entries": [ent(R1, 0x20, fp(R1, 0x20), "room_a__R1"),
                     ent(R2, 0x10, fp(R2, 0x10), "room_a__R2", 0x11111111)]},
        {"id": "room_B", "lo": VB, "hi": VB + 0x30000,
         "entries": [ent(R2, 0x10, fp(R2, 0x10), "room_b__R2", 0x22222222)]},
        # find_entry takes the first row at a vma; only the second one matches.
        {"id": "room_C", "lo": VB, "hi": VB + 0x30000,
         "entries": [ent(R3, 0x10, 0xDEADBEEF, "room_c__R3"), ent(R3, 0x10, fp(R3, 0x10), "room_c__R3b")]},
    ]


ARMS = {VB + H: ["pc_port/x.c:1"], VB + V: ["pc_port/y.c:2"]}


class Census(unittest.TestCase):
    def setUp(self):
        self.chunk = build_chunk()
        self.tables = tables_for(self.chunk)
        self.res = c.census_room("mTEST", self.chunk, self.tables, ARMS)
        self.cls = {a - VB: k for a, k in self.res["_classes"].items()}

    def test_classes(self):
        self.assertEqual(self.cls[D], "data")             # lui-named data word ahead of the stub
        self.assertEqual(self.cls[S], "stub0")            # named only from descriptor slot 0
        self.assertEqual(self.cls[H], "hand")
        self.assertEqual(self.cls[U], "UNCOVERED")        # a gap entry, slot 6
        self.assertEqual(self.cls[T], "UNCOVERED")        # tail-call body
        self.assertEqual(self.cls[R1], "registry")
        self.assertEqual(self.cls[R1 + 8], "internal")
        self.assertEqual(self.cls[R2], "ambiguous")       # two symbols, two closures
        self.assertEqual(self.cls[R3], "row_unresolved")
        self.assertEqual(self.cls[V], "hand")
        self.assertEqual(self.cls[V + 0xC], "hand")       # a label inside an armed gap body
        self.assertEqual(self.cls[S2], "UNCOVERED")       # a stub named by a lui pair, not slot 0
        self.assertEqual(self.res["counts"]["UNCOVERED"], 3)

    def test_gap_bodies(self):
        bodies = {g["entry"] - VB: (g["size"], g["class"]) for g in self.res["gap_bodies"]}
        self.assertEqual(bodies, {S: (8, "stub0"), H: (0xC, "hand"), U: (0xC, "UNCOVERED"),
                                  T: (0xC, "UNCOVERED"), V: (0x18, "hand"), S2: (8, "UNCOVERED")})

    def test_jr_only(self):
        base = c.census_room("mTEST", self.chunk, self.tables, ARMS, use_j=False)
        changed = {a - VB for a, k in base["_classes"].items() if self.res["_classes"][a] != k}
        self.assertEqual(changed, {T})
        self.assertEqual(base["_classes"][VB + T], "data")

    def test_no_code(self):
        self.assertIsNone(c.census_room("mNONE", bytes(0x40), self.tables, ARMS))


class Resolve(unittest.TestCase):
    def tables(self, a: dict, b: dict) -> list[dict]:
        return [{"id": "room_X", "lo": VB, "hi": VB + 0x100, "entries": [a]},
                {"id": "room_Y", "lo": VB, "hi": VB + 0x100, "entries": [b]}]

    def entry(self, size: int, chunk: bytes, sym: str, clo: int = 0) -> dict:
        return {"vma": VB, "size": size, "fp": c.occ.fnv1a(chunk[:size]), "symbol": sym, "closure": clo}

    def test_equivalences(self):
        chunk = struct.pack("<4I", li(1), li(2), JR, 0)
        same8 = self.tables(self.entry(8, chunk, "a"), self.entry(8, chunk, "b"))
        self.assertEqual(c.resolve(same8, VB, chunk)[1], 1)                       # 8-byte stubs
        same_sym = self.tables(self.entry(16, chunk, "s"), self.entry(16, chunk, "s"))
        self.assertIsNotNone(c.resolve(same_sym, VB, chunk)[0])
        same_clo = self.tables(self.entry(16, chunk, "a", 7), self.entry(16, chunk, "b", 7))
        self.assertIsNotNone(c.resolve(same_clo, VB, chunk)[0])
        distinct = self.tables(self.entry(16, chunk, "a", 7), self.entry(16, chunk, "b", 9))
        self.assertEqual(c.resolve(distinct, VB, chunk), (None, 2))
        beyond = self.tables(self.entry(16, chunk, "a"), self.entry(16, chunk, "b"))
        beyond[0]["entries"][0]["size"] = beyond[1]["entries"][0]["size"] = 0x20
        self.assertEqual(c.resolve(beyond, VB, chunk), (None, 0))                 # window past the chunk


class Registry(unittest.TestCase):
    TEXT = (
        'static const PeGuestCodeEntry pe_gc_exe_e[] = {\n'
        '    {0x8001220Cu, 0x2ECu, 0x00000000u, pe_gct_0, "func_8001220C", 0x00000000u, 0, 0u},\n'
        '};\n'
        'static const PeGuestCodeTable pe_gc_exe = {"exe", 0x80010000u, 0x800E0808u, 0, 0x0u, pe_gc_exe_e, 1u};\n'
        'static const PeGuestCodeEntry pe_gc_ovl_039F_e[] = {\n'
        '    {0u, 0u, 0u, 0, 0, 0u, 0, 0u},\n'
        '};\n'
        'static const PeGuestCodeTable pe_gc_ovl_039F = {"ovl_039F", 0x8010BCF8u, 0x8011ECF8u, '
        '"e7331867b4d1464da4d5f386d9137eaa1c98422b", 0x13000u, pe_gc_ovl_039F_e, 0u};\n'
        'static const PeGuestCodeEntry pe_gc_room_m0012i_e[] = {\n'
        '    {0x8018F20Cu, 0xA48u, 0x25276BF1u, pe_gct_2335, "room_4af0187a__func_8018F20C", 0x8B80254Fu, 0, 0u},\n'
        '};\n'
        'static const PeGuestCodeTable pe_gc_room_m0012i = {"room_m0012i", 0x8018EFE8u, 0x801B37E8u, '
        '"7b38ab836272d86fa4d88969ecc87a59a104b284", 0x24800u, pe_gc_room_m0012i_e, 1u};\n'
        'static const PeGuestCodeTable *const pe_gc_tables[] = {\n'
        '    &pe_gc_exe,\n    &pe_gc_room_m0012i,\n    &pe_gc_ovl_039F,\n};\n')

    def test_parse(self):
        ts = c.registry_tables(self.TEXT)
        self.assertEqual([t["id"] for t in ts], ["exe", "room_m0012i", "ovl_039F"])
        self.assertEqual(ts[2]["entries"], [])
        self.assertEqual(ts[1]["entries"][0], {"vma": 0x8018F20C, "size": 0xA48, "fp": 0x25276BF1,
                                               "symbol": "room_4af0187a__func_8018F20C",
                                               "closure": 0x8B80254F})

    def test_count_mismatch(self):
        with self.assertRaises(ValueError):
            c.registry_tables(self.TEXT.replace("pe_gc_room_m0012i_e, 1u", "pe_gc_room_m0012i_e, 2u"))


class Arms(unittest.TestCase):
    def test_sites(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / "pc_port/game").mkdir(parents=True)
            (root / "pc_port/tests").mkdir(parents=True)
            (root / "pc_port/game/a.c").write_text(
                "/* fn==0x8018F000u in a comment */\n"
                "int f(unsigned fn){\n"
                "    if (fn==0x8018F020u) return 1;\n"
                "    switch (fn) { case 0x8018F030u: return 2; }\n"
                "    if (fn==0x80010000u) return 3;\n"
                "    return 0; }\n")
            (root / "pc_port/tests/t.c").write_text("int g(unsigned fn){ return fn==0x8018F040u; }\n")
            self.assertEqual(c.arm_sites(root), {0x8018F020: ["pc_port/game/a.c:3"],
                                                 0x8018F030: ["pc_port/game/a.c:4"]})


if __name__ == "__main__":
    unittest.main()

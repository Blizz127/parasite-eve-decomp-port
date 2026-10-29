#!/usr/bin/env python3
"""Unit tests for hook_writer_census.py on synthetic code (no retail bytes).

Run: python3 tools/analysis/test_hook_writer_census.py
"""
from __future__ import annotations

import struct
import sys
import hashlib
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import hook_writer_census as c  # noqa: E402

VMA = 0x80010000
HOOK = 0x80030000          # synthetic hook word
IDENT = 0x80020100         # the identity the synthetic dispatcher handles
SETTER = 0x80020000        # synthetic setter: D_HOOK = a0
NOP = 0
JR = c.JR_RA


def lui(rt, hi): return (0x0F << 26) | (rt << 16) | (hi & 0xFFFF)
def addiu(rt, rs, imm): return (0x09 << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)
def ori(rt, rs, imm): return (0x0D << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)
def sw(rt, off, base): return (0x2B << 26) | (base << 21) | (rt << 16) | (off & 0xFFFF)
def swl(rt, off, base): return (0x2A << 26) | (base << 21) | (rt << 16) | (off & 0xFFFF)
def lw(rt, off, base): return (0x23 << 26) | (base << 21) | (rt << 16) | (off & 0xFFFF)
def move(rd, rs): return (rs << 21) | (rd << 11) | 0x21          # addu rd,rs,$zero
def jal(t): return (0x03 << 26) | ((t >> 2) & 0x03FFFFFF)
def beq(rs, rt, off): return (0x04 << 26) | (rs << 21) | (rt << 16) | (off & 0xFFFF)


def hi_lo(addr):
    lo = addr & 0xFFFF
    return ((addr + 0x8000) >> 16) & 0xFFFF, lo - 0x10000 if lo & 0x8000 else lo


def li32(rt, addr):
    hi, lo = hi_lo(addr)
    return [lui(rt, hi), addiu(rt, rt, lo)]


def image(words_at: dict[int, list[int]], size=0x30000) -> bytes:
    w = [0] * (size // 4)
    for addr, seq in words_at.items():
        i = (addr - VMA) // 4
        w[i:i + len(seq)] = seq
    return struct.pack("<%dI" % len(w), *w)


def blob(words_at, size=0x30000, gp=c.GP):
    b = c.Blob("EXE", image(words_at, size), VMA, gp=gp)
    return b


def family(**kw):
    fam = {"key": "t", "title": "test hook", "base": HOOK, "count": 1, "handled": [IDENT],
           "site": ("nowhere.c", "x"), "setters": [{"fn": SETTER, "val": ("reg", 4)}],
           "forwarders": [], "expected": {}, "port_names": []}
    fam.update(kw)
    return fam


HH, HL = hi_lo(HOOK)
SETTER_BODY = [lui(1, HH), JR, sw(4, HL, 1)]       # D_HOOK = a0 in the jr delay slot


def classes(res):
    return [(r["kind"], r["class"]) for r in res["rows"] if r["kind"] != "initial"]


class TraceTests(unittest.TestCase):
    def test_literal_pair_and_delay_slot(self):
        b = blob({0x80010100: li32(5, IDENT) + [jal(SETTER), addiu(4, 0, 7)]})
        k = (0x80010108 - VMA) // 4
        self.assertEqual(c.arg_value(b, k, ("reg", 5)), ("lit", IDENT))
        self.assertEqual(c.arg_value(b, k, ("reg", 4)), ("lit", 7))      # delay slot runs first

    def test_forwarded_skips_padding(self):
        # previous function ends at 0x80010100; two padding words; entry 0x80010110
        b = blob({0x80010100: [JR, NOP, NOP, NOP], 0x80010110: [addiu(29, 29, -24), jal(SETTER), NOP]})
        k = (0x80010114 - VMA) // 4
        self.assertEqual(c.arg_value(b, k, ("reg", 4)), ("forwarded", 4, (0x80010110 - VMA) // 4))

    def test_call_clobbers_caller_saved_not_callee_saved(self):
        seq = li32(4, IDENT) + li32(16, IDENT) + [jal(0x80025000), NOP, move(5, 16), jal(SETTER), NOP]
        b = blob({0x80010100: seq})
        k = (0x80010100 + 4 * 7 - VMA) // 4
        self.assertEqual(c.arg_value(b, k, ("reg", 4))[0], "unresolved")  # a0 crossed a call
        self.assertEqual(c.arg_value(b, k, ("reg", 5)), ("lit", IDENT))    # $s0 survives it

    def test_merge_is_unresolved(self):
        # a branch targets the instruction between the literal and the call
        seq = [beq(0, 0, 3), NOP] + li32(4, IDENT) + [NOP, jal(SETTER), NOP]
        b = blob({0x80010100: seq})
        k = (0x80010100 + 4 * 5 - VMA) // 4
        self.assertTrue(c.arg_value(b, k, ("reg", 4))[1].startswith("merge"))

    def test_stack_argument(self):
        seq = [addiu(2, 0, 0x55), sw(2, 0x10, 29), jal(SETTER), NOP]
        b = blob({0x80010100: seq})
        self.assertEqual(c.arg_value(b, (0x80010108 - VMA) // 4, ("stack", 0x10)), ("lit", 0x55))

    def test_swl_swr_spans(self):
        self.assertEqual(c.store_span(0x2A, 0x800B0CC7), (0x800B0CC4, 0x800B0CC8))
        self.assertEqual(c.store_span(0x2E, 0x800B0CC5), (0x800B0CC5, 0x800B0CC8))
        self.assertEqual(c.store_span(0x2B, 0x800B0CC8), (0x800B0CC8, 0x800B0CCC))

    def test_stack_argument_does_not_cross_another_call(self):
        # Outgoing argument space belongs to the callee during an intervening
        # call; its earlier contents cannot prove the next call's argument.
        seq = [sw(0, 0x10, 29), jal(0x80025000), NOP, jal(SETTER), NOP]
        b = blob({0x80010100: seq})
        self.assertEqual(c.arg_value(b, (0x8001010C - VMA) // 4,
                                     ("stack", 0x10))[0], "unresolved")


class CensusTests(unittest.TestCase):
    def run_census(self, words_at, **kw):
        fam = family(**kw)
        return c.census_family(fam, [blob(words_at)])

    def test_identity_setter_passes(self):
        res = self.run_census({SETTER: SETTER_BODY, 0x80010100: li32(4, IDENT) + [jal(SETTER), NOP]},
                              expected={SETTER + 8: ("body", "setter")})
        self.assertEqual(res["fail"], 0, res["rows"])
        self.assertIn(("setter", "identity"), classes(res))
        self.assertIn(("store", "expected/body"), classes(res))

    def test_negative_control_planted_store(self):
        # A planted direct store of a non-identity literal must fail.
        planted = li32(2, 0x80020200) + [lui(1, HH), sw(2, HL, 1)]
        res = self.run_census({SETTER: SETTER_BODY, 0x80010200: planted},
                              expected={SETTER + 8: ("body", "setter")})
        self.assertEqual([k for k in classes(res) if k[0] == "store" and k[1] == "other"], [("store", "other")])
        self.assertGreater(res["fail"], 0)

    def test_negative_control_planted_setter_call(self):
        res = self.run_census({SETTER: SETTER_BODY, 0x80010100: li32(4, 0x80020200) + [jal(SETTER), NOP],
                               0x80010300: [jal(0x80010100 - 8), NOP]},   # reference the caller
                              expected={SETTER + 8: ("body", "setter")})
        self.assertIn(("setter", "other"), classes(res))
        self.assertGreater(res["fail"], 0)

    def test_unreferenced_writer(self):
        # The same planted call inside a function nothing references.
        b = blob({SETTER: SETTER_BODY, 0x80010100: li32(4, 0x80020200) + [jal(SETTER), NOP]})
        b.starts = [(0x80010100, "func_80010100"), (SETTER, "func_80020000")]
        res = c.census_family(family(expected={SETTER + 8: ("body", "setter")}), [b])
        self.assertIn(("setter", "other-unreferenced"), classes(res))
        self.assertEqual(res["fail"], 0)

    def test_gp_relative_and_struct_base_stores(self):
        gp = HOOK + 0x100                      # a $gp within 16 bits of the synthetic hook
        base = HOOK - 0x10                     # struct base below the hook
        seq = li32(2, 0x80020300) + [sw(2, HOOK - gp, 28)] + li32(16, base) + [sw(2, 0x10, 16)]
        res = c.census_family(family(near=[base]), [blob({0x80010100: seq}, gp=gp)])
        kinds = classes(res)
        self.assertEqual(sum(1 for k in kinds if k == ("store", "other")), 2, res["rows"])
        self.assertIn(("near-base", "undeclared"), kinds)

    def test_swl_outside_the_hook_is_not_a_writer(self):
        seq = li32(8, HOOK - 0x18) + [addiu(7, 0, 1), swl(7, 0x17, 8)]   # writes HOOK-4..HOOK-1
        res = self.run_census({0x80010100: seq})
        self.assertEqual([k for k in classes(res) if k[0] == "store"], [])

    def test_struct_store_in_return_delay_slot_is_a_writer(self):
        base = HOOK - 0x10
        seq = li32(8, base) + li32(2, 0x80020300) + [JR, sw(2, 0x10, 8)]
        res = self.run_census({0x80010100: seq})
        self.assertIn(("store", "other"), classes(res))
        self.assertGreater(res["fail"], 0)

    def test_accepted_exception(self):
        res = self.run_census({SETTER: SETTER_BODY, 0x80010100: li32(4, 0x80020200) + [jal(SETTER), NOP],
                               0x80010300: [jal(0x80010100), NOP]},
                              expected={SETTER + 8: ("body", "setter")},
                              accepted={0x80010108: "reviewed"})
        self.assertIn(("setter", "accepted/other"), classes(res))
        self.assertEqual(res["fail"], 0)

    def test_forwarder_chain(self):
        fwd = 0x80010400                       # fwd(x) = setter(x)
        res = self.run_census({SETTER: SETTER_BODY,
                               0x800103F0: [JR, NOP], fwd: [jal(SETTER), NOP, JR, NOP],
                               0x80010500: li32(4, IDENT) + [jal(fwd), NOP]},
                              expected={SETTER + 8: ("body", "setter")},
                              forwarders=[{"fn": fwd, "val": ("reg", 4)}])
        kinds = classes(res)
        self.assertIn(("setter", "forwarded"), kinds)
        self.assertIn(("setter", "identity"), kinds)
        self.assertEqual(res["fail"], 0, res["rows"])

    def test_undeclared_forwarder_fails(self):
        res = self.run_census({SETTER: SETTER_BODY, 0x800103F0: [JR, NOP],
                               0x80010400: [jal(SETTER), NOP, JR, NOP]},
                              expected={SETTER + 8: ("body", "setter")})
        self.assertIn(("setter", "forwarded-undeclared"), classes(res))
        self.assertGreater(res["fail"], 0)

    def test_table_index_rules(self):
        res = c.census_family(family(count=4, sources=[0, 2], handled=[IDENT],
                                     setters=[{"fn": SETTER, "idx": ("reg", 4), "val": ("reg", 5)}]),
                              [blob({0x80010100: [addiu(4, 0, 3)] + li32(5, IDENT) + [jal(SETTER), NOP],
                                     0x80010200: [addiu(4, 0, 9)] + li32(5, IDENT) + [jal(SETTER), NOP]})])
        idx = [r["class"] for r in res["rows"] if r["kind"] == "setter"]
        self.assertEqual(idx, ["index", "index"])        # 3 outside the sources, 9 outside the table

    def test_initial_value_is_a_writer(self):
        res = self.run_census({HOOK: [0x80020200]})
        init = [r for r in res["rows"] if r["kind"] == "initial"]
        self.assertEqual([r["class"] for r in init], ["other"])
        self.assertGreater(res["fail"], 0)


class SwitchTableTests(unittest.TestCase):
    def test_planted_entry_fails(self):
        tab = {"key": "s", "title": "t", "base": 0x80011000, "count": 3, "labels": [0x80012000, 0x80012010]}
        good = blob({0x80011000: [0x80012000, 0x80012010, 0x80012000]})
        bad = blob({0x80011000: [0x80012000, 0x80012014, 0x80012000]})
        self.assertEqual(c.switch_census(tab, good)["fail"], 0)
        self.assertEqual(c.switch_census(tab, bad)["fail"], 1)


class InputTests(unittest.TestCase):
    def test_room_inventory_does_not_depend_on_extraction_cache(self):
        sys.path.insert(0, str(c.REPO / "tools/extract"))
        import peimg
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            exe = bytearray(0x1000)
            struct.pack_into("<II", exe, 0x18, VMA, 0x800)
            # Two rooms, each with a single code sector. No peimg/ cache.
            struct.pack_into("<IIII", exe, 0x810, 1, 1 << 20, 2, 1 << 20)
            code = struct.pack("<II", JR, NOP) + bytes(2040)
            image_data = bytes(2048) + code + code
            (root / "SLUS_006.62").write_bytes(exe)
            (root / "PE.IMG").write_bytes(image_data)
            manifest = {
                "image": {"extracted": "PE.IMG", "disc_lba": 1000,
                          "size": len(image_data), "sha1": hashlib.sha1(image_data).hexdigest()},
                "exe": {"sha1": hashlib.sha1(exe).hexdigest(), "room_table": VMA + 0x10,
                        "room_table_entries": 2, "room_base_lba": 1000,
                        "range_table": VMA + 0x30, "range_table_entries": 1,
                        "dest_table": VMA + 0x34, "dest_table_entries": 1},
                "overlays": {},
            }
            with patch.object(peimg, "load_manifest", return_value=manifest):
                bs = c.load_blobs(root, root)
            self.assertEqual([b.name for b in bs], ["EXE", "room_m0001i_c2", "room_m0002i_c2"])
            self.assertTrue(all(b.code == (0, 2) for b in bs[1:]))
            self.assertFalse((root / "peimg").exists())


if __name__ == "__main__":
    unittest.main()

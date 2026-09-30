#!/usr/bin/env python3
"""Retail-instruction oracle for the port lane round-7 hand adapters.

Runs the retail EXE (build/extracted/disc1/SLUS_006.62, SHA-1 checked) in the
pe_battle_hud_oracle MIPS interpreter on seeded, synthetic guest state and
writes pc_port/tests/retail_port7_cases.h: per case the seeded words, the
arguments, the retail return value and an FNV-1a hash of the observed ranges.
No game data is written.  The native test (pc_port/tests/test_port7.h) seeds
the same words and must reproduce every hash and return value.

Leaves: func_80068710 (grid cursor step, recursive), func_80083644 (pad-record
mode handler), func_80084C4C (pad-record receive hook), func_8001CBA0 /
func_8001CE88 (polygon edge push-out / edge search, retail GTE normalize
func_80078134), func_8008A068 (SEQ header parse + 24-voice init), func_80081E70 (DsRead
ready callback; only its paths that stay off CD hardware: no header sector,
no retry through func_80082204, no CdlPause), func_8001D170 (actor vs
boundary polygon step) and func_800DE7A8 (battle particle callback; the
move/expire and default modes; mode 2 draws through func_800CEE20), func_80058E44 / func_80058FEC (inventory
move check and entry swap), func_80054F58 (equipment exchange check),
func_80088344 (SEQ voice LFO/volume/pan/pitch tick), func_80048254
(equipment-exchange confirm; builds windows in the D_800A22E0 pool; a case
whose retail call graph leaves the interpreter's supported set is
regenerated).  A case on which retail executes a DIV `break` (or exceeds the
instruction budget) is regenerated.  The callbacks are
real retail leaves that the native dispatch registry resolves:
D_8009B728 = func_8005E8A4 (D_8009D124 += a0 counts notifications; its
D_8009D128 += a1 side is not observed because a1 is a stale register),
D_8009B740 = func_8004E970 (returns D_8009CF0C), mode handler +0x18 =
func_80084F8C.

Usage: pe_port7_oracle.py [--write-header]   (default: verify the header)
"""
import hashlib
import random
import struct
import sys

from pe_battle_hud_oracle import ROOT, execute
from pe_transition_loader_oracle import fnv

EXE_SHA1 = '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
REC, RX, TX, OBJ, CELLS = 0x80150000, 0x80150200, 0x80150300, 0x80150380, 0x80150400
EOBJ, PTS = 0x80150500, 0x80150600
RANGES = [(0x150000, 0x100), (0x150200, 0x40), (0x150300, 0x40), (0x150380, 0x40),
          (0x150400, 0x100), (0x9CDB8, 0x10), (0x9D124, 4), (0x9B728, 4),
          (0x9B740, 4), (0x9CF0C, 4), (0x150500, 0x60), (0x150600, 0x60), (0x9CE2C, 4),
          (0x150700, 0x80), (0x150800, 0x80), (0xB8AC0, 0x1AA0), (0xB2900, 0x40),
          (0x9D2C8, 4), (0x9D2C4, 4), (0x9D2DC, 4), (0x9CDE8, 4), (0xBCD50, 4),
          (0xBCD5C, 4), (0x9B6EC, 0x24), (0x9B580, 4), (0xB8AB4, 8), (0xA3608, 4),
          (0x150900, 4), (0x150A00, 0x230), (0x9D254, 4), (0x9D2F8, 4), (0x9D264, 4),
          (0x150C00, 0x10), (0xE27EC, 4), (0x9D000, 0xA0), (0xC0E00, 0x1100),
          (0xA1E00, 0x200), (0xA8028, 0x1000), (0x9CF98, 4), (0x150D00, 0x200),
          (0x151000, 0x400), (0x9B8F8, 0x200), (0x9D2C0, 4), (0x9D2CC, 4),
          (0xA22E0, 0xD80), (0x9D150, 0x10), (0x9CF00, 0x100), (0xA1888, 0x10),
          (0x92478, 0xC30), (0x150F00, 0x90), (0xB0E08, 4)]
ENTRY = {0: 0x80068710, 1: 0x80083644, 2: 0x80084C4C, 3: 0x8001CBA0, 4: 0x8001CE88,
         5: 0x8008A068, 6: 0x80081E70, 7: 0x8001D170, 8: 0x800DE7A8,
         9: 0x80058E44, 10: 0x80058FEC, 11: 0x80054F58, 12: 0x80088344,
         13: 0x80048254}
CASES_PER_ENTRY = {0: 500, 1: 300, 2: 500, 3: 400, 4: 500, 5: 150, 6: 600, 7: 300, 8: 300,
                   9: 250, 10: 350, 11: 500, 12: 700, 13: 500}
VOICE = 0x80151010


def gen_voice(rng):
    seeds = []
    rec = bytearray(rng.getrandbits(8) for _ in range(0x130))
    def w16(o, v):
        struct.pack_into('<H', rec, o + 0x10, v & 0xFFFF)
    def w32(o, v):
        struct.pack_into('<I', rec, o + 0x10, v & 0xFFFFFFFF)
    w32(0x38, rng.getrandbits(32) & rng.choice([0x837, 0x1, 0x2, 0x4, 0x20, 0x10, 0x800, 0xFFFFFFFF]))
    w32(0xF4, rng.choice([0, 1, 2, 3, 0x10, 0x13, rng.getrandbits(32)]))
    for base in (0x1C, 0x20, 0x24):
        w32(base, 0x80151200 + 2 * rng.randrange(0, 0x60))
    for o in (0x8A, 0x9E):
        w16(o, rng.choice([0, 0, 1]))
    for o in (0x8E, 0xA2, 0xB0):
        w16(o, rng.choice([1, 1, 2, 0, 7]))
    w16(0x5C, rng.randrange(0, 24))
    for i in range(0, 0x130, 4):
        seeds.append((VOICE - 0x10 + i, struct.unpack_from('<I', rec, i)[0]))
    tab = [rng.getrandbits(16) for _ in range(0x80)]
    for _ in range(4):
        k = rng.randrange(0, 0x7D)
        tab[k] = tab[k + 1] = 0
        tab[k + 2] = (-rng.randrange(1, 40)) & 0xFFFF
    for i in range(0, 0x80, 2):
        seeds.append((0x80151200 + 2 * i, tab[i] | tab[i + 1] << 16))
    seeds += [(0x8009D2C8, CTL), (CTL + 0x48, rng.getrandbits(32)),
              (0x8009D2C0, rng.choice([0, 1, 1, 4, 4, 2])),
              (0x8009D2CC, rng.getrandbits(16) | rng.choice([0, 0, 0x10, 0x7F, 0x80, 0xC0, 0xFF]) << 16 |
               rng.getrandbits(8) << 24)]
    return seeds, (VOICE, rng.choice([0x1, 0x2, 0x800000, 0x400000, rng.getrandbits(24)]), 0, 0)
ALTLIST, EQUIP = 0x80150D00, 0x80150E00


def inv_record(v):
    if 0x100 <= v < 0x180:
        return 0x800BEEAC + (v << 5)
    if 1 <= v < 0x100:
        return 0x800A8028 + 0x100 + (v - 1) * 32 if v - 1 < 0x40 else None
    if 0x200 <= v < 0x209:
        return 0x8009DE64 + (v << 5)
    return None


def gen_inv(rng, entry):
    seeds = []
    nslot = rng.choice([0, 1, 4, 10, 30])
    seeds.append((0x8009D044, nslot))
    for i in range(0, 40, 2):
        seeds.append((0x800A1E00 + 2 * i, (rng.randrange(-2, 30) & 0xFFFF) |
                      (rng.randrange(-2, 30) & 0xFFFF) << 16))
    def rid():
        return rng.choice([0, 0, rng.randrange(1, 0x41), rng.randrange(1, 0x41),
                           rng.randrange(0x100, 0x180), rng.randrange(0x200, 0x209),
                           rng.randrange(0x41, 0x100), 0x7FFF, 0xFFFF])
    lst = [rid() for _ in range(32)]
    eq = [rid() for _ in range(16)]
    for base, ids in ((0x800C0E48, lst), (EQUIP, eq), (ALTLIST, lst[::-1])):
        for i in range(0, len(ids), 2):
            seeds.append((base + 2 * i, ids[i] | ids[i + 1] << 16))
    for v in set(lst + eq):
        rec = inv_record(v)
        if rec is not None:
            kind = rng.choice([8, 8, 0x13, 0x14, 0x15, 0x16, 0x12, rng.getrandbits(8)])
            seeds.append((rec + 4, rng.getrandbits(8) | (rng.choice([0, 0x20, rng.getrandbits(8)]) << 8)
                          | kind << 16 | rng.getrandbits(8) << 24))
            if entry in (11, 13):
                ent = bytes([rng.choice([0, 0, 1, 2, 3, 5, 7, 11])] +
                            [rng.choice([0, 8, 9, 10, 0x28, 0x29, 0x2A, 0x48, 0x49, 0xE8, 0x2B,
                                         0x20, rng.getrandbits(8)]) for _ in range(11)])
                for j in range(0, 12, 4):
                    seeds.append((rec + 0x14 + j, struct.unpack_from('<I', ent, j)[0]))
    seeds += [(0x800A8034, 0x100), (0x800A8038, 0x100 + 0x40 * 32),
              (0x800C0E20, rng.randrange(-2, 32) & 0xFF | (rng.getrandbits(8) << 8) |
               (rng.randrange(-2, 32) & 0xFF) << 16 | rng.getrandbits(8) << 24),
              (0x800C0E0C, rng.choice([0, 1, 5, 20, 31, 32, 60])),
              (0x8009D07C, EQUIP), (0x8009CF98, rng.choice([0, 1])),
              (0x8009D008, rng.getrandbits(32))]
    if entry == 9:
        seeds += [(0x8009D048, rng.choice([0x800C0E48, 0x800C0E48, ALTLIST])),
                  (0x8009D050, rng.choice([0, 1, 5, 20, 32]))]
        return seeds, (rng.randrange(-3, 34) & 0xFFFFFFFF, 0, 0, 0)
    if entry == 13:
        focus = 0x80150F00
        kind = rng.choice([6, 0x1B, 0xB, 0x1C, 6, 0xB])
        seeds += [(focus + 0x24, kind), (focus + 0x34, rng.choice([1, 1, 2])),
                  (focus + 0x44, rng.choice([-1, 0, 1, 2, 3]) & 0xFFFFFFFF),
                  (focus + 0x48, rng.choice([-1, 0, 0, 1]) & 0xFFFFFFFF),
                  (0x8009D15C, focus), (0x8009D154, 0), (0x8009D158, 0x800A22E0)]
        for k in range(23):
            seeds.append((0x800A22E0 + 0x90 * k, 0x800A22E0 + 0x90 * (k + 1)))
        for wid in (4, 0x2C):
            # frame spec D_80092478 + id*16 (func_8005DA8C) and list spec
            # D_80092888 + id*32 (func_8005DAB4)
            for j in range(4):
                seeds.append((0x80092478 + wid * 16 + 4 * j,
                              rng.randrange(0, 0x140) | rng.randrange(0, 0xF0) << 16))
            row = [0x40, 0x30, rng.choice([1, 2]), rng.choice([1, 3, 4]), 4, 8, 12, 0]
            for j, wv in enumerate(row):
                seeds.append((0x80092888 + wid * 32 + 4 * j, wv))
        for j in range(4):
            seeds.append((0x800A1888 + 4 * j, rng.choice([0, 0, 1, 5, -1]) & 0xFFFFFFFF))
        seeds += [(0x8009CF0C, rng.choice([0, 1])), (0x8009D090, rng.randrange(-1, 20) & 0xFFFFFFFF),
                  (0x8009D094, rng.randrange(-1, 20) & 0xFFFFFFFF), (0x8009D098, 0),
                  (0x8009D09C, 0)]
        return seeds, (0, 0, 0, 0)
    if entry == 11:
        sl = [rng.randrange(-2, 34), rng.randrange(-2, 34)]
        seeds += [(0x8009D090, sl[0] & 0xFFFFFFFF), (0x8009D094, sl[1] & 0xFFFFFFFF),
                  (0x8009D098, rng.choice([0, 1])), (0x8009D09C, rng.choice([0, 1])),
                  (0x8009D04C, rng.choice([0, 0, ALTLIST])),
                  (0x8009D054, rng.choice([0, 3, 20, 32]))]
        if rng.random() < 0.6:  # pin one side to the equipped cursor slot
            seeds.append((0x800C0E20, rng.getrandbits(16) | (rng.choice(sl) & 0xFF) << 16 |
                          rng.getrandbits(8) << 24))
        return seeds, (rng.choice([0, 1, 2]), rng.choice([-1, 0, 1, 2, 5, 10]) & 0xFFFFFFFF, 0, 0)
    a, c = rng.choice([0x33, 0x34]), rng.choice([0x33, 0x34])
    return seeds, (a, rng.randrange(0, 16), c, rng.randrange(0, 16))
ACTOR, PART = 0x80150A00, 0x80150C00


def gen_actor(rng):
    seeds, args = gen_edges(rng, 4)
    n = args[3]
    px, pz = [(a + 0x8000 & 0xFFFF) - 0x8000 for a in args[:2]]
    seeds = [sd for sd in seeds if not (EOBJ <= sd[0] < EOBJ + 0x60)]
    seeds = [sd for sd in seeds if sd[0] != 0x8009CE2C]
    step = rng.choice([0, 1 << 14, 1 << 16, 3 << 16, 20 << 16])
    f28 = (px << 16) + rng.getrandbits(16)
    f30 = (pz << 16) + rng.getrandbits(16)
    f40 = f28 + rng.randrange(-step, step + 1)
    f48 = f30 + rng.randrange(-step, step + 1)
    words = {0x24: rng.getrandbits(16) | rng.choice([0x1000, 0x800, 0x1800, 0xFFFF]) << 16,
             0x28: f28, 0x2C: rng.getrandbits(32), 0x30: f30, 0x40: f40,
             0x44: rng.getrandbits(32), 0x48: f48,
             0x224: rng.choice([1, 16, 64, 200, 800, 0xFFFF]) | rng.getrandbits(16) << 16}
    for off in range(0, 0x230, 4):
        v = words.get(off)
        if v is None:
            v = rng.getrandbits(32) if rng.random() < 0.1 else 0
        seeds.append((ACTOR + off, v & 0xFFFFFFFF))
    seeds += [(0x8009D254, ACTOR), (0x8009D2F8, PTS), (0x8009D264, n | rng.getrandbits(16) << 16)]
    return seeds, (0, 0, 0, 0)


def gen_particle(rng):
    seeds = [(PART + i, rng.getrandbits(32)) for i in range(0, 0x10, 4)]
    seeds.append((0x800E27EC, rng.choice([0, 1, 0x1F, 0x20, 0x21, 0x40, rng.getrandbits(32)])))
    return seeds, (rng.choice([0, 1, 1, 1, 3, 0xFFFFFFFF]), PART, 0, 0)


def bcd(n):
    return ((n // 10) << 4) | (n % 10)


def gen_ds(rng):
    pos = rng.choice([-1, rng.randrange(0, 5000), rng.randrange(0, 5000)])
    last = rng.choice([pos - 1, pos, pos + 1, rng.randrange(-5, 5000)])
    rcb = [pos, last, rng.choice([0, 0x8005E8A4, 0x8005E8A4]), rng.choice([0, 1, 1, 2]),
           rng.choice([0, 0, -2, -7]), rng.getrandbits(32), rng.getrandbits(32),
           rng.choice([0, 1, 2]), rng.choice([0, 2, 5])]
    seeds = [(0x8009B6EC + 4 * i, v & 0xFFFFFFFF) for i, v in enumerate(rcb)]
    lba = rng.randrange(150, 330000)
    mm, ss, ff = lba // 4500, lba // 75 % 60, lba % 75
    st = rng.getrandbits(8) & ~0x20
    seeds.append((0x8009B580, rng.getrandbits(8) | st << 8 | bcd(mm) << 16 | bcd(ss) << 24))
    seeds += [(0x8009B584, bcd(ff) | rng.getrandbits(24) << 8), (0x800B8AB4, rng.getrandbits(32)),
              (0x800B8AB8, rng.getrandbits(32)), (0x800A3608, rng.choice([0, 1])),
              (0x8009D124, 0), (0x80150900, rng.getrandbits(32))]
    return seeds, (rng.choice([0, 1, 1, 1, 2, 3, 4, 4, 5]), 0x80150900, 0, 0)
CTL, SEQ = 0x80150700, 0x80150800


def gen_seq(rng):
    seeds = [(0x8009D2C8, CTL)]
    for off in range(0, 0x80, 4):
        seeds.append((CTL + off, rng.getrandbits(32)))
    mask = rng.choice([0, 1, 0xFFFFFF, 0x800001, rng.getrandbits(24), rng.getrandbits(24)])
    words = [rng.getrandbits(8) << 24 | mask, rng.getrandbits(32), rng.getrandbits(32),
             rng.getrandbits(32)]
    offs = [rng.randrange(0, 0x400) for _ in range(24)]
    blob = b''.join(struct.pack('<I', w) for w in words) + b''.join(struct.pack('<H', o) for o in offs)
    blob += bytes(0x80 - len(blob))
    for i in range(0, 0x80, 4):
        seeds.append((SEQ + i, struct.unpack_from('<I', blob, i)[0]))
    for k in range(24):
        seeds.append((0x800BA560 + 0x11C * k + 0xF0, rng.choice([k, rng.getrandbits(5), 0x18, 0xFFFFFFFF])))
        seeds.append((0x800B8AC0 + 0x11C * k + 0xF4, rng.getrandbits(32)))
    for off in range(0, 0x40, 4):
        seeds.append((0x800B2900 + off, rng.getrandbits(32)))
    for a in (0x8009D2C4, 0x8009D2DC, 0x8009CDE8, 0x800BCD50, 0x800BCD5C):
        seeds.append((a, rng.getrandbits(32)))
    return seeds, (SEQ, 0, 0, 0)


def gen_edges(rng, entry):
    n = rng.choice([1, 2, 3, 3, 4, 4, 5, 6, 8, 11])
    span = rng.choice([64, 300, 1500, 6000])
    cx, cz = rng.randrange(-4000, 4000), rng.randrange(-4000, 4000)
    seeds = []
    pts = []
    for k in range(n):
        x = cx + rng.randrange(-span, span + 1)
        z = cz + rng.randrange(-span, span + 1)
        pts.append((x, z))
        seeds.append((PTS + 8 * k, (rng.getrandbits(16)) | ((x & 0xFFFF) << 16)))
        seeds.append((PTS + 8 * k + 4, (rng.getrandbits(16)) | ((z & 0xFFFF) << 16)))
    seeds.append((0x8009CE2C, rng.choice([1, 16, 64, 200, 500, 2000]) | (rng.getrandbits(16) << 16)))
    px, pz = rng.choice(pts)
    px += rng.randrange(-span, span + 1) // 2
    pz += rng.randrange(-span, span + 1) // 2
    if entry == 3:
        f40, f48 = (px << 16) + rng.getrandbits(16), (pz << 16) + rng.getrandbits(16)
        step = rng.choice([0, 1 << 12, 1 << 16, 5 << 16, 40 << 16])
        f28 = f40 + rng.randrange(-step, step + 1)
        f30 = f48 + rng.randrange(-step, step + 1)
        for off in range(0, 0x60, 4):
            v = {0x28: f28, 0x30: f30, 0x40: f40, 0x48: f48}.get(off, rng.getrandbits(32))
            seeds.append((EOBJ + off, v))
        return seeds, (EOBJ, PTS, n, rng.randrange(0, n))
    return seeds, (((px + 0x8000) & 0xFFFF) - 0x8000 & 0xFFFFFFFF, ((pz + 0x8000) & 0xFFFF) - 0x8000 & 0xFFFFFFFF, PTS, n)


def gen_68710(rng):
    seeds = []
    n = rng.choice([0, 1, 2, 3, 5, 8, 12, 16, 24, 40, 64])
    seeds.append((OBJ + 0x24, rng.getrandbits(16) | (n << 16)))
    cells = []
    for _ in range(n):
        if rng.random() < 0.08:
            e = rng.getrandbits(32)
        else:
            x = 16 * rng.randrange(0, 12) + (rng.choice([0, 0, 0, 1, 8]) if rng.random() < 0.1 else 0)
            y = 16 * rng.randrange(0, 8)
            e = (x << 22) | (y << 12) | rng.getrandbits(12)
        cells.append(e)
    for i, e in enumerate(cells):
        seeds.append((CELLS + 4 * i, e))
    if cells and rng.random() < 0.8:
        e = rng.choice(cells)
        cx, cy = e >> 22, (e >> 12) & 0x3FF
    else:
        cx, cy = rng.randrange(-32, 0x200), rng.randrange(-32, 0x200)
    seeds += [(0x8009CDB8, rng.getrandbits(32)), (0x8009CDBC, rng.getrandbits(8)),
              (0x8009CDC0, cx & 0xFFFFFFFF), (0x8009CDC4, cy & 0xFFFFFFFF)]
    args = (OBJ, CELLS, rng.choice([0, 1, 0x80, 0xFF]) , rng.choice([0, 0, 0, 1, 0x40]))
    return seeds, args


def record_seeds(rng):
    rec = bytearray(rng.getrandbits(8) for _ in range(0x100))
    phase = rng.choice([0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, 0x80, 0xFD, 0xFE, 0xFF, 0xFF])
    rec[0x46] = phase
    rec[0x44] = rng.choice([0, 1, 2, 3, 4, 8, 16, 34])
    rec[0x47] = rng.choice([0, 1, 2, 7, 0xFE, 0xFF])
    rec[0xEA] = rng.choice([0, 1, 2, 3, 8, 0xFF])
    for o in (0x36, 0x37, 0x4A, 0x50, 0xE8):
        if rng.random() < 0.5:
            rec[o] = 0
    if rng.random() < 0.4:
        struct.pack_into('<H', rec, 0xE6, 0)
    struct.pack_into('<I', rec, 0x30, TX)
    struct.pack_into('<I', rec, 0x3C, RX)
    struct.pack_into('<I', rec, 0x14, rng.choice([0, 0, 0x80150800]))
    struct.pack_into('<I', rec, 0x18, rng.choice([0, 0, 0, 0x80084F8C]))
    struct.pack_into('<I', rec, 0x04, rng.choice([0, 0, 1]))
    struct.pack_into('<I', rec, 0xEC, rng.choice([0, 8, rng.getrandbits(16)]))
    rx = bytearray(rng.getrandbits(8) for _ in range(0x40))
    rx[0] = rng.choice([rx[0], 0xF3, 0xF0 | (rx[0] & 15), 0x41, 0x73, 0x12, 0x00])
    if rng.random() < 0.5:
        rx[1] = 0
    tx = bytearray(rng.getrandbits(8) for _ in range(0x40))
    seeds = []
    for base, blob in ((REC, rec), (RX, rx), (TX, tx)):
        for i in range(0, len(blob), 4):
            seeds.append((base + i, struct.unpack_from('<I', blob, i)[0]))
    seeds += [(0x8009B728, 0x8005E8A4), (0x8009B740, 0x8004E970),
              (0x8009CF0C, rng.choice([0, 1])), (0x8009D124, 0)]
    return seeds


# Synthetic pan-law table (D_8009B8F8, 256 shorts incl. the mono D_8009B9F8),
# applied after the range fill in every case on both sides (P7_common).
_prng = random.Random(0x8009B8F8)
SYNTH_COMMON = [(0x8009B8F8 + i, _prng.getrandbits(32)) for i in range(0, 0x200, 4)]


def run(ex, entry, seeds, args):
    ram = bytearray(0x200000)
    ram[0x10000:0x10000 + len(ex) - 0x800] = ex[0x800:]
    for lo, size in RANGES:
        ram[lo:lo + size] = bytes(size)
    for a, v in SYNTH_COMMON + list(seeds):
        struct.pack_into('<I', ram, a & 0x1FFFFF, v & 0xFFFFFFFF)
    global LAST_RAM
    LAST_RAM = ram
    regs = execute(ram, ENTRY[entry], args,
                   instruction_budget=60000 if entry in (3, 7) else 400000)
    state = b''.join(bytes(ram[lo:lo + size]) for lo, size in RANGES)
    return regs[2] & 0xFFFFFFFF, fnv(state)


DUMP = int(sys.argv[sys.argv.index('--dump') + 1]) if '--dump' in sys.argv else None
LAST_RAM = None


def main():
    ex = (ROOT / 'build/extracted/disc1/SLUS_006.62').read_bytes()
    assert hashlib.sha1(ex).hexdigest() == EXE_SHA1
    rng = random.Random(0x80068710)
    runs, words, cases = [], [], []
    rejected = {}
    common = [(0x80000000 + a + i, struct.unpack_from('<I', ex, a - 0x10000 + 0x800 + i)[0])
              for a in (0x960BC, 0x96250) for i in range(0, 0x180, 4)] + SYNTH_COMMON
    common_addr = {a for a, _ in common}
    for entry, count in CASES_PER_ENTRY.items():
        for _ in range(count):
            while True:
                if entry == 0:
                    seeds, args = gen_68710(rng)
                elif entry in (3, 4):
                    seeds, args = gen_edges(rng, entry)
                elif entry == 5:
                    seeds, args = gen_seq(rng)
                elif entry == 6:
                    seeds, args = gen_ds(rng)
                elif entry == 7:
                    seeds, args = gen_actor(rng)
                elif entry == 8:
                    seeds, args = gen_particle(rng)
                elif entry == 12:
                    seeds, args = gen_voice(rng)
                elif entry in (9, 10, 11, 13):
                    seeds, args = gen_inv(rng, entry)
                else:
                    seeds, args = record_seeds(rng), (REC, 0, 0, 0)
                try:
                    result, h = run(ex, entry, seeds, args)
                    break
                except AssertionError as exc:
                    if entry not in (3, 4, 7, 13):
                        raise
                    rejected[entry] = rejected.get(entry, 0) + 1
                    if entry == 13 and rejected[entry] <= 3:
                        print('entry 13 rejected:', exc)
            # A zero word inside an observed range (and outside the common
            # block) is already zero-filled.  Consecutive words form one run.
            kept = [(a, v & 0xFFFFFFFF) for a, v in seeds
                    if v or a in common_addr or
                    not any(lo <= (a & 0x1FFFFF) < lo + sz for lo, sz in RANGES)]
            first = len(runs)
            for a, v in kept:
                if runs and runs[-1][0] + 4 * runs[-1][1] == a and len(runs) > first:
                    runs[-1][1] += 1
                else:
                    runs.append([a, 1, len(words)])
                words.append(v)
            cases.append((entry, args, first, len(runs), result, h))
            if DUMP is not None and len(cases) - 1 == DUMP:
                blob = b''.join(bytes(LAST_RAM[lo:lo + sz]) for lo, sz in RANGES)
                (ROOT / f'build/lanes/port/p7_retail_{DUMP}.bin').write_bytes(blob)
                print('dumped case', DUMP)
                return
    out = ['/* Generated by pc_port/tools/pe_port7_oracle.py --write-header:',
           ' * retail-instruction results for the round-7 hand adapters. */',
           'static const uint32_t P7_ranges[][2]={']
    out += [f'    {{0x{lo:X}u,0x{size:X}u}},' for lo, size in RANGES]
    # libgte lookup tables read by func_80078004 / func_80078134 (same
    # precedent as retail_polygon_boundary_cases.h POLY_common) plus the
    # synthetic pan-law block.
    out += ['};', 'static const uint32_t P7_common[][2]={']
    out += [f'    {{0x{a:08X}u,0x{v:08X}u}},' for a, v in common]
    out += ['};', '/* seeded runs: {guest address, word count, first word in P7_words} */',
            'static const uint32_t P7_runs[][3]={']
    out += [f'    {{0x{a:08X}u,{n},{w}}},' for a, n, w in runs]
    out += ['};', 'static const uint32_t P7_words[]={']
    out += ['    ' + ','.join(f'0x{v:X}' for v in words[i:i + 12]) + ','
            for i in range(0, len(words), 12)]
    out += ['};', 'static const struct { unsigned entry; uint32_t args[4]; unsigned first,end;'
            ' uint32_t result; uint64_t hash; } P7_cases[]={']
    for entry, args, first, end, result, h in cases:
        a = ','.join(f'0x{x & 0xFFFFFFFF:X}u' for x in args)
        out.append(f'    {{{entry},{{{a}}},{first},{end},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out += ['};', '']
    header = '\n'.join(out)
    path = ROOT / 'pc_port/tests/retail_port7_cases.h'
    if '--write-header' in sys.argv:
        path.write_text(header)
    else:
        assert path.read_text() == header, 'retail_port7_cases.h is stale'
    print(f'PASS {len(cases)} original round-7 cases ({len(words)} seeded words, {len(runs)} runs)')
    print('regenerated (retail DIV break / budget / unsupported):', rejected)


if __name__ == '__main__':
    main()

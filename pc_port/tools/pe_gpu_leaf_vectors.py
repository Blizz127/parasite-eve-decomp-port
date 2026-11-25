#!/usr/bin/env python3
"""Black-box vectors for small libgpu packet-word leaves (original code from
the user's EXE under the test-only interpreter).

Each case:
- loads the EXE text;
- fills a record zone (0x80100000-0x80101FFF) with seeded words;
- sets the two halfwords at 0x80095750/0x80095752 (global screen limits)
  from seeded or edge values;
- calls one entry with seeded arguments.

Argument models: 0x80076170 and 0x80076208 take two integers (seeded, plus
signed 16-bit edges and full 32-bit values). 0x800762BC takes a record
pointer, or 0 in about one case in eight. 0x80077AC4 takes two record
pointers, which may be equal.

Each line records the return value, every data load in order (address,
size, value) and every store in order. Stack traffic in the interpreter's
frame range 0x801F0000-0x801FF000 is not logged.

Pattern 1 (0x80076170 / 0x80076208): every pair from a signed 16-bit edge
list for the arguments, against limits from an edge list.
Pattern 1 (0x800762BC): every record byte/halfword field from edge lists.

SetDefDrawEnv 0x80074924 (`drawenv`): (env pointer, x, y, w, h); h is
the fifth argument and is written straight into the test RAM at
0x801FF010 (the original's 56(sp) after its 40-byte frame), so that write
is not logged as a store (the original's load of it appears in L). The
video-mode word 0x800956EC is seeded from (0, 1, 2, 0xFFFFFFFF) or a seed.
Integers mix 16-bit edges around 256 and 288 and full 32-bit values.

Usage: pe_gpu_leaf_vectors.py OUT.txt [ENTRY ... | drawenv]
"""
import struct
import sys

import pe_pad_leaf_vectors as pl

v = pl.v
ENTRIES = (0x80076170, 0x80076208, 0x800762BC, 0x80077AC4)
ZONE = (0x80100000, 0x2000)
LIMITS = 0x95750
E16 = (0, 1, 2, 0xFF, 0x100, 0x1FF, 0x200, 0x3FF, 0x400, 0x7FF, 0x800,
       0x7FFF, 0x8000, 0xFFFF, 0xFFFE, 0xFC00, 0xF800)
E32 = E16 + (0x10000, 0x12345, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF,
             0xFFFF0000, 0x0000FFFF, 0x00010400)
LIM = (0, 1, 2, 0x100, 0x140, 0x1E0, 0x200, 0x280, 0x3FF, 0x400, 0x401,
       0x7FFF, 0x8000, 0xFFFF)


def xs32(x):
    x ^= (x << 13) & 0xFFFFFFFF
    x ^= x >> 17
    x ^= (x << 5) & 0xFFFFFFFF
    return x & 0xFFFFFFFF


def num(x):
    k = x % 5
    if k == 4:
        return E32[(x >> 8) % len(E32)]
    return (x >> 8) & (0xF, 0x3FF, 0xFFFF, 0xFFFFFFFF)[k]


def prep(base, seed):
    ram = bytearray(base)
    x = seed * 2654435761 & 0xFFFFFFFF or 1
    for a in range(ZONE[0] & 0x1FFFFF, (ZONE[0] & 0x1FFFFF) + ZONE[1], 4):
        x = xs32(x)
        struct.pack_into('<I', ram, a, x)
    x = xs32(x)
    for k in range(2):
        x = xs32(x)
        lim = LIM[x % len(LIM)] if x & 0x100 else (x >> 12) & 0xFFFF
        struct.pack_into('<H', ram, LIMITS + 2 * k, lim)
    return ram, x


def ptr(x):
    return ZONE[0] + (x % (ZONE[1] - 0x20)) // 4 * 4


def args_for(entry, x):
    x = xs32(x)
    if entry == 0x800762BC:
        return [0 if x % 8 == 0 else ptr(x >> 3)], x
    if entry == 0x80077AC4:
        a = ptr(x)
        x = xs32(x)
        return [a, a if x % 16 == 0 else ptr(x)], x
    a = num(x)
    x = xs32(x)
    return [a, num(x)], x


def run(ram, entry, args):
    pl.LOADS.clear()
    pl.STORES.clear()
    pl.CUR[0] = entry
    regs = {i: a for i, a in enumerate(args, 4)}
    regs[31] = v.RET
    r = v.execute(ram, entry, (), stop_at={v.RET}, initial_regs=regs,
                  instruction_budget=10000)
    return r[2] & 0xFFFFFFFF, list(pl.LOADS), list(pl.STORES)


def line(entry, s, pattern, args, res):
    ret, loads, stores = res
    return '%08X %d %d A %s RET %08X L %s S %s' % (
        entry, s, pattern, ' '.join('%X' % a for a in args), ret,
        ' '.join('%08X:%d=%X' % l for l in loads) or '-',
        ' '.join('%08X:%d=%X' % t for t in stores) or '-')


HEDGE = (0, 1, 255, 256, 257, 287, 288, 289, 240, 480, 512, 0x7FFF, 0x8000,
         0xFFFF, 0xFFFFFFFF, 0xFFFF8000, 0x80000000, 0x7FFFFFFF, 0x10100)


def drawenv_cases(base):
    out = []
    for k in range(6000):
        s = 0xA0000 + k
        ram, x = prep(base, s)
        x = xs32(x)
        mode = (0, 1, 2, 0xFFFFFFFF)[x % 4] if x & 0x10 else x
        struct.pack_into('<I', ram, 0x956EC, mode & 0xFFFFFFFF)
        args = [ptr(x >> 5)]
        for i in range(4):
            x = xs32(x)
            args.append(HEDGE[(x >> 8) % len(HEDGE)] if x % 3 else num(x))
        if k < len(HEDGE) * 4:            # every h edge against each video mode
            args[4] = HEDGE[k // 4]
            struct.pack_into('<I', ram, 0x956EC, (0, 1, 2, 0xFFFFFFFF)[k % 4])
        struct.pack_into('<I', ram, 0x1FF010, args[4] & 0xFFFFFFFF)
        out.append(line(0x80074924, s, 0, args, run(ram, 0x80074924, args[:4])))
    return out


def main():
    if sys.argv[2:] == ['drawenv']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = drawenv_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out))
        return
    entries = [int(a, 16) for a in sys.argv[2:]] or ENTRIES
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out = []
    for entry in entries:
        for seed in range(1, 2001):
            s = seed * 7919
            ram, x = prep(base, s)
            args, x = args_for(entry, x)
            out.append(line(entry, s, 0, args, run(ram, entry, args)))
        i = 0
        if entry in (0x80076170, 0x80076208):
            for a0 in E16:
                for a1 in E16:
                    for lim in LIM:
                        s = 0x80000 + i
                        i += 1
                        ram, x = prep(base, s)
                        struct.pack_into('<H', ram, LIMITS + (i & 2), lim)
                        args = [a0 | (0xFFFF0000 if i & 4 else 0), a1]
                        out.append(line(entry, s, 1, args, run(ram, entry, args)))
        if entry == 0x800762BC:
            b8 = (0, 1, 7, 8, 9, 0x7F, 0x80, 0xF8, 0xFF)
            h16 = (0, 1, 7, 8, 0x80, 0xF8, 0xFF, 0x100, 0x7FFF, 0x8000, 0xFFF8, 0xFFFF)
            for bx in b8:
                for by in b8:
                    for hw in h16:
                        for hh in h16:
                            if (bx + by) % 3 and hw != hh:
                                continue
                            s = 0x90000 + i
                            i += 1
                            ram, x = prep(base, s)
                            p = ptr(x)
                            m = p & 0x1FFFFF
                            ram[m] = bx
                            ram[m + 2] = by
                            struct.pack_into('<HH', ram, m + 4, hw, hh)
                            out.append(line(entry, s, 1, [p], run(ram, entry, [p])))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out))


if __name__ == '__main__':
    main()

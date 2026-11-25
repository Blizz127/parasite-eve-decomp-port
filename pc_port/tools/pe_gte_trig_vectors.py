#!/usr/bin/env python3
"""Black-box vectors for the integer libgte trig leaves (original code from
the user's EXE under the test-only interpreter): rsin 0x80077CF4, its
helper 0x80077D30, rcos 0x80077DC4, RotMatrix 0x800794C4, RotMatrixYXZ
0x80079754, RotMatrixZYX 0x800799E4 and RotMatrixZ 0x80079E14.

Each case loads the EXE (text and data, so the original's tables are in
guest memory at their own addresses), fills an angle zone (0x80100000-
0x80100FFF) and an output zone (0x80101000-0x80101FFF) with seeded words,
and calls one entry.

Argument models:
- 0x80077CF4 / 0x80077D30 / 0x80077DC4: one integer. Pattern 0 is seeded
  (small, 16-bit and full 32-bit values); pattern 1 sweeps -0x11000 to
  0x11000 in steps of 3 plus signed edges.
- 0x800794C4 / 0x80079754 / 0x800799E4: (angle vector pointer, matrix
  pointer). Pattern 0
  seeds the three angle halfwords; pattern 1 sweeps one halfword over
  0x0000-0xFFFF in steps of 7 (others seeded) for each of the three.
- 0x80079E14: (angle, matrix pointer). Pattern 0 seeded; pattern 1 sweeps
  the angle like the sine entries.

Pattern 2 replaces both lookup tables in the test memory with synthetic
seeded values (the quarter table 0x8009589C, 0x401 halfwords, and the
sin/cos table 0x800966EC, 0x1000 words; every value nonzero, within
+-0x1000). Real data has zeros at the quadrant boundaries, which hides the
sign choices there. Pattern 2 cases are 1000 seeded cases per entry, plus,
for the scalar entries, every argument in 0..0x1000 and +-2 around each
multiple of 0x400 from -0x2000 to 0x2000. For the matrix entries they are
each angle halfword at those boundary values, negated too. The synthetic
values are test data, not the game's.

Each line records the return value, every data load in order (address,
size, value) and every store in order, in the format of
pe_gpu_leaf_vectors.py (stack-frame traffic is not logged). Check with
pe_gte_trig_vectors_check.c.

Faithful-only: a case is dropped when the original touched memory the
harness does not log other than the real stack frame (0x801FE000-
0x801FEFFF): kernel RAM below 0x1000, the interpreter's jump buffer, or the
rest of the 0x801F0000 band. This happens only for 0x80077D30 given raw
arguments (its table index is then out of range); the count is printed.
0x80077D30 cases are also limited to arguments -0x10000..0x10FFF (signed),
so its table reads stay inside the 2 MiB of RAM without wrapping; the
interpreter wraps addresses, which is not something to model.

Usage: pe_gte_trig_vectors.py OUT.txt [ENTRY ...]
"""
import struct
import sys

import pe_gpu_leaf_vectors as gv

v = gv.v
pl = gv.pl
FRAME = (0x1FE000, 0x1FF000)
UNFAITHFUL = [False]
_logged = pl._logged


def _faithful_logged(a):
    r = _logged(a)
    if not r and not FRAME[0] <= a < FRAME[1]:
        UNFAITHFUL[0] = True
    return r


pl._logged = _faithful_logged
SCALAR = (0x80077CF4, 0x80077D30, 0x80077DC4)
VECTOR = (0x800794C4, 0x80079754, 0x800799E4)
ENTRIES = SCALAR + VECTOR + (0x80079E14,)
AZONE = (0x80100000, 0x1000)
OZONE = (0x80101000, 0x1000)
EDGES = (0, 1, 2, 0x3FF, 0x400, 0x401, 0x7FF, 0x800, 0x801, 0xBFF, 0xC00,
         0xFFF, 0x1000, 0x1001, 0x7FFF, 0x8000, 0xFFFF, 0x10000, 0x7FFFFFFF,
         0x80000000, 0x80000001, 0xFFFFFFFF, 0xFFFFF000, 0xFFFFF001,
         0xFFFFF800, 0xFFFF8000, 0xFFFF7FFF)


def prep(base, seed):
    ram = bytearray(base)
    x = seed * 2654435761 & 0xFFFFFFFF or 1
    for a in range(AZONE[0] & 0x1FFFFF, (OZONE[0] & 0x1FFFFF) + OZONE[1], 4):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, x)
    return ram, x


QTABLE = (0x9589C, 0x401, 2)
SCTABLE = (0x966EC, 0x1000, 4)


def synth_tables(ram, x):
    """Synthetic table values in place of the game's (pattern 2)."""
    for base, n, size in (QTABLE, SCTABLE):
        for i in range(n * size // 2):
            x = gv.xs32(x)
            val = (x % 0x2000) - 0x1000 or 1
            struct.pack_into('<h', ram, base + 2 * i, val)
    return x


def ptr(zone, x, align):
    return zone[0] + (x % (zone[1] - 0x40)) // align * align


def num(x):
    k = x % 4
    return (x >> 8) & (0xFFF, 0xFFFF, 0x1FFFF, 0xFFFFFFFF)[k]


def main():
    entries = [int(a, 16) for a in sys.argv[2:]] or ENTRIES
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out = []
    dropped = 0
    for entry in entries:
        cases = []
        for seed in range(1, 3001):
            cases.append((seed * 7919, 0, None))
        if entry in SCALAR or entry == 0x80079E14:
            cases += [(0x80000 + i, 1, a & 0xFFFFFFFF)
                      for i, a in enumerate(list(range(-0x11000, 0x11001, 3)) + list(EDGES))]
        else:
            cases += [(0x80000 + k * 0x10000 + h, 1, (k, h))
                      for k in range(3) for h in range(0, 0x10000, 7)]
        bounds = [k * 0x400 + d for k in range(-8, 9) for d in range(-2, 3)]
        cases += [(0xC0000 + i, 2, None) for i in range(1000)]
        if entry in SCALAR or entry == 0x80079E14:
            cases += [(0xD0000 + i, 2, a & 0xFFFFFFFF)
                      for i, a in enumerate(list(range(0, 0x1001)) + bounds)]
        else:
            cases += [(0xE0000 + k * 0x1000 + i, 2, (k, h & 0xFFFF))
                      for k in range(3) for i, h in enumerate(bounds)]
        for s, pattern, sweep in cases:
            ram, x = prep(base, s)
            if pattern == 2:
                x = synth_tables(ram, x)
            x = gv.xs32(x)
            if entry in SCALAR:
                args = [num(x) if sweep is None else sweep]
            elif entry == 0x80079E14:
                y = gv.xs32(x)
                args = [num(x) if sweep is None else sweep, ptr(OZONE, y, 4)]
            else:
                a = ptr(AZONE, x, 2)
                x = gv.xs32(x)
                args = [a, ptr(OZONE, x, 4)]
                if sweep is not None:
                    k, h = sweep
                    struct.pack_into('<H', ram, (a & 0x1FFFFF) + 2 * k, h)
            if entry == 0x80077D30 and not -0x10000 <= (args[0] ^ 0x80000000) - 0x80000000 <= 0x10FFF:
                continue
            UNFAITHFUL[0] = False
            res = gv.run(ram, entry, args)
            if UNFAITHFUL[0]:
                dropped += 1
                continue
            out.append(gv.line(entry, s, pattern, args, res))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'dropped (unfaithful)', dropped)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Black-box vectors for small libds / libcd leaves (original code from the
user's EXE under the test-only interpreter): CQ_clear_queue 0x8007E594,
the command-queue pop 0x8007E5C4, the next-slot getter 0x8007E6B0 and the
status-lane getter 0x8007FBF0.

Each case loads the EXE, fills a record zone (0x80100000-0x80101FFF) and the
command-queue globals 0x800A3540-0x800A361F (eight 24-byte records, then
the index / count words) with seeded words, and calls one entry. Pattern 0
seeds the queue's index words with in-range values (0..7, counts -2..9)
about half the time; otherwise index words are drawn from -0x6C00..0xE3FF
(so base + 24 * index stays inside the 2 MiB of RAM, which the interpreter
would otherwise wrap) and the count word is arbitrary. Pattern 1 sweeps
the index words over every pair of edge values (-1, 0, 1, 6, 7, 8, 9,
0x7FFFFFFF, 0x80000000). For 0x8007FBF0 the argument is drawn from -64..
+64 and from -0x26000 .. +0x58FFF (the scaled address stays inside RAM).
Cases that exhaust the instruction budget (a wild index can make the pop
loop spin) are skipped and counted. Faithful-only: a case is dropped when
the original touched memory the harness does not log (these leaves use no
stack frame, so nothing is exempt), e.g. a record index or table index
landing in the 0x801F0000 band; the count is printed.

Each line records the return value, every data load in order (address,
size, value) and every store in order (pe_gpu_leaf_vectors.py format; the
interpreter's stack frame is not logged). Check with
pe_ds_leaf_vectors_check.c.

Usage: pe_ds_leaf_vectors.py OUT.txt [ENTRY ...]
"""
import struct
import sys

import pe_gpu_leaf_vectors as gv

v = gv.v
pl = gv.pl
FRAME = (0x1FF000, 0x1FF000)   # these leaves have no stack frame: nothing is exempt
UNFAITHFUL = [False]
_logged = pl._logged


def _faithful_logged(a):
    r = _logged(a)
    if not r and not FRAME[0] <= a < FRAME[1]:
        UNFAITHFUL[0] = True
    return r


pl._logged = _faithful_logged
ENTRIES = (0x8007E594, 0x8007E5C4, 0x8007E6B0, 0x8007FBF0)
QUEUE = (0xA3540, 0xA3620)
IDX = (0xA3600, 0xA3604, 0xA3608)
EDGE = (0xFFFFFFFF, 0, 1, 6, 7, 8, 9, 0x7FFFFFFF, 0x80000000)


def prep(base, seed):
    ram, x = gv.prep(base, seed)
    for a in range(QUEUE[0], QUEUE[1], 4):
        x = gv.xs32(x)
        val = x if x % 3 else (x >> 8) % 4      # some small words, so records match
        struct.pack_into('<I', ram, a, val)
    for i, a in enumerate(IDX):
        x = gv.xs32(x)
        if x & 1:
            val = (x >> 4) % 8 if i < 2 else ((x >> 4) % 12) - 2
        elif i < 2:       # index words: record address base + 24 * idx stays in RAM
            val = (x >> 4) % 0x15000 - 0x6C00
        else:
            val = x
        struct.pack_into('<I', ram, a, val & 0xFFFFFFFF)
    return ram, x


def main():
    entries = [int(a, 16) for a in sys.argv[2:]] or ENTRIES
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out, spun, dropped = [], 0, 0
    for entry in entries:
        cases = [(seed * 7919, 0) for seed in range(1, 4001)]
        if entry in (0x8007E5C4, 0x8007E6B0):
            cases += [(0x90000 + k, 1) for k in range(len(EDGE) ** 3)]
        for s, pattern in cases:
            ram, x = prep(base, s)
            if pattern == 1:
                k = s - 0x90000
                n = len(EDGE)
                for i, a in enumerate(IDX):
                    struct.pack_into('<I', ram, a, EDGE[(k // n ** i) % n])
            x = gv.xs32(x)
            if entry == 0x8007E594:
                args = [gv.ptr(x)]
            elif entry == 0x8007FBF0:
                # 0x8009B574 + 4 * a0 stays inside the 2 MiB of RAM
                args = [((x >> 8) % 129 - 64) & 0xFFFFFFFF if x & 1 else
                        ((x >> 8) % 0x7F000 - 0x26000) & 0xFFFFFFFF]
            else:
                args = []
            UNFAITHFUL[0] = False
            try:
                res = gv.run(ram, entry, args)
            except AssertionError as e:
                if 'instruction budget' not in str(e):
                    raise
                spun += 1
                continue
            if UNFAITHFUL[0]:
                dropped += 1
                continue
            out.append(gv.line(entry, s, pattern, args, res))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'skipped (budget)', spun, 'dropped (unfaithful)', dropped)


if __name__ == '__main__':
    main()

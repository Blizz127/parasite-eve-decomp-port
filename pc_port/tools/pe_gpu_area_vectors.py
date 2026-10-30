#!/usr/bin/env python3
"""Black-box vectors for SetTexWindow (0x80075B4C) / SetDrawArea (0x80075B84).

Runs the original code from the user's EXE under the test-only interpreter
(pe_battle_hud_oracle.execute) and writes one TSV row per case: entry, VRAM
width/height at 0x80095750/52, RECT (or NULL), packet words.  Fails if any RAM
outside the packet and the stack changes.  The rows are checked against the
host model by pe_gpu_area_vectors_check.c.  Output stays outside the repo.
Usage: pe_gpu_area_vectors.py OUT.tsv
"""
import sys, struct, itertools
from pe_menu_window_oracle import fixture, ROOT
from pe_battle_hud_oracle import execute
exe = (ROOT/'build/disc1.candidate.exe').read_bytes()
PKT = 0x160000
vals = (0, 1, 7, 8, 9, 15, 16, 17, 31, 32, 33, 64, 100, 127, 128, 129, 200, 255, 256, 257,
        320, 511, 512, 513, 640, 1000, 1023, 1024, 1025, 2047, 2048, 4095, 32767,
        -1, -7, -8, -9, -32, -100, -255, -256, -257, -1024, -32768)
BASE = {}
def run(entry, rect, limh=512, null=False, limw=1024):
    key = (entry, null)
    if key not in BASE:
        r0, _, a0 = fixture(exe, dict(entry=entry, rect=(0, 0, 32, 32), null=null))
        BASE[key] = (bytes(r0), a0)
    r = bytearray(BASE[key][0]); args = BASE[key][1]
    struct.pack_into('<4h', r, 0x149C00, *rect)
    struct.pack_into('<H', r, 0x95750, limw); struct.pack_into('<H', r, 0x95752, limh)
    struct.pack_into('<III', r, PKT, 0xA5A5A5A5, 0x5A5A5A5A, 0xC3C3C3C3)
    before = bytes(r)
    execute(r, (0x80075B4C, 0x80075B84)[entry - 5], args)
    diff = []
    for b in range(0, len(r), 4096):
        if r[b:b+4096] != before[b:b+4096]:
            diff += [i for i in range(b, min(b+4096, len(r))) if r[i] != before[i] and not (PKT <= i < PKT + 12)]
    # stack writes below sp are scratch; report anything else
    other = [i for i in diff if not (0x1FE000 <= i < 0x1FF000)]
    w = struct.unpack_from('<III', r, PKT)
    return w, other
out = []
rng = __import__('random').Random(20261030)
rects = set()
for _ in range(1500):
    rects.add(tuple(rng.choice(vals) if rng.random() < .6 else rng.randint(-32768, 32767) for _ in range(4)))
for x, y, w, h in itertools.product((0, 5, 8, 100, 255, 256, 1020, -20), (0, 3, 224, 510, 511, -30), (0, 1, 8, 16, 32, 100, 256), (0, 16, 64, 255, 300)):
    rects.add((x, y, w, h))
rects = sorted(rects)
bad = 0
for entry in (5, 6):
    for limw, limh in ((1024, 512), (1024, 1024), (512, 256), (0, 0)):
        if entry == 5 and (limw, limh) != (1024, 512):
            continue
        for rect in rects:
            w, other = run(entry, rect, limh, limw=limw)
            if other: bad += 1; print('SIDE', entry, rect, [hex(i) for i in other[:8]], file=sys.stderr)
            out.append((entry, limw, limh, rect, w))
w, other = run(5, (0, 0, 32, 32), null=True)
out.append((5, 1024, 512, None, w))
# SetDrawArea with rect = 0: the RECT is read from guest address 0, which
# holds {10, 20, 30, 40} here (the checker seeds the same bytes).
r, s, args = fixture(exe, dict(entry=6, rect=(0, 0, 32, 32)))
struct.pack_into('<4h', r, 0, 10, 20, 30, 40)
struct.pack_into('<III', r, PKT, 0xA5A5A5A5, 0x5A5A5A5A, 0xC3C3C3C3)
execute(r, 0x80075B84, (PKT | 0x80000000, 0))
out.append((6, 1024, 512, None, struct.unpack_from('<III', r, PKT)))
with open(sys.argv[1], 'w') as f:
    for e, lw, lh, rect, w in out:
        f.write(f"{e}\t{lw}\t{lh}\t{'NULL' if rect is None else ','.join(map(str, rect))}\t" + ' '.join(f'{x:08X}' for x in w) + '\n')
print('cases', len(out), 'side-effect cases', bad)
sys.exit(1 if bad else 0)

#!/usr/bin/env python3
"""Black-box vectors for the TIM image uploader at guest 0x800718D0 (libgpu
"layout" row, SDK name Gpu_LoadTimImage), original code from the user's EXE
under the test-only interpreter.  Written for the clean-room provider
pc_port/platform/pe_plat_gpu_tim.c, checked by pc_port/tools/pe_gpu_tim_check.c.

The entry takes one argument, the address of a TIM image in guest RAM, and
returns an address.  It has exactly one callee, 0x8007506C (LoadImage, a
rectangle pointer and a data pointer).  The callee is not run: reaching its
entry records the call and returns a scripted v0 (0, 1, -1 or a seeded
word), which proves the result and the second call do not depend on it.  The
callee takes a rectangle in guest memory, so each call records the eight
bytes at its first argument (what the callee would read), appended as hex.

Line: ENTRY seed pattern A tim RET r L addr:size=value... S - RETS v CALL
8007506C,a0,a1@stores#RECTHEX[ 8007506C,a0,a1@stores#RECTHEX]
(L as in pe_gpu_leaf_vectors.py; calls in the order the original made them).

Patterns:
 0  random: the flag word (offset 4), the first block size word (offset 8)
    and the rest of the neighbourhood are seeded words.
 1  well-formed TIM images: pixel modes 0..3 (the low flag bits), with and
    without a palette block, the usual ID 0x10 and other ID words, odd and
    zero rectangle sizes, data of the declared length.
 2  flag sweep: every single flag bit, every single cleared bit, and
    seeded words, each with and without a palette-sized block word.
 3  block-size edges: 0, 1, 2, 3, 4, 8, 12, 13, 16, 0x7FFFFFFF, 0x80000000,
    0x80000010, 0xFFFFFFF0, 0xFFFFFFFF and seeded words, against flags with
    and without bit 3.
 4  pointer edges: the image at the KSEG0 / KSEG1 / KUSEG alias of RAM and
    at several RAM places (derived rectangle addresses stay inside RAM).

Cases whose derived rectangle address leaves the 2 MiB of RAM are dropped
(the callee's reads are then undefined for the checker); cases with any
unlogged access by the original outside its stack frame are dropped; both
counts are printed.

Usage: pe_gpu_tim_vectors.py OUT.txt
"""
import struct
import sys

import pe_cd_leaf_vectors as cl

gv = cl.gv
v = cl.v
pl = cl.pl

ENTRY = 0x800718D0
LOADIMAGE = 0x8007506C
ZONE = 0x80100000
ZONE_SIZE = 0x2000
M32 = 0xFFFFFFFF
SCRIPTED = (0, 1, M32, None)           # None: a seeded word

BNUM_EDGES = (0, 1, 2, 3, 4, 8, 12, 13, 16, 0x1C, 0x7FFFFFFF, 0x80000000,
              0x80000010, 0xFFFFFFF0, 0xFFFFFFFC, 0xFFFFFFFF, 0x100, 0x1000)
ID_WORDS = (0x10, 0x10, 0x10, 0, 1, 0x11, 0x20, 0xFFFFFFFF, 0x80000010)


def in_ram(a, n=8):
    p = a & 0x1FFFFFFF
    return p + n <= 0x200000


def put32(ram, a, w):
    struct.pack_into('<I', ram, a & 0x1FFFFF, w & M32)


def put16(ram, a, w):
    struct.pack_into('<H', ram, a & 0x1FFFFF, w & 0xFFFF)


def ptr(x, align=4):
    return ZONE + (x % (ZONE_SIZE - 0x400)) // 4 * 4


def tim_image(ram, tim, x, pmode, clut, idw, cw, ch, iw, ih):
    """A TIM image at tim: header, optional palette block, pixel block."""
    put32(ram, tim, idw)
    put32(ram, tim + 4, pmode | (8 if clut else 0))
    p = tim + 8
    if clut:
        n = cw * ch * 2
        put32(ram, p, 12 + n)
        put16(ram, p + 4, x & 0x3FF); put16(ram, p + 6, (x >> 10) & 0x1FF)
        put16(ram, p + 8, cw); put16(ram, p + 10, ch)
        for k in range(0, n, 4):
            x = gv.xs32(x)
            put32(ram, p + 12 + k, x)
        p += 12 + n
    n = iw * ih * 2
    put32(ram, p, 12 + n)
    put16(ram, p + 4, (x >> 3) & 0x3FF); put16(ram, p + 6, (x >> 13) & 0x1FF)
    put16(ram, p + 8, iw); put16(ram, p + 10, ih)
    for k in range(0, n, 4):
        x = gv.xs32(x)
        put32(ram, p + 12 + k, x)
    return x


def run_case(base, s, pattern, build):
    """build(ram, x) -> (tim, x); returns a vector line, or None if dropped."""
    ram, x = gv.prep(base, s)
    tim, x = build(ram, x)
    x = gv.xs32(x)
    sc = SCRIPTED[x % 4]
    if sc is None:
        x = gv.xs32(x)
        sc = x
    cl.UNFAITHFUL[0] = False
    try:
        ret, loads, stores, calls = cl.run_with_callees(
            ram, ENTRY, [tim], {LOADIMAGE: sc}, {LOADIMAGE: 8})
    except Exception:
        STAT['fault'] += 1
        return None
    if cl.UNFAITHFUL[0]:
        STAT['unfaithful'] += 1
        return None
    for rec, blob, m in calls:
        if not in_ram(rec[1]) or blob is None or len(blob) != 16:
            STAT['range'] += 1
            return None
    call_text = ' '.join('%08X,%08X,%08X@%d#%s' % (rec[0], rec[1], rec[2], m, blob)
                         for rec, blob, m in calls) or '-'
    STAT['calls'][len(calls)] = STAT['calls'].get(len(calls), 0) + 1
    return gv.line(ENTRY, s, pattern, [tim], (ret, loads, stores)) + \
        ' RETS %X CALL %s' % (sc, call_text)


STAT = {'fault': 0, 'unfaithful': 0, 'range': 0, 'calls': {}}


def cases(base):
    out = []

    def add(seed, pattern, build):
        r = run_case(base, seed, pattern, build)
        if r:
            out.append(r)

    # pattern 0: random neighbourhood
    for k in range(1, 2501):
        def b(ram, x):
            x = gv.xs32(x)
            t = ptr(x >> 4)
            x = gv.xs32(x)
            flag = (0, 1, 2, 3, 8, 9, 0xA, 0xB, x & 0xF, x)[(x >> 8) % 10] if x & 1 else x
            put32(ram, t + 4, flag)
            x = gv.xs32(x)
            put32(ram, t + 8, (0, 4, 0xC, 0x10, 0x20, 0x100, x & 0xFFF, x)[x % 8])
            return t, x
        add(k * 7919, 0, b)

    # pattern 1: well-formed TIM images
    sizes = (0, 1, 2, 3, 4, 5, 7, 8, 16, 17, 32)
    seed = 0x100000
    for pmode in range(4):
        for clut in (0, 1):
            for idw in ID_WORDS:
                for rep in range(14):
                    seed += 1

                    def b(ram, x, pmode=pmode, clut=clut, idw=idw, rep=rep):
                        x = gv.xs32(x)
                        t = ptr(x >> 4)
                        x = gv.xs32(x)
                        cw, ch = sizes[x % len(sizes)], sizes[(x >> 8) % len(sizes)]
                        x = gv.xs32(x)
                        iw, ih = sizes[x % len(sizes)], sizes[(x >> 8) % len(sizes)]
                        if rep < 2:
                            cw = ch = iw = ih = 0
                        elif rep == 2:
                            cw, ch, iw, ih = 16, 1, 64, 16
                        elif rep == 3:
                            cw, ch, iw, ih = 256, 1, 1, 1
                        x = tim_image(ram, t, x, pmode, clut, idw, cw, ch, iw, ih)
                        return t, x
                    add(seed, 1, b)

    # pattern 2: flag sweep
    seed = 0x200000
    for rep in range(6):
        for bit in range(32):
            for form in range(3):
                seed += 1

                def b(ram, x, bit=bit, form=form, rep=rep):
                    x = gv.xs32(x)
                    t = ptr(x >> 4)
                    flag = (1 << bit) if form == 0 else (M32 ^ (1 << bit)) if form == 1 \
                        else (x | (1 << bit))
                    put32(ram, t + 4, flag)
                    x = gv.xs32(x)
                    put32(ram, t + 8, (0x10, 0x20, 12 + 2 * (rep + 1), x & 0xFFF)[rep % 4])
                    return t, x
                add(seed, 2, b)
    for k in range(600):
        seed += 1

        def b(ram, x):
            x = gv.xs32(x)
            t = ptr(x >> 4)
            x = gv.xs32(x)
            put32(ram, t + 4, x)
            x = gv.xs32(x)
            put32(ram, t + 8, x & 0xFFFF)
            return t, x
        add(seed, 2, b)

    # pattern 3: block-size edges against flags
    seed = 0x300000
    for flag in (0, 8, 9, 0xB, 0x10, 0xFFFFFFF7, 0xFFFFFFFF, 0x7FFFFFF8):
        for bn in BNUM_EDGES:
            for rep in range(3):
                seed += 1

                def b(ram, x, flag=flag, bn=bn):
                    x = gv.xs32(x)
                    t = ptr(x >> 4)
                    put32(ram, t + 4, flag)
                    put32(ram, t + 8, bn)
                    return t, x
                add(seed, 3, b)
    for k in range(400):
        seed += 1

        def b(ram, x):
            x = gv.xs32(x)
            t = ptr(x >> 4)
            put32(ram, t + 4, 8 | (x & 0xFFFF0))
            x = gv.xs32(x)
            put32(ram, t + 8, x if x & 1 else x & 0x7FF)
            return t, x
        add(seed, 3, b)

    # pattern 4: pointer edges
    seed = 0x400000
    places = (0x80000008, 0x80010000, 0x80100000, 0x80101F00, 0x800A0000,
              0x801C0000, 0x801EFF00, 0x80000100, 0x80180000)
    for place in places:
        for alias in (0x00000000, 0x80000000, 0xA0000000):
            for clut in (0, 1):
                for rep in range(4):
                    seed += 1

                    def b(ram, x, place=place, alias=alias, clut=clut, rep=rep):
                        t = (place & 0x1FFFFFFF) | alias
                        x = gv.xs32(x)
                        x = tim_image(ram, t, x, x & 3, clut, 0x10,
                                      (4, 16, 0, 5)[rep], 1, (8, 0, 3, 64)[rep], (2, 5, 0, 1)[rep])
                        return t, x
                    add(seed, 4, b)
    return out


def main():
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    out = cases(bytes(v.fixture(exe, 0, 0, 0)))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'dropped', {k: STAT[k] for k in ('fault', 'unfaithful', 'range')},
          'calls-per-case', dict(sorted(STAT['calls'].items())))


if __name__ == '__main__':
    main()

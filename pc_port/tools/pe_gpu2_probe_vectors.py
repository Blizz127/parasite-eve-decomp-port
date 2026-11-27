#!/usr/bin/env python3
"""gpu2d-probe: crafted-table vectors for the libpress expander func_8010BD4C (retail-derived: keep the output under scratch,
commit only hashes).  Each case replaces a PREFIX of the 0xE0D-byte compact table by a crafted token stream (the original
reads only that prefix: the stream ends with the two-byte terminator), runs the original as a black box with the same
interpreter and acceptance rules as pe_gpu2_vectors.py `expand`, and emits a line in the same format, so
`pe_gpu2_gpu2d_check -DCHK_EXPAND <vector file> ovl_039F.bin <base.bin>` checks it (base.bin = the gpu2d expand base output).
Usage (repo root): pe_gpu2_probe_vectors.py OUT.txt SHARD NSHARDS TARGET
The token streams are generated from a seeded PRNG; the generator never looks at the provider."""
import hashlib
import random
import sys

import pe_gpu2_vectors as V

C = V.C
TOKEN_LEN_MAX = 0xE0D
# 0x80180000 is not used: the fixture RAM holds one non-zero byte inside that window, and cases whose stream is shorter
# than 0x11000 bytes read the untouched bytes (the checker zeroes the window).
DSTS = (0x80140000, 0x80150000, 0x80141001, 0x801A0000, 0xA0140000, 0x80121000, 0x80140001, 0x80140002, 0x80140003,
        0x80160040)


def stream(rng):
    """Random valid token stream; returns bytes (ending in 0xFF 0xFF) or None."""
    T = []
    pos = 0
    first = rng.choice((0, 7, 8, 0x10, 0xEF, rng.randrange(0xF0)))
    T += [first] + [rng.randrange(256) for _ in range(first + 1)]
    pos += first + 1
    budget = rng.choice((8, 30, 80, 200))
    for _ in range(budget):
        if len(T) > 700 or pos > 0x10000:
            break
        r = rng.random()
        if r < 0.22:
            form = rng.random()
            if form < 0.2:
                T += [0xF0]                      # no count byte: next token follows
            else:
                n = rng.choice((0, 1, 2, 5, 0x10, 0xEE, 0xEF, rng.randrange(0xF0)))
                T += [0xF0, n] + [rng.randrange(256) for _ in range(n + 1)]
                pos += n + 1
        elif r < 0.30:
            T += [0xF0, 0, rng.randrange(256)]       # explicit one-byte literal
            pos += 1
        else:
            op = rng.choice((0xF1, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF8, 0xFA, 0xFB, 0xFC, 0xFE, 0xFF))
            a = rng.choice((0, 1, 2, 3, 7, 0x10, 0x7F, 0x80, 0xF0, 0xFE, 0xFF, rng.randrange(256)))
            d = (op - 0xF1) * 256 + a + 1
            if d > pos:
                op, a = 0xF1, rng.randrange(max(1, min(256, pos)))
                d = a + 1
            if op == 0xFF and a == 0xFF:
                a = 0xFE
            if rng.random() < 0.12:
                T += [op, a, 0xF0]               # no length byte: a zero-length copy; 0xF0 (no count) is a no-op token
                continue
            e = rng.choice((0, 1, 5, 0x20, 0xEE, 0xEF, rng.randrange(0xF0)))
            seq = [e]
            while e == 0xEF and rng.random() < 0.5:
                e = rng.choice((0, 3, 0xEE, 0xEF))
                seq.append(e)
            T += [op, a] + seq
            pos += sum(x + 1 for x in seq)
            T += [0xF0]                           # stop the length chain
    T += [0xFF, 0xFF]
    if pos > 0x11000 - 16 or len(T) > TOKEN_LEN_MAX:
        return None
    return bytes(T)


def main():
    out, shard, nshards, target = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])
    blob = V.ovl_blob()
    base = C.fixture((blob, V.OVL_VRAM))
    off, size, entry = C.func_range('func_8010BD4C', V.OVL_YAML, V.OVL_VRAM)
    res0 = V.expand_one(base, entry, size, 0x80140000, [])
    d00 = 0x80140000 & 0x1FFFFF
    base_out = bytes(res0[1][d00 + i] for i in range(V.EXP_OUT))
    lines = []
    n = dropped = 0
    k = shard or 0
    while n < target and k < 4000:
        seed = 0x8010BD4C0000 + k
        k += nshards if shard is not None else 1
        rng = random.Random(seed)
        T = stream(rng)
        if T is None:
            continue
        dst = rng.choice(DSTS)
        patches = [(i, b) for i, b in enumerate(T)]
        res = V.expand_one(base, entry, size, dst, patches)
        if res is None:
            dropped += 1
            continue
        r, stores = res
        d0 = dst & 0x1FFFFF
        if len(stores) != V.EXP_OUT or min(stores) != d0:
            dropped += 1
            continue
        buf = bytes(stores[d0 + i] for i in range(V.EXP_OUT))
        ranges, i = [], 0
        while i < V.EXP_OUT:
            if buf[i] == base_out[i]:
                i += 1
                continue
            j = i
            while j + 1 < V.EXP_OUT and (buf[j + 1] != base_out[j + 1] or
                                         next((g for g in range(1, 16) if j + 1 + g >= V.EXP_OUT or buf[j + 1 + g] != base_out[j + 1 + g]), 16) < 16):
                j += 1
            ranges.append('%X:%X:%s' % (i, j - i + 1, V.rle_hex(buf[i:j + 1])))
            i = j + 1
        lines.append('8010BD4C %d X %X PATCH %s RET %X SHA %s DELTA %s' % (
            seed & 0x7FFFFFFF, dst, ' '.join('%X=%X' % p for p in patches), r, hashlib.sha256(buf).hexdigest(),
            ' '.join(ranges) or '-'))
        n += 1
        print('probe %d: dst %08X stream %d bytes' % (n, dst, len(T)), flush=True)
    open('%s.shard%d' % (out, shard or 0), 'w').write('\n'.join(lines) + '\n')
    print('shard %s: %d cases, %d dropped' % (shard, n, dropped))


if __name__ == '__main__':
    main()

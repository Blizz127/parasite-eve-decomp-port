#!/usr/bin/env python3
"""cdgen37: black-box vectors for pc_port/platform/pe_plat_mdec_reset.c (guest 0x8010BE3C,
the libpress MDEC reset wrapper in the movie module overlay ovl_039F).

The ORIGINAL code (overlay read from Disc 1, sha256 checked) runs as an opaque
black box under the test-only interpreter; nothing here reads, transliterates
or disassembles it.  Its two exits (the only jal targets in the wrapper's body
[0x8010BE3C, first jr ra + 8)) are stopped and recorded, with independently
drawn scripted return values:
    0x80073C94  (libetc ResetCallback, port: pe_plat_reset_callback)
    0x8010C0FC  (the MDEC reset core)
Per case: the mode argument (a0), seeded a1..a3, the two scripted returns; the
line records the original's returned v0, the calls made in order (target and
the four argument registers) and the logged loads/stores (always empty: the
wrapper itself touches no guest memory; asserted on every vector).

Prerequisites: build/disc1.candidate.exe (symlink in a worktree) and
PE_DISC1_BIN=<Disc 1 .bin>.
    python3 pe_mdec_reset_vectors.py OUT.txt
Line: BE3C <seed> A <a0> <a1> <a2> <a3> R <ret73c94> <retc0fc> OUT <v0> N <n> [C <target>:<a0>:<a1>:<a2>:<a3>]...
"""
import hashlib, struct, sys
import pe_cd_leaf_vectors as cd
from pe_movie_init_oracle import load
from pe_m0000i_leaves_oracle import find_disc, read_form1, ROOT
import pe_gpu_leaf_vectors as gv

OVERLAY_SHA256 = 'd0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5ba8d0b40'
ENTRY, BODY_END = 0x8010BE3C, None
CALLEES = {0x80073C94: 0, 0x8010C0FC: 0}
MODE_EDGES = (0, 0, 0, 1, 2, 3, 4, 5, 0xFF, 0x100, 0x101, 0x200, 0x8000, 0xFFFF, 0x10000, 0x10001,
              0x1000000, 0x01000001, 0x7FFFFFFF, 0x80000000, 0x80000001, 0xFFFF0000, 0xFFFFFF00,
              0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFD, 0xFF000000)
RET_EDGES = (0, 1, 2, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, 0x18, 0x19, 0x60000000)


def base_ram():
    ex, _ = load(); path = find_disc(ROOT); ov = read_form1(path, 1940, 38)
    assert hashlib.sha256(ov).hexdigest() == OVERLAY_SHA256
    ram = bytearray(0x200000); ram[0x10000:0x10000 + len(ex) - 2048] = ex[2048:]
    ram[0x10BCF8:0x10BCF8 + len(ov)] = ov
    return bytes(ram)


def exits(base):
    """Static jal/j-out targets of the wrapper body [ENTRY, first jr ra + 8).
    -> (end, sorted targets); the generator requires these to equal CALLEES."""
    end = None
    for a in range(ENTRY, ENTRY + 0x400, 4):
        if struct.unpack_from('<I', base, a & 0x1FFFFF)[0] == 0x03E00008:
            end = a + 8; break
    assert end
    t = set()
    for a in range(ENTRY, end, 4):
        w = struct.unpack_from('<I', base, a & 0x1FFFFF)[0]
        tg = (a & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
        if w >> 26 == 3 or (w >> 26 == 2 and not ENTRY <= tg < end):
            t.add(tg)
    return end, sorted(t)


def main():
    out_path = sys.argv[1]
    base = base_ram()
    end, tg = exits(base)
    assert set(tg) == set(CALLEES), [hex(x) for x in tg]
    print('body [%08X,%08X) exits %s' % (ENTRY, end, ' '.join('%08X' % x for x in tg)))
    out = []
    n0 = 0
    for seed in range(1, 6001):
        x = (seed * 7919 ^ ENTRY) & 0xFFFFFFFF or 1
        x = gv.xs32(x)
        if x % 4 == 0:
            mode = 0
        elif x % 4 == 1:
            mode = MODE_EDGES[(x >> 8) % len(MODE_EDGES)]
        elif x % 4 == 2:
            mode = (x >> 8) & 0xF
        else:
            x = gv.xs32(x); mode = x
        args = [mode]
        for _ in range(3):
            x = gv.xs32(x); args.append(x)
        rets = {}
        for t in (0x80073C94, 0x8010C0FC):
            x = gv.xs32(x)
            rets[t] = RET_EDGES[(x >> 8) % len(RET_EDGES)] if x & 1 else gv.xs32(x)
        ram = bytearray(base)
        ret, loads, stores, calls = cd.run_with_callees(ram, ENTRY, args, rets, callees=CALLEES)
        assert not loads and not stores, (hex(mode), loads, stores)
        n0 += mode == 0
        out.append('BE3C %d A %X %X %X %X R %X %X OUT %X N %d%s' % (
            seed, args[0], args[1], args[2], args[3], rets[0x80073C94], rets[0x8010C0FC], ret, len(calls),
            ''.join(' C %X:%X:%X:%X:%X' % tuple(c[0]) for c in calls)))
    open(out_path, 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'mode0', n0, 'sha256', hashlib.sha256(open(out_path, 'rb').read()).hexdigest())


if __name__ == '__main__':
    main()

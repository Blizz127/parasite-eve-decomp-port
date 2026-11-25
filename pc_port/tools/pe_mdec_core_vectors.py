#!/usr/bin/env python3
"""cdgen40: ordered black-box traces for pc_port/platform/pe_plat_mdec_core.c (guest 0x8010C0FC, the libpress
MDEC reset core in the movie overlay ovl_039F, with the two overlay helpers it runs: 0x8010C1EC (start a table
upload) and 0x8010C308 (wait while the MDEC reports busy)).

The ORIGINAL code (overlay read from Disc 1, sha256 checked) runs as an opaque black box under the test-only
interpreter; nothing here reads, transliterates or disassembles it.  Only the debug-printf stub 0x80071A74 is
stopped (recorded, scripted return); the two helpers run as original code.  Registers are plain memory in the
interpreter, so the harness models the reads the original performs:
  * MDEC status 0x1F801824 reads return a script (k1 busy words with bit 29 set, an idle word, k2 busy words,
    an idle word that repeats) independent of what was written there (a write to that address is the MDEC
    control register, not the status);
  * DMA1 CHCR 0x1F801098 reads return a seeded word (also independent of the preceding store);
  * DPCR 0x1F8010F0 and the two overlay command words 0x8010DA0C / 0x8010DA90 are seeded RAM; the pointer words
    the original loads (0x8010DB1C..0x8010DB54) keep their overlay values.
Per case the line records the ordered accesses: loads (address,size,value), stores (address,size,value) and
the printf call, in program order.  Never recorded: how the original computes anything.

Prerequisites: build/disc1.candidate.exe (symlink in a worktree) and PE_DISC1_BIN=<Disc 1 .bin>.
    python3 pe_mdec_core_vectors.py OUT.txt
Line: C0FC <seed> A <mode> EV tok...   or   C1EC <seed> A <src> <words> EV tok...   tok = L:addr:size:value | S:addr:size:value | C:target:a0:a1
"""
import hashlib, struct, sys
import pe_cd_leaf_vectors as cd
import pe_gpu_leaf_vectors as gv
from pe_mdec_reset_vectors import base_ram

pl, v = cd.pl, cd.v
ENTRY = 0x8010C0FC
PRINTF = 0x80071A74
EV = []
SCRIPT = {}
_l, _s = pl._ns['LOAD'], pl._ns['STORE']


def _addr(a):
    return (0x1F800000 if a < 0x2000 else 0x80000000) + a


def LOAD(fmt, buf, a):
    if (a & ~3) in SCRIPT:
        vals, idx = SCRIPT[a & ~3]
        w = vals[min(idx[0], len(vals) - 1)]; idx[0] += 1
        struct.pack_into('<I', buf, a & ~3, w & 0xFFFFFFFF)
    r = _l(fmt, buf, a)
    if pl._logged(a):
        EV.append(('L', _addr(a), struct.calcsize(fmt), r[0] & 0xFFFFFFFF))
    return r


def STORE(fmt, buf, a, val):
    if pl._logged(a):
        n = struct.calcsize(fmt)
        EV.append(('S', _addr(a), n, val & ((1 << (8 * n)) - 1)))
    return _s(fmt, buf, a, val)


pl._ns['LOAD'], pl._ns['STORE'] = LOAD, STORE

MODES = (0, 1, 2, 3, 4, 5, 0x100, 0x101, 0x10000, 0x10001, 0x1000000, 0x80000000, 0x80000001, 0xFFFF0000,
         0xFFFFFF00, 0xFFFFFFFE, 0xFFFFFFFF, 0xFF, 0x7FFFFFFF)
DPCR_E = (0, 0x08, 0x88, 0x10, 0x333333BB, 0xFFFFFFFF, 0x12345678, 0x80, 0x77777777)
CMD_E = (0x40000001, 0x60000000, 0, 0xFFFFFFFF, 0x80000000, 0x00000001, 0x20000000, 0x3A00FFFF, 0x12345678)
IDLE_E = (0, 0x80040000, 0x40000000, 0x10000000, 0x08000000, 0xDFFFFFFF, 0x1FFFFFFF, 0xC0000000, 0x00010000, 0xFFFF)
BUSY_E = (0x20000000, 0x20000001, 0xE0040000, 0xFFFFFFFF, 0x2000FFFF, 0x30000000, 0x28000000)
CH_E = (0, 1, 0x01000000, 0x01000200, 0xFFFFFFFF, 0x10000000, 0x12345678)


def pick(x, edges):
    return edges[(x >> 8) % len(edges)] if x & 1 else (x >> 3) & 0xFFFFFFFF


def run(base, mode, seeds, scripts, pr, entry=ENTRY, a1=0x11):
    ram = bytearray(base)
    for a, val in seeds:
        struct.pack_into('<I', ram, a & 0x1FFFFF, val & 0xFFFFFFFF)
    EV.clear(); SCRIPT.clear()
    for a, vals in scripts.items():
        SCRIPT[a] = [list(vals), [0]]
    pl.LOADS.clear(); pl.STORES.clear(); pl.CUR[0] = entry
    regs = {4: mode, 5: a1, 6: 0x22, 7: 0x33, 31: v.RET}
    st = cd.Stop((v.RET, PRINTF))
    r = v.execute(ram, entry, (), stop_at=st, initial_regs=regs, instruction_budget=400000)
    if st.hit == PRINTF:
        EV.append(('C', PRINTF, r[4] & 0xFFFFFFFF, r[5] & 0xFFFFFFFF))
        regs = dict(enumerate(r)); regs[2] = pr
        st = cd.Stop((v.RET, PRINTF))
        r = v.execute(ram, r[31], (), stop_at=st, initial_regs=regs, instruction_budget=400000)
        assert st.hit == v.RET
    return list(EV)


def tok(e):
    return ('%s:%X:%d:%X' % e) if e[0] in 'LS' else 'C:%X:%X:%X' % (e[1], e[2], e[3])


def main():
    base = base_ram()
    out, n0, n1, nx = [], 0, 0, 0
    for seed in range(1, 6001):
        x = (seed * 104729 ^ ENTRY) & 0xFFFFFFFF or 1
        x = gv.xs32(x)
        k = x % 10
        mode = 0 if k < 3 else 1 if k < 6 else (MODES[(x >> 8) % len(MODES)] if x & 0x1000 else (x >> 4) & 0xF)
        x = gv.xs32(x); dpcr = pick(x, DPCR_E)
        x = gv.xs32(x); c1 = pick(x, CMD_E)
        x = gv.xs32(x); c2 = pick(x, CMD_E)
        x = gv.xs32(x); ch1 = pick(x, CH_E)
        x = gv.xs32(x); pr = pick(x, (0, 1, 0x18, 0xFFFFFFFF))
        vals = []
        for _ in range(2):
            x = gv.xs32(x); kk = (0, 1, 2, 3, 4, 7, 40)[x % 7]
            for _ in range(kk):
                x = gv.xs32(x); vals.append(BUSY_E[(x >> 8) % len(BUSY_E)] | 0x20000000 if x & 2 else 0x20000000 | ((x >> 4) & 0xDFFFFFFF))
            x = gv.xs32(x); vals.append(pick(x, IDLE_E) & ~0x20000000)
        seeds = ((0x10F0, dpcr), (0x10DA0C & 0x1FFFFF, c1), (0x10DA90 & 0x1FFFFF, c2))
        ev = run(base, mode, seeds, {0x1824: vals, 0x1098: [ch1]}, pr)
        n0 += mode == 0; n1 += mode == 1; nx += mode > 1
        out.append('C0FC %d A %X EV %s' % (seed, mode, ' '.join(tok(e) for e in ev)))
    nu = 0
    for seed in range(1, 3001):
        x = (seed * 15485863 ^ 0x8010C1EC) & 0xFFFFFFFF or 1
        x = gv.xs32(x)
        src = (0x8010DA0C, 0x8010DA90)[x % 2] if x & 4 else 0x80100000 + ((x >> 8) % 0x3000) * 4
        x = gv.xs32(x)
        words = (0, 1, 0x1F, 0x20, 0x21, 0x3F, 0x40, 0x41, 0x7FE0, 0x7FFF, 0xFFFF, 0x10000, 0x1FFFE0, 0xFFFFFFE0, 0xFFFFFFFF, 0x80000000)[(x >> 8) % 16] if x & 1 else (x >> 3) & 0xFFFFFFFF
        x = gv.xs32(x); dpcr = pick(x, DPCR_E)
        x = gv.xs32(x); c1 = pick(x, CMD_E)
        vals = []
        x = gv.xs32(x); kk = (0, 1, 2, 3, 7)[x % 5]
        for _ in range(kk):
            x = gv.xs32(x); vals.append(0x20000000 | ((x >> 4) & 0xDFFFFFFF))
        x = gv.xs32(x); vals.append(pick(x, IDLE_E) & ~0x20000000)
        ev = run(base, src, ((0x10F0, dpcr), (src & 0x1FFFFF, c1)), {0x1824: vals}, 0, entry=0x8010C1EC, a1=words)
        nu += 1
        out.append('C1EC %d A %X %X EV %s' % (seed, src, words, ' '.join(tok(e) for e in ev)))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('c1ec cases', nu)
    print('cases', len(out), 'mode0', n0, 'mode1', n1, 'other', nx, 'sha256', hashlib.sha256(open(sys.argv[1], 'rb').read()).hexdigest())


if __name__ == '__main__':
    main()

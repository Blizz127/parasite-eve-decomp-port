#!/usr/bin/env python3
"""cdgen49b: black-box vectors for retail DsRead 0x80080E34 (libcd CdRom_StartRead, the function the hand pe_libcd.c collapses into
func_8006E6D4 and the generated func_8006E6D4 TU reaches as a loud boundary).  ORIGINAL code from the retail EXE runs under the
shared test-only interpreter; nothing here reads a provider.

Entry: 0x80080E34(pos, sectors, dest, mode) -> int.  (pos: guest pointer to the 4 CdlLOC bytes or 0 = use the last position;
sectors: stored at 0x8009B6B4; dest: stored at 0x8009B6B0; mode: low byte | 0x20 is passed to the command sequence.)
Stopped callees (every one is a bound pe_plat host symbol in the public headers; arity as in the retail call):
  800822AC  pe_plat_cd_error_active()            0 args    abort / error check
  80080CDC  pe_plat_ds_last_pos(0)               1 arg     returns a pointer to the last position (4 bytes, read here with lwl/lwr)
  8007F0C8  pe_plat_cd_command_sequence_host     5 args    (mode|0x20, &pos copy, 6, 0x80080F64, -1 at sp+0x10); the 4 bytes the
                                                            pointer argument points to are an extra token field (what the sequence
                                                            would read): the position copy the function builds is observable that way
  80073A44  pe_plat_hostfb_vsync(-1)             1 arg     VSync
  800824F0  pe_plat_ds_data_callback(cb)         1 arg     DMACallback(3?)-style exchange: returns the previous callback word
State read: 0x8009B6CC (1 = busy -> returns 0), 0x8009B6B8 bit 0 (install the DMA callback 0x80081110), the 4 position bytes.
State written: 0x8009B6AC = 0x200, 0x8009B6B0 = dest, 0x8009B6B4 = sectors, 0x8009B6C4 = VSync result, 0x8009B6C0 = DMA callback exchange
result (only when 0x8009B6B8 bit 0), 0x8009B6CC = 1.
Seeds: RUNTIME-OBSERVED tuples (every func_8006E6D4 call in the pe_cdx_cdgen49_trace.c logs is one DsRead call: pos = CdIntToPos(lba + off),
sectors, dest, mode 0x80; the read returned nonzero (ret=1) so the command sequence returned 1) replayed verbatim and perturbed; a product of
the gate dimensions; random fill.
Usage: pe_cdx_cdgen49b_vectors.py OUT_DIR [-n N] [--trace F ...]
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_cdx_cdgen49_vectors as g   # noqa: E402  (helpers, trace reader, own case loop)
import pe_cdx_vectors as m   # noqa: E402

c, gv, v = m.c, m.gv, m.v
M32, ZONE = g.M32, g.ZONE
DS = 0x80080E34
ABORT, LASTPOS, SEQ, VSYNC, DMACB = 0x800822AC, 0x80080CDC, 0x8007F0C8, 0x80073A44, 0x800824F0
STATE, PEND = 0x8009B6CC, 0x8009B6B8
RT = []


def bcd(n):
    return ((n // 10) << 4) | (n % 10)


def cdlloc(lba):
    """CdIntToPos(lba) = BCD minute / second / frame of lba + 150 (psx-spx), 4th byte 0"""
    n = (lba + 150) & M32
    return bytes([bcd(n // 75 // 60 % 100), bcd(n // 75 % 60), bcd(n % 75), 0])


def put_bytes(ram, a, bs):
    ram[a & 0x1FFFFF:(a & 0x1FFFFF) + len(bs)] = bs


POSP = [ZONE + 0x100 * i for i in range(16)]
SECV = (0, 1, 2, 3, 5, 0xF, 0x16, 0x22, 0x55, 0x80, 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, 0x10000)
DESTV = (0x800A8028, 0x801ED800, 0x801229A0, 0x80100000, 0x80100004, 0x801FFFFC, 0, 0xFFFFFFFF, 0x80000000, 0xA0100000, 0x00100000)
MODEV = (0x80, 0x80, 0x80, 0, 1, 0x20, 0xA0, 0xFF, 0x100, 0x1FF, 0x7FFFFF80, 0xFFFFFF00, 0x81)
SEQV = (1, 1, 1, 0, 2, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, 0x100)
VSV = (0, 1, 0x1234, 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000)
CBV = (0, 0x80081110, 0x80123450, 0xFFFFFFFF, 0x8007F7E8)
NCELL = 3 * 3 * 3 * 2 * 2 * 3      # state(0 / 1 / other) x abort(0 / 1 / -1) x pos(ptr / 0 / ptr-edge) x pend bit0 x seq ok/fail x callbacks


def ds_hook(ram, x, args, rets, seed):
    k = seed - 1
    h = g.hsh(seed, 0xB1)
    if m.PILOT[0]:
        g.CLASSES.append('pilot')
        return x, None
    nrt = len(RT)
    # fixed pointers for the 'last position' reply (read with lwl/lwr, 4 bytes)
    lp = ZONE + 0x1000 + 0x40 * ((h >> 3) % 64)
    rets[LASTPOS] = lp
    put_bytes(ram, lp, bytes([(h >> 8) & 0xFF, (h >> 16) & 0xFF, (h >> 24) & 0xFF, h & 0xFF]))
    if k < nrt * 3:
        t = RT[k // 3]
        var = k % 3
        lba = (t['args'][0] + t['args'][1]) & M32
        pos = POSP[(k // 3) % len(POSP)]
        put_bytes(ram, pos, cdlloc(lba))
        args[:] = [pos, t['args'][3], t['args'][2], 0x80]
        g.put32(ram, STATE, 0)
        g.put32(ram, PEND, 0)
        rets[ABORT] = 0
        rets[SEQ] = t['ret'] & M32 or 1
        rets[VSYNC] = (h >> 5) & M32
        rets[DMACB] = 0
        if var == 0:
            g.CLASSES.append('runtime-args')
        else:
            g.CLASSES.append('runtime-args-perturbed')
            for e in (((h >> 4) % 7,) if var == 1 else ((h >> 4) % 7, (h >> 9) % 7)):
                if e == 0:
                    g.put32(ram, STATE, (1, 2, 0xFFFFFFFF)[(h >> 12) % 3])
                elif e == 1:
                    rets[ABORT] = (1, 2, 0xFFFFFFFF)[(h >> 14) % 3]
                elif e == 2:
                    args[0] = 0
                elif e == 3:
                    g.put32(ram, PEND, (1, 3, h)[(h >> 16) % 3])
                    rets[DMACB] = CBV[(h >> 18) % len(CBV)]
                elif e == 4:
                    rets[SEQ] = (0, 2, 0xFFFFFFFF)[(h >> 20) % 3]
                elif e == 5:
                    args[3] = MODEV[(h >> 22) % len(MODEV)]
                else:
                    args[1] = SECV[(h >> 25) % len(SECV)]
        return x, None
    p = k - nrt * 3
    if p < NCELL * 4:
        cell, rep = divmod(p, 4)
        hi = g.hsh(cell, rep, 0x44)
        st, ab, ps, pe, sq, cb = cell % 3, (cell // 3) % 3, (cell // 9) % 3, (cell // 27) % 2, (cell // 54) % 2, (cell // 108) % 3
        g.put32(ram, STATE, (0, 1, (2, 0xFFFFFFFF, 0x100, hi)[(hi >> 3) % 4])[st])
        rets[ABORT] = (0, 1, (2, 0xFFFFFFFF)[(hi >> 5) & 1])[ab]
        pos = POSP[(hi >> 7) % len(POSP)]
        put_bytes(ram, pos, bytes([(hi >> 8) & 0xFF, (hi >> 16) & 0xFF, (hi >> 24) & 0xFF, hi & 0xFF]))
        args[0] = (pos, 0, (ZONE + 0x3FFC, 0x801FFFFC, 0x801FE000)[(hi >> 9) % 3])[ps]
        g.put32(ram, PEND, (hi | 1) if pe else (hi & ~1 & M32))
        rets[SEQ] = 0 if sq else SEQV[(hi >> 11) % len(SEQV)]
        rets[VSYNC] = VSV[(hi >> 15) % len(VSV)]
        rets[DMACB] = (0, CBV[(hi >> 17) % len(CBV)], 0x80081110)[cb]
        args[1] = SECV[(hi >> 19) % len(SECV)]
        args[2] = DESTV[(hi >> 22) % len(DESTV)]
        args[3] = MODEV[(hi >> 26) % len(MODEV)]
        g.CLASSES.append('product')
        return x, None
    g.CLASSES.append('random')
    g.put32(ram, STATE, (0, 0, 0, 1, 2, h)[(h >> 3) % 6])
    g.put32(ram, PEND, (0, 1, 3, h)[(h >> 6) % 4])
    rets[ABORT] = (0, 0, 1, 0xFFFFFFFF, h)[(h >> 8) % 5] & M32
    pos = POSP[(h >> 10) % len(POSP)]
    put_bytes(ram, pos, struct.pack('<I', gv.xs32(h)))
    args[0] = pos if (h >> 14) % 4 else 0
    args[1] = SECV[(h >> 15) % len(SECV)]
    args[2] = DESTV[(h >> 19) % len(DESTV)]
    args[3] = MODEV[(h >> 23) % len(MODEV)]
    rets[SEQ] = SEQV[(h >> 27) % len(SEQV)]
    rets[VSYNC] = VSV[(h >> 3) % len(VSV)]
    rets[DMACB] = CBV[(h >> 9) % len(CBV)]
    return x, None


def main():
    argv = sys.argv[1:]
    n = 480
    traces = []
    while '-n' in argv:
        i = argv.index('-n'); n = int(argv[i + 1]); del argv[i:i + 2]
    while '--trace' in argv:
        i = argv.index('--trace'); traces.append(argv[i + 1]); del argv[i:i + 2]
    outdir = argv[0]
    os.makedirs(outdir, exist_ok=True)
    global RT
    RT = g.read_traces(traces)['issue']
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    c.CALL_CAP[0] = 400
    g.RT = {'issue': RT, 'timeout': []}
    m.spec(DS, 'ds_read', 'pnpw', {ABORT: 0, LASTPOS: 1, SEQ: 5, VSYNC: 1, DMACB: 1}, batch='cdgen49b', deref={SEQ: (1, 4)})
    c._check_callees_complete(base, DS, tuple(m.SPEC[DS]['callees']))
    m.HOOKS[DS] = ds_hook
    o, cl, st = g.gen(base, DS, max(n, len(RT) * 3 + NCELL * 4 + 120))
    with open(os.path.join(outdir, 'dsread.txt'), 'w') as f:
        f.write('\n'.join(o) + '\n')
    with open(os.path.join(outdir, 'dsread.classes'), 'w') as f:
        f.write('\n'.join('%d %s' % (i + 1, k) for i, k in enumerate(cl)) + '\n')
    tally = {}
    for k in cl:
        tally[k] = tally.get(k, 0) + 1
    rep = 'dsread entry=%08X cases=%d %s classes=%s runtime-tuples=%d' % (DS, len(o), ' '.join('%s=%d' % kv for kv in st.items()), tally, len(RT))
    open(os.path.join(outdir, 'gen.log'), 'w').write(rep + '\n')
    print(rep)


if __name__ == '__main__':
    main()

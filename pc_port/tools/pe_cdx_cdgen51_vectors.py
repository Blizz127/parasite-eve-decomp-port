#!/usr/bin/env python3
"""cdgen51: black-box vectors for libspu SpuClearReverbWorkArea, retail 0x8008D610 (one argument: the reverb mode; returns an int).
The ORIGINAL code runs under the test-only interpreter (pe_cdx_vectors.run_case); nothing here reads a provider.

What the entry does (observed over the sets, not quoted): for a mode >= 10 (unsigned) or a mode whose work-area start (word table at 0x8009B46C, one
word per mode, SPU address units) the heap-contains entry 0x80085BB4 reports as allocated (any nonzero result) it returns -1 at once.  Otherwise it
zero-fills the mode's reverb work area through the SPU transfer interface, in chunks of at most 0x400 bytes: mode 0 clears 0x10 << shift bytes at
0xFFF0 << shift, mode n >= 1 clears (0x10000 - table[n]) << shift bytes from table[n] << shift (shift = word 0x8009B424 & 31).  Per chunk it
  stores the transfer address (halfword 0x8009B414 and the SPU register 0x1A6 through the register base pointer 0x8009B3FC), clears 0x8009B44C,
  rewrites SPU register 0x1AA (read, & 0xFFCF, | 0x20), writes SPU_DELAY (through the pointer word 0x8009B410), the transfer bookkeeping words
  0x8009B450 / 0x8009B454, and the DMA channel 4 registers (through the pointer words 0x8009B400 / 04 / 08: MADR = the zero buffer 0x8009C4C0,
  BCR, CHCR), then waits for the completion event (the BIOS event veneer 0x8008D7B0 with the handle word 0x8009B384, a stopped callee).
It saves the busy word 0x8009B418 (cleared when it is 1) and the callback word 0x8009B434 (cleared when nonzero) around the loop and restores them.
The transfer helper 0x8007D778 and its two leaves (0x8007DC84, 0x8007DC5C) run INLINE as original code: the register-level footprint (SPU registers,
SPU_DELAY, DMA4 registers, bookkeeping words) is part of the observable behaviour, like the cdgen44 / cdgen46 SPU family.  Stopped callees: the
heap-contains entry 0x80085BB4 (bound provider pe_plat_spub_heap_contains; arity 1, scripted result) and the event wait 0x8008D7B0 (arity 1 = the
handle; scripted per-call results).  The DMA completion itself is therefore the scripted event wait.
Cases: every mode 0..9 with the real work-area table and one-parameter variations (shift word, busy word, callback word, register base: the real
0x1F801C00 and a RAM mirror 0x80151000, event handle, event results, SPU register 0x1AA and SPU_DELAY initial values), table-entry edges (0x10000,
0xFFFF, 0xFFF0, ... : zero-, one- and two-chunk work areas), invalid modes (10, 11, 0x100, 0x80000000, 0xFFFFFFF6, 0xFFFFFFFF), heap results, and a random remainder.
Cases whose work area is so large that the original does not finish inside the instruction / call budget are dropped (counted).  Own-written-load
rule default (loads of bytes the original wrote itself are not logged; a case mixing own-written and external bytes is dropped, counted).

Usage: pe_cdx_cdgen51_vectors.py OUTDIR [--random N] [--seed S]      writes OUTDIR/clear.txt (+ .classes, + stats line)"""
import os
import random
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_cdx_cdhost_vectors as g   # noqa: E402  (shared interpreter hooks, seq_model; also registers the cdgen47 specs)

m, c, gv, pl, v = g.m, g.c, g.gv, g.pl, g.v
ENTRY = 0x8008D610
HEAP, WAIT = 0x80085BB4, 0x8008D7B0
INLINE = (0x8007D778, 0x8007DC84, 0x8007DC5C)
M32 = 0xFFFFFFFF
TABLE = 0x8009B46C
REAL_TABLE_WORDS = 10      # the ten-word retail table at TABLE is read from the user's EXE image (the fixture base) at run time, never embedded
RAMBASE = 0x80151000
STATS = {}


def setup():
    m.spec(ENTRY, 'spu_clear_reverb_work_area', 'w', {HEAP: 1, WAIT: 1}, batch='cdgen51', n=1)
    sp = m.SPEC[ENTRY]
    sp['budget'] = 400000
    sp['inline'] = INLINE
    c.CALL_CAP[0] = 1200
    return sp


def run(base, sp, sysc, k, s):
    ram = bytearray(base)

    def sw(a, val):
        struct.pack_into('<I', ram, a & 0x1FFFFF, val & M32)

    def sh(a, val):
        struct.pack_into('<H', ram, a & 0x1FFFFF, val & 0xFFFF)
    mode = k['mode']
    for i, val in enumerate(struct.unpack_from('<%dI' % REAL_TABLE_WORDS, base, TABLE & 0x1FFFFF)):
        sw(TABLE + 4 * i, val)
    if 0 <= mode < 10 and k['entry'] is not None:
        sw(TABLE + 4 * mode, k['entry'])
    sw(0x8009B424, k['shift'])
    sw(0x8009B418, k['busy'])
    sw(0x8009B434, k['callback'])
    sw(0x8009B3FC, k['regbase'])
    sw(0x8009B384, k['handle'])
    sw(0x8009B410, 0x1F801014)
    for a, val in ((0x8009B400, 0x1F8010C0), (0x8009B404, 0x1F8010C4), (0x8009B408, 0x1F8010C8)):
        sw(a, val)
    # SPU register image behind the register base: initial 0x1AA / 0x1A6 words (a plain RAM mirror or the register page the interpreter treats as memory)
    reg = (k['regbase'] & 0x1FFFFFFF)
    sh(reg + 0x1AA, k['spucnt'])
    sh(reg + 0x1A6, k['spuaddr'])
    sw(0x1F801014, k['delay'])
    rets, models = {}, {}
    rets[HEAP] = [k['heap']]
    models[HEAP] = g.seq_model([k['heap']])
    rets[WAIT] = list(k['waits'])
    models[WAIT] = g.seq_model(k['waits'])
    g.reset_models()
    c.UNFAITHFUL[0] = False
    try:
        res = m.run_case(ram, ENTRY, [mode & M32], rets, sp, sysc, models)
    except (struct.error, IndexError, g.MipsBreak):
        STATS['drop:exception'] = STATS.get('drop:exception', 0) + 1
        return None
    except AssertionError as e:
        key = 'drop:budget' if 'instruction budget' in str(e) else 'drop:unsupported'
        STATS[key] = STATS.get(key, 0) + 1
        return None
    finally:
        g.reset_models()
    if c.UNFAITHFUL[0]:
        STATS['drop:own-written-mixed-or-unfaithful-load'] = STATS.get('drop:own-written-mixed-or-unfaithful-load', 0) + 1
        return None
    ret, loads, stores, calls = res
    text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % mm_ for rec, mm_ in calls) or '-'
    order = tuple(dict.fromkeys(m.canon(q) for q in sp['callees']))
    ln = gv.line(ENTRY, s, 0, [mode & M32], (ret, loads, stores)) + ' RETS %s CALL %s' % (
        ','.join('%08X=%s' % (q, m.fmt_ret(rets[q])) for q in order if q in rets) or '-', text)
    if len(ln.split()) > 15000:
        STATS['drop:too-long-for-checker-token-capacity'] = STATS.get('drop:too-long-for-checker-token-capacity', 0) + 1
        return None
    return ln


def canon_case(mode):
    return dict(mode=mode, entry=None, shift=3, busy=0, callback=0, regbase=0x1F801C00, handle=0x100, waits=[1], heap=0, spucnt=0, spuaddr=0, delay=0x200931E1)


def cases(nrandom, seed):
    out = []
    rng = random.Random(seed)
    modes = list(range(10))
    for mode in modes:
        k0 = canon_case(mode)
        out.append((k0, 'canon:mode%d' % mode))
        for shift in (0, 1, 2, 4, 5, 0x23, 31, M32):
            k = dict(k0, shift=shift)
            out.append((k, 'shift:%d:mode%d' % (shift & M32, mode)))
        for busy in (1, 2, M32):
            out.append((dict(k0, busy=busy), 'busy:%X:mode%d' % (busy, mode)))
        for cb in (0x80085098, 0x12345678):
            out.append((dict(k0, callback=cb), 'callback:mode%d' % mode))
        out.append((dict(k0, busy=1, callback=0x80085098), 'busy1+callback:mode%d' % mode))
        out.append((dict(k0, regbase=RAMBASE), 'ramregs:mode%d' % mode))
        for handle in (0, 0xF1000009, M32):
            out.append((dict(k0, handle=handle), 'handle:%X:mode%d' % (handle, mode)))
        for waits in ([0], [M32], [2], [1, 0], [0, 1], [1, 1, 0]):
            out.append((dict(k0, waits=waits), 'waits:%s:mode%d' % ('/'.join('%X' % w for w in waits), mode)))
        for heap in (1, 2, M32, 0x80000000):
            out.append((dict(k0, heap=heap), 'heap:%X:mode%d' % (heap, mode)))
        for spucnt in (0x20, 0xFFFF, 0x8000, 0x30, 0xFFCF):
            out.append((dict(k0, spucnt=spucnt), 'spucnt:%X:mode%d' % (spucnt, mode)))
        for delay in (0, M32):
            out.append((dict(k0, delay=delay), 'delay:%X:mode%d' % (delay, mode)))
        if mode >= 1:
            for e in (0x10000, 0xFFFF, 0xFFF0, 0xFFC0, 0xFF00, 0x8000, 0xF000, 0, 0x10001, 0xFFFFFFFF):
                out.append((dict(k0, entry=e), 'entry:%X:mode%d' % (e & M32, mode)))
                out.append((dict(k0, entry=e, shift=0), 'entry+shift0:%X:mode%d' % (e & M32, mode)))
    for mode in (10, 11, 0x100, 0x80000000, 0xFFFFFFF6, M32):
        for heap in (0, 1):
            out.append((dict(canon_case(mode), heap=heap), 'invalid:mode%X' % mode))
    for _ in range(nrandom):
        mode = rng.choice(modes + modes + [10, 11, M32])
        k = canon_case(mode)
        k.update(shift=rng.choice((3, 3, 3, 2, 1, 0, 4, rng.getrandbits(32))), busy=rng.choice((0, 0, 1, 1, 2, rng.getrandbits(32))),
                 callback=rng.choice((0, 0, 0x80085098, rng.getrandbits(32))), regbase=rng.choice((0x1F801C00, 0x1F801C00, RAMBASE)),
                 handle=rng.choice((0x100, 0, 0xF1000009, rng.getrandbits(32))), waits=rng.choice(([1], [1], [0], [M32], [1, 0], [0, 1, 1], [rng.getrandbits(32)])),
                 heap=rng.choice((0, 0, 0, 1, M32, rng.getrandbits(32))), spucnt=rng.choice((0, 0x20, 0xFFFF, rng.getrandbits(16))),
                 spuaddr=rng.choice((0, 0xFFFF, rng.getrandbits(16))), delay=rng.choice((0, 0x200931E1, M32, rng.getrandbits(32))))
        if 1 <= mode < 10 and rng.random() < 0.5:
            k['entry'] = rng.choice((0x10000, 0xFFFF, 0xFFF0, 0xFFC0, 0xFF00, rng.getrandbits(16), 0xF800))
        out.append((k, 'random'))
    return out


def main():
    argv = sys.argv[1:]
    nrandom, seed = 600, 1
    for flag in ('--random', '--seed'):
        if flag in argv:
            i = argv.index(flag)
            val = int(argv[i + 1])
            del argv[i:i + 2]
            if flag == '--random':
                nrandom = val
            else:
                seed = val
    outdir = argv[0]
    os.makedirs(outdir, exist_ok=True)
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    sp = setup()
    sysc = m.syscall_pcs(base, ENTRY)
    c._check_callees_complete(base, ENTRY, tuple(sp['callees']), inline=INLINE)
    allc = cases(nrandom, seed)
    lines, classes = [], []
    for i, (k, cls) in enumerate(allc):
        r = run(base, sp, sysc, k, 1000 + i)
        if r is None:
            continue
        lines.append(r)
        classes.append(cls)
    open(os.path.join(outdir, 'clear.txt'), 'w').write('\n'.join(lines) + '\n')
    open(os.path.join(outdir, 'clear.txt.classes'), 'w').write(''.join('%d %s\n' % (i + 1, t) for i, t in enumerate(classes)))
    print('clear.txt cases=%d tried=%d own_written_loads_excluded=%d mixed=%d %s' % (len(lines), len(allc), sum(m.OWNLOAD.values()), sum(m.MIXEDLOAD.values()),
          ' '.join('%s=%d' % kv for kv in sorted(STATS.items()))))


if __name__ == '__main__':
    main()

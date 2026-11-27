#!/usr/bin/env python3
"""cdgen50: black-box vectors for the retail CD read poll 0x800811E4 (the synchronous loader-side poll the DsRead path spins in).
The ORIGINAL code runs under the test-only interpreter (pe_cdx_vectors.run_case); nothing here reads a provider.

What the entry does (observed, not quoted): it asks the retrace clock (VSync with argument -1, stopped callee 0x80073A44, a returned value) and
compares it, signed, with the stored retrace stamp 0x8009B6C4 plus 1200; once the clock is past that deadline it runs the abort entry
0x80081268 (clears the read-active word 0x8009B6CC, and when that word was 1 it calls into the command chain), and the result is -1;
otherwise the result is the stored status word 0x8009B6B4.  The ready entry 0x8007F608 (stopped callee) is called with the argument in every case.

Two sets are written (flavour):
  poll.txt       the abort entry 0x80081268 runs INLINE as original code: its load of 0x8009B6CC and the store that clears it are part of the
                 observable footprint ("the clear of 0x8009B6CC is visible"); its own callees (0x8007F69C 0x800822BC 0x800824F0 0x8007FCBC 0x8007EE84,
                 reached only when the active word is 1) are stopped callees recorded with no arguments;
  poll_stop.txt  0x80081268 is a STOPPED callee (recorded, no effect): the shape of a provider that reaches the abort entry through its bound symbol.
Cases: a grid over the stored stamp (small values, +-1200 around the signed boundary, the 0x7FFFFFFF / 0x80000000 / 0xFFFFFFFF wraps), the clock
value relative to the deadline (-2000 .. +2, +100, extreme offsets, absolute edge values), the active word (0 / 1 / 2 / 3 / all ones), the status word,
the words read by the abort chain (0x8009B6B8, and 0x8009B6C0 when the first is 1), the argument, and a random remainder.  Seeded noise words 0x8009B570..0x8009B5A4 (DsSystemStatus
0x8009B574, issue / pending / queue words): the entry itself never reads them (the ready entry, a stopped callee, would).  The generator's own-written
load rule is the default (loads of bytes the original wrote itself are not logged; a case mixing own-written and external bytes is dropped, counted).

Usage: pe_cdx_cdgen50_vectors.py OUTDIR [--random N] [--seed S]     writes OUTDIR/poll.txt, poll_stop.txt (+ .classes, + stats line)"""
import os
import random
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_cdx_cdhost_vectors as g   # noqa: E402  (shared interpreter hooks, seq_model; also registers the cdgen47 specs)

m, c, gv, pl, v = g.m, g.c, g.gv, g.pl, g.v
ENTRY = 0x800811E4
VSYNC, READY, ABORT = 0x80073A44, 0x8007F608, 0x80081268
ABORT_CALLEES = (0x8007F69C, 0x800822BC, 0x800824F0, 0x8007FCBC, 0x8007EE84)
STAMP, STATUS, ACTIVE, CHAINW = 0x8009B6C4, 0x8009B6B4, 0x8009B6CC, 0x8009B6B8
NOISE = tuple(range(0x8009B570, 0x8009B5A8, 4))
M32 = 0xFFFFFFFF

STAMPS = (0, 1, 100, 1199, 1200, 0x12345678, 0x7FFFF000, 0x7FFFFB4F, 0x7FFFFB50, 0x7FFFFFFF, 0x80000000, 0x80000001, 0xFFFFFB4F, 0xFFFFFB50, 0xFFFFFFFF)
DELTAS = (-2000, -1201, -1200, -1000, -2, -1, 0, 1, 2, 3, 100, 0x7FFFFFFF, -0x80000000, 0x40000000)
ABSNOW = (0, 1, 1199, 1200, 1201, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0xFFFFFFFE)
VARIANTS = ((0, 0, 0, 0), (1, 1, 0, 1), (1, 2, 1, 0x1F8), (2, M32, 0, 2), (1, 0x80000000, 2, M32), (3, 5, 1, 0x7FFFFFFF))   # (active, status, chain word, fp)
STATS = {}


def setup(flavour):
    callees = {VSYNC: 1, READY: 1}
    if flavour == 'stop':
        callees[ABORT] = 0
    for t in ABORT_CALLEES:
        callees[t] = 0
    m.spec(ENTRY, 'cd_read_poll', 'w', callees, batch='cdgen50', n=1)
    sp = m.SPEC[ENTRY]
    sp['budget'] = 20000
    if flavour != 'stop':
        sp['inline'] = (ABORT,)
    return sp


def run(base, sp, sysc, case, s, flavour):
    stamp, now, active, status, chainw, fp, noise_seed, ready_ret, inner_ret = case
    ram = bytearray(base)
    nr = random.Random(noise_seed)
    for a in NOISE:
        struct.pack_into('<I', ram, a & 0x1FFFFF, nr.choice((0, 1, 2, M32, nr.getrandbits(32))))
    chainw2 = nr.choice((0, 1, 2, M32, nr.getrandbits(32)))       # 0x8009B6C0: read by the abort chain when the word at 0x8009B6B8 is 1
    for a, val in ((STAMP, stamp), (STATUS, status), (ACTIVE, active), (CHAINW, chainw), (0x8009B6C0, chainw2)):
        struct.pack_into('<I', ram, a & 0x1FFFFF, val & M32)
    rets, models = {}, {}
    for fn, vals in ((VSYNC, [now & M32]), (READY, [ready_ret])):
        rets[fn] = list(vals)
        models[fn] = g.seq_model(vals)
    for t in ABORT_CALLEES + ((ABORT,) if flavour == 'stop' else ()):
        rets[t] = [inner_ret]
        models[t] = g.seq_model([inner_ret])
    g.reset_models()
    c.UNFAITHFUL[0] = False
    try:
        res = m.run_case(ram, ENTRY, [fp], rets, sp, sysc, models)
    except (struct.error, IndexError, g.MipsBreak):
        STATS['drop:exception'] = STATS.get('drop:exception', 0) + 1
        return None
    except AssertionError as e:
        STATS['drop:' + ('budget' if 'instruction budget' in str(e) else 'unsupported')] = STATS.get('drop:budget', 0) + 1
        return None
    finally:
        g.reset_models()
    if c.UNFAITHFUL[0]:
        STATS['drop:own-written-mixed-or-unfaithful-load'] = STATS.get('drop:own-written-mixed-or-unfaithful-load', 0) + 1
        return None
    ret, loads, stores, calls = res
    text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % mm_ for rec, mm_ in calls) or '-'
    order = tuple(dict.fromkeys(m.canon(k) for k in sp['callees']))
    return gv.line(ENTRY, s, 0, [fp], (ret, loads, stores)) + ' RETS %s CALL %s' % (
        ','.join('%08X=%s' % (k, m.fmt_ret(rets[k])) for k in order if k in rets) or '-', text)


def cases(nrandom, seed):
    out = []     # (case tuple, class)
    k = 0
    for stamp in STAMPS:
        for d in DELTAS:
            now = (stamp + 1200 + d) & M32
            for j, (active, status, chainw, fp) in enumerate(VARIANTS):
                k += 1
                out.append(((stamp, now, active, status, chainw, fp, k, (0, 1, M32, 2)[k % 4], (0, 1, M32)[k % 3]), 'grid:stamp%08X:d%+d' % (stamp, d)))
    for stamp in STAMPS:
        for now in ABSNOW:
            for j, (active, status, chainw, fp) in enumerate(VARIANTS[:4]):
                k += 1
                out.append(((stamp, now, active, status, chainw, fp, k, (0, 1, M32, 2)[k % 4], (0, 1, M32)[k % 3]), 'abs:now%08X' % now))
    rng = random.Random(seed)
    for _ in range(nrandom):
        k += 1
        stamp = rng.choice(STAMPS + (rng.getrandbits(32),))
        now = rng.choice(((stamp + 1200 + rng.choice((-3, -2, -1, 0, 1, 2, 3))) & M32, rng.getrandbits(32), (stamp + rng.randrange(0, 3000)) & M32))
        out.append(((stamp, now, rng.choice((0, 1, 1, 2, 3, M32, rng.getrandbits(32))), rng.choice((0, 1, 2, M32, 0x80000000, rng.getrandbits(32))),
                     rng.choice((0, 1, 2, rng.getrandbits(32))), rng.choice((0, 1, 0x1F8, M32, rng.getrandbits(32))), k, rng.choice((0, 1, M32)), rng.choice((0, 1, M32))), 'random'))
    return out


def main():
    argv = sys.argv[1:]
    nrandom, seed = 400, 1
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
    sysc = m.syscall_pcs(base, ENTRY)
    allc = cases(nrandom, seed)
    for flavour, fname in (('inline', 'poll.txt'), ('stop', 'poll_stop.txt')):
        sp = setup(flavour)
        c._check_callees_complete(base, ENTRY, tuple(sp['callees']), inline=tuple(sp.get('inline', ())))
        STATS.clear()
        lines, classes = [], []
        for i, (case, cls) in enumerate(allc):
            r = run(base, sp, sysc, case, 1000 + i, flavour)
            if r is None:
                continue
            lines.append(r)
            classes.append(cls)
        open(os.path.join(outdir, fname), 'w').write('\n'.join(lines) + '\n')
        open(os.path.join(outdir, fname + '.classes'), 'w').write(''.join('%d %s\n' % (i + 1, t) for i, t in enumerate(classes)))
        print('%s flavour=%s cases=%d tried=%d own_written_loads_excluded=%d mixed=%d %s' % (fname, flavour, len(lines), len(allc), sum(m.OWNLOAD.values()),
              sum(m.MIXEDLOAD.values()), ' '.join('%s=%d' % kv for kv in sorted(STATS.items()))))


if __name__ == '__main__':
    main()

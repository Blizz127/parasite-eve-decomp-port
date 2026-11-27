#!/usr/bin/env python3
"""cdgen49: black-box vectors for the live functions of platform/pe_libcd.c.

ORIGINAL code from the retail EXE (build/disc1.candidate.exe, sha1 452fb033...) runs under the shared test-only
interpreter (pe_cdx_vectors.run_case).  Nothing here reads a provider.

Sets (one vector file each):
  readissue          retail 0x8006E6D4 (CD sector-read issue, the game function the hand file calls func_8006E6D4).
                     DsRead 0x80080E34 is a STOPPED callee with a scripted return (modelling choice, see REPORT: no
                     bound host symbol exists for it, run inline it would need the whole retail CD command engine plus a
                     controller model).  Stopped: 8007F72C 8007F778 8007F7A8 80080B44 80071A74(printf, 3 args) and the BIOS
                     exit 800719E4, which is TERMINAL (the case ends there, retail never returns from it).
  timeout_poll       the timeout arm of the SDK status poll 0x8007B010 as a slice: entry 0x8007B0DC .. end 0x8007B148
                     (puts 80073C5C, printf 80071A74 with 4 string arguments, flush 8007B9EC)
  timeout_issue      the same arm in the SDK command issuer 0x8007B558: entry 0x8007B7B8 .. end 0x8007B824 (other registers,
                     other context string, identical behaviour)
  readissue_dsinline reference set only: 0x8006E6D4 with DsRead 0x80080E34 run INLINE as original code; DsRead's own callees are the
                     stopped ones (800822AC abort check, 80080CDC last position, 8007F0C8 command sequence (5 args, 5th at sp+0x10),
                     80073A44 VSync, 800824F0 DMACallback).  This documents what retail's DsRead stores and calls synchronously;
                     no object implements DsRead as a function, so there is no checker target.
  timeout_poll_flushinline   reference set only: the poll arm with the flush 8007B9EC run inline (its CD register protocol and
                     state stores are part of the vectors; CD interrupt-flag model as cdgen42 8007BBFC)
Seeds: RUNTIME-OBSERVED tuples (--trace logs of pe_cdx_cdgen49_trace.c) replayed verbatim and perturbed, a product of the
gate dimensions, random fill.  A side file OUT.classes maps line number -> class.

Usage: pe_cdx_cdgen49_vectors.py OUT_DIR [-n N] [--trace F ...]
"""
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_cdx_vectors as m   # noqa: E402
from pe_battle_hud_oracle import MipsBreak   # noqa: E402

c, gv, v = m.c, m.gv, m.v
M32 = 0xFFFFFFFF
ZONE = c.ZONE

FLAGS = 0x800B0CD8
HW = 0x800B0DD4
RD_RDY, RD_QL, RD_DT, RD_EXIT, RD_POS, RD_DSREAD, RD_PRINTF = 0x8007F72C, 0x8007F778, 0x8007F7A8, 0x800719E4, 0x80080B44, 0x80080E34, 0x80071A74
TA_ENTRY, TA_END = 0x8007B0DC, 0x8007B148
TB_ENTRY, TB_END = 0x8007B7B8, 0x8007B824
PUTS, FLUSH = 0x80073C5C, 0x8007B9EC

def reg(entry, name, kinds, callees, **kw):
    m.spec(entry, name, kinds, callees, batch='cdgen49', **kw)


def put32(ram, a, val):
    struct.pack_into('<I', ram, a & 0x1FFFFF, val & M32)


def put16(ram, a, val):
    struct.pack_into('<H', ram, a & 0x1FFFFE, val & 0xFFFF)


def put8(ram, a, val):
    ram[a & 0x1FFFFF] = val & 0xFF


def hsh(*k):
    h = 0x9E3779B1
    for q in k:
        h = gv.xs32(((h ^ (q & M32)) * 2654435761) & M32 or 1)
    return h


# --------------------------------------------------------------------------------------------------------- traces
def read_traces(paths):
    out = {'issue': [], 'timeout': []}
    seen = set()
    for p in paths:
        for ln in open(p, errors='replace'):
            mm = re.match(r'T 8006E6D4 (\w+) (\w+) (\w+) (\w+) \| M flags=(\w+) hw=(\w+) .*\| G rdy=(\w+) ql=(\w+) dt=(\w+) \| R ret=(\w+)', ln)
            if mm:
                t = tuple(int(g, 16) for g in mm.groups())
                if t not in seen:
                    seen.add(t)
                    out['issue'].append(dict(args=t[:4], flags=t[4], hw=t[5], rdy=t[6], ql=t[7], dt=t[8], ret=t[9], src=os.path.basename(p)))
                continue
            mm = re.match(r'T TimeoutRecover \| M b294=(\w+) b295=(\w+) b296=(\w+) b2AFD5=(\w+) ctx=(\w+) tbl=(\w+),(\w+)', ln)
            if mm:
                t = tuple(int(g, 16) for g in mm.groups())
                if t not in seen:
                    seen.add(t)
                    out['timeout'].append(dict(b294=t[0], b295=t[1], b296=t[2], bfd5=t[3], ctx=t[4], src=os.path.basename(p)))
    return out


RT = {'issue': [], 'timeout': []}
CLASSES = []          # class of the case just generated (read by gen())

# ------------------------------------------------------------------------------------------------ readissue hook
E32 = (0, 1, 2, 3, 0x10, 0x20, 0x80, 0xFF, 0x100, 0x7FFF, 0x8000, 0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0xFFFFFFFE)
FLAGBASE = (0, 3, 0x4000, 0x40000003, 0x4000C202, 0x01004000, 0xFEFFBFFF, 0xFFFFFFFF, 0x00800000)
DESTS = (0x800A8028, 0x801ED800, 0x801229A0, 0x8012F1A0, 0x800F34F8, 0x80100000, 0x80100004, 0x801FFFFC, 0x80000000, 0, 0xFFFFFFFF, 0xA0100000, 0x00100000)
LBAS = (0, 1, 0x16, 0x43D, 0x44C, 0x478, 0x794, 0x7C7, 0x10, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x1869F, 0x20000)
OFFS = (0, 0, 0, 1, 2, 0x10, 0x100, 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000)
SECS = (0, 1, 2, 3, 5, 0xF, 0x16, 0x22, 0x55, 0x80, 0x100, 0x10000, 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000)
RDYV = (1, 1, 1, 0, 2, 0xFFFFFFFF, 0x100, 0x80000001)
QLV = (0, 0, 0, 1, 0xFFFFFFFF, 2, 0x80000000, 0x100)
DSV = (1, 1, 0, 0, 0xFFFFFFFF, 2, 0x7FFFFFFF, 0x80000000, 0x100, 0xFFFFFFFE)
NCELL = 2 * 3 * 2 * 3 * 5       # busy x rdy(1/0/other) x ql(0/nz) x dt(eq / low-differ / high-differ) x dsread(1/0/-1/other/big)


def issue_hook(ram, x, args, rets, seed):
    k = seed - 1
    nrt = len(RT['issue'])
    h = hsh(seed, 0x61)
    if m.PILOT[0]:
        CLASSES.append('pilot')
        return x, None
    if k < nrt * 3:
        t = RT['issue'][k // 3]
        var = k % 3
        args[:] = list(t['args'])
        put32(ram, FLAGS, t['flags']); put16(ram, HW, t['hw'])
        rets[RD_RDY], rets[RD_QL], rets[RD_DT] = t['rdy'] & M32, t['ql'] & M32, t['dt'] & M32
        rets[RD_DSREAD] = 1 if t['ret'] == 1 else 0
        if var == 0:
            CLASSES.append('verbatim')
        else:
            CLASSES.append('perturbed')
            # one or two edits of values the original reads or tests: flags word, a getter, the DsRead result, an argument
            edits = ((h >> 4) % 6, (h >> 9) % 6) if var == 2 else ((h >> 4) % 6,)
            for e in edits:
                if e == 0:
                    put32(ram, FLAGS, (t['flags'] ^ (1 << ((h >> 12) % 32))) & M32)
                elif e == 1:
                    rets[RD_RDY] = (0, 2, 0xFFFFFFFF, 1)[(h >> 15) % 4]
                elif e == 2:
                    rets[RD_QL] = (1, 0xFFFFFFFF, 2)[(h >> 17) % 3]
                elif e == 3:
                    rets[RD_DT] = (t['hw'] + (1, 0x10000, 0x8000)[(h >> 19) % 3]) & M32
                elif e == 4:
                    rets[RD_DSREAD] = (0, 0xFFFFFFFF, 2)[(h >> 21) % 3]
                else:
                    args[(h >> 23) % 4] = E32[(h >> 25) % len(E32)]
        return x, None
    p = k - nrt * 3
    if p < NCELL * 4:
        cell, rep = divmod(p, 4)
        busy, rdyc, qlc, dtc, dsc = cell % 2, (cell // 2) % 3, (cell // 6) % 2, (cell // 12) % 3, cell // 36
        hi = hsh(cell, rep, 0x33)
        base = FLAGBASE[(hi >> 3) % len(FLAGBASE)]
        put32(ram, FLAGS, (base | 0x01000000) if busy else (base & ~0x01000000 & M32))
        hw = (0, 1, 2, 0x7FFF, 0xFFFF, hi >> 8 & 0xFFFF)[(hi >> 20) % 6]
        put16(ram, HW, hw)
        rets[RD_RDY] = (1, 0, RDYV[4 + (hi >> 5) % 4])[rdyc]
        rets[RD_QL] = (0, QLV[3 + (hi >> 7) % 5])[qlc]
        rets[RD_DT] = (hw, (hw ^ (1, 2, 0x80, 0x8000)[(hi >> 9) % 4]) & 0xFFFF, (hw + (0x10000, 0x80000000, 0xFFFF0000)[(hi >> 11) % 3]) & M32)[dtc]
        rets[RD_DSREAD] = (1, 0, 0xFFFFFFFF, DSV[6 + (hi >> 13) % 4], DSV[2 + (hi >> 15) % 3 + 2])[dsc] & M32
        args[0] = LBAS[(hi >> 2) % len(LBAS)]
        args[1] = OFFS[(hi >> 6) % len(OFFS)]
        args[2] = DESTS[(hi >> 10) % len(DESTS)]
        args[3] = SECS[(hi >> 14) % len(SECS)]
        CLASSES.append('product')
        return x, None
    CLASSES.append('random')
    put32(ram, FLAGS, h if (h >> 5) & 1 else FLAGBASE[(h >> 8) % len(FLAGBASE)])
    hw = (h >> 9) & 0xFFFF
    put16(ram, HW, hw)
    rets[RD_RDY] = RDYV[(h >> 3) % len(RDYV)]
    rets[RD_QL] = QLV[(h >> 25) % len(QLV)]
    rets[RD_DT] = hw if (h >> 26) % 3 else (hw ^ (1 << ((h >> 27) % 16))) & M32
    rets[RD_DSREAD] = DSV[(h >> 28) % len(DSV)]
    for i in range(4):
        h = gv.xs32(h)
        args[i] = (E32[(h >> 7) % len(E32)], h)[h & 1]
    return x, None


DS_ABORT, DS_POS, DS_SEQ, DS_VSYNC, DS_DMACB = 0x800822AC, 0x80080CDC, 0x8007F0C8, 0x80073A44, 0x800824F0
DS_STATE, DS_PEND = 0x8009B6CC, 0x8009B6B8


def dsinline_hook(ram, x, args, rets, seed):
    """issue_hook gating, then the state DsRead reads: 0x8009B6CC (1 = busy -> returns 0), the abort check, the last-position pointer
    (a0 == 0 path), the pending word 0x8009B6B8 bit 0 (DMACallback install), and the scripted results of its callees"""
    x, _ = issue_hook(ram, x, args, rets, seed)
    h = hsh(seed, 0x91)
    if m.PILOT[0]:
        return x, None
    put32(ram, DS_STATE, (0, 0, 0, 1, 2, 0xFFFFFFFF, 0x100)[h % 7])
    put32(ram, DS_PEND, (0, 1, 2, 3, h)[(h >> 4) % 5])
    rets[DS_ABORT] = (0, 0, 0, 1, 2, 0xFFFFFFFF)[(h >> 8) % 6]
    rets[DS_POS] = ZONE + 0x40 * ((h >> 12) % 64)
    rets[DS_SEQ] = (1, 1, 0, 2, 0xFFFFFFFF, 0x7FFFFFFF, h)[(h >> 18) % 7] & M32
    rets[DS_VSYNC] = (0, 1, 0x1234, 0x7FFFFFFF, 0xFFFFFFFF, h >> 3)[(h >> 22) % 6] & M32
    rets[DS_DMACB] = (0, 0x80081110, 0x80123450, 0xFFFFFFFF)[(h >> 26) % 4]
    CLASSES[-1] = CLASSES[-1] + '+ds'
    return x, None


# --------------------------------------------------------------------------------------------- timeout-arm hook
IDX = (0, 1, 2, 3, 4, 5, 8, 9, 0x10, 0x1F, 0x20, 0x7F, 0x80, 0xFF)
CTX = (0x80011BA0, 0x80011BCC, 0, 0x80150000, 0xFFFFFFFF, 0x80011B18, 0x1F801000)
S2, S3, S4, S5, S8 = 18, 19, 20, 21, 30
SP_SLICE = 0x801FEFC0      # the stack pointer the retail function has after its 0x40-byte prologue when entered the way the readissue entry is (outgoing-argument stores land in the window the checker excludes)


def make_timeout_hook(tag, tbl_a, tbl_b, cmd_tbl):
    """tbl_a: base of the name table indexed by byte 0x8009B294 / 0x8009B295 (0x8009B05C), cmd_tbl: 0x8009AFDC indexed by byte
    0x8009AFD5.  The slice reads the two index bytes, the context word 0x800A3480 and three table slots."""
    def put_slots(ram, i294, i295, ifd5, hi, same):
        a = tbl_a + 4 * i294
        b = tbl_a + 4 * i295
        cmd = cmd_tbl + 4 * ifd5
        vals = [(0x80150000 + 0x20 * j, hi >> j & M32, 0, M32, 0x80011C00 + 8 * j, 0x80000000)[(hi >> (3 * j)) % 6] for j in range(3)]
        put32(ram, a, vals[0]); put32(ram, b, vals[1]); put32(ram, cmd, vals[2])
        if same == 1:
            put32(ram, b, vals[0])         # both index bytes name one slot: the two printed arguments are equal

    def hook(ram, x, args, rets, seed):
        k = seed - 1
        h = hsh(seed, 0x71 + tag)
        if m.PILOT[0]:
            CLASSES.append('pilot')
            return x, None
        nrt = len(RT['timeout'])
        if k < nrt * 2:
            t = RT['timeout'][k // 2]
            put8(ram, 0x8009B294, t['b294']); put8(ram, 0x8009B295, t['b295']); put8(ram, 0x8009B296, t['b296']); put8(ram, 0x8009AFD5, t['bfd5'])
            put32(ram, 0x800A3480, CTX[tag] if k % 2 == 0 else t['ctx'] or CTX[tag])
            put_slots(ram, t['b294'], t['b295'], t['bfd5'], h, 0)
            CLASSES.append('verbatim-state' if k % 2 == 0 else 'verbatim-ctx')
            return x, None
        p = k - nrt * 2
        i294, i295, ifd5 = IDX[(h >> 3) % len(IDX)], IDX[(h >> 8) % len(IDX)], IDX[(h >> 13) % len(IDX)]
        if p % 7 == 0:
            i295 = i294
        if p % 11 == 0:
            ifd5 = i294
        put8(ram, 0x8009B294, i294); put8(ram, 0x8009B295, i295); put8(ram, 0x8009B296, (h >> 18) & 0xFF); put8(ram, 0x8009AFD5, ifd5)
        put32(ram, 0x800A3480, CTX[(h >> 21) % len(CTX)] if p % 3 else CTX[tag])
        put_slots(ram, i294, i295, ifd5, hsh(seed, 5), (h >> 28) & 1)
        if p % 13 == 0:                                # all three names from one slot class: equal pointers
            put32(ram, tbl_a + 4 * i294, 0x80150000)
        CLASSES.append('product' if p < 200 else 'random')
        return x, None
    return hook


def cd_hook(inner):
    """chain the CD interrupt-flag model of cdgen42 (the flush's `lbu IF & 7` poll loop runs inline in the reference set)"""
    import pe_cdx_specs
    cdirq = pe_cdx_specs._cdirq_hook(m)

    def hook(ram, x, args, rets, seed):
        x, mdl = inner(ram, x, args, rets, seed)
        x, _ = cdirq(ram, x, args, rets, seed)
        return x, mdl
    return hook


# ------------------------------------------------------------------------------------------------------ generator
def gen(base, entry, n):
    """Own case loop (m.cases() minus discovery/seed_state: the hook seeds everything the entry reads).  Returns (lines, classes, stats)."""
    sp = m.SPEC[entry]
    sysc = m.syscall_pcs(base, entry)
    out, cls = [], []
    spun = dropped = term = 0
    seed = 0
    del CLASSES[:]
    while len(out) < n and seed < 8 * n:
        seed += 1
        s = seed * 7919 ^ entry
        ram, x, ring, size = c.prep(base, s)
        args = []
        for kd in sp['kinds']:
            x = gv.xs32(x)
            args.append(m._arg(kd, x))
        x = gv.xs32(x)
        rets = {}
        for i, cc in enumerate(sp['callees']):
            x = gv.xs32(x)
            rets[m.canon(cc)] = (0, 1, 0xFFFFFFFF, gv.xs32(x))[x % 4]
        n0 = len(CLASSES)
        x, models = m.HOOKS[entry](ram, x, args, rets, seed)
        c.UNFAITHFUL[0] = False
        try:
            res = m.run_case(ram, entry, args, rets, sp, sysc, models)
        except (struct.error, IndexError):
            dropped += 1
            del CLASSES[n0:]
            continue
        except MipsBreak:
            dropped += 1
            del CLASSES[n0:]
            continue
        except AssertionError as e:
            if 'instruction budget' not in str(e):
                raise
            spun += 1
            del CLASSES[n0:]
            continue
        finally:
            c.MEMSEQ.clear()
            c.CdIrq.active = False
        if c.UNFAITHFUL[0]:
            dropped += 1
            del CLASSES[n0:]
            continue
        ret, loads, stores, calls = res
        toks = []
        for j, (rec, mm) in enumerate(calls):
            t = ','.join('%08X' % y for y in rec) + '@%d' % mm
            if j == len(calls) - 1 and m.TERMINATED[0] is not None:
                t += '!'
                term += 1
            toks.append(t)
        text = ' '.join(toks) or '-'
        order = tuple(dict.fromkeys(m.canon(k) for k in sp['callees'])) + tuple(k for k in rets if k not in {m.canon(q) for q in sp['callees']})
        out.append(gv.line(entry, s, 0, args, (ret, loads, stores)) +
                   ' RETS %s CALL %s' % (','.join('%08X=%s' % (k, m.fmt_ret(rets[k])) for k in order) or '-', text))
        cls.append(CLASSES[-1] if len(CLASSES) > n0 else '?')
    return out, cls, dict(spun=spun, dropped=dropped, terminated=term, tries=seed)


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
    RT = read_traces(traces)
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    c.CALL_CAP[0] = 400
    report = []

    def run(name, entry, count, mode=None):
        o, cl, st = gen(base, entry, count)
        with open(os.path.join(outdir, name + '.txt'), 'w') as f:
            f.write('\n'.join(o) + '\n')
        with open(os.path.join(outdir, name + '.classes'), 'w') as f:
            f.write('\n'.join('%d %s' % (i + 1, k) for i, k in enumerate(cl)) + '\n')
        tally = {}
        for k in cl:
            tally[k] = tally.get(k, 0) + 1
        report.append('%s entry=%08X cases=%d %s classes=%s' % (name, entry, len(o), ' '.join('%s=%d' % kv for kv in st.items()), tally))

    # readissue
    entry = 0x8006E6D4
    reg(entry, 'cd_read_issue', 'wwpw', {RD_RDY: 0, RD_QL: 0, RD_DT: 0, RD_EXIT: 1, RD_POS: 2, RD_DSREAD: 4, RD_PRINTF: 3},
        terminal=(RD_EXIT,))
    c._check_callees_complete(base, entry, tuple(m.SPEC[entry]['callees']))
    m.HOOKS[entry] = issue_hook
    run('readissue', entry, max(n, len(RT['issue']) * 3 + NCELL * 4 + 120))

    # reference: DsRead inline (its callees stopped)
    reg(entry, 'cd_read_issue_dsinline', 'wwpw', {RD_RDY: 0, RD_QL: 0, RD_DT: 0, RD_EXIT: 1, RD_POS: 2, RD_PRINTF: 3,
                                                   DS_ABORT: 0, DS_POS: 1, DS_SEQ: 5, DS_VSYNC: 1, DS_DMACB: 1}, terminal=(RD_EXIT,))
    m.HOOKS[entry] = dsinline_hook
    run('readissue_dsinline', entry, 1000)
    reg(entry, 'cd_read_issue', 'wwpw', {RD_RDY: 0, RD_QL: 0, RD_DT: 0, RD_EXIT: 1, RD_POS: 2, RD_DSREAD: 4, RD_PRINTF: 3},
        terminal=(RD_EXIT,))
    m.HOOKS[entry] = issue_hook

    # timeout arms (slices): flush stopped
    tcall = {PUTS: 1, RD_PRINTF: 5, FLUSH: 0}
    reg(TA_ENTRY, 'timeout_arm_poll', '', dict(tcall), void=True, end_at=TA_END,
        regs={29: SP_SLICE, S2: 0x8009B294, S4: 0x8009B05C, S5: 0x8009B295, S8: 0x8009AFDC})
    m.HOOKS[TA_ENTRY] = make_timeout_hook(0, 0x8009B05C, 0x8009B05C, 0x8009AFDC)
    run('timeout_poll', TA_ENTRY, max(n, len(RT['timeout']) * 2 + 300))
    reg(TB_ENTRY, 'timeout_arm_issue', '', dict(tcall), void=True, end_at=TB_END,
        regs={29: SP_SLICE, S2: 0x8009B294, S3: 0x8009B05C, S4: 0x8009B295, S5: 0x8009AFDC})
    m.HOOKS[TB_ENTRY] = make_timeout_hook(1, 0x8009B05C, 0x8009B05C, 0x8009AFDC)
    run('timeout_issue', TB_ENTRY, max(n, len(RT['timeout']) * 2 + 300))
    # reference set: flush inline (register protocol + state stores part of the vectors)
    reg(TA_ENTRY, 'timeout_arm_poll_flushinline', '', {PUTS: 1, RD_PRINTF: 5}, void=True, end_at=TA_END,
        regs={29: SP_SLICE, S2: 0x8009B294, S4: 0x8009B05C, S5: 0x8009B295, S8: 0x8009AFDC})
    m.HOOKS[TA_ENTRY] = cd_hook(make_timeout_hook(0, 0x8009B05C, 0x8009B05C, 0x8009AFDC))
    run('timeout_poll_flushinline', TA_ENTRY, 150)
    open(os.path.join(outdir, 'gen.log'), 'w').write('\n'.join(report) + '\n')
    print('\n'.join(report))
    print('runtime tuples', {k: len(x) for k, x in RT.items()})


if __name__ == '__main__':
    main()

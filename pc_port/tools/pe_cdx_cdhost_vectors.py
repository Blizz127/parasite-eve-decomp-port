#!/usr/bin/env python3
"""cdgen47: black-box vectors for the six CD host providers with no clean-room derivation record
(retail 0x8007AAB4 ack, 0x8007B010 status poll, 0x8007B558 command issue, 0x8007BF44 sector DMA,
0x8007C564 stream handler, 0x8007CEAC DMA issue).  The ORIGINAL code runs under the test-only interpreter
(pe_cdx_vectors.run_case); nothing here reads a provider.  Specs: pe_cdx_specs.py (batch cdgen47).

What is new relative to pe_cdx_vectors.cases():
 * RUNTIME-OBSERVED SEEDS.  --capture FILE.. reads logs written by the linker-wrap logger pe_cdx_cdgen47_capture.c
   (one record per distinct (entry, args, RAM snapshot, native register reads, callee calls) tuple seen while the
   current port ran: boot, Day-1 route to field_start, ...).  A tuple is replayed VERBATIM (arguments, the snapshot
   regions written into the seeded RAM, the register reads the native made turned into per-address read sequences:
   a model input, never an expected output);
 * PERTURBED cases: one to three words the original actually READ on a runtime tuple are replaced by neighbours /
   edge values (x-1, x+1, 0, 1, 0xFF, ..), arguments vary, register read sequences are lengthened (k "not ready"
   reads before the observed value) or bit-flipped;
 * DISCOVERY cases: the generator's own random seeding over the words the original reads (pilot discovery seeded from
   the runtime tuples so words on runtime-reached branches are found, no 20 % filter);
 * a CD controller model (psx-spx): IF flags + queued responses, response FIFO behind 0x1F801801, RSLRRDY/DRQSTS
   bits in 0x1F801800, DMA3 CHCR busy countdown (REGSEQ read sequences per register address);
 * per-call callee return sequences (ack types, VSync progressions) so retry / timeout loops run to their exits;
 * a side file OUT.classes (line number -> verbatim | perturbed | discovery) for the report's per-class tables.

Usage: pe_cdx_cdhost_vectors.py OUT.txt [-n N] [--capture F ..] [--only ENTRY] [--tuple-cap K] [--silent] [--stop A,B]
  --silent: the diagnostic-text callees (printf 0x80071A74, puts 0x80073C5C) are silent stops (not recorded, no effect)
"""
import hashlib
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_cdx_vectors as m   # noqa: E402  (shared interpreter, run_case and line format)
from pe_cdx_specs import load_specs   # noqa: E402
from pe_battle_hud_oracle import MipsBreak   # noqa: E402

c, gv, pl, v = m.c, m.gv, m.pl, m.v
ZONE = c.ZONE
load_specs(m, 'cdgen47')
c.CALL_CAP[0] = 400         # callee stops per case before a poll loop counts as spun (shared default 8192: far slower, same outcome for these entries)
ENTRIES = [e for e, s in sorted(m.SPEC.items()) if s['batch'] == 'cdgen47']
SENT = {0x8009AFB4: 0x80131140, 0x8009AFB8: 0x80131180, 0x800B0CCC: 0x801311C0}   # callback words -> sentinel targets
RAMREG = ((0x8009AF00, 0x900), (0x800A3400, 0x400), (0x800A8000, 0x80), (0x800B0C00, 0x100), (0x800B8A80, 0x80),
          (0x800BE980, 0x78), (0x800C0D00, 0x100), (0x800C20B8, 0x18))
EDGE8 = (0, 1, 2, 3, 4, 5, 6, 7, 0x0F, 0x10, 0x1F, 0x20, 0x40, 0x7F, 0x80, 0xFF)
EDGE32 = (0, 1, 2, 3, 4, 5, 8, 0x10, 0x20, 0x40, 0x7F, 0x80, 0xFF, 0x100, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0xFFFFFFFE)

# ------------------------------------------------------- register read sequences (model input)
REGSEQ = {}            # buffer offset (register address - 0x1F800000) -> [size, values, reads]
_prev_load = pl._ns['LOAD']


def _regseq_load(fmt, buf, a):
    q = REGSEQ.get(a)
    if q is not None:
        vals = q[1]
        val = vals[min(q[2], len(vals) - 1)]
        q[2] += 1
        struct.pack_into(fmt, buf, a, val & ((1 << (8 * struct.calcsize(fmt))) - 1))
    return _prev_load(fmt, buf, a)


pl._ns['LOAD'] = _regseq_load
WAIT = [False]          # the case's register model makes the entry wait (not-ready reads before ready): class suffix `-wait`
FIFO = {'on': False, 'q': [], 'hi': 0}      # CD response FIFO model (reads of 0x1F801801 pop one byte)
_hw_prev = pl._ns['LOAD']


def _fifo_load(fmt, buf, a):
    if FIFO['on'] and a not in REGSEQ:
        if a == 0x1801 and fmt == '<B':
            buf[0x1801] = FIFO['q'].pop(0) if FIFO['q'] else 0
        elif a == 0x1800 and fmt == '<B':
            buf[0x1800] = (buf[0x1800] & 0x03) | (0x20 if FIFO['q'] else 0) | FIFO['hi'] | 0x08
    return _hw_prev(fmt, buf, a)


pl._ns['LOAD'] = _fifo_load


def reg_off(addr):
    return addr - 0x1F800000 if 0x1F800000 <= addr < 0x1F802000 else None


# ----------------------------------------------------------------- capture parsing
def parse_capture(paths):
    """entry -> list of dict(args, mem{addr: bytes}, ptr{argidx: bytes}, regs[(addr,size,val)], calls[str]) (distinct)."""
    out = {}
    seen = set()
    for p in paths:
        for line in open(p):
            if not line.startswith('T '):
                continue
            parts = line.rstrip('\n').split(' | ')
            if len(parts) < 4:
                continue
            h = parts[0].split()
            entry = int(h[1], 16)
            args = [int(t, 16) for t in h[2:]]
            mem, ptr = {}, {}
            for tok in parts[1].split()[1:]:
                k, hx = tok.split(':')
                if k.startswith('A'):
                    ptr[int(k[1:], 16)] = bytes.fromhex(hx)
                else:
                    mem[int(k, 16)] = bytes.fromhex(hx)
            regs = []
            for tok in parts[2].split()[1:]:
                for item in tok.split(';'):
                    if not item or item == '-':
                        continue
                    kd, ad, sz, vl = item.split(':')
                    regs.append((int(ad, 16), int(sz), int(vl, 16)))
            calls = [t for t in parts[3].split()[1:] for t in t.split(';') if t and t != '-']
            vb = int(parts[4].split()[1]) if len(parts) > 4 else None
            key = hashlib.sha256(repr((entry, args, sorted(mem.items()), sorted(ptr.items()), regs, calls)).encode()).hexdigest()
            if key in seen:
                continue
            seen.add(key)
            out.setdefault(entry, []).append(dict(args=args, mem=mem, ptr=ptr, regs=regs, calls=calls, src=os.path.basename(p), vblank=vb))
    return out


def apply_snapshot(ram, mem):
    for lo, data in mem.items():
        off = lo & 0x1FFFFF
        ram[off:off + len(data)] = data


def apply_tuple(ram, t, args_n):
    apply_snapshot(ram, t['mem'])
    for i, data in t['ptr'].items():
        if i < len(t['args']):
            a = t['args'][i]
            if 0x80000000 <= a < 0x801FFF00:
                ram[a & 0x1FFFFF:(a & 0x1FFFFF) + len(data)] = data


def regseq_from(t, k_extra=None):
    """Per-address read sequences from the native's register reads (observed order)."""
    REGSEQ.clear()
    seqs = {}
    for ad, sz, vl in t['regs']:
        o = reg_off(ad)
        if o is not None:
            seqs.setdefault((o, sz), []).append(vl)
    for (o, sz), vals in seqs.items():
        vals = list(vals)
        if k_extra and (o, sz) in k_extra:
            vals = [k_extra[(o, sz)][0]] * k_extra[(o, sz)][1] + vals
        REGSEQ[o] = [sz, vals, 0]


# ----------------------------------------------------------------- argument kinds
def arg47(k, x):
    if k == 'b':
        return (x >> 4) & 0xFF if x & 3 else EDGE8[(x >> 8) % len(EDGE8)]
    if k == 'c':
        return 3 if x & 1 else (x >> 8) % 8
    if k == 'w':
        return EDGE32[(x >> 8) % len(EDGE32)] if x & 1 else gv.xs32(x)
    if k == 'p':
        return ZONE + (((x >> 4) % 0x400) * 4) if x % 5 else 0
    return gv.xs32(x)


# ----------------------------------------------------------------- callee return models
def seq_model(vals):
    st = [0]

    def f(ram, r):
        val = vals[min(st[0], len(vals) - 1)]
        st[0] += 1
        return val
    return f


def make_models(entry, sp, x, rets, cbs):
    models = {}
    for cc in sorted(sp['callees']):
        x = gv.xs32(x)
        k = x % 4
        canon = m.canon(cc)
        if cc == 0x8007AAB4:            # CD interrupt acknowledge: k "nothing" results, then an interrupt type / state
            fin = (1, 2, 3, 4, 5, 6, 0, 0xFFFFFFFF, 7, 0x80)[(x >> 8) % 10]
            seq = [0] * ((x >> 4) % 4) + [(x >> 12) % 7] * ((x >> 16) % 2) + [fin]
        elif cc == 0x80073A44:          # VSync: a counter that advances by `step` per call from a seeded base
            base = (0, 1, 0x10, 0xFFF0, 0x7FFFFFF0, gv.xs32(x + 3) & 0xFFFFFF)[(x >> 4) % 6]
            step = (1, 1, 2, 5, 0x10, 0x100, 0x1000, 0x20000)[(x >> 10) % 8]
            seq = [(base + i * step) & 0xFFFFFFFF for i in range(1500)]
        elif cc == 0x80073DE8:
            seq = [(0, 1, 2, 0x10, 0x100, 0x8000, 0xFFFF, gv.xs32(x) & 0xFFFF)[(x >> 4) % 8]]
        elif cc == 0x8007B010 or cc == 0x8007A488:
            seq = [(0, 1, 0xFFFFFFFF, 2, 3, 0xFFFFFFFE, 0x80000000, 0x7FFFFFFF)[(x >> 4) % 8]] * 2
            if (x >> 8) % 3 == 0:
                seq = [(x >> 12) & 0xFF] * 3 + seq
        elif cc in (0x80071A74, 0x80073C5C):
            seq = [0]
        elif cc in SENT.values() or cc in (0x8007C214, 0x8007CEAC, 0x8007B9EC):
            seq = [(0, 1, 0xFFFFFFFF, 2)[(x >> 4) % 4]]
        else:
            seq = [(0, 1, 0xFFFFFFFF)[k % 3]]
        rets[canon] = list(seq)
        models[cc] = seq_model(seq)
    return x, models


# ----------------------------------------------------------------- CD controller model per case
def cd_model(ram, x, seed, tuple_seeded):
    x = gv.xs32(x)
    c.CdIrq.active = True
    c.CdIrq.flags = (0, 1, 2, 3, 4, 5, x >> 8)[x % 7] & 0xFF
    x = gv.xs32(x)
    c.CdIrq.queue = [1 + (x >> (3 * i)) % 5 for i in range(x % 4)]
    c.CdIrq.noise = ((x >> 12) & 0x18) if seed % 2 else 0
    x = gv.xs32(x)
    ram[0x1800] = x & 0xFF
    c.CdIrq.refresh(ram)
    FIFO['on'] = True
    x = gv.xs32(x)
    FIFO['q'] = [(x >> (3 * i)) & 0xFF if (x >> 24) & 1 else (i * 37 + 5) & 0xFF for i in range(x % 11)]
    FIFO['hi'] = (0, 0x40, 0x80, 0xC0)[(x >> 26) % 4]
    return x


def reset_models():
    REGSEQ.clear()
    WAIT[0] = False
    FIFO['on'] = False
    FIFO['q'] = []
    FIFO['hi'] = 0
    c.CdIrq.active = False
    c.MEMSEQ.clear()


def entry_hw(entry, ram, x, args):
    """Poll-model register sequences for the register loops the entry runs (model input, see header)."""
    x = gv.xs32(x)
    if entry == 0x8007BF44:
        # DRQSTS (bit 6 of 0x1F801800) not ready for k reads then ready; DMA3 CHCR (0x1F8010B8) busy k reads then idle
        k1, k2 = (0, 0, 0, 0, 0, 1, 2, 3)[(x >> 4) % 8], (0, 0, 0, 0, 0, 1, 2, 3)[(x >> 8) % 8]
        WAIT[0] = k1 > 0 or k2 > 0
        REGSEQ[0x1800] = [1, [0x00 if (x >> 12) & 1 else 0x20] * k1 + [0x40 | 0x20 | (x >> 14 & 3)], 0]
        REGSEQ[0x10B8] = [4, [0x11000000] * k2 + [(0, 0x01000000, 0x00000200, 0x10000000)[(x >> 16) % 4] & ~0x01000000], 0]
    if entry == 0x8007C564:
        # MDEC-out DMA1 CHCR (0x1F801098, the retail read of the MDEC busy state): idle, busy (bit 24), mixed bits, and k busy reads then idle;
        # without this the register is single-valued (0) in every case and a seam that merely consumes the read tests nothing
        k = (0, 0, 0, 1, 2, 3)[(x >> 4) % 6]
        busy = (0x01000000, 0x11000000, 0x01000000 | (x >> 8 & 0xFFFF), 0x10000000, 0xFFFFFFFF, 0x00000001)[(x >> 10) % 6]
        idle = (0, 0x00000200, 0x00FFFFFF & (x >> 12), 0x10000000 & ~0x01000000)[(x >> 14) % 4]
        REGSEQ[0x1098] = [4, [busy] * (k + 1) + [idle] if (x >> 18) & 1 else [busy] * k + [idle], 0]
        WAIT[0] = False
    if entry == 0x8007CEAC and args:
        # the entry waits for the channel's CHCR busy bit (bit 24, 0x1F801088 + 0x10 * channel) to clear, polling up to 0x10000
        # times, then prints a timeout message: k busy reads then idle.  k up to 200; the 0x10000-poll timeout branch is NOT
        # sampled (65537 register reads exceed the checker's token capacity, see REPORT)
        k = (0, 0, 0, 0, 1, 2, 3, 7, 40, 200)[(x >> 4) % 10]
        WAIT[0] = k > 0
        busy = 0x01000000 | ((x >> 8) & 0x7E000000) | (x & 0xFF)
        k2 = (0, 0, 0, 0, 1, 2, 3, 9)[(x >> 16) % 8]            # then DRQSTS (bit 6 of 0x1F801800): k2 not-ready reads, then ready
        WAIT[0] = WAIT[0] or k2 > 0
        REGSEQ[0x1800] = [1, [(x >> 20) & 0x3F & ~0x40] * k2 + [0x40 | ((x >> 24) & 0x3F)], 0]
        REGSEQ[0x1088 + 0x10 * (args[0] & 7)] = [4, [busy] * k + [(0, 0x00000200, 0x10000000, 0x00ABCDEF & ~0x01000000, 0xFEFFFFFF)[(x >> 12) % 5]], 0]
    return x


# ----------------------------------------------------------------- discovery / case loop
def run_one(base, entry, sp, sysc, s, kind, tup, x_extra, D, tuples_for_base, ov=None):
    ram, x, ring, size = c.prep(base, s)
    x = gv.xs32(x + x_extra)
    reset_models()
    args = []
    for ch in sp['kinds']:
        x = gv.xs32(x)
        args.append(arg47(ch, x))
    if tup is not None:
        apply_tuple(ram, tup, len(sp['kinds']))
        args = list(tup['args'][:len(sp['kinds'])]) + [0] * max(0, len(sp['kinds']) - len(tup['args']))
    elif tuples_for_base:
        x = gv.xs32(x)
        snap = tuples_for_base[x % len(tuples_for_base)]['mem']      # a runtime-observed base state: the libcd data blocks only
        apply_snapshot(ram, {k_: v_ for k_, v_ in snap.items() if k_ in (0x8009AF00, 0x800A3400)})
    x = cd_model(ram, x, s, tup is not None)
    if tup is not None:
        regseq_from(tup)
    # callback words: 0 or a sentinel (jalr target is recorded, not run)
    if kind != 'verbatim':
        for cw, sent in SENT.items():
            x = gv.xs32(x)
            struct.pack_into('<I', ram, cw & 0x1FFFFF, (0, sent, sent, 0)[x % 4])
    if kind == 'perturbed':
        x = perturb(ram, x, tup, args, sp, D)
    elif kind == 'discovery':
        x = seed_words(ram, x, D)
    x = entry_hw(entry, ram, x, args) if kind != 'verbatim' or HW_ON_VERBATIM[0] else x       # a verbatim tuple keeps the observed register reads (--hw-on-verbatim: replaced by the poll-wait model)
    for (a, sz), val in sorted((k_, v_) for k_, v_ in (ov or {}).items()):          # explorer overrides: one word each, applied last
        off = a & 0x1FFFFF
        if sz == 1:
            ram[off] = val & 0xFF
        elif sz == 2:
            struct.pack_into('<H', ram, off & ~1, val & 0xFFFF)
        else:
            struct.pack_into('<I', ram, off & ~3, val & 0xFFFFFFFF)
    rets = {}
    x, models = make_models(entry, sp, x, rets, None)
    if sysc:
        rets[0x5C000000] = 0
    return ram, args, rets, models, x


def perturb(ram, x, tup, args, sp, D):
    """One to three single-word edits on words the original read on runtime tuples, or an argument / register change."""
    x = gv.xs32(x)
    n = 1 + x % 3
    words = sorted(D)
    for i in range(n):
        x = gv.xs32(x)
        op = (x >> 4) % 8
        if op >= 6 and sp['kinds']:                # argument edit
            j = (x >> 8) % len(sp['kinds'])
            args[j] = arg47(sp['kinds'][j], gv.xs32(x))
        elif op == 5 and REGSEQ:                   # register read sequences: k extra "not ready" reads, or a flipped bit
            key = sorted(REGSEQ)[(x >> 8) % len(REGSEQ)]
            q = REGSEQ[key]
            first = q[1][0]
            q[1] = [first ^ (1 << ((x >> 12) % (8 * q[0])))] * ((x >> 16) % 4) + q[1] if (x >> 20) & 1 else [first ^ (1 << ((x >> 12) % (8 * q[0])))] + q[1][1:]
        elif words:
            a, sz = words[(x >> 8) % len(words)]
            off = a & 0x1FFFFF
            if sz == 1:
                cur = ram[off]
                ram[off] = (cur + (1, 0xFF, 0)[(x >> 12) % 3] if (x >> 14) & 1 else EDGE8[(x >> 12) % len(EDGE8)]) & 0xFF
            elif sz == 2:
                cur = struct.unpack_from('<H', ram, off & ~1)[0]
                struct.pack_into('<H', ram, off & ~1, (cur + (1, 0xFFFF, 0)[(x >> 12) % 3] if (x >> 14) & 1 else EDGE8[(x >> 12) % len(EDGE8)]) & 0xFFFF)
            else:
                cur = struct.unpack_from('<I', ram, off & ~3)[0]
                struct.pack_into('<I', ram, off & ~3, (cur + (1, 0xFFFFFFFF, 0, 2)[(x >> 12) % 4] if (x >> 14) & 1 else EDGE32[(x >> 12) % len(EDGE32)]) & 0xFFFFFFFF)
    return x


def seed_words(ram, x, D):
    for a, sz in sorted(D):
        x = gv.xs32(x)
        if x % 5 == 0:
            continue
        off = a & 0x1FFFFF
        if a in SENT:
            continue
        if sz == 1:
            ram[off] = (0, 1, 2, 3, 4, 5, 0x7F, 0x80, 0xFF, x >> 12 & 0xFF)[(x >> 8) % 10]
        elif sz == 2:
            struct.pack_into('<H', ram, off & ~1, (0, 1, 2, 0x7FFF, 0x8000, 0xFFFF, x >> 12 & 0xFFFF)[(x >> 8) % 7])
        else:
            cur = struct.unpack_from('<I', ram, off & ~3)[0]
            if 0x1F800000 <= cur < 0x1F810000 or 0x80000000 <= cur < 0x80200000:
                continue                                    # a pointer / register-block word keeps its (runtime / EXE) value
            k = (x >> 4) % 6
            val = (ZONE + (x >> 10) % 0x400 * 4) if k == 5 else (x >> 8 & 0x3F) if k == 4 else \
                m.CLASS32[(x >> 8) % len(m.CLASS32)] if k < 4 else x
            struct.pack_into('<I', ram, off & ~3, val & 0xFFFFFFFF)
    return x


def collect_D(base, entry, sp, sysc, tuples):
    """Words (state, not code/stack/zone) the original reads: pilots + every runtime tuple run verbatim; every word, no filter."""
    D = set()
    pilots = [(None, i) for i in range(60)] + [(t, i) for i, t in enumerate(tuples)]
    for tup, i in pilots:
        s = (i + 1) * 104729 ^ entry
        try:
            ram, args, rets, models, x = run_one(base, entry, sp, sysc, s, 'verbatim' if tup else 'discovery', tup, 0, D if False else set(), tuples)
        except (struct.error, IndexError):
            continue
        c.UNFAITHFUL[0] = False
        try:
            res = m.run_case(ram, entry, args, rets, sp, sysc, models)
        except (AssertionError, struct.error, IndexError, MipsBreak):
            continue
        finally:
            reset_models()
        for a, sz, _val in res[1]:
            if 0x80090000 <= a < 0x80100000 or 0x1F801000 <= a < 0x1F803000:
                if a >= 0x80000000 and sz < 4:
                    D.add((a, sz))
                else:
                    D.add((a & ~3 if sz == 4 else a, sz))
    return D


def alts(a, sz, cur):
    """Alternative values for one state word (explorer children)."""
    if sz == 1:
        out = [0, 1, 2, (cur + 1) & 0xFF, (cur - 1) & 0xFF, 0x7F, 0x80, 0xFF, 3, 4, 5]
    elif sz == 2:
        out = [0, 1, 2, (cur + 1) & 0xFFFF, (cur - 1) & 0xFFFF, 0x8000, 0xFFFF]
    else:
        out = [0, 1, 2, 3, (cur + 1) & 0xFFFFFFFF, (cur - 1) & 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, ZONE + 0x40]
    seen, res = {cur}, []
    for v_ in out:
        if v_ not in seen:
            seen.add(v_)
            res.append(v_)
    return res


def explore(base, entry, sp, sysc, pool, tuples, want, runs_cap):
    """Path exploration over the state words the original reads: start from random / runtime-observed base states, and for
    every state word a run reads for the first time create children that set that one word to an alternative value; keep a
    run when its access path (ordered loads, callee tokens) is new.  Returns [(seed, tuple|None, overrides)]."""
    kept, sigs, work, runs, seed = [], set(), [], 0, 0
    for i in range(24):
        work.append(((i + 1) * 31337 ^ entry, tuples[i % len(tuples)] if tuples else None, {}))
    wi = 0
    visited = set()
    while wi < len(work) and len(kept) < want and runs < runs_cap:
        s, tup, ov = work[wi]
        wi += 1
        runs += 1
        try:
            ram, args, rets, models, x = run_one(base, entry, sp, sysc, s, 'verbatim' if tup else 'discovery', tup, 0, set(), pool, ov)
        except (struct.error, IndexError):
            continue
        c.UNFAITHFUL[0] = False
        res = None
        try:
            res = m.run_case(ram, entry, args, rets, sp, sysc, models)
        except (AssertionError, struct.error, IndexError, MipsBreak):
            pass
        finally:
            reset_models()
        loads = list(res[1]) if res else list(pl.LOADS)       # a dropped / spun run still shows which words it read: expand it
        if res and not c.UNFAITHFUL[0]:
            ret, _l, stores, calls = res
            sig = hash((tuple((a, sz) for a, sz, _v in loads if a < 0x801FE000 or a >= 0x80200000), tuple(tuple(r_) for r_, _n in calls), ret))
            if sig not in sigs:
                sigs.add(sig)
                kept.append((s, tup, dict(ov)))
        first = []
        for a, sz, val in loads:
            if 0x80090000 <= a < 0x80102000 and (a & ~3 if sz == 4 else a, sz) not in visited and a not in SENT:
                key = (a & ~3 if sz == 4 else a, sz)
                visited.add(key)
                first.append((key, val))
        for key, val in first:
            for av in alts(key[0], key[1], val):
                nov = dict(ov)
                nov[key] = av
                work.append((s, tup, nov))
    return kept, runs


def gen_entry(base, entry, n, tuples, tuple_cap, stats, pool):
    """Cases for one entry.  With runtime tuples: every tuple verbatim, half of the rest perturbed, the remainder discovery;
    without tuples (the runtime never called the entry): discovery on runtime-observed base states (`pool`)."""
    sp = m.SPEC[entry]
    sysc = m.syscall_pcs(base, entry)
    c._check_callees_complete(base, entry, tuple(sp['callees']) + tuple(sp.get('inline', ())), inline=tuple(sp.get('inline', ())))
    for cb_ in STOP_EXTRA:
        sp['callees'].setdefault(cb_, 2)
    for t in tuples:
        # real callback targets seen at runtime are stopped like the sentinels (arity 2, results scripted)
        for tok in t['calls']:
            f = tok.split(',')
            if f[0] in ('G', 'D') and len(f) > 1 and int(f[1], 16) not in sp['callees']:
                sp['callees'][int(f[1], 16)] = 2
    tuples = tuples[:tuple_cap]
    D = set() if VERBATIM_ONLY[0] else collect_D(base, entry, sp, sysc, tuples) if tuples else collect_D_pool(base, entry, sp, sysc, pool)
    n_ver = len(tuples)
    n_pert = 0 if VERBATIM_ONLY[0] else (n - n_ver) // 2 if tuples else 0
    if VERBATIM_ONLY[0]:
        n = n_ver
    plan = [dict(kind='verbatim', tup=t, ov=None, s=None) for t in tuples]
    plan += [dict(kind='perturbed', tup=tuples[i % len(tuples)], ov=None, s=None) for i in range(n_pert)]
    want_disc = n - len(plan)
    kept, runs = ([], 0) if VERBATIM_ONLY[0] else explore(base, entry, sp, sysc, pool, tuples, want_disc, EXPLORE_RUNS.get(entry, 600))
    explored = len(kept)
    plan += [dict(kind='discovery', tup=t, ov=ov, s=s, explored=True) for s, t, ov in kept[:want_disc]]
    plan += [dict(kind='discovery', tup=None, ov=None, s=None)] * max(0, n - len(plan))
    out, classes, spun, dropped, seed = [], [], 0, 0, 0
    for item in plan:
        kind, tup = item['kind'], item['tup']
        for attempt in range(8):                 # a dropped / spun case is replaced by the next seed of the same class
            seed += 1
            if item.get('explored'):
                s, xe, ov = item['s'], 0, item['ov']
                if attempt:
                    break
            else:
                s, xe, ov = seed * 7919 ^ entry, seed, None
            try:
                ram, args, rets, models, x = run_one(base, entry, sp, sysc, s, kind if not item.get('explored') or not tup else 'verbatim', tup, xe, D, tuples or pool, ov)
            except (struct.error, IndexError):
                dropped += 1
                continue
            wait = WAIT[0]
            c.UNFAITHFUL[0] = False
            try:
                res = m.run_case(ram, entry, args, rets, sp, sysc, models)
            except (struct.error, IndexError, MipsBreak):
                dropped += 1
                continue
            except AssertionError as e:
                if 'instruction budget' not in str(e):
                    raise
                spun += 1
                continue
            finally:
                reset_models()
            if c.UNFAITHFUL[0]:
                dropped += 1
                continue
            ret, loads, stores, calls = res
            text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % mm for rec, mm in calls) or '-'
            order = tuple(dict.fromkeys(m.canon(k) for k in sp['callees'])) + tuple(k for k in rets if k not in {m.canon(q) for q in sp['callees']})
            out.append(gv.line(entry, s, 0, args, (ret, loads, stores)) +
                       ' RETS %s CALL %s' % (','.join('%08X=%s' % (k, m.fmt_ret(rets[k])) for k in order) or '-', text))
            classes.append(kind + ('-explore' if item.get('explored') else '') + ('-wait' if wait else ''))
            break
    stats[entry] = dict(cases=len(out), verbatim=sum(k.startswith('verbatim') for k in classes), perturbed=sum(k.startswith('perturbed') for k in classes),
                        discovery=sum(k.startswith('discovery') for k in classes), explored=sum('-explore' in k for k in classes),
                        wait=sum(k.endswith('-wait') for k in classes), spun=spun, dropped=dropped, tuples=len(tuples), D=len(D), runs=runs)
    return out, classes


STOP_EXTRA = []              # --stop A,B: extra stopped callees (arity 2) for every generated entry, in addition to the runtime-observed callback targets
HW_ON_VERBATIM = [False]     # --hw-on-verbatim: the wait model (k not-ready reads before ready) on top of the verbatim tuples' arguments and RAM state
VERBATIM_ONLY = [False]      # --verbatim-only: every given tuple once, verbatim, nothing else (the credits-leg sets)
EXPLORE_RUNS = {0x8007C564: 3000, 0x8007CEAC: 1500, 0x8007B558: 800, 0x8007B010: 400}


def collect_D_pool(base, entry, sp, sysc, pool):
    D = set()
    for i in range(120):
        s = (i + 1) * 104729 ^ entry
        try:
            ram, args, rets, models, x = run_one(base, entry, sp, sysc, s, 'discovery', None, 0, set(), pool)
        except (struct.error, IndexError):
            continue
        c.UNFAITHFUL[0] = False
        try:
            res = m.run_case(ram, entry, args, rets, sp, sysc, models)
        except (AssertionError, struct.error, IndexError, MipsBreak):
            continue
        finally:
            reset_models()
        for a, sz, _val in res[1]:
            if 0x80090000 <= a < 0x80100000 or 0x1F801000 <= a < 0x1F803000:
                D.add((a & ~3 if sz == 4 else a, sz))
    return D


def main():
    argv = sys.argv[1:]
    n, caps, only, tcap = 400, [], None, 150
    while True:
        if '-n' in argv:
            i = argv.index('-n'); n = int(argv[i + 1]); del argv[i:i + 2]
        elif '--capture' in argv:
            i = argv.index('--capture'); j = i + 1
            while j < len(argv) and not argv[j].startswith('-'):
                caps.append(argv[j]); j += 1
            del argv[i:j]
        elif '--only' in argv:
            i = argv.index('--only'); only = int(argv[i + 1], 16); del argv[i:i + 2]
        elif '--stop' in argv:
            i = argv.index('--stop')
            STOP_EXTRA.extend(int(t, 16) for t in argv[i + 1].split(',') if t)     # extra stopped callees (arity 2): real callback targets the runtime provider never called
            del argv[i:i + 2]
        elif '--hw-on-verbatim' in argv:
            argv.remove('--hw-on-verbatim'); HW_ON_VERBATIM[0] = True
        elif '--verbatim-only' in argv:
            argv.remove('--verbatim-only'); VERBATIM_ONLY[0] = True
        elif '--tuple-cap' in argv:
            i = argv.index('--tuple-cap'); tcap = int(argv[i + 1]); del argv[i:i + 2]
        else:
            break
    silent = '--silent' in argv
    if silent:
        argv.remove('--silent')
        for e in ENTRIES:
            m.SPEC[e]['silent'] = (0x80071A74, 0x80073C5C)
    outp = argv[0]
    for e in ENTRIES:
        m.SPEC[e].setdefault('budget', {0x8007CEAC: 6000, 0x8007BF44: 20000}.get(e, 30000))
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    allt = parse_capture(caps) if caps else {}
    pool = [t for e in sorted(allt) for t in allt[e]]       # runtime-observed RAM snapshots: base states for every entry
    lines, classes, stats = [], [], {}
    for e in ENTRIES:
        if only and e != only:
            continue
        o, cl = gen_entry(base, e, max(n, m.SPEC[e].get('n', 0)), allt.get(e, []), tcap, stats, pool)
        lines += o
        classes += cl
    open(outp, 'w').write('\n'.join(lines) + '\n')
    open(outp + '.classes', 'w').write('\n'.join('%d %s %08X' % (i + 1, k, int(l.split()[0], 16)) for i, (k, l) in enumerate(zip(classes, lines))) + '\n')
    for e in ENTRIES:
        if e in stats:
            st = stats[e]
            print('%08X %s cases=%d verbatim=%d perturbed=%d discovery=%d (explored=%d) wait_cases=%d tuples=%d spun=%d dropped=%d state_words=%d explorer_runs=%d' % (
                e, m.SPEC[e]['name'], st['cases'], st['verbatim'], st['perturbed'], st['discovery'], st['explored'], st['wait'],
                st['tuples'], st['spun'], st['dropped'], st['D'], st['runs']))


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Shared black-box engine for the sw-vec-gpu2 vector generators (batches
gpu2a..gpu2d).  Original code from the user's EXE (and, for libpress, the
extracted overlay ovl_039F) runs under the test-only interpreter; only its
observable behaviour is recorded (return value, ordered load/store/call
events).  No decompiled or translated source is read.

Differences from pe_gpu_rows_vectors.run():
  * the function's own address range (from configs/USA/disc1.yaml rows) is
    known, so every pc outside it is an *external call*; unknown externals
    are stubbed automatically (named func_XXXXXXXX, scripted return) and
    reported in the case's R list, so callees need not be pre-declared;
  * an optional overlay blob is mapped (libpress);
  * jump-table style indirect calls through guest function pointers use the
    same stub rule, so hooks need no special casing.
"""
import re
import struct
import sys
from pathlib import Path

import pe_gpu_rows_vectors as R

v = R.v
RET = v.RET
ROOT = v.ROOT
Rng = R.Rng
put32 = R.put32
put16 = R.put16
EVENTS = R.EVENTS
MODEL = R.MODEL
ZONE = R.ZONE
HDR = 0x8000F800                     # exe file offset 0x800 -> vram 0x80010000: vram = off + HDR


def yaml_rows(path='configs/USA/disc1.yaml'):
    rows = []
    for ln in open(ROOT / path):
        m = re.match(r'\s*- \[(0x[0-9A-Fa-f]+), (\w+)(?:, (\w+))?\]', ln)
        if m:
            rows.append((int(m.group(1), 16), m.group(2), m.group(3)))
    return rows


def func_range(name, path='configs/USA/disc1.yaml', vram_off=HDR):
    rows = yaml_rows(path)
    for i, (off, kind, nm) in enumerate(rows):
        if nm == name:
            end = rows[i + 1][0] if i + 1 < len(rows) else off + 4
            return off, end - off, off + vram_off
    # not a named row (e.g. inside an asm span): size = distance to the next row (upper bound, used only to tell
    # internal pcs from external calls)
    addr = int(name[5:], 16)
    off = addr - vram_off
    for i, (o, kind, nm) in enumerate(rows):
        nxt = rows[i + 1][0] if i + 1 < len(rows) else o + 4
        if o <= off < nxt:
            return off, nxt - off, addr
    raise KeyError(name)


class RangeStops:
    """stop_at container: any pc outside [lo,hi) (except inside other
    explicitly allowed ranges) is an external call; RET and BIOS vectors
    stop as well."""

    def __init__(self, lo, hi, known):
        self.lo, self.hi, self.known = lo, hi, known
        self.hit = None

    def __contains__(self, pc):
        if pc == RET or pc in (0xA0, 0xB0, 0xC0):
            self.hit = pc
            return True
        if pc < self.lo or pc >= self.hi:
            for klo, khi in self.known:           # callee bodies the case runs INLINE (cdgen63 / cdgen64 shapes); empty for every earlier batch
                if klo <= pc < khi:
                    return False
            self.hit = pc
            return True
        return False


def fixture(overlay=None):
    ram = bytearray(R.fixture())
    if overlay:
        data, vram = overlay
        ram[vram & 0x1FFFFF:(vram & 0x1FFFFF) + len(data)] = data
    return ram


def run(ram, entry, args, size, rets=None, model=None, budget=100000, max_calls=64, ranges=()):
    """rets: {callee address: [return values]} (default for unknown callee
    addresses: 0).  Returns (events, result, callee_rets_used dict name->list)."""
    rets = rets or {}
    EVENTS.clear()
    MODEL.update(stat=[0], chcr=[0], t1=(0, 1), n={})
    if model:
        MODEL.update(model)
    used = {}
    regs = {i + 4: a for i, a in enumerate(args)}
    regs[31] = RET
    pc = entry
    seen = {}
    for _ in range(max_calls + 1):
        st = RangeStops(entry, entry + size, ranges)
        try:
            r = R.execute(ram, pc, (), stop_at=st, initial_regs=regs, instruction_budget=budget)
        except AssertionError as e:
            return list(EVENTS), 'SPIN' if str(e) == 'instruction budget' else 'FAULT', seen
        if st.hit == RET:
            return list(EVENTS), r[2] & 0xFFFFFFFF, seen
        if st.hit in (0xA0, 0xB0, 0xC0):
            sp = r[29] & 0x1FFFFF
            stk = struct.unpack_from('<4I', ram, sp + 16)
            EVENTS.append(('C', 'BIOS_%s%02X' % ({0xA0: 'A', 0xB0: 'B', 0xC0: 'C'}[st.hit], r[9] & 0xFF),
                           r[4] & 0xFFFFFFFF, r[5] & 0xFFFFFFFF, r[6] & 0xFFFFFFFF, r[7] & 0xFFFFFFFF)
                          + (('stk', stk[0], stk[1]) if (r[9] & 0xFF) == 0x3F else ()))
            r[2] = 0
            regs = {i: x for i, x in enumerate(r)}
            pc = r[31]
            continue
        if st.hit is None or st.hit == 0:
            return list(EVENTS), 'JUMP0', seen
        name = 'func_%08X' % st.hit
        lst = rets.get(st.hit, [0])
        k = used.get(st.hit, 0)
        used[st.hit] = k + 1
        seen[name] = lst
        EVENTS.append(('C', name, r[4] & 0xFFFFFFFF, r[5] & 0xFFFFFFFF, r[6] & 0xFFFFFFFF, r[7] & 0xFFFFFFFF))
        r[2] = lst[min(k, len(lst) - 1)] & 0xFFFFFFFF
        regs = {i: x for i, x in enumerate(r)}
        pc = r[31]
    return list(EVENTS), 'CALLS', seen


def emit(entry, seed, args, seen, res, events):
    return R.emit(entry, seed, args, seen, res, events)


def cold_exe_check():
    exe = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    import hashlib
    return hashlib.sha1(exe).hexdigest()


# ---------------------------------------------------------------------------
# Case runner with leftover-register discrimination.
#
# Every case is executed twice from identical memory: once with only the
# declared arguments set, once with *garbage* in every register the function
# is not documented to take (v0/v1, unused a-regs, t0-t9).  A callee argument,
# or the return value, that differs between the two runs is a leftover
# register the original merely forwarded, not a contract: it is recorded as
# `_` and never compared.  Any other difference between the runs (events,
# stores, callee order) means the case depends on garbage and is dropped.
GARBAGE_REGS = (2, 3, 8, 9, 10, 11, 12, 13, 14, 15, 24, 25)


# ---- extended hardware model: GPUSTAT (0x1814), DMA2 CHCR (0x10A8), DMA6 CHCR (0x10E8) and GPUREAD (0x1810)
# loads come from scripted sequences (psx-spx); root counter 1 as in pe_gpu_rows_vectors; all else aliases RAM.
def _load2(fmt, buf, a):
    if struct.calcsize(fmt) == 4 and a in (0x1098, 0x1824) and ({0x1098: 'chcr1', 0x1824: 'mdec1'}[a] in MODEL):
        # cdgen63: DMA1 CHCR (0x1F801098) and MDEC1 status (0x1F801824) read from a scripted sequence when the case supplies one
        val = (R.seq_next({0x1098: 'chcr1', 0x1824: 'mdec1'}[a]),)
        if R.logged(a):
            R.EVENTS.append(('L', 0x1F800000 + a, 4, val[0] & 0xFFFFFFFF))
        return val
    if struct.calcsize(fmt) == 4 and a in (0x1810, 0x10E8):
        val = (R.seq_next({0x1810: 'gp0', 0x10E8: 'chcr6'}[a]),)
        if R.logged(a):
            R.EVENTS.append(('L', 0x1F800000 + a, 4, val[0] & 0xFFFFFFFF))
        return val
    return R._load(fmt, buf, a)


R._ns['LOAD'] = _load2
PRINTF = (0x80071A74, 0x80130200)   # the libgpu log function and the sentinel the vectors substitute for it
# Arity of guest callees whose registers beyond the arguments carry leftovers.  Source of each entry: the public
# prototype in pc_port/include/pe_plat/*.h where one exists (func_80076C34: gpu_swap.h, four arguments), else the
# behaviour observed in the oracle (an argument is real when the callee's own events consume it).
ARITY = {0x80076C34: 4, 0x80071A34: 3, 0x80073E10: 1, 0x80073CF4: 2, 0x80073C94: 0, 0x8010C0FC: 1, 0x80077294: 1, 0x80077404: 0,
         0x80075EE0: 2, 0x80074E28: 2, 0x800773D0: 0, 0x80073A44: 1, 0x8007C444: 2, 0x80076B58: 2, 0x80076B98: 2,
         0x80076354: 2, 0x80076664: 2, 0x800768A0: 2, 0x80076434: 2}


def _nconv(ram, a):
    n = 0
    i = 0
    a &= 0x1FFFFF
    while i < 256 and a + i + 1 < len(ram) and ram[a + i]:
        if ram[a + i] == 0x25:
            if ram[a + i + 1] == 0x25:
                i += 1
            else:
                n += 1
        i += 1
    return n


ARITY_EXTRA = {}     # per-spec additions (cdgen64: the sentinel guest functions standing for the table-slot target and the log vector); empty for every earlier batch


def _printf_mask(events, ram):
    out = []
    for e in events:
        if e[0] == 'C' and int(e[1][5:], 16) in ARITY_EXTRA:
            k = ARITY_EXTRA[int(e[1][5:], 16)]
            e = e[:2] + tuple(x if i < k else None for i, x in enumerate(e[2:6])) + tuple(e[6:])
        elif e[0] == 'C' and int(e[1][5:], 16) in ARITY:
            k = ARITY[int(e[1][5:], 16)]
            e = e[:2] + tuple(x if i < k else None for i, x in enumerate(e[2:6])) + tuple(e[6:])
        if e[0] == 'C' and int(e[1][5:], 16) in PRINTF and e[2] is not None:
            k = 1 + _nconv(ram, e[2])
            e = e[:2] + tuple(x if i < k else None for i, x in enumerate(e[2:6])) + tuple(e[6:])
        out.append(e)
    return out


def _run_once(ram, entry, args, size, rets, model, budget, max_calls, garbage, inline=()):
    rets = rets or {}
    EVENTS.clear()
    MODEL.pop('chcr1', None)
    MODEL.pop('mdec1', None)
    MODEL.update(stat=[0], chcr=[0], chcr6=[0], gp0=[0], t1=(0, 1), n={})
    if model:
        MODEL.update(model)
    used = {}
    regs = dict(garbage)
    regs.update({i + 4: a for i, a in enumerate(args)})
    regs[31] = RET
    pc = entry
    seen = {}
    for _ in range(max_calls + 1):
        st = RangeStops(entry, entry + size, inline)
        try:
            r = R.execute(ram, pc, (), stop_at=st, initial_regs=regs, instruction_budget=budget)
        except AssertionError as e:
            return list(EVENTS), 'SPIN' if str(e) == 'instruction budget' else 'FAULT', seen
        if st.hit == RET:
            return list(EVENTS), r[2] & 0xFFFFFFFF, seen
        if st.hit in (0xA0, 0xB0, 0xC0):
            return list(EVENTS), 'BIOS', seen
        if not st.hit:
            return list(EVENTS), 'JUMP0', seen
        name = 'func_%08X' % st.hit
        lst = rets.get(st.hit, [0])
        k = used.get(st.hit, 0)
        used[st.hit] = k + 1
        seen[name] = lst
        EVENTS.append(('C', name, r[4] & 0xFFFFFFFF, r[5] & 0xFFFFFFFF, r[6] & 0xFFFFFFFF, r[7] & 0xFFFFFFFF))
        r[2] = lst[min(k, len(lst) - 1)] & 0xFFFFFFFF
        regs = {i: x for i, x in enumerate(r)}
        pc = r[31]
    return list(EVENTS), 'CALLS', seen


def case_run(base, setup, entry, size, args, nargs, rets=None, model=None, budget=100000, max_calls=96, gseed=1, inline=()):
    """Returns (events, ret_or_None, seen) with `_` masks, or None."""
    ram1 = bytearray(base)
    setup(ram1)
    ram2 = bytearray(ram1)
    ev1, r1, s1 = _run_once(ram1, entry, args, size, rets, model, budget, max_calls, {}, inline)
    if isinstance(r1, str):
        return None
    g = Rng(gseed * 7919 + entry)
    garbage = {i: g.next() | 0x80000000 for i in GARBAGE_REGS}
    a2 = list(args[:nargs]) + [g.next() for _ in range(4 - nargs)]
    ev2, r2, s2 = _run_once(ram2, entry, a2, size, rets, model, budget, max_calls, garbage, inline)
    if isinstance(r2, str) or len(ev1) != len(ev2):
        return None
    out = []
    for e1, e2 in zip(ev1, ev2):
        if e1[0] != e2[0] or e1[1] != e2[1]:
            return None
        if e1[0] == 'C':
            if e1[0:2] != e2[0:2]:
                return None
            out.append(e1[:2] + tuple(x if x == y else None for x, y in zip(e1[2:6], e2[2:6])) + tuple(e1[6:]))
        else:
            if e1 != e2:
                return None
            out.append(e1)
    out = _printf_mask(out, ram1)
    if bytes(ram1) != bytes(ram2):
        # final RAM differs: only acceptable if no logged difference (stack frame noise)
        diff = [i for i in range(0, len(ram1)) if ram1[i] != ram2[i]]
        if any(not (0x1F0000 <= i < 0x1FF000) for i in diff[:64]) and \
                any(not (0x1F0000 <= i < 0x1FF000) for i in diff):
            return None
    return out, (r1 if r1 == r2 else None), s1


def fmt_events(events):
    out = []
    for e in events:
        if e[0] == 'C':
            s = 'C:%s:%s' % (e[1], ','.join('_' if x is None else '%X' % x for x in e[2:6]))
            if len(e) > 6:
                s += ':stk=%X,%X' % e[7:9]
            out.append(s)
        else:
            out.append('%s:%08X:%d=%X' % e)
    return ' '.join(out) or '-'


def emit2(entry, seed, args, nargs, seen, ret, events):
    rs = ';'.join('%s=%s' % (n, ','.join('%X' % x for x in l)) for n, l in sorted(seen.items())) or '-'
    a = list(args[:nargs]) + [0] * (4 - nargs)
    return '%08X %d A %s R %s RET %s EV %s' % (entry, seed, ' '.join('%X' % x for x in a), rs,
                                               '_' if ret is None else '%X' % ret, fmt_events(events))

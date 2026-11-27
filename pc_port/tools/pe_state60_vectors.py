#!/usr/bin/env python3
"""Black-box vectors for 60 small libgpu / libspu / libapi state leaves
(original code from the user's EXE under the test-only interpreter).

Batches (one generator name per family; do not reuse):
  state60a  SPU register setters / heap / DMA-callback leaves           (26)
  state60b  RCNT counters, GPU control/query words, pad-init flags,
            video-mode and callback-state words                         (17)
  state60c  GPU packet setters / link / draw-page                       (13)
  state60d  draw-environment, draw-mode and draw-offset words           (4)

Usage: pe_state60_vectors.py OUTDIR state60a|state60b|state60c|state60d [ENTRY ...]
Writes OUTDIR/<name>.txt (one vector file per function) and OUTDIR/stats.json.

Method (nothing here reads any function's C source; the leaves are executed
as original machine code from the retail EXE in the pe_cd_leaf_vectors /
pe_irq_callback_vectors interpreter, and its load/store logging is reused):
- Argument kinds come from the declaration-only headers pe_plat/state.h and
  pe_plat/gpu_primitive.h (types only): w word, m small mode/index, n count,
  s signed 16-bit (sign-extended in the register), i int (s edges plus 0x7FFFFFFF, 0x80000000,
  0x10000, 0x10100, 0xFFFF0000), u unsigned 16-bit,
  b byte, p record-zone pointer (0x80100000-0x80101FFF, never wild).
- Read discovery: each leaf is probed with seeded random arguments; every
  logged load outside the zone and the stack frame is recorded (address,
  width) and seeded in later probes, until no new address appears (the
  fixed point; the discovered set is printed and stored in stats.json).
  Addresses that hold an address in the EXE image (0x1F80xxxx or 0x80xxxxxx)
  are 'pointer globals': pattern 0 keeps the EXE value, pattern 1 redirects
  RAM pointers into the zone.
- Hardware registers (offsets 0x1000-0x1FFF of the I/O page) read plain
  memory in the interpreter, so an echo of an earlier write would be encoded
  as device behaviour.  Instead every discovered register read is served
  from a per-case sequence that is independent of writes (0-4 pre-values,
  then a repeated final value; both polarities of busy bits occur); the
  checker replays register reads in the original's order.
- External callees (any jal target / out-of-body j target that is not one
  of the 60 entries) are stopped and recorded as CALL fn,a0..a3@n (n = number
  of stores before the call) with a scripted v0 (RETS).  Callees that are
  among the 60 run as original code, so their effects appear in the caller's
  vectors.
- Pattern numbers: 0 random, 1 pointer globals redirected into the zone,
  2 argument edge sweep, 3 argument edge cross product, 4 state-word bit
  sweep (single-bit and single-hole values, one discovered word at a time),
  5 timeout-path cases (a wait loop polled thousands of times until its bound; at most 6 per function).
  A register load token is `addr:size=value@k`: k = stores (RAM and register) the original had already made.
- Cases that exhaust the instruction budget (spun) or touch memory the
  harness cannot log faithfully are dropped and counted in stats.json.

Line: ENTRY seed pattern A args RET r L addr:size=value[@k]... S addr:size=value...
RETS fn=r,... CALL fn,a0,a1,a2,a3@n ...   (`-` for an empty list)
"""
import json
import os
import re
import struct
import sys

import pe_cd_leaf_vectors as cd

gv = cd.gv
v = cd.v
pl = cd.pl
xs = gv.xs32

# The interpreter has `div` but no `divu` (SPU address/unit conversions use it).  Re-create its execute()
# from the same logging-patched source with `divu` added (a zero divisor still asserts: the original traps).
_DIVU = '''            elif fn == 27:
                n, d = r[rs], r[rt]
                assert d, ('divu by zero', hex(pc), n)
                lo, hi = (n // d) & 0xFFFFFFFF, (n % d) & 0xFFFFFFFF
'''
_anchor = '            elif fn in (32,34):'
assert _anchor in pl._src and 'fn == 27' not in pl._src
exec(compile(pl._src.replace(_anchor, _DIVU + _anchor), 'pe_state60_divu_execute', 'exec'), pl._ns)
v.execute = pl._ns['execute']

ZONE, ZSIZE = 0x80100000, 0x2000
STACK = (0x1F0000, 0x1FF000)
RAM_MASK = 0x1FFFFF

# name -> (arg kinds, return kind).  Return kinds: v void, w 32-bit, s i32,
# u16, u8.  Types are from the declaration-only headers.
SIG = {
    # state60a: SPU
    'func_8007DCAC': ('', 'v'), 'func_8007D1D4': ('m', 'w'), 'func_8007D454': ('pn', 'v'),
    'func_80085984': ('', 'v'), 'func_80085F74': ('p', 'v'), 'func_800862F4': ('mussS', 'v'),
    'func_80085C44': ('m', 'w'), 'func_80085A64': ('m', 'w'), 'func_80085EB4': ('w', 'w'),
    'func_80085B34': ('w', 'w'), 'func_80085BB4': ('w', 'w'), 'func_80085A04': ('np', 'w'),
    'func_80089EB8': ('m', 'w'), 'func_8007DD14': ('w', 'v'), 'func_80089F28': ('uu', 'v'),
    'func_80085D84': ('w', 'w'), 'func_80085DC4': ('w', 'v'), 'func_80085F14': ('m', 'w'),
    'func_80089F08': ('mp', 'v'), 'func_8007DC04': ('m', 'v'), 'func_8007DAE0': ('mww', 'v'),
    'func_8007DB24': ('mw', 'w'), 'func_8007DC84': ('', 'v'), 'func_8007DBC8': ('mm', 'w'),
    'func_8007DC5C': ('', 'v'), 'func_80085F44': ('w', 'v'),
    # state60b: counters / control / pad / video
    'func_80076B58': ('pn', 'w'), 'func_80076BE0': ('w', 'w'), 'func_8007633C': ('', 'w'),
    'func_80076B20': ('w', 'v'), 'func_80076B44': ('m', 'u8'), 'func_80073DF8': ('', 'u16'),
    'func_80085814': ('usm', 'w'), 'func_800858E8': ('m', 'w'), 'func_80085918': ('m', 'w'),
    'func_800858B0': ('m', 'w'), 'func_8008594C': ('m', 'w'), 'func_80074CB8': ('', 'u8'),
    'func_8007DEA4': ('w', 'v'), 'func_80074A14': ('w', 'w'), 'func_80073DE8': ('', 'u16'),
    'func_80074A28': ('', 's'), 'func_8007DEB0': ('', 's'),
    # state60c: GPU packet setters
    'func_80077CB4': ('pp', 's'), 'func_80077B04': ('pm', 'v'), 'func_80077B34': ('pm', 'v'),
    'func_80077B64': ('p', 'v'), 'func_80077B84': ('p', 'v'), 'func_80077BA4': ('p', 'v'),
    'func_80077BC4': ('p', 'v'), 'func_80077BE4': ('p', 'v'), 'func_80077C04': ('p', 'v'),
    'func_80077C24': ('p', 'v'), 'func_80077C44': ('p', 'v'), 'func_80077C64': ('p', 'v'),
    'func_80077C84': ('pmmw', 'w'),
    # state60d: draw env / mode / offset
    'func_800749D8': ('piiii', 'w'), 'func_80076150': ('mmw', 'w'), 'func_800762A0': ('ii', 's'),
    'func_80075C04': ('pp', 'v'),
}
BATCH = {
    'state60a': ['func_8007DCAC', 'func_8007D1D4', 'func_8007D454', 'func_80085984', 'func_80085F74',
                 'func_800862F4', 'func_80085C44', 'func_80085A64', 'func_80085EB4', 'func_80085B34',
                 'func_80085BB4', 'func_80085A04', 'func_80089EB8', 'func_8007DD14', 'func_80089F28',
                 'func_80085D84', 'func_80085DC4', 'func_80085F14', 'func_80089F08', 'func_8007DC04',
                 'func_8007DAE0', 'func_8007DB24', 'func_8007DC84', 'func_8007DBC8', 'func_8007DC5C',
                 'func_80085F44'],
    'state60b': ['func_80076B58', 'func_80076BE0', 'func_8007633C', 'func_80076B20', 'func_80076B44',
                 'func_80073DF8', 'func_80085814', 'func_800858E8', 'func_80085918', 'func_800858B0',
                 'func_8008594C', 'func_80074CB8', 'func_8007DEA4', 'func_80074A14', 'func_80073DE8',
                 'func_80074A28', 'func_8007DEB0'],
    'state60c': ['func_80077CB4', 'func_80077B04', 'func_80077B34', 'func_80077B64', 'func_80077B84',
                 'func_80077BA4', 'func_80077BC4', 'func_80077BE4', 'func_80077C04', 'func_80077C24',
                 'func_80077C44', 'func_80077C64', 'func_80077C84'],
    'state60d': ['func_800749D8', 'func_80076150', 'func_800762A0', 'func_80075C04'],
}
ALL = {int(n[5:], 16): n for b in BATCH.values() for n in b}
assert len(ALL) == 60

E32 = (0, 1, 2, 3, 4, 7, 8, 0xF, 0x10, 0x1F, 0x20, 0x7F, 0x80, 0xFF, 0x100, 0x1FF, 0x200, 0x7FFF, 0x8000,
       0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFE, 0xFFFFFFFF)
EM = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0xF, 0x10, 0x11, 0x17, 0x18, 0x19, 0x1F, 0x20, 0x7F, 0x80,
      0xFF, 0x100, 0x101, 0x1FF, 0x200, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0xFFFFFFF0)
EN = (0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 64)
ES = (0, 1, 2, 0x7F, 0x80, 0xFF, 0x100, 0x1FF, 0x200, 0x3FF, 0x400, 0x7FFF, -1, -2, -0x80, -0x81, -0x100,
      -0x400, -0x7FFF, -0x8000)
EU = (0, 1, 2, 0xFF, 0x100, 0x1FF, 0x200, 0x3FF, 0x400, 0x7FFF, 0x8000, 0x8001, 0xFFFE, 0xFFFF)
EB = (0, 1, 2, 3, 0x7F, 0x80, 0xFF)


def edges(kind):
    if kind == 'w':
        return list(E32)
    if kind == 'm':
        return list(EM)
    if kind == 'n':
        return list(EN)
    if kind == 's':
        return [e & 0xFFFFFFFF for e in ES]
    if kind == 'S':
        return [e & 0xFFFFFFFF for e in ES]
    if kind == 'u':
        return list(EU)
    if kind == 'i':      # int parameter: signed-16 edges plus the full 32-bit extremes
        return [e & 0xFFFFFFFF for e in ES] + [0x7FFFFFFF, 0x80000000, 0x10000, 0x10100, 0xFFFF0000]
    return []


def ptr(x):
    return ZONE + (x % (ZSIZE - 0x100)) // 4 * 4


def draw_arg(kind, x):
    """One argument of the given kind from xorshift state x (returns value)."""
    if kind == 'p':
        return ptr(x >> 4)
    sel = x % 4
    e = edges(kind)
    if sel < 2:
        return e[(x >> 8) % len(e)]
    y = (x >> 8)
    if kind == 'w':
        return xs(y) if sel == 2 else y & 0xFFFF
    if kind == 'm':
        return y % 0x20 if sel == 2 else xs(y)
    if kind == 'n':
        return y % 0x30 if sel == 2 else y % 0x100
    if kind in 'sS':
        return (y & 0xFFFF) - (0x10000 if y & 0x8000 else 0) & 0xFFFFFFFF if sel == 2 else ((y & 0xFF) - 0x80) & 0xFFFFFFFF
    if kind == 'u':
        return y & 0xFFFF
    if kind == 'i':
        return xs(y) if sel == 2 else ((y & 0xFFFF) - (0x10000 if y & 0x8000 else 0)) & 0xFFFFFFFF
    return y & 0xFF


# ---------------------------------------------------------------------------
# Register read sequences (served independently of writes).
REGSEQ = {}
REGLOAD_K = []      # per I/O-page load, in order: number of logged stores already made (register protocol order)
_prev_load = pl._ns['LOAD']


def _regseq_load(fmt, buf, a):
    n = struct.calcsize(fmt)
    if 0x1000 <= a < 0x2000:
        REGLOAD_K.append(len(pl.STORES))
    m = REGSEQ.get((a, n))
    if m is not None:
        vals = m[0]
        val = vals[min(m[1], len(vals) - 1)]
        m[1] += 1
        if isinstance(val, tuple):      # SPUSTAT low bits mirror the last SPUCNT write (psx-spx), upper bits free
            val = (struct.unpack_from('<H', buf, 0x1DAA)[0] & 0x3F) | val[1]
        struct.pack_into('<' + {1: 'B', 2: 'H', 4: 'I'}[n], buf, a, val & ((1 << (8 * n)) - 1))
    return _prev_load(fmt, buf, a)


pl._ns['LOAD'] = _regseq_load


def prep(base, seed):
    ram = bytearray(base)
    x = (seed * 2654435761) & 0xFFFFFFFF or 1
    for a in range(ZONE & RAM_MASK, (ZONE & RAM_MASK) + ZSIZE, 4):
        x = xs(x)
        struct.pack_into('<I', ram, a, x)
    return ram, x


def is_ptr_word(w):
    return (w >> 16) == 0x1F80 or 0x80000000 <= w < 0x80200000


def seq_for(x, n, off=0):
    """Register read sequence: 0-4 pre-values then a repeated final value.
    SPUSTAT (0x1DAE) additionally has a 'mirror' value form (low six bits = last SPUCNT write)."""
    k = (0, 0, 1, 2, 4)[x % 5]
    mask = (1 << (8 * n)) - 1
    vals = []
    for i in range(k + 1):
        x = xs(x)
        sel = x % 6
        if off == 0x1DAE and n == 2 and (i == k or (x >> 12) & 1):
            r = (x >> 8) % 10
            if r < 3:
                vals.append(('mirror', (x >> 16) & 0xFFC0 & ~0x400 if r == 0 else 0))
                continue
            if r < 6:
                vals.append(0)
                continue
            if r < 8:
                vals.append((x >> 16) & 0xFBFF)
                continue
        vals.append((0, mask, x & mask, (x & mask) | 0x4, (x & mask) & ~0x4, 0x1000 & mask)[sel] & mask)
    return vals, x


def value_for(x, n, initial):
    x = xs(x)
    mask = (1 << (8 * n)) - 1
    sel = x % 8
    if n == 4:
        cands = (initial, initial, 0, 1, 2, 3, 0xFFFFFFFF, x)
    elif n == 2:
        cands = (initial, initial, 0, 1, 2, 0x7FFF, 0xFFFF, x)
    else:
        cands = (initial, initial, 0, 1, 2, 0x7F, 0xFF, x)
    val = cands[sel] & mask
    if sel >= 6 and x & 0x100 and n == 4:
        val = (0x1 << ((x >> 12) % 32)) & mask
    return val, x


# Words whose value is a shift amount / unit size: random 32-bit values make every shifted address wild,
# so draw them from small values (mostly 3) with an occasional wide one (a discriminator for a masked shift).
SHIFT_WORD = 0x9B424
SHIFT_VALUES = (3, 3, 3, 3, 0, 1, 2, 4, 5, 15, 16, 31, 32, 33, 0xFFFFFFFF)
# The allocation list read through the pointer global at 0x8009B464: 1-8 eight-byte entries (address word
# with flag bits 0x40000000 / 0x80000000 -- the last entry ends the list -- and a size word), placed in the
# zone.  Layout learnt from the existing spucallees vectors (pe_cd_leaf_vectors.py), which seed it the same way;
# random zone words would make the walk follow wild next-pointers.
LIST_PTR = 0x9B464
LIST_ENTRIES = (0x80085B34, 0x80085BB4, 0x80085A64)


# Registers whose reads are NOT an echo of the last write (status / free-running counters / interrupt
# status): served from per-case sequences independent of writes.  Every other register read is plain
# read-back memory (seeded initial value; a later read sees the last store).  psx-spx: GPUREAD/GPUSTAT
# 0x1F801810/14, SPUSTAT 0x1F801DAE, root counter current values 0x1F801100/10/20, I_STAT 0x1F801070,
# SIO status 0x1F801044.
VOLATILE_REGS = {0x1810, 0x1814, 0x1DAE, 0x1100, 0x1110, 0x1120, 0x1070, 0x1044}


class Seeder:
    """Discovered read set + per-case seeding."""

    def __init__(self, base, list_seed=False):
        self.list_seed = list_seed
        self.spans = []
        self.base = base
        self.ram = {}      # (offset, n) -> initial value
        self.reg = {}      # (offset, n)
        self.freq = {}     # (offset, n) -> number of probe runs that read it

    def add_loads(self, loads):
        new = False
        for key in {(a & RAM_MASK, n) for a, n, val in loads if (a >> 16) != 0x1F80}:
            self.freq[key] = self.freq.get(key, 0) + 1
        for a, n, val in loads:
            if ZONE <= a < ZONE + ZSIZE:
                continue
            if (a >> 16) == 0x1F80:
                key = (a - 0x1F800000, n)
                if key not in self.reg:
                    self.reg[key] = 0
                    new = True
            else:
                off = a & RAM_MASK
                if STACK[0] <= off < STACK[1]:
                    continue
                key = (off, n)
                if key not in self.ram:
                    self.ram[key] = struct.unpack_from('<' + {1: 'B', 2: 'H', 4: 'I'}[n], self.base, off)[0]
                    new = True
        return new

    def seed(self, ram, x, pattern, only=None, forced=None):
        REGSEQ.clear()
        for (off, n), init in sorted(self.ram.items()):
            fmt = '<' + {1: 'B', 2: 'H', 4: 'I'}[n]
            if forced is not None and (off, n) == forced[0]:
                struct.pack_into(fmt, ram, off, forced[1] & ((1 << (8 * n)) - 1))
                continue
            if n == 4 and is_ptr_word(init):
                if pattern == 1 and (init >> 16) != 0x1F80:
                    x = xs(x)
                    struct.pack_into(fmt, ram, off, ptr(x))
                continue          # pointer global keeps its EXE value
            if only is not None:
                continue
            if off == SHIFT_WORD and n == 4:
                x = xs(x)
                struct.pack_into(fmt, ram, off, SHIFT_VALUES[x % len(SHIFT_VALUES)])
                continue
            val, x = value_for(x, n, init)
            struct.pack_into(fmt, ram, off, val)
        for (off, n) in sorted(self.reg):
            if off & ~3 not in VOLATILE_REGS and off not in VOLATILE_REGS:
                if only is None and not (forced is not None and forced[0] == (off, n)):
                    val, x = value_for(x, n, 0)
                    struct.pack_into('<' + {1: 'B', 2: 'H', 4: 'I'}[n], ram, off, val)
                continue
            vals, x = seq_for(xs(x), n, off)
            REGSEQ[(off, n)] = [vals, 0]
        self.spans = []
        if self.list_seed:
            x = xs(x)
            lst = 0x80101000 + (x % 0x400) // 8 * 8
            struct.pack_into('<I', ram, LIST_PTR, lst)
            cnt = 1 + (x >> 12) % 8
            for i in range(cnt):
                x = xs(x)
                start = (x >> 8) & 0xFFFFF
                x = xs(x)
                length = (x >> 16) & 0x3FFF
                flags = (0x40000000 if x & 1 else 0) | (0x80000000 if i == cnt - 1 else 0)
                struct.pack_into('<II', ram, (lst & RAM_MASK) + 8 * i, start | flags, length)
                self.spans.append((start, length))
        return x


SIZES = {}      # entry address -> body size (from the yaml rows), filled by main()


def callee_scan(base, entry, size, seen=None):
    """External callees of entry's body, including those of any of the 60 entries it calls (those run as
    original code, so their own jal targets must be stopped too)."""
    seen = seen if seen is not None else set()
    seen.add(entry)
    end, tg = entry + size, set()
    for a in range(entry, end, 4):
        w = struct.unpack_from('<I', base, a & RAM_MASK)[0]
        t = (a & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
        if w >> 26 == 3 or (w >> 26 == 2 and not entry <= t < end):
            tg.add(t)
    ext = set(t for t in tg if t not in ALL)
    for t in tg & set(ALL):
        if t not in seen:
            ext |= set(callee_scan(base, t, SIZES[t], seen))
    return sorted(ext)


# Leaves that index the SPU register block by an argument: with an out-of-range index the oracle's 21-bit
# address masking wraps the access into the RAM image (or into other device pages), which is not hardware
# behaviour the vectors can state.  Such a leaf only touches its libspu state words (0x8009B3xx-0x8009B4xx), the
# zone and the SPU register block (0x1F801C00-0x1F801DFF); any other access marks the case unfaithful and drops it.
SPU_INDEXED = {0x80089F08, 0x8007DBC8, 0x8007DAE0, 0x8007DB24}


class Ctx:
    pass


def run_case(c, ram, args, rets):
    cd.UNFAITHFUL[0] = False
    del REGLOAD_K[:]
    stops = {k: None for k in c.callees}
    try:
        res = cd.run_with_callees(ram, c.entry, args, rets, stops)
    except (struct.error, IndexError):       # a wild address outside the 2 MiB RAM image
        raise Unfaithful()
    if cd.UNFAITHFUL[0]:
        raise Unfaithful()
    if c.entry in SPU_INDEXED:
        for kind, log in ((0, res[1]), (1, res[2])):      # loads may not touch the zone (a wrapped register read)
            for a, n, val in log:
                if (a >> 16) == 0x1F80:
                    if not 0x1F801C00 <= a < 0x1F802000:      # outside the SPU register block (0x1F801C00-0x1F801FFF)
                        raise Unfaithful()
                elif not ((kind == 1 and ZONE <= a < ZONE + ZSIZE) or 0x8009B300 <= a < 0x8009B500):
                    raise Unfaithful()                         # a wild register offset wrapped into the RAM image
    # Addresses 0x1F802000.. alias into low kernel RAM in the oracle (it masks to 21 bits and prints them as
    # 0x80002000..): an out-of-range register offset cannot be represented faithfully, so the case is dropped.
    for a, n, val in list(res[1]) + list(res[2]):
        if (a >> 16) != 0x1F80 and 0x2000 <= (a & RAM_MASK) < 0x10000:
            raise Unfaithful()
    return res


class Unfaithful(Exception):
    pass


def draw_args(c, x):
    args = []
    for k in c.kinds:
        x = xs(x)
        args.append(draw_arg(k, x))
    return args, x


def draw_rets(c, x):
    rets = {}
    for cal in c.callees:
        x = xs(x)
        rets[cal] = (0, 1, 0xFFFFFFFF, xs(x))[x % 4]
    return rets, x


def fmt_line(c, s, pattern, args, res, rets):
    ret, loads, stores, calls = res
    call_text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % m for rec, blob, m in calls) or '-'
    kq = iter(list(REGLOAD_K))
    ltxt = []
    for a, n, val in loads:                  # register loads carry @k: stores made before the read
        ltxt.append('%08X:%d=%X' % (a, n, val) + ('@%d' % next(kq) if (a >> 16) == 0x1F80 else ''))
    assert next(kq, None) is None, 'register load log out of step'
    return '%08X %d %d A %s RET %08X L %s S %s RETS %s CALL %s' % (
        c.entry, s, pattern, ' '.join('%X' % a for a in args), ret & 0xFFFFFFFF,
        ' '.join(ltxt) or '-', ' '.join('%08X:%d=%X' % t for t in stores) or '-',
        ','.join('%08X=%X' % (k, rets[k]) for k in c.callees) or '-', call_text)


def gen_entry(base, entry, size, ncase=160):
    name = ALL[entry]
    kinds, rk = SIG[name]
    c = Ctx()
    c.entry, c.kinds, c.rk = entry, kinds, rk
    c.callees = callee_scan(base, entry, size)
    seeder = Seeder(base, list_seed=entry in LIST_ENTRIES)
    stat = dict(name=name, entry='%08X' % entry, kinds=kinds, callees=['%08X' % k for k in c.callees],
                spun=0, unfaithful=0, discovery_rounds=0)
    # --- read discovery to a fixed point
    for rnd in range(8):
        grew = False
        for k in range(40):
            s = 0x5000 + rnd * 977 + k
            ram, x = prep(base, s)
            x = seeder.seed(ram, x, 0)
            args, x = draw_args(c, x)
            rets, x = draw_rets(c, x)
            try:
                res = run_case(c, ram, args, rets)
            except AssertionError:
                # a spun (or wild) case still shows which registers the wait loop reads
                grew |= seeder.add_loads(list(pl.LOADS))
                continue
            except Unfaithful:
                continue
            grew |= seeder.add_loads(res[1])
        stat['discovery_rounds'] = rnd + 1
        if not grew:
            break
    stat['ram_reads'] = sorted('%08X:%d' % (0x80000000 + o, n) for (o, n) in seeder.ram)
    stat['reg_reads'] = sorted('%08X:%d' % (0x1F800000 + o, n) for (o, n) in seeder.reg)
    out = []

    def emit(s, pattern, args, forced=None, only=None, rets=None):
        ram, x = prep(base, s)
        x = seeder.seed(ram, x, 1 if pattern == 1 else 0, only=only, forced=forced)
        if rets is None:
            rets, x = draw_rets(c, x)
        if seeder.spans and entry in (0x80085B34, 0x80085BB4) and pattern in (0, 1, 4) and xs(s) % 4:
            sh = struct.unpack_from('<I', ram, SHIFT_WORD)[0]
            st, ln = seeder.spans[xs(s ^ 5) % len(seeder.spans)]
            cand = [(st >> (sh & 31)) + d for d in (-1, 0, 1)] + [((st + ln) >> (sh & 31)) + d for d in (-1, 0, 1)]
            args = [cand[xs(s ^ 9) % len(cand)] & 0xFFFFFFFF]
        try:
            res = run_case(c, ram, args, rets)
        except AssertionError as e:
            if 'instruction budget' in str(e):
                stat['spun'] += 1
            else:                                   # execution left the code (wild jump); count + remember one example
                stat['wild'] = stat.get('wild', 0) + 1
                stat.setdefault('wild_errors', {})
                k2 = re.sub(r'(0x[0-9a-f]+|[0-9]+)', 'N', str(e))[:60]
                stat['wild_errors'][k2] = stat['wild_errors'].get(k2, 0) + 1
                stat.setdefault('wild_example', dict(seed=s, pattern=pattern, args=['%X' % a for a in args], err=str(e)))
            return
        except Unfaithful:
            stat['unfaithful'] += 1
            return
        if len(res[1]) > 600:                       # timeout-path cases carry thousands of polls; keep a few
            stat['long'] = stat.get('long', 0) + 1
            if stat['long'] > 6:
                return
            pattern = 5
        out.append(fmt_line(c, s, pattern, args, res, rets))

    # --- random cases (pattern 0, and pattern 1 for a quarter)
    for k in range(ncase):
        s = 0x10000 + k * 7919 + entry
        x = xs(s | 1)
        args, x = draw_args(c, x)
        emit(s, 1 if k % 4 == 3 else 0, args)
    # --- per-argument edge sweeps (pattern 2)
    base_args_seed = 0x777
    for i, kd in enumerate(kinds):
        for j, e in enumerate(edges(kd)):
            s = 0x20000 + i * 1000 + j + entry
            x = xs(s | 1)
            args, x = draw_args(c, x)
            args[i] = e
            emit(s, 2, args)
    # --- cross product of the reduced edge lists for two-argument entries (pattern 3)
    cross = [k for k in range(len(kinds)) if kinds[k] != 'p'][:3]
    if 2 <= len(cross) <= 3 or (len(cross) == 2):
        lists = [edges(kinds[k])[::max(1, len(edges(kinds[k])) // (8 if len(cross) == 3 else 12))] for k in cross]
        import itertools
        for j, combo in enumerate(itertools.product(*lists)):
            s = 0x30000 + j + entry
            x = xs(s | 1)
            args, x = draw_args(c, x)
            for k, e in zip(cross, combo):
                args[k] = e
            emit(s, 3, args)
    # --- state-word bit sweep (pattern 4)
    j = 0
    top = sorted(seeder.ram, key=lambda k: (-seeder.freq.get(k, 0), k))[:12]
    for (off, n), init in sorted((k, seeder.ram[k]) for k in top):
        if n == 4 and is_ptr_word(init):
            continue
        bits = 8 * n
        vals = [1 << b for b in range(bits)] + [((1 << bits) - 1) ^ (1 << b) for b in range(bits)]
        for val in vals:
            s = 0x40000 + j * 131 + entry
            j += 1
            if j > 900:
                break
            x = xs(s | 1)
            args, x = draw_args(c, x)
            emit(s, 4, args, forced=((off, n), val))
    stat['cases'] = len(out)
    return out, stat


def yaml_rows():
    rows = []
    for line in open(v.ROOT / 'configs/USA/disc1.yaml'):
        m = re.match(r'\s*- \[0x([0-9A-Fa-f]+),\s*(\w+)(?:,\s*(\w+))?\]', line)
        if m:
            rows.append((int(m.group(1), 16), m.group(2), m.group(3)))
    out = {}
    for i, (o, t, n) in enumerate(rows):
        if n:
            out[n] = (o, rows[i + 1][0] - o, o + 0x8000F800)
    return out


def main():
    if len(sys.argv) < 3 or sys.argv[2] not in BATCH:
        raise SystemExit(__doc__)
    outdir = sys.argv[1]
    os.makedirs(outdir, exist_ok=True)
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    rows = yaml_rows()
    for n, (o_, sz_, va_) in rows.items():
        if va_ in ALL:
            SIZES[va_] = sz_
    want = [int(a, 16) for a in sys.argv[3:]]
    stats = {}
    for name in BATCH[sys.argv[2]]:
        off, size, vram = rows[name]
        if want and vram not in want:
            continue
        out, stat = gen_entry(base, vram, size)
        stat.update(yaml_offset='0x%X' % off, yaml_size='0x%X' % size, batch=sys.argv[2])
        # keep each vector file under 1.9 MB: drop bit-sweep, then cross-product, then random cases from the end
        limit = 1900000
        for pat in (' 4 A ', ' 3 A ', ' 0 A '):
            while sum(len(l) + 1 for l in out) > limit and len(out) > 64:
                idx = [i for i, l in enumerate(out) if pat in l[:40]]
                if not idx:
                    break
                del out[idx[-1]]
                stat['trimmed'] = stat.get('trimmed', 0) + 1
        stat['cases'] = len(out)
        with open(os.path.join(outdir, name + '.txt'), 'w') as f:
            f.write('\n'.join(out) + '\n')
        stats[name] = stat
        print(name, stat['cases'], 'spun', stat['spun'], 'wild', stat.get('wild', 0), 'long', stat.get('long', 0), 'unfaithful', stat['unfaithful'], 'ram', len(stat['ram_reads']),
              'reg', len(stat['reg_reads']), 'callees', ' '.join(stat['callees']), flush=True)
    sp = os.path.join(outdir, 'stats.json')
    old = json.load(open(sp)) if os.path.exists(sp) else {}
    old.update(stats)
    json.dump(old, open(sp, 'w'), indent=1, sort_keys=True)


if __name__ == '__main__':
    main()

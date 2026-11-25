#!/usr/bin/env python3
"""Black-box vectors for small libpad internals (original code from the
user's EXE under the test-only interpreter).

Each case seeds memory the same way as pe_pad_info_vectors.py (records and
table zone from a xorshift32 seed and fill pattern). It also seeds the SIO
register block (0x1F801040-0x1F80105F) and the pad work words at
0x8009B700-0x8009B800 from the same seed. It then calls one entry with seeded
arguments.

Each line records (stack traffic below the interpreter's initial sp
0x801FF000, down to 0x801F0000, is an implementation detail and is not
logged):
- the return value;
- every data load in order (address, size, value);
- every store in order (address, size, value).

Argument models: 0x80084B20 takes a port number (a0 drawn from small values
and full 32-bit values). 0x80084AE8 additionally sweeps a0 over every word
address in 0x800A5A00-0x800A5FFF. For every other entry, a0 is a record pointer (one
of the two pad records or an aligned pointer into the zone) and a1-a3 are
mixed-magnitude integers.

Pattern 5 (busy hook 0x80084F8C only): after the pattern-1 fill, the record's
halfword +0xE6 is set from (0, 1, 0x0102, 0x0300, 0x7F00, 0x8000) and its
byte +0x46 is swept over 0x00-0xFF. This separates the byte rule from the
halfword's 0xFF bytes.

Pattern 6 (entries in SWEEP): after the pattern-1 fill, one record byte from
the entry's list is swept over 0x00-0xFF. For 0x80083790, the record word
+0xEC is also set to a full 32-bit value from the seed.

Pattern 7 (discriminators, batch2): 0x8008389C with +0x46 = 4, +0x48 from
(0, 1, 0x80, 0xFF) and +0x47 over 0x00-0xFF; 0x800835C0 with +0x46 from
(2, 3, 4) and both argument bytes (+0x47 and +0xE4) set to each value
0x00-0xFF; 0x80083790 over every (+0xE3, +0xE9) pair with a seeded +0xEC word.

Pattern 6 for the SWEEP_PTR entries uses the pattern-0 fill, so record
words are zone pointers.

Batch 3 pattern 7, on the pattern-0 fill:
- 0x80083644: completion state (+0x46 = 4, +0x47 = +0xEA - 1) for +0xEA in
  (1, 2, 5) x +0xE3/+0xE9 in (0, 1, 7, 0x80, 0xFF), with 12 seeded +0xEC
  words, bit 31 alternating;
- 0x800837C8: a non-busy record (+0x46 = 0xFF, +0xE6 = 0x0102) with act
  info empty (+0x04 = 0), +0xE3/+0xE9 on a step-5 grid, and a nonzero
  zone-pointer argument.

Batch 3 pattern 7 also covers:
- 0x80083644 state-4 edges: +0x47 on a step-3 grid over 0x00-0xFF, +0xEA
  from (0, 1, 2, 0x7F, 0x80, 0xFE, 0xFF) and byte 4 of the info block at
  word[+0x3C] from (0, 3, 0x7C, 0xFD, 0xFF);
- 0x8008284C: ports 0/0x10 with term and offs from signed edge values.

Batch 3 cases touching memory the harness cannot log faithfully are dropped
(see STACK3).

Usage: pe_pad_leaf_vectors.py OUT.txt [ENTRY ... | batch2 | batch3]
"""
import struct
import sys

import pe_irq_callback_vectors as v
import pe_pad_info_vectors as pi

ENTRIES = (0x80084B20, 0x80084F8C, 0x800847A0, 0x80083E70, 0x80083EE4,
           0x80083C20, 0x80083E50, 0x80083E84, 0x80083EA4, 0x80083EC4,
           0x80084AE8)
# Second batch: record state dispatchers, act size, record init, sequencer.
ENTRIES2 = (0x800835C0, 0x8008389C, 0x80083790, 0x80083D9C, 0x80084644,
            0x80084B44)
# Third batch: act-info receive state, act-data walk, PadInfoComb, act load.
ENTRIES3 = (0x80083644, 0x80083C3C, 0x8008284C, 0x800837C8)
STACK = (0x1F0000, 0x1FF000)
# Batch 3: only cases whose every data access is faithfully logged are kept.
# Accesses below 0x10000 (kernel RAM, which the interpreter also aliases to
# MMIO) or in 0x1F0000-0x1FE000 (outside real stack frames) mark a case
# unverifiable, and it is dropped. The batch also logs with the narrower
# frame range STACK3.
STACK3 = (0x1FE000, 0x1FF000)
UNFAITHFUL = [False]
# Pattern 6: record bytes swept over 0x00-0xFF on a pattern-1 fill.
SWEEP = {0x800835C0: (0x46, 0x47, 0xE4), 0x8008389C: (0x46, 0x47, 0x48),
         0x80083D9C: (0x46,), 0x80084644: (0x49,), 0x80083790: (0xE3, 0xE9),
         0x80083644: (0x46, 0x47, 0xE3, 0xE9), 0x80083C3C: (0x46, 0xE9),
         0x800837C8: (0x04, 0x07)}
# Entries whose sweep fill is pattern 0 (record words are zone pointers).
SWEEP_PTR = (0x80083644, 0x80083C3C, 0x800837C8)
REC = (0x800A5B70, 0x800A5C60)
LOADS = []
STORES = []
PORT_ARG = (0x80084B20, 0x8008284C)
CUR = [0]
ADDR_ARG = (0x80084AE8,)


def _logged(a):
    if CUR[0] in ENTRIES3:
        if a < 0x10000 or 0x1F0000 <= a < STACK3[0]:
            UNFAITHFUL[0] = True
        return a >= 0x1000 and not v.JMPBUF[0] <= a < v.JMPBUF[1] and \
            not STACK3[0] <= a < STACK3[1]
    return a >= 0x1000 and not v.JMPBUF[0] <= a < v.JMPBUF[1] and \
        not STACK[0] <= a < STACK[1]


def _load(fmt, buf, a):
    val = struct.unpack_from(fmt, buf, a)
    if _logged(a):
        base = 0x1F800000 if a < 0x2000 else 0x80000000
        LOADS.append((base + a, struct.calcsize(fmt), val[0] & 0xFFFFFFFF))
    return val


_src = v.inspect.getsource(v._hud.execute).replace('struct.pack_into(', 'STORE(')
_src = _src.replace('struct.unpack_from({32:', 'LOAD({32:')
_src = _src.replace("struct.unpack_from('<I', memory, a & ~3)", "LOAD('<I', memory, a & ~3)")


def _store(fmt, buf, a, val):
    if _logged(a):
        base = 0x1F800000 if a < 0x2000 else 0x80000000
        n = struct.calcsize(fmt)
        STORES.append((base + a, n, val & ((1 << (8 * n)) - 1)))
    v._store(fmt, buf, a, val)


_ns = dict(vars(v._hud))
_ns['STORE'] = _store
_ns['LOAD'] = _load
exec(compile(_src, v._hud.__file__, 'exec'), _ns)
v.execute = _ns['execute']


def prep(base, seed, pattern):
    ram = bytearray(base)
    struct.pack_into('<I', ram, 0x9B738, 0x80084B20)
    struct.pack_into('<I', ram, 0x9B740, 0x80084F8C)
    pi.fill(ram, seed, pattern)
    x = seed * 2654435761 & 0xFFFFFFFF or 1
    for a in list(range(0x1040, 0x1060, 2)) + list(range(0x9B700, 0x9B738, 2)) + \
            list(range(0x9B744, 0x9B800, 2)):
        x = pi.xs32(x)
        struct.pack_into('<H', ram, a, x & 0xFFFF)
    return ram, x


def num(x):
    k = x % 4
    return (x >> 8) & (0xF, 0xFF, 0xFFFF, 0xFFFFFFFF)[k]


def args_for(entry, x):
    x = pi.xs32(x)
    if entry in PORT_ARG:
        a0 = (0, 1, 0x10, 0x11)[x % 4] if x & 0x10000 else num(x)
    elif entry in ADDR_ARG and x & 0x20000:
        a0 = 0x800A5A00 + ((x >> 2) % 0x180) * 4
    elif x & 1:
        a0 = REC[(x >> 1) & 1]
    else:
        a0 = pi.ZONE_LO + ((x >> 2) % 0xE00 & ~3)
    out = [a0]
    for _ in range(3):
        x = pi.xs32(x)
        out.append(num(x))
    return out


def run(ram, entry, args):
    v.EVENTS.clear()
    v.RAM_WRITES.clear()
    LOADS.clear()
    STORES.clear()
    CUR[0] = entry
    UNFAITHFUL[0] = False
    v.call(ram, v.Bios(), entry, args)
    if UNFAITHFUL[0]:
        raise struct.error('unverifiable access (batch 3)')
    ret = [e[1] for e in v.EVENTS if e[0] == 'ret'][-1]
    return ret, list(LOADS), list(STORES)


def main():
    entries = ENTRIES3 if sys.argv[2:] == ['batch3'] else \
        ENTRIES2 if sys.argv[2:] == ['batch2'] else \
        [int(a, 16) for a in sys.argv[2:]] or ENTRIES
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out = []
    faults = 0
    for entry in entries:
        for seed in range(1, 201):
            for pattern in (0, 1, 2, 3, 4):
                s = seed * 7919 + pattern
                ram, x = prep(base, s, pattern)
                args = args_for(entry, x)
                try:
                    ret, loads, stores = run(ram, entry, args)
                except (AssertionError, struct.error):
                    faults += 1
                    continue
                out.append('%08X %d %d A %s RET %08X L %s S %s' % (
                    entry, s, pattern, ' '.join('%X' % a for a in args), ret,
                    ' '.join('%08X:%d=%X' % l for l in loads) or '-',
                    ' '.join('%08X:%d=%X' % t for t in stores) or '-'))
        if entry == 0x80084F8C:
            for h in (0, 1, 0x0102, 0x0300, 0x7F00, 0x8000):
                for b in range(256):
                    rec = REC[b & 1]
                    s = 0x50000 + h + b
                    ram, x = prep(base, s, 1)
                    struct.pack_into('<H', ram, (rec + 0xE6) & 0x1FFFFF, h)
                    ram[(rec + 0x46) & 0x1FFFFF] = b
                    args = [rec] + args_for(entry, x)[1:]
                    ret, loads, stores = run(ram, entry, args)
                    out.append('%08X %d %d A %s RET %08X L %s S %s' % (
                        entry, s, 5, ' '.join('%X' % a for a in args), ret,
                        ' '.join('%08X:%d=%X' % l for l in loads) or '-',
                        ' '.join('%08X:%d=%X' % t for t in stores) or '-'))
        for off in SWEEP.get(entry, ()):
            for b in range(256):
                rec = REC[b & 1]
                s = 0x60000 + off * 0x100 + b
                ram, x = prep(base, s, 0 if entry in SWEEP_PTR else 1)
                ram[(rec + off) & 0x1FFFFF] = b
                if entry == 0x80083790:
                    x = pi.xs32(x)
                    struct.pack_into('<I', ram, (rec + 0xEC) & 0x1FFFFF, x)
                args = [rec] + args_for(entry, x)[1:]
                try:
                    ret, loads, stores = run(ram, entry, args)
                except (AssertionError, struct.error):
                    faults += 1
                    continue
                out.append('%08X %d %d A %s RET %08X L %s S %s' % (
                    entry, s, 6, ' '.join('%X' % a for a in args), ret,
                    ' '.join('%08X:%d=%X' % l for l in loads) or '-',
                    ' '.join('%08X:%d=%X' % t for t in stores) or '-'))
        grid = []
        if entry == 0x8008389C:
            grid = [{0x46: 4, 0x48: c, 0x47: b} for c in (0, 1, 0x80, 0xFF) for b in range(256)]
        elif entry == 0x800835C0:
            grid = [{0x46: st, 0x47: b, 0xE4: b} for st in (2, 3, 4) for b in range(256)]
        elif entry == 0x80083790:
            grid = [{0xE3: e3, 0xE9: e9} for e3 in range(256) for e9 in range(256)]
        elif entry == 0x80083644:
            grid = [{0x46: 4, 0xEA: n, 0x47: (n - 1) & 0xFF, 0xE3: e3, 0xE9: e9, 'EC': k}
                    for n in (1, 2, 5) for e3 in (0, 1, 7, 0x80, 0xFF) for e9 in (0, 1, 3, 0x80, 0xFF)
                    for k in range(12)]
        elif entry == 0x800837C8:
            grid = [{0x46: 0xFF, 'E6': 0x0102, 'P4': 0, 0xE3: e3, 0xE9: e9}
                    for e3 in range(0, 256, 5) for e9 in range(0, 256, 5)]
        if entry == 0x80083644:   # state 4 edges: step byte, count, entry size
            grid += [{0x46: 4, 0x47: i, 0xEA: n, 'Q4': b}
                     for n in (0, 1, 2, 0x7F, 0x80, 0xFE, 0xFF) for i in range(0, 256, 3)
                     for b in (0, 3, 0x7C, 0xFD, 0xFF)]
        if entry == 0x8008284C:   # signed edges for term / offs
            edges = (0, 1, 2, 3, 0x7F, 0x80, 0xFF, 0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFF80, 0x80000000, 0x7FFFFFFF)
            grid += [{'ARGS': (port, t, o)} for port in (0, 0x10) for t in edges for o in edges]
        for i, sets in enumerate(grid):
            rec = REC[i & 1]
            s = 0x70000 + i
            ram, x = prep(base, s, 0 if entry in SWEEP_PTR else 1)
            for off, b in sets.items():
                if off == 'EC':   # +0xEC word: seeded, bit 31 alternating
                    x = pi.xs32(x)
                    w = (x & 0x7FFFFFFF) | (0x80000000 if b & 1 else 0)
                    struct.pack_into('<I', ram, (rec + 0xEC) & 0x1FFFFF, w)
                elif off == 'E6':
                    struct.pack_into('<H', ram, (rec + 0xE6) & 0x1FFFFF, b)
                elif off == 'P4':
                    struct.pack_into('<I', ram, (rec + 4) & 0x1FFFFF, b)
                elif off == 'Q4':   # byte 4 of the info block at word[+0x3C]
                    q = struct.unpack_from('<I', ram, (rec + 0x3C) & 0x1FFFFF)[0]
                    ram[(q + 4) & 0x1FFFFF] = b
                elif off == 'ARGS':
                    pass
                else:
                    ram[(rec + off) & 0x1FFFFF] = b
            if entry == 0x80083790:
                x = pi.xs32(x)
                struct.pack_into('<I', ram, (rec + 0xEC) & 0x1FFFFF, x)
            args = [rec] + args_for(entry, x)[1:]
            if 'ARGS' in sets:
                args = list(sets['ARGS']) + [args[3]]
            if entry == 0x800837C8:   # nonzero act-data argument
                args[1] = pi.ZONE_LO + 0x800 + (i % 0x100) * 4 + 1
            try:
                ret, loads, stores = run(ram, entry, args)
            except (AssertionError, struct.error):
                faults += 1
                continue
            out.append('%08X %d %d A %s RET %08X L %s S %s' % (
                entry, s, 7, ' '.join('%X' % a for a in args), ret,
                ' '.join('%08X:%d=%X' % l for l in loads) or '-',
                ' '.join('%08X:%d=%X' % t for t in stores) or '-'))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'faults', faults)


if __name__ == '__main__':
    main()

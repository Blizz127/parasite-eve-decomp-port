#!/usr/bin/env python3
"""Black-box vectors for libgte leaves that run GTE commands (original code
from the user's EXE under the test-only interpreter): MulMatrix0 0x800785D4,
MulRotMatrix 0x800786E4, CompMatrix 0x800787D4, ApplyMatrix 0x80078BE4 and
ApplyMatrixSV 0x80078C34. All use only command 0x486012, which
pe_gte_cmd_diff.py/.c shows the host GTE model and the interpreter agree
on.

Each case:
- starts from a canonical GTE state (pe_gte_cmd_diff.state: every register
  reads back unchanged on hardware);
- fills a matrix zone (0x80100000-0x80101FFF), a vector zone
  (0x80102000-0x80102FFF) and an output zone (0x80103000-0x80103FFF) with
  seeded words, partly drawn from edge values;
- calls the entry with zone pointers. About one case in eight makes the
  output alias the first matrix argument, and one in eight the second.
  Pattern 1 (batch 1, 2700 cases per entry): partial overlaps - the output
  shifted -16..+16 bytes from either input, or the two inputs overlapping.
  (Added after an in-context replay found CompMatrix(m0, m1, m0 + 4)
  diverging; exact aliasing alone had not exposed the read/store order.)

Each line: ENTRY seed pattern A args RET r C0 .. D0 .. C1 .. D1 .. F f
L loads S stores, with C/D the 32 control / data registers before and after
(raw, as the interpreter holds them; C0/D0 are canonical) and F whether the
interpreter's FLAG is valid. Stack-frame traffic is not logged.

Faithful-only: a case is dropped when the original moves a 16-bit GTE
register out (mfc2 / swc2 / cfc2) while the interpreter holds a
non-canonical raw value in it (hardware would sign- or zero-extend; the
interpreter would hand over the raw word), or touches memory the harness
does not log; the count is printed. When a trapping add/sub overflows
(CompMatrix's translation sums), hardware raises an overflow exception: the
case is recorded as RET TRAP with the stores made before the trap (C1/D1
repeat C0/D0 and are not meaningful). The port models the exception as a
stop request (PE_PORT_STOP_UNRESOLVED_BOUNDARY), as the retail-graph tests
DAY1_transition_view / DAY1_object_render expect.

Batch 2 (`batch2`): LoadAverageShort12 0x800783E4, LoadAverageCol
0x80078554, OuterProduct0 0x800791D0, RotTransPers 0x80079244,
RotTransPers3 0x80079274, RotTrans 0x800792D4, RotAverageNclip3 0x80079384
and RotAverageNclip4 0x80079414. They use commands 0x198003D, 0x1A8003E,
0x190003D, 0x1A0003E, 0x170000C, 0x180001, 0x280030, 0x480012, 0x1400006,
0x158002D and 0x168002E, all of which pass pe_gte_cmd_diff. Arguments
beyond the fourth are passed on the stack (16(sp) onwards), as the
original expects. Kinds: v = input vector pointer, o = output pointer (each
output gets its own pointer into the output zone), i = integer (seeded
small/edge values). The harness's own write of the stack arguments is not
logged; the original's loads of them appear in L (at 0x801FF010 on).

Batch 2 pattern 1 (2700 cases per entry): one output pointer moved onto
an input or another output, shifted -16..+16 bytes.

Read-after-write: a load the original made after storing to any of its
bytes carries a trailing ^ in L; the checker does not pre-seed those bytes
(the code under test must have written them itself before reading).

Batch 3 (`batch3`): SetFogNearFar 0x80077E64 (near, far, h), SquareRoot0
0x80078004 (value) and ratan2 0x80079FB4 (y, x): integer arguments from
small, edge and full 32-bit values, plus sweeps of small values for the
division paths. The compiler guards their divisions with `break 7`
(divide by zero) and `break 6` (INT_MIN / -1); both are exceptions on the
console, recorded as RET TRAP like the trapping add.
Batch 3 pattern 3 (discriminators): SetFogNearFar with far - near from 90
to 110 at several near values and h in (1, 768, -1); ratan2 with both
magnitudes from 2^21 to 2^31 - 1, including near-equal pairs and all four
sign combinations, plus smaller magnitudes 1193040 .. 2^21 + 64 (every
value within 64 of 2^21) to place the switch between its index rules. (An INT_MIN / -1 fog division needs near * far to land
in a window narrower than the step between neighbouring near values; no
pair was found, so that guard is not exercised.)

Batch 4 (`batch4`): VectorNormal 0x80078134 (input vector pointer, output
pointer). Its helper uses 0x190003D and SQR 0xA00428, both of which pass
pe_gte_cmd_diff. Pattern 4 sets the input words from component edges
(0, +-1, small, +-2^15, +-2^24, INT_MIN, INT_MAX and their mixes,
including the zero vector).

ApplyMatrixLV 0x80078934 (`batch5`, args matrix / long vector / out):
random cases, every VEDGE triple written as the long vector (pattern 5),
and partial overlaps (pattern 1). Its MVMVA words 0x41E012 / 0x49E012 run
on the interpreter's psx-spx MVMVA, cross-checked against the host GTE
model with pe_gte_cmd_diff (0x41E012 added 2026-10-31).

Usage: pe_gte_cmd_vectors.py OUT.txt [ENTRY ... | batch2 | batch3 | batch4 | batch5]
"""
import struct
import sys

import pe_gte_cmd_diff as cd
import pe_pad_leaf_vectors as pl

v = pl.v
_hud = v._hud
ENTRIES = (0x800785D4, 0x800786E4, 0x800787D4, 0x80078BE4, 0x80078C34)
ARGS = {0x800785D4: 'mmo', 0x800786E4: 'm', 0x800787D4: 'mmo',
        0x80078BE4: 'mvo', 0x80078C34: 'mvo'}
ENTRIES2 = (0x800783E4, 0x80078554, 0x800791D0, 0x80079244, 0x80079274,
            0x800792D4, 0x80079384, 0x80079414)
ARGS.update({0x800783E4: 'vviio', 0x80078554: 'vviio', 0x800791D0: 'vvo',
             0x80079244: 'vooo', 0x80079274: 'vvvooooo', 0x800792D4: 'voo',
             0x80079384: 'vvvoooooo', 0x80079414: 'vvvvooooooo'})
ENTRIES3 = (0x80077E64, 0x80078004, 0x80079FB4)
ENTRIES4 = (0x80078134,)
ENTRIES5 = (0x80078934,)     # ApplyMatrixLV: MVMVA 0x41E012 / 0x49E012
ARGS.update({0x80078934: 'mvo'})
ARGS.update({0x80078134: 'vo'})
VEDGE = (0, 1, 0xFFFFFFFF, 7, 0xFFFFFFF9, 0x1000, 0x7FFF, 0xFFFF8000, 0x8000,
         0x01000000, 0xFF000000, 0x80000000, 0x7FFFFFFF, 0x00123456, 0xFFEDCBAA)
ARGS.update({0x80077E64: 'iii', 0x80078004: 'i', 0x80079FB4: 'ii'})
MZONE, VZONE, OZONE = 0x80100000, 0x80102000, 0x80103000
D16 = {1, 3, 5, 7, 8, 9, 10, 11, 16, 17, 18, 19}
C16 = {4, 12, 20, 26, 27, 29, 30}
UNFAITHFUL = [False]
FRAME = (0x1FE000, 0x1FF000)


def _mv_out(kind, rd, regs):
    """Interpreter moves a GTE register out: flag a non-canonical raw value."""
    val = regs[rd] & 0xFFFFFFFF
    if kind == 'd' and rd in D16:
        canon = cd.sx16(val) if rd in (1, 3, 5, 8, 9, 10, 11) else val & 0xFFFF
        if canon != val:
            UNFAITHFUL[0] = True
    if kind == 'c' and rd in C16 - {4, 12, 20} and cd.sx16(val) != val:
        UNFAITHFUL[0] = True


_logged = pl._logged


def _faithful_logged(a):
    r = _logged(a)
    if not r and not FRAME[0] <= a < FRAME[1]:
        UNFAITHFUL[0] = True
    return r


pl._logged = _faithful_logged

_src = v.inspect.getsource(_hud.execute)
for old, new in (
        ('struct.pack_into(', 'STORE('),
        ('struct.unpack_from({32:', 'LOAD({32:'),
        ("struct.unpack_from('<I', memory, a & ~3)", "LOAD('<I', memory, a & ~3)"),
        ('final_gte.update(data=cop_data[:], control=cop_control[:])',
         'final_gte.update(data=cop_data[:], control=cop_control[:], flags_valid=flags_valid)'),
        ('if rs == 0: r[rt] = cop_data[rd]', "if rs == 0: MV_OUT('d', rd, cop_data); r[rt] = cop_data[rd]"),
        ("elif rs == 2:", "elif rs == 2:\n                MV_OUT('c', rd, cop_control)"),
        ("elif op == 58: STORE('<I',memory,a,cop_data[rt]&0xFFFFFFFF)",
         "elif op == 58: MV_OUT('d', rt, cop_data); STORE('<I',memory,a,cop_data[rt]&0xFFFFFFFF)"),
        ('elif op == 50: cop_data[rt] = struct.unpack_from', 'elif op == 50: cop_data[rt] = LOAD'),
        # argument setup (stack arguments beyond the fourth) is not the original's store
        ("STORE('<I',ram,0x1FF010+i*4", "struct.pack_into('<I',ram,0x1FF010+i*4")):
    assert old in _src, old
    _src = _src.replace(old, new)
STORED = set()
RAW = []


def _store_t(fmt, buf, a, val):
    """Store, remembering the bytes the original has written so far."""
    pl._store(fmt, buf, a, val)
    STORED.update(range(a, a + struct.calcsize(fmt)))


def _load_t(fmt, buf, a):
    """Load; marks it (RAW) when any of its bytes was stored earlier."""
    n = len(pl.LOADS)
    val = pl._load(fmt, buf, a)
    if len(pl.LOADS) > n and STORED.intersection(range(a, a + struct.calcsize(fmt))):
        RAW.append(n)
    return val


_ns = dict(vars(_hud))
_ns.update(STORE=_store_t, LOAD=_load_t, MV_OUT=_mv_out)
exec(compile(_src, _hud.__file__, 'exec'), _ns)
execute = _ns['execute']

E16 = cd.E16


def prep(base, seed):
    ram = bytearray(base)
    x = seed * 2654435761 & 0xFFFFFFFF or 1
    for a in range(MZONE & 0x1FFFFF, (OZONE & 0x1FFFFF) + 0x1000, 2):
        x = cd.xs32(x)
        val = E16[(x >> 8) % len(E16)] if x % 3 == 0 else x >> 16
        struct.pack_into('<H', ram, a, val)
    return ram, x


IEDGE = (0, 1, 0x800, 0x1000, 0x7FFF, 0x8000, 0xFFFF, 0x10000, 0x7FFFFFFF,
         0x80000000, 0xFFFFFFFF, 0xFFFFF000)


def args_for(entry, x):
    out = []
    for kind in ARGS[entry]:
        x = cd.xs32(x)
        if kind == 'i':
            out.append(IEDGE[(x >> 8) % len(IEDGE)] if x % 3 == 0 else (x >> 8) & (0xFFF, 0xFFFF)[x & 1])
        elif kind == 'm':
            out.append(MZONE + (x % 0x1FC0) // 4 * 4)
        elif kind == 'v':
            out.append(VZONE + (x % 0xFC0) // 4 * 4)
        else:
            out.append(OZONE + (x % 0xFC0) // 4 * 4)
    x = cd.xs32(x)
    if entry in ENTRIES and len(out) == 3 and x % 8 == 0:
        out[2] = out[0]
    elif entry in ENTRIES and len(out) == 3 and x % 8 == 1 and ARGS[entry][1] == 'm':
        out[2] = out[1]
    return out, x


def overlap_args(entry, x, k):
    """Pattern 1: the output partially overlaps an input (shifted by a
    multiple of 4 bytes, both directions), or the two inputs overlap."""
    args, x = args_for(entry, x)
    shift = 4 * ((k % 9) - 4)            # -16 .. +16 bytes
    which = (k // 9) % 3
    if len(args) == 3 and which < 2:
        args[2] = args[which] + shift
    elif len(args) == 3 and ARGS[entry][1] == 'm':
        args[1] = args[0] + shift
    elif len(args) == 3:
        args[2] = args[1] + shift
    return args, x


def loads_text():
    """L list; a load made after the original stored to any of its bytes
    carries a trailing ^ (read-after-write)."""
    raw = set(RAW)
    return ' '.join('%08X:%d=%X%s' % (l + ('^' if i in raw else '',))
                    for i, l in enumerate(pl.LOADS)) or '-'


def overlap_args2(entry, x, k):
    """Batch 2 pattern 1: one output pointer moved onto an input or onto
    another output, shifted -16..+16 bytes in steps of 4."""
    args, x = args_for(entry, x)
    kinds = ARGS[entry]
    outs = [i for i, c in enumerate(kinds) if c == 'o']
    srcs = [i for i, c in enumerate(kinds) if c in 'vo']
    o = outs[k % len(outs)]
    src = [i for i in srcs if i != o][(k // len(outs)) % (len(srcs) - 1)]
    args[o] = args[src] + 4 * ((k // 7) % 9 - 4)
    return args, x


_DISC = {}


def discriminators(entry):
    if entry in _DISC:
        return _DISC[entry]
    d = []
    if entry == 0x80077E64:
        for span in range(90, 111):
            for near in (0, 1, 100, 6500, 65535, 0x7FFFFF00):
                for h in (1, 768, -1):
                    d.append((near, (near + span) & 0xFFFFFFFF, h))
    elif entry == 0x80079FB4:
        mags = [1 << 21, (1 << 21) + 1, 0x3FFFFF, 1 << 24, 0x123456, 0x7654321, 1 << 30,
                0x7FFFFFFE, 0x7FFFFFFF, 0x40000001, 0x5A5A5A5A]
        for a in mags:
            for b in mags + [a - 1, a + 1, a]:
                for sy in (1, -1):
                    for sx in (1, -1):
                        d.append((sy * a, sx * min(b, 0x7FFFFFFF)))
        # the switch between the two index formulas: dividends 1193040..2^21+64
        divs = list(range(1193040, 1 << 21, 997)) + list(range((1 << 21) - 64, (1 << 21) + 65))
        for k, a in enumerate(divs):
            for b in (a + 1, a + a // 3, 0x7FFFFFFF):
                d.append((a if k % 2 else -a, b if k % 3 else -b))
    _DISC[entry] = d
    return d


def main():
    entries = ENTRIES2 if sys.argv[2:] == ['batch2'] else ENTRIES3 if sys.argv[2:] == ['batch3'] \
        else ENTRIES4 if sys.argv[2:] == ['batch4'] else ENTRIES5 if sys.argv[2:] == ['batch5'] \
        else [int(a, 16) for a in sys.argv[2:]] or ENTRIES
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out, dropped, trapped = [], 0, 0
    for entry in entries:
        cases = [(seed * 7919, 0) for seed in range(1, 6001)]
        if entry in ENTRIES and len(ARGS[entry]) == 3:
            cases += [(0xF0000 + k, 1) for k in range(2700)]
        if entry in ENTRIES2:
            cases += [(0xF0000 + k, 1) for k in range(2700)]
        if entry in ENTRIES3:     # pattern 2: small-value sweeps for the division paths
            cases += [(0xE0000 + k, 2) for k in range(6000)]
        if entry in ENTRIES4:
            cases += [(0xC0000 + k, 4) for k in range(len(VEDGE) ** 3)]
            cases += [(0xF0000 + k, 1) for k in range(2700)]
        if entry in ENTRIES5:     # pattern 5: every VEDGE triple as the long vector; pattern 1: overlaps
            cases += [(0xB0000 + k, 5) for k in range(len(VEDGE) ** 3)]
            cases += [(0xF0000 + k, 1) for k in range(2700)]
        if entry in ENTRIES3 and entry != 0x80078004:
            cases += [(0xD0000 + k, 3) for k in range(len(discriminators(entry)))]
        for s, pattern in cases:
            ram, x = prep(base, s)
            if pattern == 5:
                args, x = args_for(entry, x)
                k = s - 0xB0000
                n = len(VEDGE)
                for i, comp in enumerate((VEDGE[k % n], VEDGE[k // n % n], VEDGE[k // n // n])):
                    struct.pack_into('<I', ram, (args[1] + 4 * i) & 0x1FFFFF, comp)
            elif pattern == 4:
                args, x = args_for(entry, x)
                k = s - 0xC0000
                n = len(VEDGE)
                for i, comp in enumerate((VEDGE[k % n], VEDGE[k // n % n], VEDGE[k // n // n])):
                    struct.pack_into('<I', ram, (args[0] + 4 * i) & 0x1FFFFF, comp)
            elif pattern == 1 and entry in ENTRIES4:
                args, x = args_for(entry, x)
                args[1] = args[0] + 4 * ((s - 0xF0000) % 9 - 4)
            elif pattern == 3:
                args, x = args_for(entry, x)
                args = [v_ & 0xFFFFFFFF for v_ in discriminators(entry)[s - 0xD0000]]
            elif pattern == 2:
                args, x = args_for(entry, x)
                k = s - 0xE0000
                small = (k % 41) - 20, (k // 41 % 41) - 20, ((k * 7) % 2001) - 1000
                args = [v_ & 0xFFFFFFFF for v_ in small[:len(args)]]
            elif pattern == 1 and entry in ENTRIES2:
                args, x = overlap_args2(entry, x, s - 0xF0000)
            elif pattern == 1:
                args, x = overlap_args(entry, x, s - 0xF0000)
            else:
                args, x = args_for(entry, x)
            c0, d0 = cd.state(s ^ entry)
            pl.LOADS.clear()
            pl.STORES.clear()
            STORED.clear()
            del RAW[:]
            pl.CUR[0] = entry
            UNFAITHFUL[0] = False
            regs = {i: a for i, a in enumerate(args[:4], 4)}
            regs[31] = v.RET
            fg = {}
            try:
                r = execute(ram, entry, tuple(args), stop_at={v.RET}, initial_regs=regs,
                            initial_cop_control=dict(enumerate(c0)),
                            initial_cop_data=dict(enumerate(d0)),
                            instruction_budget=10000, final_gte=fg)
            except AssertionError as e:
                w = e.args[0] if e.args else None
                is_break = isinstance(w, tuple) and len(w) == 2 and isinstance(w[1], str) and \
                    (int(w[1], 16) & 0xFC00003F) == 0x0000000D
                if 'overflow' not in str(e) and 'division by zero' not in str(e) and not is_break:
                    raise
                # trapping add/sub overflow: hardware raises an exception here.
                # Recorded as RET TRAP with the stores made before it.
                trapped += 1
                out.append('%08X %d %d A %s RET TRAP C0 %s D0 %s C1 %s D1 %s F 0 L %s S %s' % (
                    entry, s, pattern, ' '.join('%X' % a for a in args),
                    ' '.join('%X' % a for a in c0), ' '.join('%X' % a for a in d0),
                    ' '.join('%X' % a for a in c0), ' '.join('%X' % a for a in d0),
                    loads_text(), ' '.join('%08X:%d=%X' % t for t in pl.STORES) or '-'))
                continue
            if UNFAITHFUL[0]:
                dropped += 1
                continue
            out.append('%08X %d %d A %s RET %08X C0 %s D0 %s C1 %s D1 %s F %d L %s S %s' % (
                entry, s, pattern, ' '.join('%X' % a for a in args), r[2] & 0xFFFFFFFF,
                ' '.join('%X' % a for a in c0), ' '.join('%X' % a for a in d0),
                ' '.join('%X' % (a & 0xFFFFFFFF) for a in fg['control']),
                ' '.join('%X' % (a & 0xFFFFFFFF) for a in fg['data']),
                int(fg['flags_valid']),
                loads_text(), ' '.join('%08X:%d=%X' % t for t in pl.STORES) or '-'))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'dropped (unfaithful)', dropped, 'trap cases', trapped)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Black-box vectors for the host GTE register/command contract (the
`PE_GTE_*` family) and the TransMatrix binding, for clean-room re-derivation
(sw-vec-gte, batches gte60a / gte60b / gte60c).  Vectors only; no provider.

Oracles, in priority order (the current provider is NEVER an oracle; it was
only probed, to learn each function's interface shape):

  HW   real-console traces: JaCzekanski ps1-tests `gte-fuzz` log
       gte_valid_0xc0ffee_50.log (1100 command cases, 22 commands x 50, plus
       50 "GTE 0x40" register dumps = pure write/read-back).  Each case gives
       the 64 values written (data 0-31 then control 0-31, in that order,
       mtc2/ctc2 semantics) and the 64 values read back afterwards.
  INT  the test-only interpreter (pe_battle_hud_oracle, original code path;
       pe_gte_cmd_diff.execute) for the command words it implements; at
       generation time every such word is checked against the hardware log.
  MDL  this file's own psx-spx register / command models (Regs, mvmva, gpf,
       gpl, op); at generation time each is checked against every eligible
       hardware case and the generator aborts unless all match.
  RTL  original libgte code from the user's EXE under the interpreter
       (SetRotMatrix/SetTransMatrix/SetColorMatrix/SetLightMatrix/TransMatrix).

Line format (all batches; one case per line, tokens separated by one space):
  NAME seed pattern SRC A <args|-> RET <ret,...|-> IN <64 hex> OUT <64 hex> F <0|1> L <loads|-> S <stores|->
  IN   the 64 raw words the checker writes into the GTE, in index order
       (index 0-31 PE_GTE_WriteData, 32-63 PE_GTE_WriteCtrl), before the call
  OUT  the 64 values PE_GTE_ReadData(0..31) / PE_GTE_ReadCtrl(0..31) must
       return after the call (psx-spx read widths)
  F    1 when FLAG (OUT[63]) is pinned by the oracle
  L/S  guest-memory loads (addr:size=value, ^ = after the case's own store)
       and stores (batch gte60a TransMatrix only; accessors never store)

Usage: pe_gte60_vectors.py OUT.txt a|b|c|validate
Environment: PE_GTE_HW_LOG = path of gte_valid_0xc0ffee_50.log.
"""
import os
import re
import struct
import sys

import pe_gte_cmd_diff as cd
import pe_gte_cmd_vectors as cv

pl = cv.pl
v = cv.v
M = 0xFFFFFFFF
ROOT = v.ROOT


def hw_log_path():
    p = os.environ.get('PE_GTE_HW_LOG')
    if p:
        return p
    rel = 'tools/third_party/ps1-tests/gte-fuzz/gte_valid_0xc0ffee_50.log'
    if os.path.exists(os.path.join(str(ROOT), rel)):
        return os.path.join(str(ROOT), rel)
    raise SystemExit('set PE_GTE_HW_LOG to gte_valid_0xc0ffee_50.log (ps1-tests gte-fuzz, MIT)')


def sx16(x):
    x &= 0xFFFF
    return (x - 0x10000 if x & 0x8000 else x) & M


def s16(x):
    x &= 0xFFFF
    return x - 0x10000 if x & 0x8000 else x


def s32(x):
    x &= M
    return x - (1 << 32) if x & 0x80000000 else x


def xs32(x):
    x ^= (x << 13) & M
    x ^= x >> 17
    x ^= (x << 5) & M
    return x & M


# ------------------------------------------------------------------ hardware
def parse_hw(path):
    cases, cur, sec = [], None, None
    for line in open(path):
        m = re.match(r'-+ GTE 0x([0-9a-f]+) (\S+)', line)
        if m:
            sec = (int(m.group(1), 16), m.group(2))
            continue
        if line.startswith('Test '):
            cur = {'sec': sec, 'in': [None] * 64, 'out': [None] * 64, 'cmd': None}
            cases.append(cur)
            continue
        m = re.match(r'([<>]) r\[(\d+)\] = 0x([0-9a-f]+)', line)
        if m and cur is not None:
            cur['in' if m.group(1) == '>' else 'out'][int(m.group(2))] = int(m.group(3), 16)
            continue
        m = re.match(r'GTE 0x([0-9a-f]+) (\S+) \(sf=(\d), lm=(\d), tx=(\d), vx=(\d), mx=(\d)\)', line)
        if m and cur is not None:
            cur['cmd'] = tuple([int(m.group(1), 16), m.group(2)] + [int(m.group(i)) for i in range(3, 8)])
    for c in cases:
        assert None not in c['in'] and None not in c['out']
    return cases


def hw_word(op, sf, lm, tx, vx, mx):
    return 0x4A000000 | op | (sf << 19) | (lm << 10) | (tx << 13) | (vx << 15) | (mx << 17)


# ------------------------------------------------------------ register model
class Regs:
    """psx-spx GTE register file as mtc2/ctc2 write and mfc2/cfc2 read."""

    def __init__(s):
        s.d = [0] * 32
        s.c = [0] * 32

    def wd(s, i, val):
        val &= M
        if i in (1, 3, 5, 7, 8, 9, 10, 11, 16, 17, 18, 19):
            s.d[i] = val & 0xFFFF
        elif i == 15:                    # SXYP write pushes the screen-XY FIFO
            s.d[12], s.d[13], s.d[14] = s.d[13], s.d[14], val
        elif i == 28:                    # IRGB write sets IR1-3
            for k in range(3):
                s.d[9 + k] = ((val >> (5 * k)) & 0x1F) * 0x80
        elif i in (29, 31):              # ORGB, LZCR: read-only
            pass
        else:
            s.d[i] = val

    def wc(s, i, val):
        val &= M
        if i in (4, 12, 20, 26, 27, 29, 30):
            s.c[i] = val & 0xFFFF
        elif i == 31:
            s.c[i] = val & 0x7FFFF000
        else:
            s.c[i] = val

    def rd(s, i):
        if i in (1, 3, 5, 8, 9, 10, 11):
            return sx16(s.d[i])
        if i == 15:
            return s.d[14]
        if i in (28, 29):
            o = 0
            for k in range(3):
                c = ((s.d[9 + k] ^ 0x8000) - 0x8000) >> 7
                o |= max(0, min(31, c)) << (5 * k)
            return o
        if i == 31:
            val = s.d[30]
            bits = val ^ M if val & 0x80000000 else val
            return 32 - bits.bit_length()
        return s.d[i]

    def rc(s, i):
        if i in (4, 12, 20, 26, 27, 29, 30):
            return sx16(s.c[i])
        if i == 31:
            f = s.c[31]
            return f | (0x80000000 if f & 0x7F87E000 else 0)
        return s.c[i]

    def reads(s):
        return [s.rd(i) for i in range(32)] + [s.rc(i) for i in range(32)]

    def write_all(s, inp):
        for i in range(32):
            s.wd(i, inp[i])
        for i in range(32):
            s.wc(i, inp[32 + i])

    def load_canonical(s, rd):
        """Raw store from a 64-word canonical read vector (no write effects)."""
        s.d = list(rd[:32])
        s.c = list(rd[32:])
        s.d[9:12] = [x & 0xFFFF for x in s.d[9:12]]


def state_after_writes(inp):
    r = Regs()
    r.write_all(inp)
    return r.reads()


# --------------------------------------------------------- command models
def wrap44(x):
    return ((x + (1 << 43)) & ((1 << 44) - 1)) - (1 << 43)


def mats(c, k):
    return list(struct.unpack('<10h', struct.pack('<5I', *c[k:k + 5])))[:9]


class _Fl:
    def __init__(s):
        s.f = 0

    def acc(s, x, row):
        if x >= 1 << 43:
            s.f |= 1 << (30 - row)
        if x < -(1 << 43):
            s.f |= 1 << (27 - row)
        return wrap44(x)

    def sat(s, x, row, lm):
        lo = 0 if lm else -32768
        if x > 32767:
            s.f |= 1 << (24 - row)
            return 32767
        if x < lo:
            s.f |= 1 << (24 - row)
            return lo
        return x

    def done(s, c):
        c[31] = s.f | (0x80000000 if s.f & 0x7F87E000 else 0)


def m_mvmva(rd, sf, mx, vv, cv_, lm):
    d, c = list(rd[:32]), list(rd[32:])
    sh = 12 * sf
    fl = _Fl()
    if mx == 3:
        r = d[6] & 0xFF
        rt = mats(c, 0)
        Mx = [[-(r << 4), r << 4, s16(d[8])], [rt[2]] * 3, [rt[4]] * 3]
    else:
        m = mats(c, (0, 8, 16)[mx])
        Mx = [m[0:3], m[3:6], m[6:9]]
    if vv == 3:
        V = [s16(d[9]), s16(d[10]), s16(d[11])]
    else:
        V = [s16(d[2 * vv]), s16(d[2 * vv] >> 16), s16(d[2 * vv + 1])]
    T = [0, 0, 0] if cv_ == 3 else [s32(c[(5, 13, 21)[cv_] + i]) for i in range(3)]
    mac, ir = [0] * 3, [0] * 3
    for row in range(3):
        a = T[row] * 0x1000
        if cv_ == 2:                     # far-colour: only the flags of the first step survive
            a = fl.acc(a + Mx[row][0] * V[0], row)
            fl.sat(s32(a >> sh), row, 0)
            a = fl.acc(Mx[row][1] * V[1], row)
            a = fl.acc(a + Mx[row][2] * V[2], row)
        else:
            for j in range(3):
                a = fl.acc(a + Mx[row][j] * V[j], row)
        mac[row] = s32(a >> sh)
        ir[row] = fl.sat(mac[row], row, lm)
    for i in range(3):
        d[25 + i] = mac[i] & M
        d[9 + i] = ir[i] & M
    fl.done(c)
    return cd.canon_data(d) + cd.canon_ctrl(c)


def _color_push(d, color):
    d[20:23] = d[21:23] + [color]


def m_gpf_gpl(rd, sf, lm, gpl):
    d, c = list(rd[:32]), list(rd[32:])
    sh = 12 * sf
    fl = _Fl()
    color = d[6] & 0xFF000000
    for i in range(3):
        base = s32(d[25 + i]) * (1 << sh) if gpl else 0
        mac = s32(fl.acc(base + s16(d[8]) * s16(d[9 + i]), i) >> sh)
        d[25 + i] = mac & M
        d[9 + i] = fl.sat(mac, i, lm) & M
        if not 0 <= mac >> 4 <= 255:
            fl.f |= 1 << (21 - i)
        color |= max(0, min(255, mac >> 4)) << (8 * i)
    _color_push(d, color)
    fl.done(c)
    return cd.canon_data(d) + cd.canon_ctrl(c)


def m_op(rd, sf, lm):
    d, c = list(rd[:32]), list(rd[32:])
    sh = 12 * sf
    fl = _Fl()
    diag = [s16(c[0]), s16(c[2]), s16(c[4])]
    ir = [s16(d[9]), s16(d[10]), s16(d[11])]
    for i in range(3):
        a, b = (i + 1) % 3, (i + 2) % 3
        mac = s32(fl.acc(ir[b] * diag[a] - ir[a] * diag[b], i) >> sh)
        d[25 + i] = mac & M
        d[9 + i] = fl.sat(mac, i, lm) & M
    fl.done(c)
    return cd.canon_data(d) + cd.canon_ctrl(c)


# interpreter wrapper: (op, sf, lm) -> the SDK word the interpreter implements
INT_WORDS = {
    (0x01, 1, 0): 0x180001, (0x30, 1, 0): 0x280030, (0x3F, 1, 1): 0x118043F,
    (0x06, 0, 0): 0x1400006, (0x2D, 1, 0): 0x158002D, (0x2E, 1, 0): 0x168002E,
    (0x3D, 0, 0): 0x190003D, (0x3D, 1, 0): 0x198003D, (0x3E, 0, 0): 0x1A0003E,
    (0x3E, 1, 0): 0x1A8003E, (0x0C, 0, 0): 0x170000C, (0x0C, 1, 0): 0x178000C,
    (0x28, 0, 1): 0xA00428, (0x28, 1, 1): 0xA80428,
}


def interp(word, rd):
    ram = bytearray(0x200000)
    struct.pack_into('<III', ram, cd.CODE, 0x4A000000 | (word & 0x1FFFFFF), 0x03E00008, 0)
    fg = {}
    cd.execute(ram, 0x80000000 | cd.CODE, (), stop_at={0x80000000 | (cd.CODE + 4)},
               initial_cop_control=dict(enumerate(rd[32:])), initial_cop_data=dict(enumerate(rd[:32])),
               instruction_budget=10, final_gte=fg)
    d1 = cd.canon_data([x & M for x in fg['data']])
    c1 = cd.canon_ctrl([x & M for x in fg['control']])
    return d1 + c1, int(fg['flags_valid'])


def oracle_cmd(op, sf, lm, fields, rd):
    """(out64, flag_pinned, source) for one command from a canonical state, or None."""
    if op == 0x12:
        return m_mvmva(rd, sf, fields[2], fields[1], fields[0], lm), 1, 'MDL'
    if op in (0x3D, 0x3E):
        return m_gpf_gpl(rd, sf, lm, op == 0x3E), 1, 'MDL'
    if op == 0x0C:
        return m_op(rd, sf, lm), 1, 'MDL'
    w = INT_WORDS.get((op, sf, lm))
    if w:
        out, fv = interp(w, rd)
        return out, fv, 'INT'
    return None


# ------------------------------------------------------------- validation
def validate(cases, verbose=True):
    """Check every oracle against the hardware log; returns {name: (n, ok)}."""
    res = {}

    def rec(name, ok):
        n, g = res.get(name, (0, 0))
        res[name] = (n + 1, g + int(ok))

    z = [c for c in cases if c['sec'][0] >= 0x40]
    for c in z:                                           # write/read-back model vs hardware
        rec('Regs write/read-back (GTE 0x40 dumps)', state_after_writes(c['in']) == c['out'])
    for c in cases:
        if not c['cmd']:
            continue
        op, name, sf, lm, tx, vx, mx = c['cmd']
        rd = state_after_writes(c['in'])
        if op == 0x06:
            sf = lm = 0                                   # NCLIP ignores sf/lm
        res_o = oracle_cmd(op, sf, lm, (tx, vx, mx), rd)
        if res_o is None:
            continue
        out, fv, src = res_o
        good = all(out[i] == c['out'][i] for i in range(64) if not (i == 63 and not fv))
        rec('%s %s sf=%d lm=%d' % (src, name, sf, lm) if op not in (0x12, 0x06) else '%s %s' % (src, name), good)
    return res


def print_validation(res):
    bad = 0
    for k in sorted(res):
        n, g = res[k]
        print('VALIDATE %-45s %4d / %4d' % (k, g, n))
        bad += n - g
    return bad


# -------------------------------------------------------------- generators
E16 = (0, 1, 0xFFFF, 0x7FFF, 0x8000, 0x8001, 0x1000, 0xF000, 0x0800, 0xFFFE, 0x0FFF, 0x2000)
E32 = (0, 1, M, 0x7FFFFFFF, 0x80000000, 0x00010000, 0x7FFF0000, 0x80008000, 0x7FFF7FFF,
       0x55555555, 0xAAAAAAAA, 0xFFFF8000, 0x00007FFF, 0x00001000, 0xFFFFF000, 0x01000000)
SMALL_HALF = (0x0400, 0x0800, 0x1000, 0x0FFF, 0xF000, 0xFC00, 0x0100)


def rnd(x):
    return xs32(x)


def gen_word(x, kind):
    """kind: 'h' halves, 'w' 32-bit, 'u' 16-bit unsigned."""
    x = rnd(x)
    pick = x % 8
    x = rnd(x)
    if kind == 'h':
        lo = hi = 0
        for part in range(2):
            x = rnd(x)
            if pick < 3:
                val = (x >> 8) % 0x2001 - 0x1000
            elif pick < 5:
                val = E16[(x >> 8) % len(E16)]
            else:
                val = x >> 16
            if part == 0:
                lo = val & 0xFFFF
            else:
                hi = val & 0xFFFF
        return (hi << 16 | lo) & M, x
    if kind == 'u':
        if pick < 3:
            return (x >> 8) % 0x1001, x
        if pick < 5:
            return E16[(x >> 8) % len(E16)], x
        return x >> 16, x
    if pick < 3:
        return ((x >> 8) % 0x20001 - 0x10000) & M, x
    if pick < 5:
        return E32[(x >> 8) % len(E32)], x
    return x, x


KINDS = ['h'] * 6 + ['w'] * 2 + ['h', 'h', 'h', 'h', 'w', 'h', 'u', 'u', 'u', 'u',     # data 0-19
         'w', 'w', 'w', 'w', 'w', 'w', 'w', 'w', 'w', 'w', 'w', 'w']                    # data 20-31
KINDS = (['h'] * 6 + ['w'] + ['u'] + ['h'] * 4 + ['w'] * 3 + ['w'] + ['u'] * 4 + ['w'] * 12)[:32]
KINDS += ['h', 'h', 'h', 'h', 'h', 'w', 'w', 'w', 'h', 'h', 'h', 'h', 'h', 'w', 'w', 'w',
          'h', 'h', 'h', 'h', 'h', 'w', 'w', 'w', 'w', 'w', 'u', 'u', 'w', 'u', 'u', 'w']


def gen_inputs(seed):
    """64 raw write values (data 0-31 then control 0-31), edge-biased."""
    x = seed * 2654435761 & M or 1
    out = []
    for i in range(64):
        val, x = gen_word(x, KINDS[i])
        out.append(val)
    return out, x


def hx(vals):
    return ' '.join('%X' % (a & M) for a in vals)


def case_line(name, seed, pat, src, args, ret, inp, out, fv, loads='-', stores='-'):
    return '%s %d %d %s A %s RET %s IN %s OUT %s F %d L %s S %s' % (
        name, seed, pat, src, ' '.join('%X' % (a & M) for a in args) or '-',
        ','.join('%X' % (a & M) for a in ret) or '-', hx(inp), hx(out), fv, loads, stores)


def apply_writes(inp, writes):
    """Run extra register writes (kind, index, value) after IN; returns reads."""
    r = Regs()
    r.write_all(inp)
    for kind, i, val in writes:
        (r.wd if kind == 'd' else r.wc)(i, val)
    return r.reads()


# ---- batch gte60c : raw register accessors -----------------------------
EDGE_IDX_VALUES = (0, 1, 2, 0x7F, 0x80, 0xFF, 0x7FFF, 0x8000, 0x8001, 0xFFFF, 0x10000, 0x00FF00FF,
                   0x7FFFFFFF, 0x80000000, 0x80000001, 0xFFFF0000, 0xFFFF8000, 0xFFFFFFFF, 0x5555AAAA,
                   0x0000F000, 0x7C1F, 0x03E0, 0x7FFF7FFF, 0x12345678)
LZ_EDGE = tuple([0, M, 0x80000000, 0x7FFFFFFF, 1, 0xFFFFFFFE, 0x40000000, 0xBFFFFFFF, 0xC0000000,
                 0x00FF0000, 0xFF000000, 0xFFFF0000, 0x0000FFFF, 0xFFFFFFF0, 0x0000000F]
                + [1 << k for k in range(32)] + [(M << k) & M for k in range(1, 32)]
                + [((1 << k) - 1) for k in range(1, 32)])


def lz(val):
    bits = val ^ M if val & 0x80000000 else val
    return 32 - bits.bit_length()


def gen_c(cases):
    out = []
    z = [c for c in cases if c['sec'][0] >= 0x40]
    # WriteData/WriteCtrl/ReadData/ReadCtrl : hardware write/read-back (50 dumps x 64 registers)
    for k, c in enumerate(z):
        out.append(case_line('RW', k, 0, 'HW', [], [], c['in'], c['out'], 1))
    # random write sequences through the model (checked against hardware above)
    for s in range(1, 1201):
        inp, x = gen_inputs(s * 7919)
        out.append(case_line('RW', s, 1, 'MDL', [], [], inp, state_after_writes(inp), 1))
    # single-register edge sweeps from a seeded base state
    n = 0
    for i in range(64):
        for e in EDGE_IDX_VALUES:
            n += 1
            inp, x = gen_inputs(0x30000 + n)
            r = Regs()
            r.write_all(inp)
            (r.wd if i < 32 else r.wc)(i & 31, e)
            out.append(case_line('RWONE', n, 2, 'MDL', [i, e], [], inp, r.reads(), 1))
    # register-index aliasing (indices >= 32 act on index & 31): contract observed in the
    # public test (WriteData(n + 64) / ReadData(n + 96) alias n); not a hardware fact.
    n = 0
    for i in range(32):
        for e in (0x12345678, 0xFFFF8001, 0x7FFFFFFF):
            n += 1
            inp, x = gen_inputs(0x31000 + n)
            r = Regs()
            r.write_all(inp)
            r.wd(i, e)
            out.append(case_line('ALIAS', n, 3, 'OBS', [0, i + 32 * (1 + n % 3), e], [], inp, r.reads(), 1))
            r2 = Regs()
            r2.write_all(inp)
            r2.wc(i, e)
            out.append(case_line('ALIAS', n, 3, 'OBS', [1, i + 32 * (1 + n % 3), e], [], inp, r2.reads(), 1))
    # LZCR / SetLZCS : every hardware (LZCS written, LZCR read) pair, then edge values
    for k, c in enumerate(cases):      # LZCR read after the command is unchanged by it: hardware pair
        pre = state_after_writes(c['in'])
        assert pre[31] == c['out'][31], (k, pre[31], c['out'][31])
        out.append(case_line('LZ', k, 0, 'HW', [c['in'][30]], [c['out'][31]], c['in'], pre, 1))
    for k, val in enumerate(LZ_EDGE):
        inp, x = gen_inputs(0x32000 + k)
        r = Regs()
        r.write_all(inp)
        r.wd(30, val)
        got = r.reads()
        assert got[31] == lz(val)
        out.append(case_line('LZ', k, 1, 'MDL', [val], [lz(val)], inp, got, 1))
    for k in range(400):
        inp, x = gen_inputs(0x33000 + k)
        val = rnd(x)
        if k % 2:
            val >>= (k >> 1) % 31
        r = Regs()
        r.write_all(inp)
        r.wd(30, val)
        out.append(case_line('LZ', 5000 + k, 2, 'MDL', [val], [lz(val)], inp, r.reads(), 1))
    # g_pe_gte struct: writes through WriteData/WriteCtrl must be visible in the fields, and
    # fields written directly must be visible through ReadData/ReadCtrl (HW dumps + model).
    for k, c in enumerate(z):
        out.append(case_line('STRUCT', k, 0, 'HW', [], [], c['in'], c['out'], 1))
    for s in range(1, 301):
        inp, x = gen_inputs(0x34000 + s)
        out.append(case_line('STRUCT', s, 1, 'MDL', [], [], inp, state_after_writes(inp), 1))
    return out


# ---- batch gte60a : loaders / setters / TransMatrix --------------------
def reg_after(inp, fn):
    r = Regs()
    r.write_all(inp)
    fn(r)
    return r.reads()


def i16(x):
    return s16(x)


def gen_a_regs(cases):
    out = []
    SET_SRC = 'MDL'
    for s in range(1, 301):
        inp, x = gen_inputs(0x40000 + s)
        # SetV0/1/2 (vx, vy, vz): data 2n = vy<<16 | vx, data 2n+1 = vz
        trip = []
        for _ in range(3):
            val, x = gen_word(x, 'u')
            trip.append(val & 0xFFFF)
        if s % 5 == 0:
            trip = [E16[(s * 3 + i) % len(E16)] for i in range(3)]
        for n, name in enumerate(('PE_GTE_SetV0', 'PE_GTE_SetV1', 'PE_GTE_SetV2')):
            def fn(r, n=n, t=trip):
                r.wd(2 * n, (t[1] << 16) | t[0])
                r.wd(2 * n + 1, t[2])
            out.append(case_line(name, s, 0, SET_SRC, [t for t in trip], [], inp, reg_after(inp, fn), 1))

        def fn_ir(r, t=trip):
            r.wd(9, t[0])
            r.wd(10, t[1])
            r.wd(11, t[2])
        out.append(case_line('PE_GTE_SetIR', s, 0, SET_SRC, trip, [], inp, reg_after(inp, fn_ir), 1))
        w3 = []
        for _ in range(3):
            val, x = gen_word(x, 'w')
            w3.append(val)

        def fn_bk(r, t=w3):
            for i in range(3):
                r.wc(13 + i, t[i])
        out.append(case_line('PE_GTE_SetBK', s, 0, SET_SRC, w3, [], inp, reg_after(inp, fn_bk), 1))
        val, x = gen_word(x, 'w')

        def fn_rgbc(r, t=val):
            r.wd(6, t)
        out.append(case_line('PE_GTE_SetRGBC', s, 0, SET_SRC, [val], [], inp, reg_after(inp, fn_rgbc), 1))
    return out


def gen_a_ram(cases):
    """TransMatrix + the four matrix loaders from original libgte under the interpreter."""
    exe = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out = []
    MZ, VZ = cv.MZONE, cv.VZONE
    lv_entries = {'rot': 0x80078E04, 'trans': 0x80078E94, 'color': 0x80078E64, 'light': 0x80078E34}

    def run_one(entry, ram, args, rd):
        """Execute one original leaf; returns (ret, canonical OUT, loads, stores)."""
        pl.LOADS.clear()
        pl.STORES.clear()
        cv.STORED.clear()
        del cv.RAW[:]
        pl.CUR[0] = entry
        cv.UNFAITHFUL[0] = False
        regs = {i: a for i, a in enumerate(args[:4], 4)}
        regs[31] = v.RET
        fg = {}
        r = cv.execute(ram, entry, tuple(args), stop_at={v.RET}, initial_regs=regs,
                       initial_cop_control=dict(enumerate(rd[32:])), initial_cop_data=dict(enumerate(rd[:32])),
                       instruction_budget=10000, final_gte=fg)
        o = cd.canon_data([x & M for x in fg['data']]) + cd.canon_ctrl([x & M for x in fg['control']])
        return r[2] & M, o, cv.loads_text(), list(pl.STORES), cv.UNFAITHFUL[0]

    names = {'rot': ('PE_GTE_LoadRT33', ('rot',)), 'rt': ('PE_GTE_LoadRT', ('rot', 'trans')),
             'color': ('PE_GTE_LoadLCM', ('color',)), 'light': ('PE_GTE_LoadLLM_halfs', ('light',))}
    dropped = 0
    for s in range(1, 601):
        ram, x = cv.prep(base, s * 7919)
        inp, x2 = gen_inputs(0x50000 + s)
        rd = state_after_writes(inp)
        m = MZ + (x % (0x2000 - 0x40)) // 4 * 4
        if s % 3 == 0:   # matrix halfwords from the edge list
            for k in range(10):
                x = xs32(x)
                struct.pack_into('<H', ram, (m & 0x1FFFFF) + 2 * k, E16[(x >> 8) % len(E16)])
        for key in ('rt', 'rot', 'color'):
            name, seq = names[key]
            cur_rd, loads, ok = rd, [], True
            for st in seq:
                ret, o, lt, stores, unf = run_one(lv_entries[st], ram, [m], cur_rd)
                if unf or stores:
                    ok = False
                cur_rd = o
                loads += [] if lt == '-' else lt.split(' ')
            if not ok:
                dropped += 1
                continue
            out.append(case_line(name, s, 0, 'RTL', [m], [], inp, cur_rd, 1, ' '.join(loads) or '-'))
        # LoadLLM_halfs: nine host halfwords; original SetLightMatrix on the same nine halves
        halves = [struct.unpack_from('<H', ram, (m & 0x1FFFFF) + 2 * k)[0] for k in range(9)]
        ret, o, lt, stores, unf = run_one(lv_entries['light'], ram, [m], rd)
        if unf or stores:
            dropped += 1
        else:
            out.append(case_line('PE_GTE_LoadLLM_halfs', s, 0, 'RTL', halves, [], inp, o, 1, lt))
    # TransMatrix 0x80078C94: matrix (+0x14..0x1B words <- vector words), return value, aliasing
    tm = []
    for s in range(1, 2501):
        ram, x = cv.prep(base, s * 7919)
        inp, x2 = gen_inputs(0x60000 + s)
        rd = state_after_writes(inp)
        x = xs32(x)
        m = MZ + (x % (0x2000 - 0x40)) // 4 * 4
        x = xs32(x)
        vec = VZ + (x % (0x1000 - 0x10)) // 4 * 4
        pat = 0
        if s > 1500:     # partial overlaps: the vector pointer near the translation words
            pat = 1
            k = s - 1501
            shift = 4 * ((k % 9) - 4)
            vec = m + (0x14, 0x00, 0x08, 0x20)[(k // 9) % 4] + shift
        entry = 0x80078C94
        ret, o, lt, stores, unf = run_one(entry, ram, [m, vec], rd)
        if unf:
            dropped += 1
            continue
        tm.append(case_line('pe_plat_gte_translate_matrix', s, pat, 'RTL', [m, vec], [ret], inp, o, 1, lt,
                            ' '.join('%08X:%d=%X' % t for t in stores) or '-'))
    out += tm
    print('gte60a RAM cases dropped (unfaithful or storing): %d' % dropped, file=sys.stderr)
    return out


def gen_a(cases):
    return gen_a_regs(cases) + gen_a_ram(cases)


# ---- batch gte60b : command wrappers -----------------------------------
def gen_b(cases):
    out = []
    # hardware cases: the command through Command(), and the matching wrapper
    for k, c in enumerate(cases):
        if not c['cmd']:
            continue
        op, name, sf, lm, tx, vx, mx = c['cmd']
        w = hw_word(op, sf, lm, tx, vx, mx)
        out.append(case_line('PE_GTE_Command', k, 0, 'HW', [w], [], c['in'], c['out'], 1))
        wrapper = None
        if op == 0x12:
            wrapper = ('PE_GTE_MVMVA', [(sf << 19) | (mx << 17) | (vx << 15) | (tx << 13) | (lm << 10)], [])
        elif op == 0x0C:
            wrapper = ('PE_GTE_OP', [sf, lm], [])
        elif op == 0x3D:
            wrapper = ('PE_GTE_GPF', [sf, lm], [])
        elif op == 0x3E:
            wrapper = ('PE_GTE_GPL', [sf, lm], [])
        elif op == 0x3F and sf == 1 and lm == 1:
            wrapper = ('PE_GTE_NCCT', [], [])
        elif op == 0x01 and sf == 1 and lm == 0:
            wrapper = ('PE_GTE_RTPS_coordinates', [], [c['out'][14], c['out'][19]])
        elif op == 0x30 and sf == 1 and lm == 0:
            wrapper = ('PE_GTE_RTPT_coordinates', [], c['out'][12:15] + c['out'][17:20])
        elif op == 0x06:
            wrapper = ('PE_GTE_NCLIP', [], [c['out'][24]])
        if wrapper:
            out.append(case_line(wrapper[0], k, 0, 'HW', wrapper[1], wrapper[2], c['in'], c['out'], 1))
    # AVSZ3/AVSZ4 take the depth values from a host array (SZ registers are not written)
    hw_avsz = [c for c in cases if c['cmd'] and c['cmd'][0] in (0x2D, 0x2E) and c['cmd'][2] == 1 and c['cmd'][3] == 0]
    for k, c in enumerate(hw_avsz):
        quad = c['cmd'][0] == 0x2E
        rd_in = state_after_writes(c['in'])
        zs = [rd_in[16 + i] for i in range(4)] if quad else [rd_in[17 + i] for i in range(3)]
        out.append(case_line('PE_GTE_AVSZ4' if quad else 'PE_GTE_AVSZ3', k, 0, 'HW', zs, [], c['in'], c['out'], 1))
    # interpreter / model cases: random states
    n = 0
    for s in range(1, 401):
        inp, x = gen_inputs(0x70000 + s)
        rd = state_after_writes(inp)
        # Command(): every command with an independent oracle
        for key in sorted(set(k[0] for k in INT_WORDS) | {0x12}):
            combos = [(sf, lm) for sf in (0, 1) for lm in (0, 1)]
            if key == 0x12:
                x = xs32(x)
                f = (x >> 8) % 4, (x >> 12) % 4, (x >> 16) % 4          # cv, v, mx
                sf, lm = (x >> 20) % 2, (x >> 21) % 2
                combos = [(sf, lm)]
            elif key == 0x06:
                combos = [(0, 0)]
            else:
                f = (0, 0, 0)
            for sf, lm in combos:
                if key in (0x3D, 0x3E, 0x0C):
                    pass
                elif (key, sf, lm) not in INT_WORDS and key != 0x12:
                    continue
                res = oracle_cmd(key, sf, lm, f, rd)
                if not res:
                    continue
                o, fv, src = res
                if s > 80 and key in (0x01, 0x30, 0x3F, 0x28, 0x2D, 0x2E, 0x06):
                    continue
                word = hw_word(key, sf, lm, f[0], f[1], f[2]) if key == 0x12 else (
                    INT_WORDS.get((key, sf, lm)) or hw_word(key, sf, lm, 0, 0, 0))
                n += 1
                out.append(case_line('PE_GTE_Command', n, 1, src, [word], [], inp, o, fv))
        # wrappers
        x = xs32(x)
        f = (x >> 8) % 4, (x >> 12) % 4, (x >> 16) % 4
        sf, lm = (x >> 20) % 2, (x >> 21) % 2
        o, fv, src = oracle_cmd(0x12, sf, lm, f, rd)
        out.append(case_line('PE_GTE_MVMVA', s, 1, src,
                             [(sf << 19) | (f[2] << 17) | (f[1] << 15) | (f[0] << 13) | (lm << 10)], [], inp, o, fv))
        for name, op in (('PE_GTE_GPF', 0x3D), ('PE_GTE_GPL', 0x3E), ('PE_GTE_OP', 0x0C)):
            sf, lm = (s >> 0) % 2, (s >> 1) % 2
            o, fv, src = oracle_cmd(op, sf, lm, (0, 0, 0), rd)
            out.append(case_line(name, s, 1, src, [sf, lm], [], inp, o, fv))
        o, fv, src = oracle_cmd(0x3F, 1, 1, (0, 0, 0), rd)
        out.append(case_line('PE_GTE_NCCT', s, 1, src, [], [], inp, o, fv))
        o, fv, src = oracle_cmd(0x01, 1, 0, (0, 0, 0), rd)
        out.append(case_line('PE_GTE_RTPS_coordinates', s, 1, src, [], [o[14], o[19]], inp, o, fv))
        o, fv, src = oracle_cmd(0x30, 1, 0, (0, 0, 0), rd)
        out.append(case_line('PE_GTE_RTPT_coordinates', s, 1, src, [], o[12:15] + o[17:20], inp, o, fv))
        o, fv, src = oracle_cmd(0x06, 0, 0, (0, 0, 0), rd)
        out.append(case_line('PE_GTE_NCLIP', s, 1, src, [], [o[24]], inp, o, fv))
        # AVSZ3 / AVSZ4: depths from the host array, SZ registers untouched
        for name, op, cnt in (('PE_GTE_AVSZ3', 0x2D, 3), ('PE_GTE_AVSZ4', 0x2E, 4)):
            zs = []
            for _ in range(cnt):
                val, x = gen_word(x, 'u')
                zs.append(val & 0xFFFF)
            r2 = list(rd)
            for i, zv in enumerate(zs):
                r2[(17 if cnt == 3 else 16) + i] = zv
            o, fv, src = oracle_cmd(op, 1, 0, (0, 0, 0), r2)
            o = list(o)
            for i in range(16, 20):
                o[i] = rd[i]                      # the wrapper does not write the SZ FIFO
            out.append(case_line(name, s, 1, src, zs, [], inp, o, fv))
    return out


def main():
    mode = sys.argv[2] if len(sys.argv) > 2 else ''
    cases = parse_hw(hw_log_path())
    res = validate(cases)
    bad = print_validation(res)
    if bad:
        raise SystemExit('oracle validation failed (%d mismatching hardware cases)' % bad)
    if mode == 'validate':
        return
    gen = {'a': gen_a, 'b': gen_b, 'c': gen_c}[mode]
    lines = gen(cases)
    open(sys.argv[1], 'w').write('\n'.join(lines) + '\n')
    counts = {}
    for ln in lines:
        counts[ln.split(' ', 1)[0]] = counts.get(ln.split(' ', 1)[0], 0) + 1
    for k in sorted(counts):
        print('CASES %-34s %d' % (k, counts[k]))
    print('cases', len(lines))


if __name__ == '__main__':
    main()

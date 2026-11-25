#!/usr/bin/env python3
"""Black-box vectors for the libgpu/libgte rows replaced clean-room in
docs/evidence/native-gpu-rows-20261125/ (original code from the user's EXE
under the test-only interpreter; the code itself is never read, only driven).

Engine: one ordered event log per case.  Events are
  L addr:size=value   data load (stack frame 0x801F0000-0x801FF000 and the
                      setjmp area are not logged)
  S addr:size=value   data store
  C name a0 a1 a2 a3  call into a stubbed callee (guest function the entry
                      calls; the stub returns the scripted value, writes no
                      memory)
Hardware registers (0x1F801000..) alias into low RAM in the interpreter, so
the engine models what psx-spx defines:
  * GP1 (0x1F801814) stores are commands: logged, they never change the
    status the next load of 0x1F801814 reads (GPUSTAT comes from a scripted
    read sequence, the last value repeating);
  * DMA2 CHCR (0x1F8010A8) loads come from a scripted busy sequence;
  * root counter 1 (0x1F801110) loads return base + step * n.
Usage: pe_gpu_rows_vectors.py OUT.txt ROW      ROW in dsync|...
"""
import inspect
import struct
import sys

import pe_pad_leaf_vectors as pl

v = pl.v
RET = v.RET
EVENTS = []
MODEL = {'stat': [0], 'chcr': [0], 't1': (0, 1), 'n': {}}
STACK = (0x1F0000, 0x1FF000)


def logged(a):
    return a >= 0x1000 and not v.JMPBUF[0] <= a < v.JMPBUF[1] and \
        not STACK[0] <= a < STACK[1]


def seq_next(key):
    n = MODEL['n'].get(key, 0)
    MODEL['n'][key] = n + 1
    s = MODEL[key]
    return s[min(n, len(s) - 1)]


def _load(fmt, buf, a):
    if a == 0x1814 and struct.calcsize(fmt) == 4:
        val = (seq_next('stat'),)
    elif a == 0x10A8 and struct.calcsize(fmt) == 4:
        val = (seq_next('chcr'),)
    elif a == 0x1110 and struct.calcsize(fmt) == 4:
        n = MODEL['n'].get('t1', 0)
        MODEL['n']['t1'] = n + 1
        val = ((MODEL['t1'][0] + MODEL['t1'][1] * n) & 0xFFFFFFFF,)
    else:
        val = struct.unpack_from(fmt, buf, a)
    if logged(a):
        base = 0x1F800000 if a < 0x2000 else 0x80000000
        EVENTS.append(('L', base + a, struct.calcsize(fmt), val[0] & 0xFFFFFFFF))
    return val


def _store(fmt, buf, a, val):
    n = struct.calcsize(fmt)
    if logged(a):
        base = 0x1F800000 if a < 0x2000 else 0x80000000
        EVENTS.append(('S', base + a, n, val & ((1 << (8 * n)) - 1)))
    if a == 0x1814:
        return                      # GP1 command: does not alias into GPUSTAT
    v._store(fmt, buf, a, val)


_src = inspect.getsource(v._hud.execute).replace('struct.pack_into(', 'STORE(')
_src = _src.replace('struct.unpack_from({32:', 'LOAD({32:')
_src = _src.replace("struct.unpack_from('<I', memory, a & ~3)", "LOAD('<I', memory, a & ~3)")
_ns = dict(vars(v._hud))
_ns['STORE'] = _store
_ns['LOAD'] = _load
exec(compile(_src, v._hud.__file__, 'exec'), _ns)
execute = _ns['execute']


class Stops:
    def __init__(self, callees):
        self.callees = callees
        self.hit = None

    def __contains__(self, pc):
        if pc == RET or pc in self.callees or pc in (0xA0, 0xB0, 0xC0):
            self.hit = pc
            return True
        return False


def run(ram, entry, args, callees=None, model=None, budget=200000, max_calls=64):
    """Drive one case.  callees: {guest address: (name, [return values])};
    the return list is consumed one per call, the last repeating.  Returns
    (events, result) where result is the return value, 'SPIN' if the
    instruction budget ran out, or 'CALLS' if max_calls was exceeded."""
    callees = callees or {}
    EVENTS.clear()
    MODEL.update(stat=[0], chcr=[0], t1=(0, 1), n={})
    if model:
        MODEL.update(model)
    used = {}
    regs = {i + 4: a for i, a in enumerate(args)}
    regs[31] = RET
    pc = entry
    for _ in range(max_calls + 1):
        st = Stops(callees)
        try:
            r = execute(ram, pc, (), stop_at=st, initial_regs=regs, instruction_budget=budget)
        except AssertionError as e:
            if str(e) == 'instruction budget':
                return list(EVENTS), 'SPIN'
            return list(EVENTS), 'FAULT'       # the test interpreter does not support the path
        if st.hit == RET:
            return list(EVENTS), r[2] & 0xFFFFFFFF
        if st.hit in (0xA0, 0xB0, 0xC0):
            # BIOS call through the A/B/C vector: logged by table and function
            # number (r9); returns 0.  psx-spx "BIOS Function Summary".
            sp = r[29] & 0x1FFFFF
            stk = struct.unpack_from('<4I', ram, sp + 16)
            EVENTS.append(('C', 'BIOS_%s%02X' % ({0xA0: 'A', 0xB0: 'B', 0xC0: 'C'}[st.hit], r[9] & 0xFF),
                           r[4] & 0xFFFFFFFF, r[5] & 0xFFFFFFFF, r[6] & 0xFFFFFFFF, r[7] & 0xFFFFFFFF)
                          + (('stk', stk[0], stk[1]) if (r[9] & 0xFF) == 0x3F else ()))
            r[2] = 0
            regs = {i: x for i, x in enumerate(r)}
            pc = r[31]
            continue
        if st.hit is None:
            return list(EVENTS), 'JUMP0'       # jumped through a null pointer
        name, rets = callees[st.hit]
        k = used.get(st.hit, 0)
        used[st.hit] = k + 1
        EVENTS.append(('C', name, r[4] & 0xFFFFFFFF, r[5] & 0xFFFFFFFF,
                       r[6] & 0xFFFFFFFF, r[7] & 0xFFFFFFFF))
        r[2] = rets[min(k, len(rets) - 1)] & 0xFFFFFFFF
        regs = {i: x for i, x in enumerate(r)}
        pc = r[31]
    return list(EVENTS), 'CALLS'


def fmt_events(events):
    out = []
    for e in events:
        if e[0] == 'C':
            out.append('C:%s:%X,%X,%X,%X' % e[1:6] + (':stk=%X,%X' % e[7:9] if len(e) > 6 else ''))
        else:
            out.append('%s:%08X:%d=%X' % e)
    return ' '.join(out) or '-'


def fixture():
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    return bytes(v.fixture(exe, 0, 0, 0))


# ---------------------------------------------------------------- helpers
def xs32(x):
    x ^= (x << 13) & 0xFFFFFFFF
    x ^= x >> 17
    x ^= (x << 5) & 0xFFFFFFFF
    return x & 0xFFFFFFFF


class Rng:
    def __init__(self, seed):
        self.x = (seed * 2654435761 + 12345) & 0xFFFFFFFF or 1

    def next(self):
        self.x = xs32(self.x)
        return self.x

    def pick(self, seq):
        return seq[self.next() % len(seq)]

    def word(self):
        """Mixed-magnitude 32-bit value."""
        k = self.next() % 5
        x = self.next()
        if k == 4:
            return self.pick((0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0xFFFF, 0x10000))
        return x & (0xF, 0xFF, 0xFFFF, 0xFFFFFFFF)[k]


ZONE = 0x80100000
LINE_FMT = '%08X %d A %s R %s RET %s EV %s'


def put32(ram, a, x):
    struct.pack_into('<I', ram, a & 0x1FFFFF, x & 0xFFFFFFFF)


def put16(ram, a, x):
    struct.pack_into('<H', ram, a & 0x1FFFFF, x & 0xFFFF)


def emit(entry, seed, args, rets, res, events):
    rs = ';'.join('%s=%s' % (n, ','.join('%X' % x for x in l)) for n, l in sorted(rets.items())) or '-'
    r = res if isinstance(res, str) else '%X' % res
    args = list(args) + [0] * (4 - len(args))
    return LINE_FMT % (entry, seed, ' '.join('%X' % a for a in args), rs, r, fmt_events(events))


# ------------------------------------------------------------ row: timeout
VSYNC = 0x80073A44
PTR_WORDS = {0x80095854: 0x1F801814, 0x80095858: 0x1F8010A0, 0x8009585C: 0x1F8010A4,
             0x80095860: 0x1F8010A8, 0x80095870: 0x1F8010F0, 0x80095674: 0x1F801074}


def prep_ptrs(ram, rng, zone_mode):
    """zone_mode: the register pointer words point at plain RAM words in the
    zone (distinct, aligned), so the code must go through the pointers."""
    if not zone_mode:
        return
    k = 0
    for a in PTR_WORDS:
        put32(ram, a, ZONE + 0x100 * (k + 1))
        k += 1


def timeout_cases(base, count=6000):
    out = []
    for k in range(count):
        seed = 0x77404000 + k
        rng = Rng(seed)
        ram = bytearray(base)
        zone = rng.next() % 3 == 0
        prep_ptrs(ram, rng, zone)
        lim = rng.pick((0, 1, 0xF0, 0x100, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)) if rng.next() & 1 else rng.word()
        vs = rng.pick((0, 1, 0xEF, 0xF0, 0xF1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)) if rng.next() & 1 else rng.word()
        if rng.next() % 3 == 0:
            vs = (lim + rng.pick((-1, 0, 1, 2))) & 0xFFFFFFFF
        cnt = rng.pick((0, 1, 0xEFFFF, 0xF0000, 0xF0001, 0xFFFFF, 0x100000, 0x7FFFFFFF, 0x80000000,
                        0xFFFFFFFF, 0xFFFFFFFE)) if rng.next() % 3 else rng.word()
        put32(ram, 0x80095888, lim)
        put32(ram, 0x8009588C, cnt)
        put32(ram, 0x80095874, rng.word())
        put32(ram, 0x80095878, rng.word())
        put32(ram, 0x80095884, rng.word())
        put16(ram, 0x1074, rng.word())
        put32(ram, 0x10A0, rng.word())
        put32(ram, 0x10F0, rng.word())
        if zone:
            for j in range(5):
                put32(ram, ZONE + 0x100 * (j + 1), rng.word())
            put16(ram, ZONE + 0x500, rng.word())
        stat = [rng.word() for _ in range(3)]
        chcr = [rng.word() for _ in range(2)]
        args = [rng.word() for _ in range(4)]
        ev, res = run(ram, 0x80077404, args, {VSYNC: ('VSync', [vs])}, {'stat': stat, 'chcr': chcr})
        out.append(emit(0x80077404, seed, args, {'VSync': [vs]}, res, ev))
    return out


# ------------------------------------------------------------ row: dmastart
def dmastart_cases(base, count=3000):
    out = []
    for k in range(count):
        seed = 0x76B98000 + k
        rng = Rng(seed)
        ram = bytearray(base)
        zone = rng.next() % 3 == 0
        prep_ptrs(ram, rng, zone)
        if zone:
            for j in range(6):
                put32(ram, ZONE + 0x100 * (j + 1), rng.word())
        args = [rng.word() if k % 4 else rng.pick((0, 1, 0x80000000, 0x80100000, 0xA0100000, 0xFFFFFFFF, 0x1F801810)),
                rng.word()]
        ev, res = run(ram, 0x80076B98, args)
        out.append(emit(0x80076B98, seed, args, {}, res, ev))
    return out


# --------------------------------------------------------------- row: dmaq
TO_ADDR, SEED_ADDR = 0x80077404, 0x800773D0
FN_A, FN_B, CB_A, CB_B = 0x80130000, 0x80130040, 0x80130100, 0x80130140
BUSY, READY = 0x01000000, 0x04000000
ENTRY0 = 0x800BD030


def seq_chcr(rng):
    out = [rng.pick((0, 0, 0, BUSY, 0xFFFFFFFF, 0x00FFFFFF, rng.word()))
           for _ in range(rng.next() % 5)]
    return out + [rng.pick((0, 0x00FFFFFF, 0x7E000000))]            # last value: idle


def seq_stat(rng):
    out = [rng.pick((0, 0x08000000, 0x10000000, 0x1C000000, 0x04000000, 0xFFFFFFFF, rng.word()))
           for _ in range(rng.next() % 5)]
    return out + [rng.pick((READY, 0x1C000000, 0xFFFFFFFF, READY | (rng.word() & 0xFBFFFFFF)))]   # last: ready


def dmaq_cases(base, count=12000):
    out = []
    for k in range(count):
        seed = 0x77294000 + k
        rng = Rng(seed)
        ram = bytearray(base)
        mode = rng.pick((0, 1))
        kind = rng.next() % 8
        if kind == 0:              # empty queue, high bits allowed
            r = rng.word()
            q = (r + 64 * (rng.next() % 4)) & 0xFFFFFFFF if rng.next() & 1 else r
        elif kind == 1:            # high bits on q, queue pending
            r = rng.next() % 64
            q = (r + 1 + rng.next() % 5 + 64 * (rng.next() % 3)) & 0xFFFFFFFF
        else:                      # pending, small indices including wrap
            r = rng.pick((0, 1, 2, 5, 61, 62, 63, rng.next() % 64))
            q = (r + rng.pick((1, 1, 2, 3, 4, rng.next() % 64))) % 64
        if mode == 0 and q != r and r >= 64:     # wait mode would index the queue with a wild r
            r = r % 64
            q = (r + 1 + rng.next() % 5) % 64
        put32(ram, 0x80095874, q)
        put32(ram, 0x80095878, r)
        ptr_zone = rng.next() % 6 == 0
        if ptr_zone:
            for a, z in ((0x80095854, 1), (0x80095860, 2), (0x80095674, 3)):
                put32(ram, a, ZONE + 0x100 * z)
            put32(ram, ZONE + 0x200, 0 if rng.next() & 1 else rng.pick((BUSY, 0xFFFFFFFF)))
            put32(ram, ZONE + 0x100, READY | rng.word() if rng.next() & 1 else rng.pick((0, 0x08000000)))
            put16(ram, ZONE + 0x300, rng.word())
            put32(ram, 0x80095854, ZONE + 0x100)
        put16(ram, 0x1074, rng.word())
        put32(ram, 0x80095880, rng.word())
        f754 = rng.pick((0, 0, 1, 2, 0x80000000, rng.word()))
        f758 = rng.pick((0, 0, 0, 1, rng.word())) if f754 == 0 else 0
        if rng.next() % 5 == 0:
            f754, f758 = 0, rng.pick((0, 1, 0xFF))
        put32(ram, 0x80095754, f754)
        put32(ram, 0x80095758, f758)
        cb = rng.pick((CB_A, CB_B))
        if rng.next() % 3 == 0:
            put32(ram, 0x8009566C, ZONE + 0x800)
            put32(ram, ZONE + 0x804, cb)
        else:
            put32(ram, 0x80095650, cb)
        for i in range(64):
            e = ENTRY0 + 0x60 * i
            put32(ram, e, rng.pick((FN_A, FN_B)))
            put32(ram, e + 4, rng.word())
            put32(ram, e + 8, rng.word())
            put32(ram, e + 12, rng.word())
        to = [rng.pick((0, 0, 0, 1, 0xFFFFFFFF, 0x80000000, 0xFFFFFFFE, 0x7FFFFFFF, rng.word())) for _ in range(rng.next() % 4)] or [0]
        if rng.next() % 4 == 0:
            to = [0] * (rng.next() % 3) + [0xFFFFFFFF]
        rets = {'TO': to, 'G298C': [rng.word()], 'FN': [rng.word()], 'FN2': [rng.word()],
                'CB': [rng.word()], 'CB2': [rng.word()]}
        callees = {TO_ADDR: ('TO', rets['TO']), SEED_ADDR: ('G298C', rets['G298C']),
                   FN_A: ('FN', rets['FN']), FN_B: ('FN2', rets['FN2']),
                   CB_A: ('CB', rets['CB']), CB_B: ('CB2', rets['CB2'])}
        args = [mode, rng.word(), rng.word(), rng.word()]
        ev, res = run(ram, 0x80077294, args, callees, {'stat': seq_stat(rng), 'chcr': seq_chcr(rng)},
                      budget=100000, max_calls=300)
        if isinstance(res, str):
            continue                         # spin / null jump / too many calls: not a defined case
        out.append(emit(0x80077294, seed, args, rets, res, ev))
    return out


# ------------------------------------------------------------ row: drawsync
def drawsync_cases(base, count=4000):
    out = []
    for k in range(count):
        seed = 0x74DC0000 + k
        rng = Rng(seed)
        ram = bytearray(base)
        ram[0x9574E] = rng.pick((0, 1, 2, 3, 0x7F, 0x80, 0xFF, rng.next() & 0xFF))
        if rng.next() % 3:
            put32(ram, 0x80095748, rng.pick((0x80071A74, 0x80130100, 0x80130140)))
        slot = rng.pick((0x80077294, 0x80077294, 0x80130000, 0x80130040))
        if rng.next() % 4 == 0:                   # table somewhere else in RAM
            put32(ram, 0x80095744, ZONE + 0x400)
            put32(ram, ZONE + 0x400 + 0x3C, slot)
        else:
            put32(ram, 0x80095704 + 0x3C, slot)
        mode = rng.pick((0, 1, 2, 0xFFFFFFFF, 0x80000000, rng.word()))
        rets = {'DMAQ': [rng.word()], 'HOOKA': [rng.word()], 'HOOKB': [rng.word()], 'LOG': [rng.word()], 'LOG2': [rng.word()]}
        callees = {0x80077294: ('DMAQ', rets['DMAQ']), 0x80130000: ('HOOKA', rets['HOOKA']),
                   0x80130040: ('HOOKB', rets['HOOKB']), 0x80130100: ('LOG', rets['LOG']),
                   0x80130140: ('LOG2', rets['LOG2'])}
        args = [mode, rng.word(), rng.word(), rng.word()]
        ev, res = run(ram, 0x80074DC0, args, callees, budget=20000, max_calls=20)
        if isinstance(res, str):
            continue
        out.append(emit(0x80074DC0, seed, args, rets, res, ev))
    return out


# ------------------------------------------------------------ row: dispenv
def dispenv_cases(base, count=6000, byvals=(0, 1, 2, 0xFF), seed0=0x755F0000, clamps=True):
    out = []
    E16 = (0, 1, 2, 16, 100, 255, 256, 257, 280, 281, 320, 352, 353, 384, 400, 401, 480, 512, 560, 561, 640,
           0x3FF, 0x400, 0x7FFF, 0x8000, 0xFFFF, 0xFFF0)
    for k in range(count):
        seed = seed0 + k
        rng = Rng(seed)
        ram = bytearray(base)
        env = ZONE + 0x20 * (rng.next() % 16)
        fields = [rng.pick(E16) if rng.next() % 3 else rng.next() & 0xFFFF for _ in range(8)]
        fields[2] = rng.pick((256, 320, 384, 512, 640, 280, 281, 352, 353, 400, 401, 560, 561, rng.next() & 0xFFFF))
        if rng.next() % 4 == 0:                  # tiny screen widths hit the minimum-width clamp
            fields[6] = rng.pick((0, 1, 2, 3, 4, 5, 8, 0xFFFF, 0x8000))
        by = [rng.pick(byvals) for _ in range(4)]
        for i, x in enumerate(fields):
            struct.pack_into('<H', ram, (env + 2 * i) & 0x1FFFFF, x)
        for i, x in enumerate(by):
            ram[(env + 16 + i) & 0x1FFFFF] = x
        # cache: sometimes a copy of the environment (fully or in parts), else random
        cache = 0x800957B8
        mode = rng.next() % 5
        for i, x in enumerate(fields):
            same = mode == 0 or (mode == 1 and i < 4) or (mode == 2 and i >= 4) or (mode == 3 and rng.next() % 6)
            struct.pack_into('<H', ram, (cache + 2 * i) & 0x1FFFFF, x if same else rng.next() & 0xFFFF)
        flagsame = mode in (0, 1, 2) or rng.next() % 2
        w = by[0] | by[1] << 8 | by[2] << 16 | by[3] << 24
        put32(ram, 0x800957C8, w if flagsame else rng.word())
        put32(ram, 0x800956EC, rng.pick((0, 0, 1, 1, 2, 3, 0xFF, 0x100, 0xFFFFFFFF, rng.word())))
        ram[0x9574F] = rng.pick((0, 0, 1, 0xFF))
        ram[0x9574E] = rng.pick((0, 0, 0, 1, 2, 3, 0xFF))
        if rng.next() % 3 == 0:
            put32(ram, 0x80095748, rng.pick((0x80071A74, 0x80130100, 0x80130140)))
        if rng.next() % 4 == 0:               # randomised range table / divisors
            for a in range(0x80095820, 0x80095848, 2):
                put16(ram, a, rng.pick((0x24E, 0xC4E, 0x258, 0xC58, 0x21B, 0xC9B, rng.next() & 0xFFFF)))
            for a in range(0x80095848, 0x8009584D):
                ram[a & 0x1FFFFF] = rng.pick((10, 8, 7, 5, 4, rng.next() & 0xFF))
        if rng.next() % 4 == 0:
            put32(ram, 0x80095744, ZONE + 0x600)
            put32(ram, ZONE + 0x610, rng.pick((0x80076B20, 0x80130000, 0x80130040)))
        rets = {'WCM': [rng.word()], 'HOOKA': [rng.word()], 'HOOKB': [rng.word()], 'LOG': [rng.word()], 'LOG2': [rng.word()]}
        callees = {0x80076B20: ('WCM', rets['WCM']), 0x80130000: ('HOOKA', rets['HOOKA']),
                   0x80130040: ('HOOKB', rets['HOOKB']), 0x80130100: ('LOG', rets['LOG']),
                   0x80130140: ('LOG2', rets['LOG2'])}
        args = [env, rng.word(), rng.word(), rng.word()]
        ev, res = run(ram, 0x800755F0, args, callees, budget=20000, max_calls=20)
        if isinstance(res, str):
            continue
        out.append(emit(0x800755F0, seed, args, rets, res, ev))
    if not clamps:
        return out
    # Pattern 1: every clamp boundary +-1.  A table entry is built so that the
    # unclamped value lands exactly on each bound; y bounds through sy/sh.
    k = 0
    for vm in (0, 1):
        pal = vm
        lo1, hi1, hi2 = (540, 3220, 3260) if pal else (500, 3250, 3290)
        ymin, y1hi, y2hi = (19, 303, 305) if pal else (16, 257, 258)
        for cls_w in (256, 320, 384, 512, 640):
            cls = (256, 320, 384, 512, 640).index(cls_w)
            for off in (-1, 0, 1):
                for what in ('x1lo', 'x1hi', 'x2hi', 'x2min', 'y1lo', 'y1hi', 'y2hi', 'y2min'):
                    k += 1
                    seed = 0x755F8000 + k
                    rng = Rng(seed)
                    ram = bytearray(base)
                    env = ZONE + 0x40
                    f = [0, 0, cls_w, 240, 0, 0, 0, 240]
                    tx1, tx2 = 600, 3160
                    if what == 'x1lo':
                        tx1 = lo1 + off
                    elif what == 'x1hi':
                        tx1 = hi1 + off
                    elif what == 'x2hi':
                        tx1, tx2 = 600, hi2 + off
                    elif what == 'x2min':
                        f[6] = 1
                        tx1 = 1000
                    elif what == 'y1lo':
                        f[5] = off
                    elif what == 'y1hi':
                        f[5] = y1hi - ymin + off
                    elif what == 'y2hi':
                        f[5], f[7] = 0, y2hi - ymin + off
                    elif what == 'y2min':
                        f[5] = y1hi - ymin; f[7] = 1 + off
                    for i, x in enumerate(f):
                        struct.pack_into('<H', ram, (env + 2 * i) & 0x1FFFFF, x & 0xFFFF)
                    put32(ram, 0x800956EC, vm)
                    tab = 0x80095820 + 4 * (cls + 5 * vm)
                    put16(ram, tab, tx1)
                    put16(ram, tab + 2, tx2)
                    rets = {'WCM': [1], 'HOOKA': [1], 'HOOKB': [1], 'LOG': [1], 'LOG2': [1]}
                    callees = {0x80076B20: ('WCM', rets['WCM'])}
                    args = [env, 0, 0, 0]
                    ev, res = run(ram, 0x800755F0, args, callees, budget=20000, max_calls=20)
                    if isinstance(res, str):
                        continue
                    out.append(emit(0x800755F0, seed, args, rets, res, ev))
    return out


def dispenv2_cases(base, count=8000):
    # flag/pad bytes beyond {0,1,2,FF}: DAY1_dispenv case 704 (pad0 = 8, byte 13 = 0x16, cache == environment)
    return dispenv_cases(base, count, (0, 1, 2, 3, 8, 0x16, 0x80, 0xFF), 0x755F4000, False)


# ------------------------------------------------------------ row: resetgraph
RG_CALLEES = {0x80077A28: 'FILL', 0x80073C94: 'RESETCB', 0x80077A54: 'GPUCW', 0x80077144: 'G2700',
              0x80130000: 'HOOKA', 0x80130040: 'HOOKB', 0x80130100: 'LOG', 0x80130140: 'LOG2'}


def resetgraph_cases(base, count=12000):
    """ResetGraph (0x80074A44).  Reset modes (low three bits 0, 3, 5): diagnostic print (not for 5), fill of the
    0x80 library bytes, the libetc reset callback, the table-root word handed to the GPU control word writer, the GPU
    control function with the mode, the video-mode byte, the display limits copied from a table indexed by it, the
    two cache fills; result = the video-mode byte.  Other modes: debug trace, table slot call (the light path)."""
    out = []
    for k in range(count):
        seed = 0x74A44000 + k
        rng = Rng(seed)
        ram = bytearray(base)
        kind = rng.next() % 20
        if kind < 12:                          # reset modes
            low = rng.pick((0, 3, 5))
            if rng.next() & 1:
                mode = rng.pick((0, 3, 5, 8, 0xB, 0xD, 0x10, 0x13, 0x15, 0x100, 0x103, 0x105, 0x80000000, 0x80000003,
                                 0x80000005, 0xFFFFFFF8, 0xFFFFFFFB, 0xFFFFFFFD)) if rng.next() & 1 else (rng.next() & 0xFF & ~7) | low
            else:
                mode = (rng.word() & ~7 & 0xFFFFFFFF) | low
        else:                                   # light modes
            low = rng.pick((1, 2, 4, 6, 7))
            mode = rng.pick((low, low, 8 + low, 0x100 | low, 0xFFFFFFF8 | low, (rng.word() & ~7 & 0xFFFFFFFF) | low))
        light = (mode & 7) not in (0, 3, 5)
        ram[0x9574E] = rng.pick((0, 1, 2, 3, 0x7F, 0xFF, rng.next() & 0xFF))
        if rng.next() % 3:
            put32(ram, 0x80095748, rng.pick((0x80071A74, 0x80130100, 0x80130140)))
        if light:
            slot = rng.pick((0x80077144, 0x80077144, 0x80130000, 0x80130040))
            if rng.next() % 4 == 0:
                put32(ram, 0x80095744, ZONE + 0x400)
                put32(ram, ZONE + 0x400 + 0x34, slot)
            else:
                put32(ram, 0x80095704 + 0x34, slot)
        else:
            if rng.next() % 3 == 0:
                put32(ram, 0x80095744, rng.pick((0, 0xFFFFFFFF, 0x12345678, 0x80095704, rng.word())))
            for a in range(0x800957CC, 0x800957CC + 0x40C, 2):      # display limit tables
                put16(ram, a, rng.next() if rng.next() % 4 else rng.pick((0, 0x400, 0x200, 0xFFFF)))
            ram[0x9574C] = rng.next() & 0xFF
        rets = {'FILL': [rng.word()], 'RESETCB': [rng.word()], 'GPUCW': [rng.word()],
                'G2700': [rng.pick((0, 1, 2, 5, 0x7F, 0x80, 0xFF, 0x100, 0x1234567, 0x80000001)) if rng.next() & 1 else rng.word()],
                'HOOKA': [rng.word()], 'HOOKB': [rng.word()], 'LOG': [rng.word()], 'LOG2': [rng.word()]}
        callees = {a: (n, rets[n]) for a, n in RG_CALLEES.items()}
        args = [mode, rng.word(), rng.word(), rng.word()]
        ev, res = run(ram, 0x80074A44, args, callees, budget=60000, max_calls=20)
        if isinstance(res, str):
            continue
        out.append(emit(0x80074A44, seed, args, rets, res, ev))
    return out


ROWS = {'resetgraph': resetgraph_cases, 'dispenv2': dispenv2_cases, 'timeout': timeout_cases, 'dmastart': dmastart_cases, 'dmaq': dmaq_cases, 'drawsync': drawsync_cases,
        'dispenv': dispenv_cases}


def main():
    out = []
    base = fixture()
    for row in sys.argv[2:]:
        out += ROWS[row](base)
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out))


if __name__ == '__main__':
    main()

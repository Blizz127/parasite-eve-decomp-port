#!/usr/bin/env python3
"""Black-box vector generator for the sw-vec-gpu2 libgpu/libpress worker
family (batches gpu2a, gpu2b, gpu2c, gpu2d).  See pe_gpu2_common.py for the
method (original code from the user's EXE under the test-only interpreter,
two-pass leftover-register discrimination).  No translated source is read.

Usage: pe_gpu2_vectors.py OUT.txt BATCH_OR_FUNC [...]
  BATCH: gpu2a gpu2b gpu2c gpu2d     FUNC: func_XXXXXXXX
Each case line (checker contract in pe_gpu2_check_core.h):
  ENTRY seed A a0 a1 a2 a3 R callee=ret,ret;... RET r|_ EV ev...
  ev: L:addr:size=val  S:addr:size=val  C:func_XXXXXXXX:a0,a1,a2,a3 (_ = not significant)
"""
import struct
import sys

import pe_gpu2_common as C

R = C.R
Rng = C.Rng
put32 = C.put32
put16 = C.put16
ZONE = 0x80100000
ZSIZE = 0x2000
E32 = (0, 1, 2, 3, 4, 7, 8, 0xF, 0x10, 0x3F, 0x40, 0xFF, 0x100, 0x1FF, 0x200, 0x3FF, 0x400, 0x7FF, 0x800,
       0xFFF, 0x1000, 0x7FFF, 0x8000, 0xFFFF, 0x10000, 0x12345, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0xFFFFFFFE,
       0xFFFF0000, 0xFFFF8000, 0x00010400, 0x1F801810, 0x1F801814, 0x80100000, 0x80095704)
E16 = (0, 1, 2, 3, 7, 8, 0xF, 0x10, 0x1F, 0x20, 0x3F, 0x40, 0xFF, 0x100, 0x1FF, 0x200, 0x3FF, 0x400, 0x401,
       0x7FF, 0x800, 0x7FFF, 0x8000, 0xFFFF, 0xFFFE, 0xFFF0, 0xFC00)
SPECS = {}
BATCHES = {'gpu2a': [], 'gpu2b': [], 'gpu2c': [], 'gpu2d': [], 'gpu2e': []}


OVL = {'func_8010BE3C', 'func_8010C0D8', 'func_8010BD4C'}
OVL_YAML = 'configs/USA/overlays/ovl_039F.yaml'
OVL_BIN = 'build/extracted/disc1/peimg/ovl_039F.bin'
OVL_VRAM = 0x8010BCF8
OVL_SHA1 = 'e7331867b4d1464da4d5f386d9137eaa1c98422b'


def spec(name, batch, nargs, count):
    def deco(fn):
        SPECS[name] = (fn, nargs, count)
        BATCHES[batch].append(name)
        return fn
    return deco


def word(rng):
    k = rng.next() % 6
    if k == 5:
        return rng.pick(E32)
    return rng.next() & (0xF, 0xFF, 0xFFFF, 0xFFFFFFFF, 0xFFFFFF)[k % 5]


def h16(rng):
    return rng.pick(E16) if rng.next() & 1 else rng.next() & 0xFFFF


def zptr(rng, align=4, margin=0x80):
    return ZONE + (rng.next() % (ZSIZE - margin)) // align * align


def fill_zone(ram, rng, lo=0, hi=ZSIZE):
    for a in range(lo, hi, 4):
        put32(ram, ZONE + a, word(rng))


def noop(ram):
    pass


# ============================================================ gpu2c: address
def _addr_setup(rng, p):
    def s(ram):
        fill_zone(ram, rng)
        if p >= 0x80000000:
            put32(ram, p + 4, rng.pick((0, 1, 2, 3, 0xFF, 0x100, 0xFFFF, 0x80000000, 0xFFFFFFFF)) if rng.next() & 1 else word(rng))
    return s


def _ptr_args(rng, k):
    return zptr(rng, 4)


for _n in ('func_80071944', 'func_80071964', 'func_80071994'):
    def _mk(_n=_n):
        @spec(_n, 'gpu2c', 1, 600)
        def gen(rng, k):
            p = _ptr_args(rng, k)
            return [p, 0, 0, 0], _addr_setup(rng, p), None, None
        return gen
    _mk()


@spec('func_80077A64', 'gpu2c', 4, 2500)
def gen_77a64(rng, k):
    a, b, c, d = [rng.pick(E32) if rng.next() % 3 else word(rng) for _ in range(4)]
    if k < len(E32) * 4:
        a, b = E32[k // 4], E32[(k * 7) % len(E32)]
    return [a, b, c, d], noop, None, None


@spec('func_80077AA4', 'gpu2c', 2, 1500)
def gen_77aa4(rng, k):
    a = rng.pick(E32) if k % 3 else word(rng)
    b = rng.pick(E32) if k % 2 else word(rng)
    if k < len(E32) * 4:
        a, b = E32[k // 4], E32[(k * 5) % len(E32)]
    return [a, b, 0, 0], noop, None, None



# ============================================================ shared environment
SENT = [0x80130000 + 0x40 * i for i in range(16)]          # sentinel guest functions
TBL = 0x80095704                                            # libgpu function table (image default)
STAT_POOL = (0, 0x04000000, 0x08000000, 0x0C000000, 0x10000000, 0x14000000, 0x1C000000, 0x1E000000, 0xFFFFFFFF,
             0x04000003, 0x1C0000FF, 0x10000000)
READY_POOL = (0x04000000, 0x0C000000, 0x1C000000, 0x14000000, 0x1C0000FF, 0xFFFFFFFF, 0x0C000001)
LIM = (0, 1, 2, 0x100, 0x140, 0x1E0, 0x200, 0x280, 0x3FF, 0x400, 0x401, 0x7FFF, 0x8000, 0xFFFF)
RECT_E = (0, 1, 2, 3, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 100, 255, 256, 257, 320, 511, 512, 513, 1000, 1023, 1024,
          1025, 2047, 0x7FFF, 0x8000, 0xFFFF, 0xFFFE, 0xFFF0)


def stat_seq(rng, ready=READY_POOL):
    return [rng.pick(STAT_POOL) for _ in range(rng.next() % 4)] + [rng.pick(ready)]


def chcr_seq(rng, busy_end=False):
    out = [rng.pick((0, 0x01000000, 0x01000200, 0x11000002, 0xFFFFFFFF, 0x00FFFFFF)) for _ in range(rng.next() % 4)]
    return out + [0x01000000 if busy_end else rng.pick((0, 0, 0, 0x00FFFFFF, 0x7E000000))]


def edge16(rng):
    return rng.pick(RECT_E) if rng.next() % 3 else rng.next() & 0xFFFF


def put_rect(ram, a, x, y, w, h):
    struct.pack_into('<HHHH', ram, a & 0x1FFFFF, x & 0xFFFF, y & 0xFFFF, w & 0xFFFF, h & 0xFFFF)


def mk_env(rng, ram, dbg=None, redir=False, table=False, limits=True, logger=True):
    fill_zone(ram, rng)
    ram[0x9574E] = dbg if dbg is not None else rng.pick((0, 0, 0, 0, 1, 1, 2, 3, 0xFF))
    if logger:
        put32(ram, 0x80095748, 0x80071A74 if rng.next() % 5 else SENT[8])
    if limits:
        put16(ram, 0x80095750, rng.pick(LIM) if rng.next() & 1 else rng.next() & 0xFFFF)
        put16(ram, 0x80095752, rng.pick(LIM) if rng.next() & 1 else rng.next() & 0xFFFF)
        if rng.next() % 3 == 0:
            put16(ram, 0x80095750, rng.pick((1024, 1024, 640, 512)))
            put16(ram, 0x80095752, rng.pick((512, 512, 480, 256)))
    if redir and rng.next() % 6 == 0:                     # hardware pointer words aimed at other register slots
        put32(ram, 0x80095850, 0x1F801820)
        put32(ram, 0x80095858, 0x1F8010B0)
        put32(ram, 0x8009585C, 0x1F8010B4)
        for a in (0x1820, 0x10B0, 0x10B4):
            put32(ram, a, word(rng))
    if table and rng.next() % 5 == 0:                     # libgpu function table elsewhere, sentinel slots
        put32(ram, 0x80095744, ZONE + 0x1800)
        for i in range(16):
            put32(ram, ZONE + 0x1800 + 4 * i, SENT[i])


def rets_all(rng, extra=()):
    r = {}
    for a in (0x80076C34, 0x80074E28, 0x80077404, 0x80077294, 0x80076B58, 0x80076B98, 0x80076664, 0x800768A0,
              0x80076354, 0x80073E10, 0x80073CF4, 0x80075EE0, 0x80071A34, 0x80071A74, 0x800773D0, 0x80073A44,
              0x8007C444) + tuple(SENT) + tuple(extra):
        k = rng.next() % 4
        r[a] = [rng.pick((0, 0, 1, 2, 0xFFFFFFFF, rng.next()))] if k < 2 else \
            [rng.pick((0, 0, 1, 0xFFFFFFFF, rng.next())) for _ in range(1 + rng.next() % 3)]
    # SetIntrMask (func_80073E10) returns the old I_MASK halfword: its domain is 16 bits (pe_plat_irq_set_mask is uint16_t)
    r[0x80073E10] = [x & 0xFFFF for x in r[0x80073E10]]
    return r


# ============================================================ gpu2b / gpu2c wrappers
def _wrapper_gen(nargs, rect_arg=0, data_arg=1, rets_pin=None):
    def gen(rng, k):
        rp = zptr(rng, 4)
        dp = zptr(rng, 4)
        x, y, w, h = [edge16(rng) for _ in range(4)]

        def setup(ram):
            mk_env(rng, ram, table=True)
            put_rect(ram, rp, x, y, w, h)
        rets = rets_all(rng)
        if rets_pin:
            rets[0x80074E28] = [rng.pick(rets_pin)]
        a = [rp, dp, 0, 0]
        return a, setup, rets, None
    return gen


spec('func_8007506C', 'gpu2b', 2, 600)(_wrapper_gen(2, rets_pin=(0, 0, 1, 2, 0xFFFFFFFF)))
spec('func_800750CC', 'gpu2b', 2, 600)(_wrapper_gen(2, rets_pin=(0, 0, 1, 2, 0xFFFFFFFF)))


@spec('func_80074E28', 'gpu2b', 2, 1200)
def gen_74e28(rng, k):
    rp = zptr(rng, 4)
    x, y, w, h = [edge16(rng) for _ in range(4)]
    if k % 4 == 0:
        x, y = rng.pick((0, 1, 1023, 1024, 0xFFFF, 0x8000)), rng.pick((0, 1, 511, 512, 0xFFFF, 0x8000))
    name = rng.pick((0x800118D4, 0x800118E0, 0x800118EC, 0x800118BC, 0x800118C8, 0x80011898, zptr(rng, 4)))

    def setup(ram):
        mk_env(rng, ram, dbg=rng.pick((0, 1, 1, 2, 3, 0xFF)))
        put_rect(ram, rp, x, y, w, h)
    return [name, rp, 0, 0], setup, rets_all(rng), None


@spec('func_8007512C', 'gpu2b', 3, 800)
def gen_7512c(rng, k):
    rp = zptr(rng, 4)
    x, y, w, h = [edge16(rng) for _ in range(4)]
    dx, dy = word(rng), word(rng)
    if k % 3 == 0:
        dx, dy = edge16(rng), edge16(rng)

    def setup(ram):
        mk_env(rng, ram, table=True)
        put_rect(ram, rp, x, y, w, h)
    return [rp, dx, dy, 0], setup, rets_all(rng, ()), None


@spec('func_80075358', 'gpu2c', 1, 800)
def gen_75358(rng, k):
    pk = zptr(rng, 4, 0x400)
    n = rng.pick((0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 63, 64, 127, 128, 255)) if rng.next() % 3 else rng.next() & 0xFF

    def setup(ram):
        mk_env(rng, ram, table=True)
        ram[(pk + 3) & 0x1FFFFF] = n
    return [pk, 0, 0, 0], setup, rets_all(rng), None


@spec('func_800753B4', 'gpu2c', 1, 600)
def gen_753b4(rng, k):
    ot = zptr(rng, 4, 0x400)

    def setup(ram):
        mk_env(rng, ram, table=True)
    return [ot, 0, 0, 0], setup, rets_all(rng), None


@spec('func_800752AC', 'gpu2c', 2, 800)
def gen_752ac(rng, k):
    ot = zptr(rng, 4, 0x100)
    n = rng.pick((0, 1, 2, 3, 4, 8, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x10000, 0xFFFF)) if rng.next() & 1 else word(rng)

    def setup(ram):
        mk_env(rng, ram, table=True)
        put32(ram, 0x8009580C, word(rng))
    return [ot, n, 0, 0], setup, rets_all(rng), None


@spec('func_80076354', 'gpu2c', 2, 800)
def gen_76354(rng, k):
    ot = zptr(rng, 4, 0x100)
    n = rng.pick((0, 1, 2, 3, 4, 8, 0x7FFF, 0xFFFF, 0x10000, 0xFFFFFF, 0x1000000, 0x80000000, 0xFFFFFFFF)) if rng.next() & 1 else word(rng)

    def setup(ram):
        mk_env(rng, ram)
        for a, v in ((0x80095870, 0x1F8010F0), (0x80095864, 0x1F8010E0), (0x80095868, 0x1F8010E4), (0x8009586C, 0x1F8010E8)):
            put32(ram, a, v)
        if rng.next() % 6 == 0:
            put32(ram, 0x80095864, 0x1F8010D0)          # DMA6 pointer words aimed elsewhere
            put32(ram, 0x80095868, 0x1F8010D4)
            put32(ram, 0x80095870, 0x1F8010F8)
            for a in (0x10D0, 0x10D4, 0x10F8):
                put32(ram, a, word(rng))
        put32(ram, 0x10F0, word(rng))
    model = {'chcr6': chcr_seq(rng, busy_end=rng.next() % 4 == 0)}
    r = rets_all(rng)
    r[0x80077404] = [rng.pick((0, 0, 0, 0, 1, 0xFFFFFFFF))] + ([1] if rng.next() % 3 else [])
    return [ot, n, 0, 0], setup, r, model


@spec('func_80076B58', 'gpu2c', 2, 800)
def gen_76b58(rng, k):
    p = zptr(rng, 4, 0x200)
    n = rng.pick((0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 63, 64, 100)) if rng.next() % 4 else rng.next() & 0x7F

    def setup(ram):
        mk_env(rng, ram, redir=True, limits=False)
    return [p, n, 0, 0], setup, None, None


@spec('func_8007A214', 'gpu2c', 2, 400)
def gen_7a214(rng, k):
    a, b = word(rng), word(rng)

    def setup(ram):
        fill_zone(ram, rng)
        for ad in (0x800C20C4, 0x800C0DC8, 0x800BE9EC, 0x800B89F4, 0x800A8018, 0x800A5D54, 0x800B0CD0):
            put32(ram, ad, word(rng))
    return [a, b, 0, 0], setup, rets_all(rng), None


@spec('func_8007A244', 'gpu2c', 2, 400)
def gen_7a244(rng, k):
    a, b = word(rng), word(rng)

    def setup(ram):
        fill_zone(ram, rng)
        put32(ram, 0x800C20C4, rng.pick((0, 1, 2, 3, 4, 0x10, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)) if rng.next() & 1 else word(rng))
        for ad in (0x800BE9EC, 0x800BE9E4, 0x800BE998, 0x800B89F4, 0x800B0CD0, 0x800A8018, 0x800A5D54):
            put32(ram, ad, word(rng))
    return [a, b, 0, 0], setup, rets_all(rng), None


@spec('func_8007C544', 'gpu2c', 3, 300)
def gen_7c544(rng, k):
    a, b, c = word(rng), word(rng), word(rng)

    def setup(ram):
        for ad in (0x800C0DC0, 0x800B6918, 0x800C0DBC):
            put32(ram, ad, word(rng))
    return [a, b, c, 0], setup, None, None


@spec('func_80074F44', 'gpu2c', 4, 800)
def gen_74f44(rng, k):
    rp = zptr(rng, 4)
    x, y, w, h = [edge16(rng) for _ in range(4)]
    cols = [rng.pick((0, 1, 0x7F, 0x80, 0xFE, 0xFF)) if rng.next() & 1 else rng.next() & 0xFF for _ in range(3)]

    def setup(ram):
        mk_env(rng, ram, table=True)
        put_rect(ram, rp, x, y, w, h)
    return [rp] + cols, setup, rets_all(rng), None


@spec('func_80075424', 'gpu2c', 1, 800)
def gen_75424(rng, k):
    env = zptr(rng, 4, 0x200)

    def setup(ram):
        mk_env(rng, ram, table=True)
    return [env, 0, 0, 0], setup, rets_all(rng), None


@spec('func_800754E4', 'gpu2c', 2, 800)
def gen_754e4(rng, k):
    env = zptr(rng, 4, 0x200)
    ot = zptr(rng, 4, 0x100)

    def setup(ram):
        mk_env(rng, ram, table=True)
    return [ot, env, 0, 0], setup, rets_all(rng), None



# ============================================================ gpu2e: func_80075EE0 (callee of PutDrawEnv / DrawOTagEnv)
# Added by the gpu2c author (scope addition, see docs/evidence/native-gpu2-20261126/gpu2c/AUTHOR-REPORT.md): the
# environment packet builder has no native provider; its black-box vectors are generated like every other gpu2 set.
def _env_fields(rng, ram, env):
    """A structured environment record: halfword rectangles with edge values, flag bytes, colour bytes."""
    put16(ram, env + 0x00, h16(rng)); put16(ram, env + 0x02, h16(rng))
    put16(ram, env + 0x04, h16(rng)); put16(ram, env + 0x06, h16(rng))
    put16(ram, env + 0x08, h16(rng)); put16(ram, env + 0x0A, h16(rng))
    for o in (0x0C, 0x0E, 0x10, 0x12):
        put16(ram, env + o, rng.pick((0, 0, 0, 8, 0x10, 0x20, 0xFF)) if rng.next() & 1 else h16(rng))
    put16(ram, env + 0x14, rng.pick((0, 0, 0x2A, 0x3F, 0x9F, 0x7F, 0x1FF)) if rng.next() & 1 else h16(rng))
    for o in (0x16, 0x17, 0x18):
        ram[(env + o) & 0x1FFFFF] = rng.pick((0, 1, 0, 1, 2, 0xFF))
    for o in (0x19, 0x1A, 0x1B):
        ram[(env + o) & 0x1FFFFF] = rng.pick((0, 0x7F, 0x80, 0xFF)) if rng.next() & 1 else rng.next() & 0xFF


@spec('func_80075EE0', 'gpu2e', 2, 800)
def gen_75ee0(rng, k):
    env = zptr(rng, 4, 0x200)
    pkt = env + 0x1C if rng.next() % 8 else zptr(rng, 4, 0x100)

    def setup(ram):
        mk_env(rng, ram, table=True)
        if k % 3:
            _env_fields(rng, ram, env)
    return [pkt, env, 0, 0], setup, rets_all(rng), None


# ============================================================ gpu2b: image workers
def _worker_gen(readback):
    def gen(rng, k):
        rp = zptr(rng, 4, 0x1000)
        dp = ZONE + 0x1000 + 4 * (rng.next() % 0x3F0 if readback else rng.next() % 0x100)
        x, y = rng.pick((0, 1, 5, 100, 1023, 1024, 0xFFFF, 0x8000)) if rng.next() & 1 else edge16(rng), \
            rng.pick((0, 1, 5, 100, 511, 512, 0xFFFF, 0x8000)) if rng.next() & 1 else edge16(rng)
        w = rng.pick((0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 34, 63, 64, 65, 0xFFFF, 0x8000, 0x7FFF, 1024, 1025)) \
            if rng.next() % 3 else rng.next() % 80
        h = rng.pick((0, 1, 2, 3, 4, 5, 7, 8, 16, 17, 0xFFFF, 0x8000, 512, 513)) if rng.next() % 3 else rng.next() % 12
        if k % 5 == 0:                       # clip boundary of the limits
            x, y = rng.pick((0, 1023, 1024, 640, 500)), rng.pick((0, 511, 512, 480, 300))

        def setup(ram):
            mk_env(rng, ram, redir=not readback)
            put_rect(ram, rp, x, y, w, h)
        ready = READY_POOL if not readback else (0x0C000000, 0x1C000000, 0x1E000000, 0x0C000001, 0xFFFFFFFF)
        model = {'stat': stat_seq(rng, ready), 'chcr': chcr_seq(rng, busy_end=rng.next() % 12 == 0),
                 'gp0': [rng.next() for _ in range(1 + rng.next() % 8)]}
        r = rets_all(rng)
        r[0x80077404] = [rng.pick((0, 0, 0, 0, 0, 1, 0xFFFFFFFF))] + ([1] if rng.next() % 2 else [])
        return [rp, dp, 0, 0], setup, r, model
    return gen


spec('func_80076664', 'gpu2b', 2, 1500)(_worker_gen(False))
spec('func_800768A0', 'gpu2b', 2, 1500)(_worker_gen(True))


# ============================================================ gpu2a: queue pump and its callees
# production queue entries hold these worker addresses (the ones func_80076C34 enqueues); the oracle stubs them like any callee
PUMP_WORKERS = (0x80076664, 0x800768A0, 0x80076434, 0x80076B98)

@spec('func_80076EE4', 'gpu2a', 0, 1500)
def gen_pump(rng, k):
    mode = rng.pick((0, 1, 2, 0xFFFFFFFF)) if rng.next() & 1 else word(rng)
    kind = rng.next() % 8
    if kind == 0:
        q = r = rng.next() % 64
    elif kind == 1:
        r = rng.pick((0, 1, 62, 63)); q = (r + 1) % 64
    elif kind == 2:                                  # wrap around the 64-entry ring
        r = rng.pick((60, 61, 62, 63)); q = rng.pick((0, 1, 2, 3))
    else:
        r = rng.pick((0, 1, 2, 5, rng.next() % 64)); q = (r + rng.pick((1, 1, 2, 3, 5, rng.next() % 64))) % 64
    hi = rng.pick((0, 0, 0, 0x40, 0x80, 0xFFFFFFC0)) if kind == 3 else 0

    def setup(ram):
        mk_env(rng, ram, limits=False, logger=False)
        put32(ram, 0x80095874, (q + hi) & 0xFFFFFFFF)
        put32(ram, 0x80095878, r)
        put32(ram, 0x80095880, word(rng))
        put32(ram, 0x80095754, rng.pick((0, 0, 0, 1, 2, 0x80000000, word(rng))))
        put32(ram, 0x80095758, rng.pick((0, 0, 0, 1, 0xFF, word(rng))))
        for i in range(64):
            e = 0x800BD030 + 0x60 * i
            put32(ram, e, rng.pick(PUMP_WORKERS) if rng.next() % 3 == 0 else SENT[rng.pick((0, 1, 2, 3))])
            put32(ram, e + 4, word(rng)); put32(ram, e + 8, word(rng)); put32(ram, e + 12, word(rng))
        if rng.next() % 6 == 0:                      # I_MASK pointer word aimed elsewhere
            put32(ram, 0x80095674, 0x1F801078)
            put16(ram, 0x1078, rng.next() & 0xFFFF)
    model = {'stat': [rng.pick(STAT_POOL) for _ in range(rng.next() % 5)] + [rng.pick(READY_POOL)],
             'chcr': [rng.pick((0, 0, 0x01000000, 0xFFFFFFFF, 0x00FFFFFF)) for _ in range(rng.next() % 4)] + [0]}
    rt = rets_all(rng)
    return [mode, 0, 0, 0], setup, rt, model


@spec('func_800773D0', 'gpu2a', 0, 300)
def gen_773d0(rng, k):
    def setup(ram):
        fill_zone(ram, rng)
        put32(ram, 0x80095888, word(rng)); put32(ram, 0x8009588C, word(rng))
    r = {0x80073A44: [rng.pick((0, 1, 0xEF, 0xF0, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0xFFFFFF10)) if rng.next() & 1 else rng.next()]}
    return [0, 0, 0, 0], setup, r, None


# ============================================================ gpu2d: libpress callbacks
@spec('func_8010C0D8', 'gpu2d', 1, 400)
def gen_c0d8(rng, k):
    cb = rng.pick((0, 1, 0x80130000, 0x80130040, 0x8010C0D8, 0xFFFFFFFF, 0x80000000)) if rng.next() & 1 else word(rng)

    def setup(ram):
        fill_zone(ram, rng)
    return [cb, 0, 0, 0], setup, rets_all(rng), None


@spec('func_8010BE3C', 'gpu2d', 1, 400)
def gen_be3c(rng, k):
    mode = rng.pick((0, 1, 2, 3, 4, 0xFF, 0x100, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF)) if rng.next() % 3 else word(rng)

    def setup(ram):
        fill_zone(ram, rng)
    r = rets_all(rng, extra=(0x80073C94, 0x8010C0FC))
    return [mode, 0, 0, 0], setup, r, None


# per-function case counts, tuned so every committed batch file stays under 2 MB
COUNTS = {'func_80076EE4': 1000, 'func_800773D0': 300,
          'func_8007506C': 500, 'func_800750CC': 500, 'func_80074E28': 1000, 'func_8007512C': 700,
          'func_80076664': 800, 'func_800768A0': 700,
          'func_80071944': 500, 'func_80071964': 500, 'func_80071994': 500, 'func_80077A64': 2000, 'func_80077AA4': 1200,
          'func_80075358': 500, 'func_800753B4': 400, 'func_800752AC': 500, 'func_80076354': 500, 'func_80076B58': 150,
          'func_8007A214': 400, 'func_8007A244': 400, 'func_8007C544': 300, 'func_80074F44': 500, 'func_80075424': 500,
          'func_800754E4': 500, 'func_8010C0D8': 300, 'func_8010BE3C': 300, 'func_80075EE0': 800}


def ovl_blob():
    import hashlib
    data = open(C.ROOT / OVL_BIN, 'rb').read()
    assert hashlib.sha1(data).hexdigest() == OVL_SHA1, 'ovl_039F.bin sha1 mismatch'
    return data


def build(funcs, out_path):
    base = C.fixture()
    base_ovl = C.fixture((ovl_blob(), OVL_VRAM)) if any(f in OVL for f in funcs) else None
    lines = []
    stats = []
    for name in funcs:
        gen, nargs, count = SPECS[name]
        import os
        count = int(os.environ.get('G2_COUNT', COUNTS.get(name, count)))
        if name in OVL:
            off, size, entry = C.func_range(name, OVL_YAML, OVL_VRAM)
        else:
            off, size, entry = C.func_range(name)
        n = dropped = 0
        for k in range(count):
            seed = (entry & 0xFFFF) * 0x1000 + k
            rng = Rng(seed)
            args, setup, rets, model = gen(rng, k)
            r = C.case_run(base_ovl if name in OVL else base, setup, entry, size, args, nargs, rets, model, gseed=seed)
            if r is None:
                dropped += 1
                continue
            ev, ret, seen = r
            lines.append(C.emit2(entry, seed, args, nargs, seen, ret, ev))
            n += 1
        stats.append((name, n, dropped))
        print('%s: %d cases, %d dropped' % (name, n, dropped), flush=True)
    open(out_path, 'w').write('\n'.join(lines) + '\n')
    return stats



# ============================================================ gpu2d: func_8010BD4C table expander
# Input is the retail table in ovl_039F (file offsets 0xF04.. = vram 0x8010CBFC..0x8011ECF8): hashed, never copied.
# A case = byte patches over that table + a destination; the output is every RAM byte the original writes.
EXP_TABLE = (0x8010CBFC, 0x8011ECF8)
EXP_OUT = 0x11000
EXP_BUDGET = 1_000_000


def rle_hex(b):
    out, i, n = [], 0, len(b)
    while i < n:
        j = i
        while j < n and b[j] == 0:
            j += 1
        if j - i >= 6:
            out.append('z%X' % (j - i))
            i = j
            continue
        k = i
        while k < n and not (k + 6 <= n and not any(b[k:k + 6])):
            k += 1
        out.append(b[i:k].hex())
        i = k
    return ','.join(out)


def expand_one(base, entry, size, dst, patches, reads=None):
    ram = bytearray(base)
    for off, val in patches:
        ram[(EXP_TABLE[0] + off) & 0x1FFFFF] = val
    stores = {}
    bad = []
    old_logged, old_store, old_load = R.logged, R._ns['STORE'], R._ns['LOAD']

    def st(fmt, buf, a, val):
        n = struct.calcsize(fmt)
        for i in range(n):
            stores[a + i] = (val >> (8 * i)) & 0xFF
        return old_store(fmt, buf, a, val)

    def ld(fmt, buf, a):
        n = struct.calcsize(fmt)
        lo, hi = EXP_TABLE[0] & 0x1FFFFF, EXP_TABLE[1] & 0x1FFFFF
        d0 = dst & 0x1FFFFF
        if reads is not None:
            reads.update(range(a, a + n))
        if not ((lo <= a and a + n <= hi) or (d0 <= a and a + n <= d0 + EXP_OUT)):
            bad.append(a)
        return old_load(fmt, buf, a)
    R.logged = lambda a: False
    R._ns['STORE'], R._ns['LOAD'] = st, ld
    try:
        stp = C.RangeStops(entry, entry + size, ())
        try:
            r = R.execute(ram, entry, (), stop_at=stp, initial_regs={4: dst, 5: 0, 31: C.RET}, instruction_budget=EXP_BUDGET)
        except Exception:
            return None
    finally:
        R.logged, R._ns['STORE'], R._ns['LOAD'] = old_logged, old_store, old_load
    if stp.hit != C.RET or bad:
        return None
    d0 = dst & 0x1FFFFF
    if any(not (d0 <= a < d0 + EXP_OUT) for a in stores):
        return None
    return r[2] & 0xFFFFFFFF, stores


def build_expand(out_path, target=13, shard=0, nshards=1, attempts=700):
    """One shard of the expander vectors: cases k = shard, shard+nshards, ... (k < 10 are unpatched-table cases for the
    ten destinations; the rest patch 1..6 table bytes).  Patched tables that make the original spin or write outside
    [dst, dst+0x11000) are not defined cases and are dropped.  Writes OUT.shard<N> (shard 0 also the header and the
    local-only base output OUT.base.bin)."""
    import hashlib
    blob = ovl_blob()
    base = C.fixture((blob, OVL_VRAM))
    off, size, entry = C.func_range('func_8010BD4C', OVL_YAML, OVL_VRAM)
    tsha = hashlib.sha256(blob[EXP_TABLE[0] - OVL_VRAM:EXP_TABLE[1] - OVL_VRAM]).hexdigest()
    lines = []
    reads = set()
    res0 = expand_one(base, entry, size, 0x80140000, [], reads)
    assert res0 is not None
    d00 = 0x80140000 & 0x1FFFFF
    base_out = bytes(res0[1][d00 + i] for i in range(EXP_OUT))
    lo, hi = EXP_TABLE[0] & 0x1FFFFF, EXP_TABLE[1] & 0x1FFFFF
    read_offs = sorted(a - lo for a in reads if lo <= a < hi)
    if shard == 0:
        open(out_path + '.base.bin', 'wb').write(base_out)
        lines.append('# 8010BD4C expander vectors; table range %08X..%08X sha256 %s (retail data, not stored here)' %
                     (EXP_TABLE[0], EXP_TABLE[1], tsha))
        lines.append('# the original reads %d table bytes, offsets %X..%X from %08X (table range is %X bytes)' %
                     (len(read_offs), read_offs[0], read_offs[-1], EXP_TABLE[0], hi - lo))
        lines.append('# base output (unpatched table) sha256 %s, stored locally only: %s.base.bin' %
                     (hashlib.sha256(base_out).hexdigest(), out_path))
    dsts = (0x80140000, 0x80150000, 0x80180000, 0x801A0000, 0xA0140000, 0x80121000, 0x80140001, 0x80140002, 0x80140003,
            0x80160040)
    n = dropped = 0
    k = shard
    while n < target and k < attempts:
        seed = 0x8010BD4C + k
        rng = Rng(seed)
        dst = dsts[k % len(dsts)] if k < 20 else rng.pick(dsts)
        patches = []
        if k >= 10:
            for _ in range(1 + rng.next() % 6):
                o = rng.pick(read_offs) if rng.next() % 4 else rng.next() % (hi - lo)
                patches.append((o, rng.pick((0, 1, 2, 3, 7, 0x7F, 0x80, 0xEF, 0xF1, 0xFF)) if rng.next() & 1 else rng.next() & 0xFF))
        k += nshards
        res = expand_one(base, entry, size, dst, patches)
        if res is None:
            dropped += 1
            continue
        r, stores = res
        d0 = dst & 0x1FFFFF
        if len(stores) != EXP_OUT or min(stores) != d0:
            dropped += 1
            continue
        buf = bytes(stores[d0 + i] for i in range(EXP_OUT))
        ranges, i = [], 0
        while i < EXP_OUT:
            if buf[i] == base_out[i]:
                i += 1
                continue
            j = i
            while j + 1 < EXP_OUT and (buf[j + 1] != base_out[j + 1] or
                                       next((g for g in range(1, 16) if j + 1 + g >= EXP_OUT or buf[j + 1 + g] != base_out[j + 1 + g]), 16) < 16):
                j += 1
            ranges.append('%X:%X:%s' % (i, j - i + 1, rle_hex(buf[i:j + 1])))
            i = j + 1
        lines.append('8010BD4C %d X %X PATCH %s RET %X SHA %s DELTA %s' % (
            seed, dst, ' '.join('%X=%X' % p for p in patches) or '-', r, hashlib.sha256(buf).hexdigest(),
            ' '.join(ranges) or '-'))
        n += 1
        print('shard %d expand case %d: dst %08X patches %d delta ranges %d' % (shard, n, dst, len(patches), len(ranges)), flush=True)
    open('%s.shard%d' % (out_path, shard), 'w').write('\n'.join(lines) + '\n')
    print('shard %d func_8010BD4C: %d cases, %d dropped' % (shard, n, dropped))


def main():
    out = sys.argv[1]
    funcs = []
    if sys.argv[2:3] == ['expand']:
        shard, nshards, target = (int(x) for x in sys.argv[3:6]) if len(sys.argv) > 5 else (0, 1, 96)
        build_expand(out, target, shard, nshards)
        return
    for a in sys.argv[2:]:
        funcs += BATCHES[a] if a in BATCHES else [a]
    build(funcs, out)


if __name__ == '__main__':
    main()

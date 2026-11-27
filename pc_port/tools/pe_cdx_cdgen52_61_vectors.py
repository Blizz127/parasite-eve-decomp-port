#!/usr/bin/env python3
"""cdgen52..cdgen61: black-box vectors for the CD providers named by the whole-binary audit (docs/evidence/public-openness-20261127/REPORT-2.md).
The ORIGINAL code runs under the test-only interpreter (pe_cdx_vectors.run_case); nothing here reads a provider.  This driver reuses the cdgen47
generator (pe_cdx_cdhost_vectors.gen_entry: seeded base states, the state-word explorer, the CD controller / register models, per-call callee
return sequences, own-written-load rule) and adds, per set, the entry specs, targeted callee-return sequences and register-read sequences.

Sets (the stopped callees of each entry are the retail functions its current provider reaches through a bound host symbol, from `nm -u`):
  cdgen52  libcd data sync 0x8007BDDC and its two forwarding ABIs 0x8007A910 / 0x80080B24 (the three retail entries are ONE bound symbol, so the
           forwarding sets run 0x8007BDDC inline: a provider exact on them implements the whole body for all three)
  cdgen54..61: see SETS below.
Usage: pe_cdx_cdgen52_61_vectors.py --set cdgenNN OUTDIR [-n N]      writes OUTDIR/<file>.txt (+ .classes) per entry of the set and a stats line"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_cdx_cdhost_vectors as g   # noqa: E402  (shared interpreter hooks, run_one / gen_entry, REGSEQ, make_models; registers the cdgen47 specs)

m, c, gv, pl, v = g.m, g.c, g.gv, g.pl, g.v
M32 = 0xFFFFFFFF
VSYNC, PUTS, PRINTF, FLUSH = 0x80073A44, 0x80073C5C, 0x80071A74, 0x8007B9EC
OVERRIDES = {}      # entry -> function(x, sp, rets, models) -> x   (targeted callee return sequences)
HW = {}             # entry -> function(ram, x, args) -> x            (register read sequences)
SETS = {}           # set name -> list of (file, entry, name, kinds, callees, kwargs)
N_DEFAULT = {}      # set name -> cases per entry

_orig_arg47 = g.arg47
STATUS_EDGE = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0x20, 0x80, 0xFF, 0x100, M32, 0x7FFFFFFF, 0x80000000)


def arg47(k, x):
    """extra argument kinds: 'q' = a never-null record-zone pointer (a null pointer makes the original touch low RAM, which the faithful-only rule drops);
    's' = a status / small-code edge value."""
    if k == 'q':
        return g.ZONE + (((x >> 4) % 0x400) * 4)
    if k == 's':
        return (2, 5)[(x >> 6) & 1] if x & 1 else (STATUS_EDGE[(x >> 8) % len(STATUS_EDGE)] if x & 2 else (x >> 4) & 0x1F)   # statuses 2 and 5 are the handled ones (scan 0..0x100): half the cases
    if k == 'k':       # drive command of 0x8007F0C8: 3, 0x15, 0x16, 0x1B reach the deep arms (scan over 0..0x1C and edge values); the rest of the cases keep the generator's values
        return (3, 0x15, 0x16, 0x1B)[(x >> 6) & 3] if (x >> 9) % 10 < 7 else _orig_arg47('w', x)
    return _orig_arg47(k, x)


g.arg47 = arg47
_orig_make_models = g.make_models
_orig_entry_hw = g.entry_hw


def make_models(entry, sp, x, rets, cbs):
    x, models = _orig_make_models(entry, sp, x, rets, cbs)
    ov = OVERRIDES.get(entry)
    if ov:
        x = ov(x, sp, rets, models)
    return x, models


def entry_hw(entry, ram, x, args):
    hw = HW.get(entry)
    if hw:
        return hw(ram, x, args)
    return _orig_entry_hw(entry, ram, x, args)


g.make_models = make_models
g.entry_hw = entry_hw


KEEP_MIXED_STACK = [False]


def install_keep_mixed_stack():
    """Supplementary variant (cdgen55b): a load from the original's stack window that mixes own-written and external bytes (a partly initialised local)
    is NOT dropped; it is logged as the single external bytes (their values are in the vector's loads, so the checker seeds them), like the main-RAM
    CDX_KEEP_MIXED_OWN_LOADS mode.  The default rule (drop the case) stays in force for every other set."""
    prev = pl._ns['LOAD']

    def load(fmt, buf, a):
        n = struct.calcsize(fmt)
        mixed = False
        if m.STACKW[0] <= a < m.STACKW[1] and not getattr(c, 'NO_STACK_LOADS', False):
            own = [(a + i) in m._OWN for i in range(n)]
            mixed = any(own) and not all(own)
        pre = c.UNFAITHFUL[0]
        val = prev(fmt, buf, a)
        if mixed and c.UNFAITHFUL[0] and not pre:
            c.UNFAITHFUL[0] = False
            raw = val[0]
            for i in range(n):
                if not own[i]:
                    pl.LOADS.append((0x80000000 + a + i, 1, (raw >> (8 * i)) & 0xFF))
        return val
    pl._ns['LOAD'] = load


def put_seq(rets, models, fn, seq):
    rets[m.canon(fn)] = list(seq)
    models[fn] = g.seq_model(seq)


# ----------------------------------------------------------------------------------------------------- cdgen52
def sync_override(x, sp, rets, models):
    """VSync(-1): the first value N0 stamps the deadline N0 + 0x3C0; each loop pass asks again (N1 ...): sweep N1 across the signed deadline boundary
    (-2 .. +2 around it, wrapped bases, extreme offsets); at most 8 values (the checker's per-callee sequence capacity), the last repeats."""
    x = gv.xs32(x)
    n0 = (0, 1, 0x100, 0x7FFFFFF0, 0xFFFFFC00, 0xFFFFFFFF, 0x80000000, gv.xs32(x + 7))[(x >> 4) % 8]
    x = gv.xs32(x)
    d = (-2000, -961, -2, -1, 0, 1, 2, 3, 100, 0x7FFFFFFF, -0x80000000, 0x40000000)[(x >> 4) % 12]
    n1 = (n0 + 0x3C0 + d) & M32
    x = gv.xs32(x)
    step = (0, 0, 1, 1, 7, 0x100, 0x3C0)[(x >> 4) % 7]
    seq = [n0, n1] + [(n1 + (i + 1) * step) & M32 for i in range(5)]
    put_seq(rets, models, VSYNC, seq)
    return x


def sync_hw(ram, x, args):
    """DMA3 channel control (0x1F8010B8, read through the pointer word 0x8009B2C0): k busy reads (bit 24) then an idle value."""
    x = gv.xs32(x)
    k = (0, 0, 0, 1, 2, 3, 7, 20, 40)[(x >> 4) % 9]
    busy = (0x01000000, 0x11000000, 0x01000200, 0xFFFFFFFF, 0x01FFFFFF)[(x >> 10) % 5]
    idle = (0, 0x00000200, 0x10000000, 0xFEFFFFFF, 0x00FFFFFF)[(x >> 14) % 5]
    g.REGSEQ[0x10B8] = [4, [busy] * k + [idle], 0]
    g.WAIT[0] = k > 0
    return x


OVERRIDES[0x8007BDDC] = OVERRIDES[0x8007A910] = OVERRIDES[0x80080B24] = sync_override
HW[0x8007BDDC] = HW[0x8007A910] = HW[0x80080B24] = sync_hw
_SYNC_CALLEES = {VSYNC: 1, PUTS: 1, PRINTF: 4, FLUSH: 0}
SETS['cdgen52'] = [
    ('sync', 0x8007BDDC, 'cd_data_sync', 'w', dict(_SYNC_CALLEES), dict(printf_fmt=True, n=600)),
    ('fwd_A910', 0x8007A910, 'cd_data_sync_fwd_A910', 'w', dict(_SYNC_CALLEES), dict(printf_fmt=True, inline=(0x8007BDDC,), n=600)),
    ('fwd_0B24', 0x80080B24, 'cd_data_sync_fwd_0B24', 'w', dict(_SYNC_CALLEES), dict(printf_fmt=True, inline=(0x8007BDDC,), n=600)),
]


# ----------------------------------------------------------------------------------------------------- cdgen53
H16 = (0, 1, 2, 31, 32, 63, 64, 65, 127, 128, 129, 239, 240, 241, 255, 256, 257, 319, 320, 321, 511, 512, 513, 1023, 1024, 0x7FFF, 0x8000, 0x8001, 0xFFFF, 0xFFFE, 0xFFC0)


def clear_override(x, sp, rets, models):
    """GP1 get-info results (the original asks 0x80076BE0 with 3 / 4 / 5 for the drawing-area top-left, bottom-right and offset): full 32-bit values
    with high bits set (the masks 0xFFFFF / 0xFFFFF / 0x3FFFFF are observable)."""
    x = gv.xs32(x)
    vals = []
    for i in range(3):
        x = gv.xs32(x)
        vals.append((0, 0x1111, 0x00F00001, 0xFFFFFFFF, 0xFF0FFFFF, 0x00300003, 0x7FFFFFFF, 0x80000000, gv.xs32(x))[(x >> 4) % 9])
    put_seq(rets, models, 0x80076BE0, vals)
    put_seq(rets, models, 0x80076B98, [(0, 1, M32)[(x >> 20) % 3]])
    return x


def clear_hw(ram, x, args):
    """The RECT at the first argument (x, y, w, h halfwords), the screen limit halfwords 0x80095750 / 52, the GPU status register read (pointer word 0x80095854)."""
    rect = args[0] & 0x1FFFFF
    x = gv.xs32(x)
    xv = (H16[(x >> 3) % len(H16)], H16[(x >> 8) % len(H16)], 0, 64, 128, 3, 63)[(x >> 13) % 7] if (x >> 20) % 3 else (0, 64, 128, 192, 256, 320)[(x >> 3) % 6]
    x = gv.xs32(x)
    yv = H16[(x >> 3) % len(H16)] if (x >> 12) % 3 else (0, 1, 8, 100)[(x >> 3) % 4]
    x = gv.xs32(x)
    wv = H16[(x >> 3) % len(H16)] if (x >> 12) % 4 else (64, 128, 256, 320, 63, 65)[(x >> 3) % 6]
    x = gv.xs32(x)
    hv = H16[(x >> 3) % len(H16)] if (x >> 12) % 4 else (1, 16, 64, 240, 239)[(x >> 3) % 5]
    if rect and rect + 8 <= len(ram):
        struct.pack_into('<4H', ram, rect, xv, yv, wv, hv)
    x = gv.xs32(x)
    limw = (320, 320, 256, 512, 640, 1024, 0, 1, 0x7FFF, 0x8000, 65, 64)[(x >> 3) % 12]
    limh = (240, 240, 256, 480, 512, 0, 1, 0x7FFF, 0x8000, 17)[(x >> 9) % 10]
    struct.pack_into('<HH', ram, 0x95750, limw, limh)
    x = gv.xs32(x)
    st = (0, 0x14802000, 0xFFFFFFFF, 0x1C000000, 0x000007FF, 0x00000400, 0x000003FF, 0x14800000, gv.xs32(x))[(x >> 3) % 9]
    g.REGSEQ[0x1814] = [4, [st], 0]
    return x


OVERRIDES[0x80076434] = clear_override
HW[0x80076434] = clear_hw
SETS['cdgen53'] = [
    ('clear', 0x80076434, 'gpu_clear_worker', 'pw', {0x80076BE0: 1, 0x80076B98: 1}, dict(n=1500)),
]


# ----------------------------------------------------------------------------------------------------- cdgen54..61
# one single-function set per undecided CD provider; stopped callees = the retail functions the provider reaches through a bound host symbol (nm -u), arities from
# the pe_plat prototypes; retail callees the provider does not bind run inline.
CBS = {0x80131140: 2, 0x80131180: 2, 0x801311C0: 2}      # the CD data-callback sentinels (callback words 0x8009AFB4 / 0x8009AFB8 / 0x800B0CCC), as cdgen47
def queue_override(x, sp, rets, models):
    """Command-queue helpers: the next-slot entry returns a record-zone slot pointer (the original stores through it), the status lane a small code, the position
    converter an edge integer, the text helper a flag."""
    x = gv.xs32(x)
    slot = g.ZONE + ((x >> 6) % 0x300) * 4
    put_seq(rets, models, 0x8007E6B0, [slot, slot + 0x24])
    x = gv.xs32(x)
    put_seq(rets, models, 0x8007FBF0, [(0, 1, 2, 3, 0x10, M32, 0x80, 5)[(x >> 4) % 8]] * 2)
    x = gv.xs32(x)
    put_seq(rets, models, 0x8007E8F4, [(0, 1, 2, M32)[(x >> 4) % 4]])
    x = gv.xs32(x)
    pos = (0, 1, 2, 0x10, 0x100, 0x1000, 0x4B000, 0x7FFFFFFF, M32, 0x80000000, (x >> 12) & 0x7FFFF)[(x >> 4) % 11]
    put_seq(rets, models, 0x80080C48, [pos, pos])
    return x


OVERRIDES[0x8007EE84] = queue_override
OVERRIDES[0x8007F0C8] = queue_override
SETS['cdgen54'] = [('query', 0x8007F418, 'cd_response_query', 'wp', {0x8007FC64: 1, 0x80080998: 2}, dict(n=800))]
SETS['cdgen55'] = [('sequence', 0x8007F0C8, 'cd_command_sequence', 'wqwww', {0x80080C48: 1, 0x8007E6B0: 0, 0x8007FBF0: 1, 0x8007E8F4: 0}, dict(inline=(0x80080950,), n=1000))]
SETS['cdgen56'] = [('queue', 0x8007EE84, 'cd_queue_command', 'wqww', {0x8007E6B0: 0, 0x80080950: 2, 0x8007FBF0: 1, 0x8007E8F4: 0}, dict(n=1000))]
SETS['cdgen57'] = [('open', 0x80081314, 'cd_read_stream_open', 'pw', {0x8007F0C8: 5, 0x800824F0: 1, 0x800824C8: 1}, dict(n=800))]
SETS['cdgen58'] = [('issue', 0x8007FCFC, 'cd_issue_wrapper', 'ww', {0x8007B9EC: 0, 0x80080950: 2, 0x8007B558: 4}, dict(n=800))]
SETS['cdgen59'] = [('init', 0x8007F994, 'cd_initialize_callbacks', '', {0x8007BBFC: 0, 0x8007BAC0: 0, 0x8007FA2C: 0, 0x800812F4: 1, 0x80073D58: 2}, dict(n=400))]
SETS['cdgen60'] = [('status', 0x8008068C, 'cd_unexpected_status', 'sq', {**CBS, 0x80080778: 2, 0x80080164: 2}, dict(n=800))]
def ready_hw(ram, x, args):
    """The data-ready callback chain word 0x800B0CC8: none, or a stopped sentinel (the original jumps through it with three argument registers)."""
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0xB0CC8, (0, 0, 0x801311C0, 0x80131140, 0x80131180)[(x >> 4) % 5])
    return x


HW[0x8007C214] = ready_hw
SETS['cdgen61'] = [('ready', 0x8007C214, 'cd_data_ready_callback', '', {0x80131140: 3, 0x80131180: 3, 0x801311C0: 3}, dict(n=800))]


SETS['cdgen55b'] = [('sequence_deep', 0x8007F0C8, 'cd_command_sequence', 'wqkww', {0x80080C48: 1, 0x8007E6B0: 0, 0x8007FBF0: 1, 0x8007E8F4: 0}, dict(inline=(0x80080950,), n=1500))]
N_KEEP_STACK = ('cdgen55b',)


def status_dispatch_hw(ram, x, args):
    """cdgen60b: the original is func_8008068C(status & 0xFF).  Status 5: when bit 4 of the byte at 0x8009B56C is set it stores two state words and, when the
    callback word 0x800A36A4 and the word at 0x8009B554 are both nonzero, calls the callback with 5 (the dispatch arm); status 2: the byte at 0x8009B558 == 0xE
    and bit 7 of byte +1 differing from byte +0x29 stores three state words.  Seed exactly those words."""
    if CUR[0] not in ('cdgen60b', 'cdgen60c'):
        return _orig_entry_hw(0x8008068C, ram, x, args)
    x = gv.xs32(x)
    ram[0x9B56C] = (0x10, 0x10, 0x10, 0x00, 0x1F, 0xEF, 0x30, 0xFF)[(x >> 4) % 8]
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B554, (0, 1, 1, 2, M32, 0x80000000)[(x >> 4) % 6])
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0xA36A4, (0, 0x801311C0, 0x80131140, 0x80131180, 0x80080778, 0x80080164, 0x801311C0)[(x >> 4) % 7])
    x = gv.xs32(x)
    ram[0x9B558] = (0xE, 0xE, 0xE, 0, 1, 0xD, 0xFF)[(x >> 4) % 7]
    ram[0x9B559] = (0x00, 0x80, 0x7F, 0xFF)[(x >> 10) % 4]
    ram[0x9B558 + 0x29] = (0x00, 0x80, 0x7F, 0xFF)[(x >> 14) % 4]
    return x


HW[0x8008068C] = status_dispatch_hw
SETS['cdgen60b'] = [('status_dispatch', 0x8008068C, 'cd_unexpected_status', 's', {0x80131140: 1, 0x80131180: 1, 0x801311C0: 1, 0x80080778: 1, 0x80080164: 1}, dict(n=1200))]


# ----------------------------------------------------------------------------------------------------- supplementary variants 54b / 56b / 58b
CUR = [None]      # the set being generated (the entry hooks below act only for their own variant; every other set keeps the earlier behaviour)
_p54 = g.arg47


def _slot_addr(k):
    return (0x800A3610 + 16 * k) & M32


def hw54(ram, x, args):
    """cdgen54b: the matched response-ring entry gets nonzero, distinct words +4 / +8 / +12 (word +4 with bytes 1..3 nonzero, low byte zero or not); the
    scanned slot is planted along the scan order (index word 0x800A3690, stride 16, eight slots, signed wrap rules), for sequence 0 the previous slot."""
    if CUR[0] not in ('cdgen54b', 'cdgen54c'):
        return _orig_entry_hw(0x8007F418, ram, x, args)
    x = gv.xs32(x)
    idx = (0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 9, 12, M32, 0xFFFFFFFD)[(x >> 4) % 14]       # includes idx > 7 and negative idx
    struct.pack_into('<I', ram, 0xA3690, idx)
    sidx = idx - (1 << 32) if idx & 0x80000000 else idx
    x = gv.xs32(x)
    if (x >> 20) % 5 == 0:
        args[0] = 0                  # sequence 0: the scan starts at the previous slot
    seq = args[0]
    if seq == 0:
        k = sidx - 1 if sidx - 1 >= 0 else 7
        slots = [k]
    else:
        slots, k = [], sidx
        for i in range(8):
            slots.append(k)
            k += 1
            if not k < 8:
                k = 0
        j = (x >> 8) % 8
        slots = [slots[j]]
    k = slots[0]
    a = _slot_addr(k)
    if not (0x80000000 <= a < 0x80200000 - 16):
        return x
    off = a & 0x1FFFFF
    ST54C = None
    if CUR[0] == 'cdgen54c':
        # cdgen54c: nine distinct nonzero bytes: the status byte (entry byte +4) and the payload bytes 5..12; the response pointer overlaps the record in 85 % of the cases
        bs, y = [], x
        while len(bs) < 9:
            y = gv.xs32(y)
            b_ = (y >> 8) & 0xFF
            if b_ and b_ not in bs:
                bs.append(b_)
        if (x >> 24) % 3 == 0:
            bs[0], bs[1] = 0x02, 0xEA
        ST54C = bs
        if (x >> 28) % 20 < 17:
            args[1] = (0x800A3504, 0x800A3500, 0x800A3508, 0x800A3503, 0x800A3505, 0x800A350C)[(x >> 4) % 6]
    w4 = (0x00000102, 0x11223302, 0xFFFFFF05, 0x00000100, 0x11223300, 0xFFFFFF00, 0x02, 0x05, 0x7F000081, 0x01010101, 0x80000000, 0x00000200)[(x >> 12) % 12]
    x = gv.xs32(x)
    w8 = (x | 1) & M32
    x = gv.xs32(x)
    w12 = ((x ^ 0xA5A5A5A5) | 0x10000) & M32
    if ST54C:
        w4 = ST54C[0] | ST54C[1] << 8 | ST54C[2] << 16 | ST54C[3] << 24
        w8 = ST54C[4] | ST54C[5] << 8 | ST54C[6] << 16 | ST54C[7] << 24
        w12 = ST54C[8] | ((x >> 3) & 0xFFFFFF) << 8
    if seq != 0:
        struct.pack_into('<I', ram, off, seq)
    elif struct.unpack_from('<I', ram, off)[0] == 0:
        struct.pack_into('<I', ram, off, (x >> 4 | 1) & M32)
    struct.pack_into('<III', ram, off + 4, w4, w8, w12)
    return x


def hw56(ram, x, args):
    """cdgen56b: parameter 0 in a third of the cases, queue count word 0x800A3608 over 0..9 and negative values, serial word 0x8009B53C around 0 / -1 /
    0x7FFFFFFF, command-table word zero / nonzero, status-lane word 0x8009B574 0 / 1 / 2, and (half the cases) a queue entry whose serial matches the one
    just assigned so the text helper is reachable."""
    if CUR[0] != 'cdgen56b':
        return _orig_entry_hw(0x8007EE84, ram, x, args)
    x = gv.xs32(x)
    if (x >> 4) % 3 == 0:
        args[1] = 0
    x = gv.xs32(x)
    cnt = (0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 9, M32, 0xFFFFFFF8, 0x80000000, 0x7FFFFFFF)[(x >> 4) % 15]
    struct.pack_into('<I', ram, 0xA3608, cnt)
    x = gv.xs32(x)
    ser = (0, 0, M32, M32, 0xFFFFFFFE, 0x7FFFFFFF, 0x80000000, 1, 0x1234, (x >> 8) & M32)[(x >> 4) % 10]
    struct.pack_into('<I', ram, 0x9B53C, ser)
    x = gv.xs32(x)
    tbl = (0, 1, 0x80000000, M32, (x >> 8) | 1)[(x >> 4) % 5]
    if (x >> 12) % 5 < 3:
        tbl = tbl or 1
    cmd = args[0] & 0xFF
    struct.pack_into('<I', ram, (0x9B4BC + 4 * cmd) & 0x1FFFFF, tbl)
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B574, (0, 1, 2, 1)[(x >> 4) % 4])
    x = gv.xs32(x)
    inc = lambda v: ((v + 1) & M32) if ((v + 1) & M32) else ((v + 2) & M32)
    s1 = inc(ser)
    path1 = tbl != 0 and args[1] != 0
    s2 = inc(s1) if path1 else s1
    idx = (0, 1, 2, 3, 4, 5, 6, 7, 7, M32, 8)[(x >> 4) % 11]
    struct.pack_into('<I', ram, 0xA3604, idx)
    sidx = idx - (1 << 32) if idx & 0x80000000 else idx
    ea = (0x800A3540 + 24 * sidx) & M32
    if (x >> 12) % 2 == 0 and 0x80000000 <= ea < 0x80200000 - 4:
        want = (s1, s2, s1, (x >> 14) | 1)[(x >> 16) % 4]
        struct.pack_into('<I', ram, ea & 0x1FFFFF, want & M32)
    return x


def ov56(x, sp, rets, models):
    if CUR[0] != 'cdgen56b':
        return queue_override(x, sp, rets, models)
    x = gv.xs32(x)
    slot = g.ZONE + ((x >> 6) % 0x300) * 4
    seq = [slot, slot + 0x24]
    put_seq(rets, models, 0x8007E6B0, seq)
    x = gv.xs32(x)
    lane = (1, 1, 1, 0, 2, 0x80000000, 5, 0x10, M32)[(x >> 4) % 9]        # the status lane scripted value: 1 is the one that continues
    put_seq(rets, models, 0x8007FBF0, [lane] * 2)
    x = gv.xs32(x)
    put_seq(rets, models, 0x8007E8F4, [(0, 1, 2, M32)[(x >> 4) % 4]])
    return x


_hw_old = dict(HW)
HW[0x8007F418] = hw54
HW[0x8007EE84] = hw56
OVERRIDES[0x8007EE84] = ov56
_arg_null = g.arg47


def arg47b(k, x):
    if k == 'P':       # response pointer with a null in about one case in four
        return 0 if (x >> 5) % 4 == 0 else g.ZONE + (((x >> 4) % 0x400) * 4)
    return _arg_null(k, x)


g.arg47 = arg47b
SETS['cdgen54b'] = [('query_ring', 0x8007F418, 'cd_response_query', 'wP', {0x8007FC64: 1, 0x80080998: 2}, dict(n=1200))]
SETS['cdgen54c'] = [('query_overlap', 0x8007F418, 'cd_response_query', 'wP', {0x8007FC64: 1}, dict(inline=(0x80080998,), n=1200))]
SETS['cdgen56b'] = [('queue_deep', 0x8007EE84, 'cd_queue_command', 'wqww', {0x8007E6B0: 0, 0x80080950: 2, 0x8007FBF0: 1, 0x8007E8F4: 0}, dict(n=1500))]
SETS['cdgen60c'] = [('status_dispatch_a1', 0x8008068C, 'cd_unexpected_status', 'sw', {0x80131140: 2, 0x80131180: 2, 0x801311C0: 2, 0x80080778: 2, 0x80080164: 2}, dict(n=1200))]
SETS['cdgen58b'] = [('issue_sweep', 0x8007FCFC, 'cd_issue_wrapper', 'ww', {0x8007B9EC: 0, 0x80080950: 2, 0x8007B558: 4}, dict(n=0))]


def plan58b():
    """cdgen58b: the command byte store 0x8009B558 takes a0 & 0xFF; commands 7 and 8 read the byte 0x8009B58B and may replace the command by 1 and clear
    the word 0x8009B590.  Sweep: commands 7 and 8 and byte 0..255 over all four (data 0 / nonzero) x (table word zero / nonzero) combinations, commands 6 and 9
    two combinations per byte, 512 other commands with a random byte."""
    items = []
    x = 0x58B58B
    def add(cmd, byte, dnz, tnz, garbage):
        nonlocal x
        x = gv.xs32(x)
        a0 = cmd | (((x >> 8) << 8) & M32 if garbage else 0)
        data = (g.ZONE + ((x >> 4) % 0x400) * 4) if dnz else 0
        tab = ((x >> 12) | 1) & M32 if tnz else 0
        items.append((a0, data, cmd, byte, tab))
    for cmd in (7, 8):
        for byte in range(256):
            for combo in range(4):
                add(cmd, byte, combo & 1, combo >> 1, (byte + combo) % 4 == 0)
    for cmd in (6, 9):
        for byte in range(256):
            for ci in (0, 1):
                combo = (byte + 2 * ci + cmd) % 4
                add(cmd, byte, combo & 1, combo >> 1, (byte + ci) % 4 == 0)
    cmds = (0, 1, 2, 3, 4, 5, 0xA, 0x10, 0x15, 0x1B, 0x80, 0xFF)
    for i in range(512):
        x = gv.xs32(x)
        cmd = cmds[(x >> 4) % len(cmds)] if (x >> 12) % 4 else (x >> 16) & 0xFF
        add(cmd, (x >> 20) & 0xFF if (x >> 3) & 1 else (0, 1, 2, 0x7F, 0x80, 0xFF)[(x >> 8) % 6], (x >> 2) & 1, (x >> 9) & 1, (x >> 10) & 1)
    return items


def run_sweep58b(outdir):
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    c.CALL_CAP[0] = 400
    fname, entry, ename, kinds, callees, kw = SETS['cdgen58b'][0]
    m.spec(entry, ename, kinds, callees, batch='cdgen58b')
    sp = m.SPEC[entry]
    sp.setdefault('budget', 30000)
    sysc = m.syscall_pcs(base, entry)
    c._check_callees_complete(base, entry, tuple(sp['callees']), inline=())
    D = g.collect_D_pool(base, entry, sp, sysc, [])
    out, classes, dropped, spun = [], [], 0, 0
    for n, (a0, data, cmd, byte, tab) in enumerate(plan58b()):
        s, xe = (n + 1) * 7919 ^ entry, n + 1
        ov = {(0x8009B58B, 1): byte, (0x8009B5A4 + 4 * cmd, 4): tab}
        try:
            ram, args, rets, models, x = g.run_one(base, entry, sp, sysc, s, 'discovery', None, xe, D, [], ov)
        except (struct.error, IndexError):
            dropped += 1
            continue
        args[0], args[1] = a0, data
        c.UNFAITHFUL[0] = False
        try:
            res = m.run_case(ram, entry, args, rets, sp, sysc, models)
        except (struct.error, IndexError, g.MipsBreak):
            dropped += 1
            continue
        except AssertionError as e:
            if 'instruction budget' not in str(e):
                raise
            spun += 1
            continue
        finally:
            g.reset_models()
        if c.UNFAITHFUL[0]:
            dropped += 1
            continue
        ret, loads, stores, calls = res
        text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % mm for rec, mm in calls) or '-'
        order = tuple(dict.fromkeys(m.canon(k) for k in sp['callees'])) + tuple(k for k in rets if k not in {m.canon(q) for q in sp['callees']})
        out.append(gv.line(entry, s, 0, args, (ret, loads, stores)) + ' RETS %s CALL %s' % (','.join('%08X=%s' % (k, m.fmt_ret(rets[k])) for k in order) or '-', text))
        classes.append('sweep cmd=%02X byte=%02X' % (cmd, byte))
    os.makedirs(outdir, exist_ok=True)
    open(os.path.join(outdir, fname + '.txt'), 'w').write('\n'.join(out) + '\n')
    open(os.path.join(outdir, fname + '.txt.classes'), 'w').write('\n'.join('%d %s' % (i + 1, k) for i, k in enumerate(classes)) + '\n')
    print('%s %08X %s cases=%d dropped=%d spun=%d state_words=%d own_written_loads=%d mixed=%d' % (fname, entry, ename, len(out), dropped, spun, len(D), sum(m.OWNLOAD.values()), sum(m.MIXEDLOAD.values())))


# ----------------------------------------------------------------------------------------------------- cdgen55c: command low-byte sweep of 0x8007F0C8
SETS['cdgen55c'] = [('sequence_sweep', 0x8007F0C8, 'cd_command_sequence', 'wqkww', {0x80080C48: 1, 0x8007E6B0: 0, 0x8007FBF0: 1, 0x8007E8F4: 0}, dict(inline=(0x80080950,), n=0))]
N_KEEP_STACK = N_KEEP_STACK + ('cdgen55c',)
LANES_55C = (0, 1, 2, 5, 0x80, M32)


def plan55c():
    """Command low byte 0x00..0x40 x 28 cases (every value), plus 300 cases over random low bytes 0x41..0xFF; high 24 bits of the command random; per case:
    converted position (positive in 9 of 10), queue depth word, serial word, scripted status-lane result (cycled over 0 / 1 / 2 / 5 / 0x80 / -1 so every value
    meets every low byte), queue-entry match for the text helper in 3 of 4 lane-1 cases, the slot-allocator sequence (a zero result in 1 of 12 cases)."""
    items, x = [], 0x55C55C
    def add(low, i):
        nonlocal x
        x = gv.xs32(x)
        hi = (x >> 8) & 0xFFFFFF
        pos = ((1, 2, 0x10, 0x100, 0x4B000, 0x7FFFFFFF, (x >> 12) & 0x7FFFFF | 1)[(x >> 4) % 7] if (x >> 20) % 10 else (0, M32, 0x80000000)[(x >> 24) % 3])
        x = gv.xs32(x)
        depth = (0, 0, 0, 0, 0, 4, 5, 0x7FFFFFFF, 1, 2, 3, 6, 7, 8)[(x >> 4) % 14]
        ser = (0, 0, M32, 0x1234, 0x7FFFFFFF, M32 - 1, (x >> 8) & M32)[(x >> 4) % 7]
        x = gv.xs32(x)
        lane = LANES_55C[(i + low) % 6]
        plant = lane == 1 and (x >> 4) % 4 != 0
        idx = (0, 1, 2, 3, 7)[(x >> 8) % 5]
        zero_at = ((x >> 12) % 3) if (x >> 16) % 12 == 0 else None
        items.append(dict(low=low, hi=hi, pos=pos, depth=depth, ser=ser, lane=lane, plant=plant, idx=idx, zero_at=zero_at, slot=g.ZONE + ((x >> 6) % 0x300) * 4))
    for low in range(0x41):
        for i in range(28):
            add(low, i)
    for i in range(300):
        x = gv.xs32(x)
        add(0x41 + (x >> 8) % 0xBF, i)
    return items


def run_sweep55c(outdir):
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    c.CALL_CAP[0] = 400
    install_keep_mixed_stack()
    fname, entry, ename, kinds, callees, kw = SETS['cdgen55c'][0]
    m.spec(entry, ename, kinds, callees, batch='cdgen55c', **kw)
    sp = m.SPEC[entry]
    sp.setdefault('budget', 30000)
    sysc = m.syscall_pcs(base, entry)
    c._check_callees_complete(base, entry, tuple(sp['callees']) + tuple(sp.get('inline', ())), inline=tuple(sp.get('inline', ())))
    D = g.collect_D_pool(base, entry, sp, sysc, [])
    out, classes, dropped, spun = [], [], 0, 0
    inc = lambda q: ((q + 1) & M32) if ((q + 1) & M32) else ((q + 2) & M32)
    for n, it in enumerate(plan55c()):
        s, xe = (n + 1) * 7919 ^ entry, n + 1
        try:
            ram, args, rets, models, x = g.run_one(base, entry, sp, sysc, s, 'discovery', None, xe, D, [], None)
        except (struct.error, IndexError):
            dropped += 1
            continue
        args[2] = (it['hi'] << 8 | it['low']) & M32
        struct.pack_into('<I', ram, 0xA3608, it['depth'])
        struct.pack_into('<I', ram, 0x9B53C, it['ser'])
        struct.pack_into('<I', ram, 0xA3604, it['idx'])
        if it['plant']:
            struct.pack_into('<I', ram, (0x800A3540 + 24 * it['idx']) & 0x1FFFFF, inc(it['ser']))
        put_seq(rets, models, 0x80080C48, [it['pos'], it['pos']])
        put_seq(rets, models, 0x8007FBF0, [it['lane']] * 2)
        slots = [it['slot'] + 0x24 * k for k in range(8)]
        if it['zero_at'] is not None:
            slots[it['zero_at']] = 0
        put_seq(rets, models, 0x8007E6B0, slots)
        c.UNFAITHFUL[0] = False
        try:
            res = m.run_case(ram, entry, args, rets, sp, sysc, models)
        except (struct.error, IndexError, g.MipsBreak):
            dropped += 1
            continue
        except AssertionError as e:
            if 'instruction budget' not in str(e):
                raise
            spun += 1
            continue
        finally:
            g.reset_models()
        if c.UNFAITHFUL[0]:
            dropped += 1
            continue
        ret, loads, stores, calls = res
        text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % mm for rec, mm in calls) or '-'
        order = tuple(dict.fromkeys(m.canon(k) for k in sp['callees'])) + tuple(k for k in rets if k not in {m.canon(q) for q in sp['callees']})
        out.append(gv.line(entry, s, 0, args, (ret, loads, stores)) + ' RETS %s CALL %s' % (','.join('%08X=%s' % (k, m.fmt_ret(rets[k])) for k in order) or '-', text))
        classes.append('sweep low=%02X lane=%X' % (it['low'], it['lane']))
    os.makedirs(outdir, exist_ok=True)
    open(os.path.join(outdir, fname + '.txt'), 'w').write('\n'.join(out) + '\n')
    open(os.path.join(outdir, fname + '.txt.classes'), 'w').write('\n'.join('%d %s' % (i + 1, k) for i, k in enumerate(classes)) + '\n')
    print('%s %08X %s cases=%d dropped=%d spun=%d state_words=%d own_written_loads=%d mixed=%d' % (fname, entry, ename, len(out), dropped, spun, len(D), sum(m.OWNLOAD.values()), sum(m.MIXEDLOAD.values())))


def run_set(name, outdir, n_override=None):
    CUR[0] = name
    if name == 'cdgen58b':
        return run_sweep58b(outdir)
    if name == 'cdgen55c':
        return run_sweep55c(outdir)
    os.makedirs(outdir, exist_ok=True)
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    c.CALL_CAP[0] = 400
    if name in N_KEEP_STACK:
        install_keep_mixed_stack()
    for fname, entry, ename, kinds, callees, kw in SETS[name]:
        kw = dict(kw)
        n = n_override or kw.pop('n', 600)
        kw.pop('n', None)
        m.spec(entry, ename, kinds, callees, batch=name, **kw)
        sp = m.SPEC[entry]
        sp.setdefault('budget', 30000)
        stats = {}
        out, classes = g.gen_entry(base, entry, n, [], 150, stats, [])
        open(os.path.join(outdir, fname + '.txt'), 'w').write('\n'.join(out) + '\n')
        open(os.path.join(outdir, fname + '.txt.classes'), 'w').write('\n'.join('%d %s %08X' % (i + 1, k, int(l.split()[0], 16)) for i, (k, l) in enumerate(zip(classes, out))) + '\n')
        st = stats[entry]
        print('%s %08X %s cases=%d discovery=%d explored=%d spun=%d dropped=%d state_words=%d explorer_runs=%d own_written_loads=%d mixed=%d' % (
            fname, entry, ename, st['cases'], st['discovery'], st['explored'], st['spun'], st['dropped'], st['D'], st['runs'],
            sum(m.OWNLOAD.values()), sum(m.MIXEDLOAD.values())))


def main():
    argv = sys.argv[1:]
    name, n = None, None
    if '--set' in argv:
        i = argv.index('--set')
        name = argv[i + 1]
        del argv[i:i + 2]
    if '-n' in argv:
        i = argv.index('-n')
        n = int(argv[i + 1])
        del argv[i:i + 2]
    if name not in SETS:
        raise SystemExit('unknown set %r; known: %s' % (name, ' '.join(sorted(SETS))))
    run_set(name, argv[0], n)


if __name__ == '__main__':
    main()

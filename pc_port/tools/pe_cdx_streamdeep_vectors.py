#!/usr/bin/env python3
"""cdgen47_stream_deep: deep black-box states for the CD stream handler 0x8007C564 (original code under the test-only interpreter).

States are built the way the suite's own stream oracle builds them (pc_port/tests/test_cd_stream.h CDSTREAM_SeedCase, fixtures in
retail_cd_stream_cases.h: the 21 seed words, the five header halfwords of the incoming 2048-byte sector at 0x80160000 + idx * 2048,
the ring of eight 32-byte records at [0x800C0DC8] (state halfwords), the MDEC busy / idle redirect, the register pointer table), and then:
  * every fixture case verbatim (66 cases, 33 classes x 2 seeds);
  * single-field edits of each of the 33 class baselines over edge values of EVERY seed word, header halfword, record state,
    ring / destination pointer, 'first' / 'occupied' / busy / payload seed, and DMA3 / DMA1 busy register sequences;
  * random two- and three-field combinations of those edits;
  * a case is kept when its access path (ordered load addresses, store addresses, ordered callee tokens) is new, at most K per path
    (fixtures always kept; the fixtures' non-zero callback word, which the oracle host routes to a registry hook, is replaced by the sentinel stop 0x801311C0), so the set is path-diverse rather than large.
Stopped callees as in cdgen47 (0x8007A488 cd_ready, 0x8007C214 data-ready, 0x8007CEAC DMA issue, the D_800B0CCC callback at the sentinel
0x801311C0); the original's own-written loads are excluded and a mixed own / external load drops the case (counted, section `drops`).

Usage: pe_cdx_streamdeep_vectors.py OUT.txt [--per-path K] [--combos N] [--seed S]
Writes OUT.txt (vector lines, pe_cdx_common.h format) and OUT.txt.classes (`line class`: fixture:<name>/<seed> | edit:<field> | combo)."""
import os
import random
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_cdx_cdhost_vectors as g   # noqa: E402  (shared interpreter hooks, run_case, REGSEQ, make_models)

m, c, gv, pl, v = g.m, g.c, g.gv, g.pl, g.v
ENTRY = 0x8007C564
CONTROL_STORES = {0x8009B374, 0x800BE998, 0x800A8018, 0x800B0CD0, 0x800BCD7C, 0x800C0DC4, 0x800A34A0, 0x800C0DC0, 0x800B6918}   # control words: their stored VALUES are part of the dedup signature
import collections
STATS = collections.Counter()
SENT = 0x801311C0
DMAWIN = 0x80180000          # the DMA3 source window of the generator / checker (the CD data stream stand-in), 2048 bytes
FIX = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tests', 'retail_cd_stream_cases.h')
RANGES = [(0x9B294, 3), (0x9B374, 4), (0xA3460, 68), (0xA5D54, 4), (0xA8018, 12), (0xB0CD0, 4), (0xB6914, 8), (0xB8620, 4), (0xB89F4, 4),
          (0xBCD7C, 4), (0xBE998, 4), (0xBE9E4, 8), (0xC0DB8, 20), (0xC20C4, 4), (0x150000, 20480)]
SEEDADDR = [0xB89F4, 0xA801C, 0xA8020, 0xC0DB8, 0xBCD7C, 0x9B296, 0x9B295, 0xA3470, 0xA3468, 0xBE998, 0xBE9E4, 0xC20C4, 0xC0DC0, 0xB6918,
            0xC0DBC, 0xB8620, 0xB6914, 0xA8018, 0xA5D54, 0xB0CCC, 0x945E6]
BYTE_FIELDS = {5, 6}          # init[5], init[6]: 0x9B296 / 0x9B295 are bytes in SeedCase's use (stored as words by the oracle: the oracle's PE_StoreU32)
EDGE = {
    0: (0, 1, 2, 0xFFFF, 0x7FFFFFFF), 1: (0, 1, 2, 0xFFFFFFFF), 2: (0, 1, 2, 3, 4, 0xFFFFFFFF),
    3: (0, 0x80160000, 0x80160800, 0x80161000, 0x801FEFC0, 0x801FE000, 0x801FEFE0, 0x801FE7C0, 0x801FE100, 0x80160001, 0x80100000, 0xFFFFFFFF),
    4: (0, 1, 2, 3), 5: tuple(range(9)) + (0xFF,), 6: tuple(range(9)) + (0xFF,), 7: (0, 1, 0x1F8, 0xFFFFFFFF), 8: (0, 1, 0x1F8, 0xFFFFFFFF),
    9: tuple(range(10)) + (0xFFFFFFFF,), 10: (0, 1, 2, 8, 0xFFFFFFFF), 11: (0, 1, 2, 7, 8, 9, 0xFFFFFFFF), 12: (0, 1, 2, 0x80150000),
    13: (0, 1, 2, 3), 14: (0, 1, 99, 0xFFFFFFFF), 15: (0, 1, 2, 3, 4, 5), 16: (0, 1, 2, 3, 4, 5), 17: (0, 1, 2, 0xFFFFFFFF), 18: (0, 1, 2),
    19: (0, SENT), 20: (0, 1, 2),
}
HDR = (0, 1, 2, 3, 5, 7, 0x160, 0x800, 0xC00, 0x7FFF, 0x8000, 0xFFFF)
KINDS = [('init', i) for i in range(21)] + [('hdr', i) for i in range(5)] + [('rec', i) for i in range(8)] + \
        [('occ', 0), ('first', 0), ('busy', 0), ('seed', 0), ('ring', 0), ('dst', 0), ('chcr', 0), ('mdec', 0), ('fifo', 0), ('stale', 0)]


def vals(kind, i):
    if kind == 'init':
        return EDGE[i]
    if kind == 'hdr':
        return HDR
    if kind == 'rec':
        return (0, 1, 2, 3, 4, 5)
    if kind in ('occ', 'first'):
        return (0, 1, 2, 3, 4, 5)
    if kind == 'busy':
        return (0, 1)
    if kind == 'seed':
        return (0, 1, 37, 165, 255)
    if kind == 'ring':
        return (0x80150000, 0x80150020, 0x80150100, 0)
    if kind == 'dst':
        return (0x80170000, 0x801FEFC0, 0)
    if kind == 'chcr':          # DMA3 CHCR (0x1F8010B8) read sequence: idle / busy bit 24 / k busy then idle
        return ('idle', 'busy', 'busy1', 'busy3', 'mixed')
    if kind == 'stale':         # the uninitialised frame word read back into the record word at +0x1C (suite value / zero / all ones)
        return (0x55667788, 0, 0xFFFFFFFF, 0x12345678)
    if kind == 'fifo':          # data FIFO bytes behind 0x1F801802: none = register page value (0), distinct / reversed ramps (which four bytes form the word at slot+0x1C is observable)
        return ('none', 'distinct', 'ramp', 'rev')
    if kind == 'mdec':          # the MDEC busy word behind 0x8009B34C
        return (0, 0x01000000, 0x11000000, 0x00000001, 0xFFFFFFFF)


def load_fixtures():
    out = []
    for line in open(FIX):
        mm = re.match(r'/\* (\S+) \*/ \{\{([^}]*)\},\{([^}]*)\},(\d+)u,(\d+)u,(\d+)u,(\d+)u,(\d+)u', line)
        if not mm:
            continue
        nm = lambda t: [int(x.rstrip('u')) for x in t.split(',')]
        out.append(dict(name=mm.group(1), init=[SENT if (j == 19 and val) else val for j, val in enumerate(nm(mm.group(2)))], hdr=nm(mm.group(3)), occ=int(mm.group(4)), first=int(mm.group(5)),
                        busy=int(mm.group(6)), seed=int(mm.group(7)), rec={}, ring=0x80150000, dst=None, chcr='idle', mdec=None, fifo='none', stale=0x55667788))
    # the physical-device states of the suite's DAY2_cd_dma test (record ring all free, FIFO read by hand, data by DMA3 = source pointer 0, ring size 8,
    # one-part frame of channel 0; 'chopped' = mode flag 0x800A801C == 0): DMA-fetched header and payload, publication without the data-ready callback
    for chopped in (0, 1):
        init = [0] * 21
        init[1] = 0 if chopped else 1
        init[11] = 8
        out.append(dict(name='phys-dma-' + ('chopped' if chopped else 'idle'), init=init, hdr=[0x160, 0, 0, 1, 1], occ=0, first=0, busy=0, seed=37,
                        rec={i: 0 for i in range(8)}, ring=0x80150000, dst=None, chcr='idle', mdec=None, fifo='distinct', stale=0x55667788))
    return out


def build_ram(base, st):
    ram = bytearray(base)
    for j, (off, n) in enumerate(RANGES):
        ram[off:off + n] = bytes([0xCC]) * n if j == 14 else bytes(n)       # the suite fills the ring / destination area with 0xCC
    for a, val in zip(SEEDADDR, st['init']):
        struct.pack_into('<I', ram, a & 0x1FFFFF, val & 0xFFFFFFFF)
    ring = st['ring']
    struct.pack_into('<I', ram, 0xC0DC8, ring)
    rb = ring & 0x1FFFFF
    if st['ring'] and rb + 8 * 32 < len(ram):
        for i in range(8):
            struct.pack_into('<H', ram, rb + i * 32, st['rec'].get(i, 4))
        struct.pack_into('<H', ram, rb, st['first'] & 0xFFFF)
        idx = st['init'][9]
        if idx < 8:
            struct.pack_into('<H', ram, rb + idx * 32, st['occ'] & 0xFFFF)
    for i in range(6144):
        ram[0x160000 + i] = (st['seed'] + i * 17) & 0xFF
    src = st['init'][3]
    if 0x801FE000 <= src < 0x80200000:        # source pointer in the guest stack window (cd_stream_source_stack_alias): the window holds a copy of the sector bytes
        n = min(2048, 0x80200000 - src)
        ram[src & 0x1FFFFF:(src & 0x1FFFFF) + n] = ram[0x160000:0x160000 + n]
    inc = 0x160000 + (st['init'][4] & 3) * 2048
    for i in range(5):
        struct.pack_into('<H', ram, inc + i * 2, st['hdr'][i] & 0xFFFF)
    for a, val in ((0x9B27C, 0x1F801800), (0x9B32C, 0x1F801800), (0x9B280, 0x1F801801), (0x9B284, 0x1F801802), (0x9B288, 0x1F801803),
                   (0x9B338, 0x1F801803), (0x9B33C, 0x1F801018), (0x9B340, 0x1F801020), (0x9B35C, 0x1F8010B8)):
        struct.pack_into('<I', ram, a, val)
    # the uninitialised frame word of the original's local frame at frame+8 (stack 0x801FEFE8 when entered at sp 0x801FF000): the oracle scripts and the suite
    # seed 0x55667788 there (the suite at the host scratch window 0x801FFE88); the record word at +0x1C is that stale word when the FIFO is not read by hand
    struct.pack_into('<I', ram, 0x1FEFE8, st.get('stale', 0x55667788) & 0xFFFFFFFF)
    ram[0x1FEFF0:0x1FEFF8] = bytes(8)
    mdec = st['mdec'] if st['mdec'] is not None else (0x01000000 if st['busy'] else 0)
    struct.pack_into('<I', ram, 0x9B34C, 0x8013001C if st['busy'] else 0x1F801098)
    struct.pack_into('<I', ram, 0x13001C, mdec)
    if st['dst'] is not None:
        struct.pack_into('<I', ram, 0xC0DC4, st['dst'])
    return ram


def apply_edit(st, kind, i, val):
    st = dict(st, init=list(st['init']), hdr=list(st['hdr']), rec=dict(st['rec']))
    if kind == 'init':
        st['init'][i] = val
    elif kind == 'hdr':
        st['hdr'][i] = val
    elif kind == 'rec':
        st['rec'][i] = val
    else:
        st[kind] = val
    return st


def run(base, sp, sysc, st, s):
    """Run one state; returns (line_text, signature) or None (dropped / spun / unfaithful)."""
    ram = build_ram(base, st)
    g.reset_models()
    c.CdIrq.active = False
    ch = st['chcr']
    seqs = {'idle': [0], 'busy': [0x01000000], 'busy1': [0x01000000, 0], 'busy3': [0x01000000] * 3 + [0], 'mixed': [0x11000000, 0x00000200, 0]}[ch]
    g.REGSEQ[0x10B8] = [4, seqs, 0]
    fifo = st.get('fifo', 'none')
    if fifo != 'none':
        g.REGSEQ[0x1802] = [1, {'distinct': [(0x31 + i * 0x13) & 0xFF for i in range(16)], 'ramp': [0x41 + i for i in range(16)],
                                'rev': [(0xF0 - 7 * i) & 0xFF for i in range(16)]}[fifo], 0]
    rets = {}
    x = gv.xs32(s * 2654435761 & 0xFFFFFFFF or 1)
    x, models = g.make_models(ENTRY, sp, x, rets, None)
    # DMA3 data effect (the channel moves the next bytes of the CD data stream to the address the issuer is given): the data window is the
    # incoming sector buffer, placed at DMAWIN; the model writes the words to RAM, logs them as stores (they appear in the expected RAM
    # image and in the '@n' write counts), marks them own-written (the original's later loads of them are not input) and appends the
    # consumed window words to the loads (the checker seeds its stand-in with them).  Without it a DMA-fetched record stays unwritten and
    # every DMA-path case ends as a bad sector.
    inc = 0x160000 + (st['init'][4] & 3) * 2048
    ram[DMAWIN & 0x1FFFFF:(DMAWIN & 0x1FFFFF) + 2048] = ram[inc:inc + 2048]
    consumed = [0]

    def dma_model(ram_, r):
        a, words = r[5] & 0x1FFFFF, min(r[7] & 0xFFFF, 512)
        n = min(words * 4, 2048 - consumed[0])
        if n <= 0 or a + n > len(ram_):
            return 0
        off = (DMAWIN & 0x1FFFFF) + consumed[0]
        data = bytes(ram_[off:off + n])
        for o in range(0, n, 4):
            wv = struct.unpack_from('<I', data, o)[0]
            pl.LOADS.append((DMAWIN + consumed[0] + o, 4, wv))
        ram_[a:a + n] = data
        m._OWN.update(range(a, a + n))
        for o in range(0, n, 4):
            pl.STORES.append((0x80000000 + a + o, 4, struct.unpack_from('<I', data, o)[0]))
        consumed[0] += n
        return 0
    models[0x8007CEAC] = dma_model
    c.UNFAITHFUL[0] = False
    try:
        res = m.run_case(ram, ENTRY, [], rets, sp, sysc, models)
    except (struct.error, IndexError, g.MipsBreak):
        return None
    except AssertionError as e:        # instruction budget (poll spun) or the original jumped into code the interpreter cannot run (a wild callback word)
        STATS['assert:' + ('budget' if 'instruction budget' in str(e) else 'unsupported')] += 1
        if 'instruction budget' in str(e):
            STATS['budget:dma3-chcr=' + ch + ('' if ch != 'idle' else '(other poll)')] += 1
            tail = collections.Counter(la for la, _n, _v in pl.LOADS[-24:])
            STATS['budget-poll-load:%s:%08X' % (ch, tail.most_common(1)[0][0] if tail else 0)] += 1
        return None
    finally:
        g.reset_models()
    if c.UNFAITHFUL[0]:
        STATS['drop:own-written-mixed-or-unfaithful-load'] += 1
        return None
    ret, loads, stores, calls = res
    text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % mm_ for rec, mm_ in calls) or '-'
    order = tuple(dict.fromkeys(m.canon(k) for k in sp['callees']))
    line = gv.line(ENTRY, s, 0, [], (ret, loads, stores)) + ' RETS %s CALL %s' % (
        ','.join('%08X=%s' % (k, m.fmt_ret(rets[k])) for k in order if k in rets) or '-', text)
    sig = (tuple(a for a, _n, _v in loads if a < 0x801FE000 or a >= 0x80200000), tuple(a for a, _n, _v in stores), tuple(r_[0] for r_, _x in calls),
           tuple((a, _v) for a, _n, _v in stores if a in CONTROL_STORES),
           tuple(_v for a, _n, _v in loads if a == 0x1F801802),
           tuple(_v for a, _n, _v in stores if 0x80150000 <= a < 0x80150100 and a % 32 == 0x1C))
    return line, sig


def main():
    argv = sys.argv[1:]
    per, combos, seed = 2, 3000, 1
    for flag in ('--per-path', '--combos', '--seed'):
        if flag in argv:
            i = argv.index(flag)
            val = int(argv[i + 1])
            del argv[i:i + 2]
            if flag == '--per-path':
                per = val
            elif flag == '--combos':
                combos = val
            else:
                seed = val
    outp = argv[0]
    sp = m.SPEC[ENTRY]
    sp['budget'] = 30000
    sp['callees'][SENT] = 1          # the D_800B0CCC callback receives $a0 (D_800A34A0) at one jalr site: record it
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    sysc = m.syscall_pcs(base, ENTRY)
    c._check_callees_complete(base, ENTRY, tuple(sp['callees']) + tuple(sp.get('inline', ())), inline=tuple(sp.get('inline', ())))
    fixtures = load_fixtures()
    rng = random.Random(seed)
    lines, classes, seen = [], [], {}
    stats = dict(fixtures=0, edits_tried=0, edits_kept=0, combos_tried=0, combos_kept=0, dropped_or_spun=0, own_written_mixed_drops=0)
    mix0 = dict(m.MIXEDLOAD)

    def add(st, tag, s, always):
        stats['runs'] = stats.get('runs', 0) + 1
        if stats['runs'] % 500 == 0:
            print('progress runs=%d kept=%d' % (stats['runs'], len(lines)), file=sys.stderr, flush=True)
        r = run(base, sp, sysc, st, s)
        if r is None:
            stats['dropped_or_spun'] += 1
            return False
        line, sig = r
        if not always and seen.get(sig, 0) >= per:
            return False
        seen[sig] = seen.get(sig, 0) + 1
        lines.append(line)
        classes.append(tag)
        return True

    for k, fx in enumerate(fixtures):
        if add(fx, 'fixture:%s/%d' % (fx['name'], fx['seed']), 1000 + k, True):
            stats['fixtures'] += 1
    bases = [fx for fx in fixtures if fx['seed'] == 37]
    s = 5000
    for fx in bases:
        for kind, i in KINDS:
            for val in vals(kind, i):
                s += 1
                stats['edits_tried'] += 1
                if add(apply_edit(fx, kind, i, val), 'edit:%s%d' % (kind, i), s, False):
                    stats['edits_kept'] += 1
    for _ in range(combos):
        fx = rng.choice(fixtures)
        st = fx
        tags = []
        for _j in range(rng.choice((2, 2, 3))):
            kind, i = rng.choice(KINDS)
            st = apply_edit(st, kind, i, rng.choice(vals(kind, i)))
            tags.append('%s%d' % (kind, i))
        s += 1
        stats['combos_tried'] += 1
        if add(st, 'combo:' + '+'.join(tags), s, False):
            stats['combos_kept'] += 1
    stats['own_written_mixed_drops'] = sum(m.MIXEDLOAD.values()) - sum(mix0.values())
    stats['own_written_loads_excluded'] = sum(m.OWNLOAD.values())
    open(outp, 'w').write('\n'.join(lines) + '\n')
    open(outp + '.classes', 'w').write(''.join('%d %s\n' % (i + 1, t) for i, t in enumerate(classes)))
    print('cases=%d distinct_paths=%d per_path_cap=%d %s %s' % (len(lines), len(seen), per, ' '.join('%s=%d' % kv for kv in stats.items()), ' '.join('%s=%d' % kv for kv in sorted(STATS.items()))))


if __name__ == '__main__':
    main()

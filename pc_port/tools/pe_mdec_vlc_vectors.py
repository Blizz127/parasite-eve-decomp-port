#!/usr/bin/env python3
"""Vectors for pc_port/platform/pe_plat_mdec_vlc.c (clean-room STR/BS VLC decoder).

The ORIGINAL libpress VLC decoder (movie module overlay ovl_039F, entry
0x8010C89C, after the table builder 0x8010BD4C) is driven as an opaque black box
under the test-only interpreter, exactly as pe_movie_complete_frame_oracle.py
does: nothing here reads, transliterates or disassembles that code.  Inputs
are (a) real STR video frames of the owner's disc, sampled across all movies
(only hashes are recorded, the checker re-reads the frames from the disc), and
(b) synthetic bitstreams built by the encoder below from psx-spx
(https://psx-spx.consoledev.net/cdromfileformats/ "Huffman codes for AC values
BS v1/v2/v3/iki", "DC v1/v2/ea", "DC v3", "BS v1/v2/v3 header").  The AC table
below is a second, independent transcription made by parsing the fetched page.

Recorded per vector: output length, output FNV-1a64 over [0,max(declared,end)),
the original's returned input cursor; cases where the original never terminates
are recorded as HANG (the checker only requires the clean-room decoder not to
claim success on them).  Writes pc_port/tests/retail_mdec_vlc_cases.h.
    python3 pe_mdec_vlc_vectors.py [--check]
"""
import hashlib, json, os, random, struct, sys
from pe_movie_init_oracle import load, ROOT, execute
from pe_m0000i_leaves_oracle import find_disc, read_form1
from pe_cd_ack_oracle import Stops

OVERLAY_SHA256 = 'd0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5ba8d0b40'
BASE_IN, BASE_OUT, BASE_TAB = 0x80150000, 0x80160000, 0x80130000

AC = [('10', [0xFE00]), ('11s', [0x0001]), ('011s', [0x0401]), ('010xs', [0x0002, 0x0801]),
      ('0011xs', [0x1001, 0x0C01]), ('00101s', [0x0003]),
      ('00100xxxs', [0x3401, 0x0006, 0x3001, 0x2C01, 0x0C02, 0x0403, 0x0005, 0x2801]),
      ('0001xxs', [0x1C01, 0x1801, 0x0402, 0x1401]), ('00001xxs', [0x0802, 0x2401, 0x0004, 0x2001]),
      ('0000001xxxs', [0x4001, 0x1402, 0x0007, 0x0803, 0x0404, 0x3C01, 0x3801, 0x1002]),
      ('00000001xxxxs', [0x000B, 0x2002, 0x1003, 0x000A, 0x0804, 0x1C02, 0x5401, 0x5001,
                         0x0009, 0x4C01, 0x4801, 0x0405, 0x0C03, 0x0008, 0x1802, 0x4401]),
      ('000000001xxxxs', [0x2802, 0x2402, 0x1403, 0x0C04, 0x0805, 0x0407, 0x0406, 0x000F,
                          0x000E, 0x000D, 0x000C, 0x6801, 0x6401, 0x6001, 0x5C01, 0x5801]),
      ('0000000001xxxxs', list(range(0x1F, 0x0F, -1))),
      ('00000000001xxxxs', [0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
                            0x40E, 0x40D, 0x40C, 0x40B, 0x40A, 0x409, 0x408]),
      ('000000000001xxxxs', [0x412, 0x411, 0x410, 0x40F, 0x1803, 0x4002, 0x3C02, 0x3802,
                             0x3402, 0x3002, 0x2C02, 0x7C01, 0x7801, 0x7401, 0x7001, 0x6C01])]


def neg(v):
    return (v & 0xFC00) | ((-(v & 0x3FF)) & 0x3FF)


ENC = {}
for pat, vals in AC:
    nx = pat.count('x'); pre = pat.replace('x', '').replace('s', ''); hs = 's' in pat
    for i, v in enumerate(vals):
        xb = format(i, '0%db' % nx) if nx else ''
        for s in ((0, 1) if hs else (0,)):
            val = neg(v) if s else v
            assert val not in ENC
            ENC[val] = pre + xb + (str(s) if hs else '')
ACVALUES = sorted(v for v in ENC if v != 0xFE00)


class W:
    def __init__(s): s.b = []
    def put(s, bits): s.b.append(bits)
    def bytes(s):
        bits = ''.join(s.b)
        while len(bits) % 16: bits += '0'
        out = bytearray()
        for i in range(0, len(bits), 16): out += struct.pack('<H', int(bits[i:i + 16], 2))
        while len(out) % 4: out += b'\0'
        return bytes(out)


def ac_bits(v, esc=False):
    return ENC[v] if (not esc and v in ENC) else '000001' + format(v, '016b')


def frame(blocks, count=None, quant=1, ver=2, eof=True, esc=False, eob=True, pad=64, ident=0x3800):
    w = W(); hw = 0
    for dc, acs in blocks:
        w.put(format(dc & 0x3FF, '010b')); hw += 1
        for a in acs: w.put(ac_bits(a, esc)); hw += 1
        if eob: w.put('10'); hw += 1
    if eof: w.put('0111111111')
    if count is None: count = (hw + 1) // 2
    return struct.pack('<4H', count & 0xFFFF, ident, quant, ver) + w.bytes() + bytes(pad)


def dc3_code(off, luma):
    if off == 0: return '100' if luma else '00'
    s = 1 if off > 0 else 0; m = abs(off)
    if m == 1: return ('00' if luma else '01') + str(s)
    k = m.bit_length() - 1
    n = (m - (1 << k)) if s else (((2 << k) - 1) - m)
    pre = ({1: '01', 2: '101', 3: '110', 4: '1110', 5: '11110', 6: '111110', 7: '1111110'} if luma else
           {1: '10', 2: '110', 3: '1110', 4: '11110', 5: '111110', 6: '1111110', 7: '11111110'})[k]
    return pre + str(s) + format(n, '0%db' % k)


def frame3(mbs, quant=1, count=None, ver=3, eof=True, pad=64):
    w = W(); hw = 0
    for mb in mbs:
        for i, (off, acs) in enumerate(mb):
            w.put(dc3_code(off, i >= 2)); hw += 1
            for a in acs: w.put(ac_bits(a)); hw += 1
            w.put('10'); hw += 1
    if eof: w.put('1111111111')
    if count is None: count = (hw + 1) // 2
    return struct.pack('<4H', count, 0x3800, quant, ver) + w.bytes() + bytes(pad)


def fnv(data):
    h = 14695981039346656037
    for b in data: h = ((h ^ b) * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return h


def env():
    ex, _ = load(); path = find_disc(ROOT); ov = read_form1(path, 1940, 38)
    assert hashlib.sha256(ov).hexdigest() == OVERLAY_SHA256
    ram = bytearray(0x200000); ram[0x10000:0x10000 + len(ex) - 2048] = ex[2048:]
    ram[0x10BCF8:0x10BCF8 + len(ov)] = ov
    execute(ram, 0x8010BD4C, (BASE_TAB, 0), instruction_budget=2000000)
    return ram, path


def run_original(ram, data, budget=2000000):
    """-> None when the original does not terminate within the budget, else
    (output bytes [0,max(declared,end)), cursor bytes, return value)."""
    r = bytearray(ram); r[0x150000:0x150000 + len(data)] = data
    declared = 4 + (struct.unpack_from('<I', data)[0] & 65535) * 4
    r[0x160000:0x160000 + declared + 16] = bytes([0xCD]) * (declared + 16)
    stops = Stops((0x8010CBAC, 0x8010CBBC)); stops.last = 0
    try:
        regs = execute(r, 0x8010C89C, (BASE_IN, BASE_OUT, BASE_TAB, 0), stop_at=stops, instruction_budget=budget)
    except AssertionError as e:
        assert 'instruction budget' in str(e); return None
    assert stops.last == 0x8010CBAC
    end = regs[5] - BASE_OUT
    return bytes(r[0x160000:0x160000 + max(end, declared)]), regs[4] - BASE_IN, end


def frames_on_disc(path):
    out = []; chunks = []; cur = None
    with open(path, 'rb') as f:
        lba = 0
        while True:
            buf = f.read(2352 * 512)
            if not buf: break
            for i in range(len(buf) // 2352):
                raw = buf[i * 2352:(i + 1) * 2352]
                if raw[24:28] != b'\x60\x01\x01\x80' or not raw[18] & 8: continue
                index, total, frame_no, size, w, h = struct.unpack_from('<HHIIHH', raw, 28)
                L = lba + i
                if index == 0: cur = dict(first=L, total=total, frame=frame_no, size=size, secs=1)
                elif cur and index == cur['secs'] and frame_no == cur['frame']: cur['secs'] += 1
                else: cur = None; continue
                if cur and index == total - 1:
                    cur['last'] = L; out.append(cur); cur = None
            lba += len(buf) // 2352
    return out


def read_frame(path, fr):
    data = bytearray()
    with open(path, 'rb') as f:
        for lba in range(fr['first'], fr['last'] + 1):
            f.seek(lba * 2352); raw = f.read(2352)
            if raw[18] & 8 and raw[24:28] == b'\x60\x01\x01\x80': data += raw[56:2072]
    return bytes(data)


def synthetic():
    rnd = random.Random(20261125)
    cases = []
    def add(name, data): cases.append((name, data))
    def nodc(): return rnd.choice([x for x in range(1024) if x != 0x1FF])
    # every AC code, both signs, one per block; each in its own six-block macroblock group
    for chunk in range(0, len(ACVALUES), 36):
        blocks = [(nodc(), [v]) for v in ACVALUES[chunk:chunk + 36]]
        while len(blocks) % 6: blocks.append((0, []))
        add('ac-codes-%d' % chunk, frame(blocks))
    add('eob-only', frame([(7, [])] * 12))
    raws = [0, 1, 0xFE00, 0xFFFF, 0x03FF, 0x0400, 0x8000, 0x7C00, 0xFC00, 0x1234] + [rnd.randrange(65536) for _ in range(38)]
    add('escape-raw', frame([(nodc(), [v]) for v in raws] + [(0, [])] * 2, esc=True))
    add('dc-values', frame([(dc, []) for dc in (0, 1, 0x1FE, 0x200, 0x201, 0x3FF, 0x2FF, 0x100, 0x300, 0x3FE, 5, 0x1FD)]))
    for q in (0, 1, 2, 31, 63, 64, 65535): add('quant-%d' % q, frame([(7, [1, 0x401])] * 6, quant=q))
    add('64-ac-with-eob', frame([(3, [1] * 64)] * 6))
    add('run-overflow', frame([(3, [0x3C01, 0x3C01, 0x3C01])] * 6))
    base = frame([(9, [1, 2])] * 12); cnt = struct.unpack_from('<H', base)[0]
    for c in (0, 1, 2, cnt - 1, cnt, cnt + 1, cnt + 31, 128, 65535):
        d = bytearray(base); struct.pack_into('<H', d, 0, c); add('count-%d' % c, bytes(d))
    for hw_blocks, acs in ((7, [1]), (9, [1]), (6, [1, 2, 3, 4, 5, 6, 7])):
        for c in (0, 100): add('oddlen-%d-%d-%d' % (hw_blocks, len(acs), c), frame([(5, acs)] * hw_blocks, count=c))
    for v in (0, 1, 2): add('version-%d' % v, frame([(9, [1, 2])] * 12, ver=v))
    add('bad-id', frame([(9, [1, 2])] * 12, ident=0x3801))
    add('eof-mid-macroblock', frame([(1, []), (0x1FF, []), (2, []), (3, []), (4, []), (5, [])]))
    add('eof-first', frame([(0x1FF, [])]))
    # v3
    offs = [0, 1, -1, 2, 3, -2, -3, 4, 7, -4, -7, 8, 15, -8, -15, 16, 31, -16, -31, 32, 63, -32, -63, 64, 127, -64, -127, 128, 255, -128, -255]
    mbs = []
    for off in offs:
        mbs.append([(off if i < 3 else 0, []) for i in range(6)])
        mbs.append([(0 if i < 3 else off, []) for i in range(6)])
    add('v3-offsets', frame3(mbs))
    add('v3-with-ac', frame3([[(1, [1, 2]), (-1, [3]), (2, []), (5, [0x401]), (-3, []), (0, [7])]] * 5))
    add('v3-quant-7', frame3([[(3, [1]), (-1, []), (2, []), (1, []), (-3, []), (0, [])]] * 4, quant=7))
    add('v3-wrap', frame3([[(255, [])] * 6] * 12))
    for v in (4, 5, 7, 0x100, 0x8000, 0xFFFF): add('v3-version-%#x' % v, frame3([[(3, [1]), (-1, []), (2, []), (1, []), (-3, []), (0, [])]] * 4, ver=v))
    # random streams
    for i in range(24):
        n = rnd.randint(1, 40) * 6
        blocks = [(nodc(), [rnd.choice(ACVALUES) for _ in range(rnd.choice([0, 0, 1, 2, 5, 12, 30]))]) for _ in range(n)]
        add('rand-v2-%d' % i, frame(blocks, quant=rnd.randrange(64)))
    for i in range(24):
        n = rnd.randint(1, 40)
        mbs = [[(rnd.randint(-255, 255), [rnd.choice(ACVALUES) for _ in range(rnd.choice([0, 0, 1, 3, 9]))]) for _ in range(6)] for _ in range(n)]
        add('rand-v3-%d' % i, frame3(mbs, quant=rnd.randrange(64)))
    # within-spec streams: whole macroblocks, EOF at a macroblock start, run totals <= 63 per block, quant < 64
    def block_acs():
        acs = []; k = 0
        for _ in range(rnd.choice([0, 1, 2, 4, 8, 20, 40])):
            v = rnd.choice(ACVALUES); run = (v >> 10) & 63
            if k + run + 1 > 63: break
            k += run + 1; acs.append(v)
        return acs
    for i in range(30):
        add('valid-v2-%d' % i, frame([(rnd.randrange(511), block_acs()) for _ in range(6 * rnd.randint(1, 30))],
                                     quant=rnd.randrange(64), ver=rnd.choice([1, 2])))
    for i in range(30):
        add('valid-v3-%d' % i, frame3([[(rnd.randint(-255, 255), block_acs()) for _ in range(6)] for _ in range(rnd.randint(1, 30))],
                                      quant=rnd.randrange(64)))
    # hangs in the original: no end-of-frame code, truncated stream, unused code
    add('hang-no-eof', frame([(9, [1, 2])] * 12, eof=False))
    add('hang-unused-code', frame([(9, [1, 2])] * 2, eof=False)[:8] + struct.pack('<2H', 0x2400, 0x0000) + bytes(40))
    add('hang-no-eob-63', frame([(3, [1] * 63)] * 6, eob=False))
    return cases


def main():
    ram, path = env()
    frames = frames_on_disc(path)
    rnd = random.Random(7)
    by_movie = []; start = 0
    for i in range(1, len(frames) + 1):
        if i == len(frames) or frames[i]['first'] - frames[i - 1]['last'] > 400 or frames[i]['frame'] < frames[i - 1]['frame']:
            by_movie.append(frames[start:i]); start = i
    real = []
    for m in by_movie:
        pick = {0, len(m) - 1, len(m) // 2} | {rnd.randrange(len(m)) for _ in range(3)}
        real += [m[i] for i in sorted(pick)]
    rows = []; hangs = []
    for fr in real:
        data = read_frame(path, fr); res = run_original(ram, data, 3000000)
        assert res is not None, fr
        out, cursor, end = res
        rows.append((fr['first'], fr['last'], fr['total'], fr['frame'], len(data), fnv(data), len(out), fnv(out), cursor))
        print('real', fr['frame'], len(out), flush=True)
    syn = []
    for name, data in synthetic():
        res = run_original(ram, data, 2500000)
        if res is None:
            hangs.append((name, data)); print('syn', name, 'HANG', flush=True)
        else:
            syn.append((name, data, res)); print('syn', name, len(res[0]), flush=True)
    L = ['/* Original libpress VLC decoder (func_8010C89C) outputs on real STR frames and synthetic psx-spx bitstreams.',
         ' * Generated by pc_port/tools/pe_mdec_vlc_vectors.py; real frames are referenced by LBA + hashes only. */']
    L.append('static const struct { uint32_t first,last,chunks,frame,size; uint64_t in_hash; uint32_t outlen; uint64_t out_hash; uint32_t cursor; } MDECVLC_real[]={')
    for r in rows: L.append('{%du,%du,%du,%du,%du,UINT64_C(0x%016X),%du,UINT64_C(0x%016X),%du},' % (r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8]))
    L.append('};')
    def arr(name, data):
        return 'static const uint8_t %s[]={%s};' % (name, ','.join(str(b) for b in data))
    for i, (name, data, res) in enumerate(syn): L.append(arr('MDECVLC_in%d' % i, data))
    for i, (name, data) in enumerate(hangs): L.append(arr('MDECVLC_hang%d' % i, data))
    L.append('static const struct { const char *name; const uint8_t *in; uint32_t insize,outlen; uint64_t out_hash; uint32_t cursor; } MDECVLC_syn[]={')
    for i, (name, data, res) in enumerate(syn):
        L.append('{"%s",MDECVLC_in%d,%du,%du,UINT64_C(0x%016X),%du},' % (name, i, len(data), len(res[0]), fnv(res[0]), res[1]))
    L.append('};')
    L.append('static const struct { const char *name; const uint8_t *in; uint32_t insize; } MDECVLC_hang[]={')
    for i, (name, data) in enumerate(hangs): L.append('{"%s",MDECVLC_hang%d,%du},' % (name, i, len(data)))
    L.append('};')
    output = '\n'.join(L) + '\n'
    p = ROOT / 'pc_port/tests/retail_mdec_vlc_cases.h'
    if '--check' in sys.argv: assert p.read_text() == output, 'case header differs'
    else: p.write_text(output)
    print('PASS %d real frames, %d synthetic terminating, %d original-hang vectors' % (len(rows), len(syn), len(hangs)))


if __name__ == '__main__':
    main()

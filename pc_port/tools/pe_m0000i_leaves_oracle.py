#!/usr/bin/env python3
"""Original M0000I overlay leaves including 80192740 (C02C==0 graph)."""
import hashlib, struct, sys
from pe_battle_hud_oracle import ROOT, execute
from pe_btl14_m0005i_publish_oracle import find_disc, read_form1, load_u32

RANGES = ((0xB0DCE, 8), (0xB0E4C, 4), (0x19C1F8, 8), (0x140000, 8),
          (0x19C058, 4), (0x19CAA8, 0xA8), (0x1EA268, 0xA0),
          (0x19C9C0, 4), (0x142000, 8), (0x142110, 0xC0), (0x142200, 4),
          (0x19C1F0, 4), (0x19C02C, 4), (0x1EA598, 0x30), (0x147FFC, 4),
          (0x19CA80, 0x40), (0x144024, 8), (0x19CC52, 4), (0x1EA370, 0x34))
SHA_OVERLAY = 'c51e36c27422e990d4683d73dc9fc2633e0924721dd0c242a8efc2e8520a4edb'
SHA_91DE8 = None
SHA_9BF8C = None
SHA_93AB0 = None
SHA_941A4 = None
SHA_94108 = None
SHA_92740 = None
SHA_93B5C = None


def load_overlay():
    exe = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    ranges = struct.unpack_from('<5H', exe, 0x93168 - 0x10000 + 0x800)
    overlay = read_form1(find_disc(ROOT), 1013 + ranges[3], ranges[4] - ranges[3])
    assert hashlib.sha256(overlay).hexdigest() == SHA_OVERLAY
    base = load_u32(exe, 0x80011614)
    global SHA_91DE8, SHA_9BF8C, SHA_93AB0, SHA_941A4, SHA_94108, SHA_92740, SHA_93B5C
    SHA_91DE8 = hashlib.sha256(overlay[0x80191DE8 - base:0x80191E30 - base]).hexdigest()
    SHA_9BF8C = hashlib.sha256(overlay[0x8019BF8C - base:0x8019BFC4 - base]).hexdigest()
    SHA_93AB0 = hashlib.sha256(overlay[0x80193AB0 - base:0x80193B5C - base]).hexdigest()
    SHA_941A4 = hashlib.sha256(overlay[0x801941A4 - base:0x801942FC - base]).hexdigest()
    SHA_94108 = hashlib.sha256(overlay[0x80194108 - base:0x801941A4 - base]).hexdigest()
    SHA_92740 = hashlib.sha256(overlay[0x80192740 - base:0x80192800 - base]).hexdigest()
    SHA_93B5C = hashlib.sha256(overlay[0x80193B5C - base:0x80194108 - base]).hexdigest()
    return exe, overlay, base


def fingerprint(r):
    h = 14695981039346656037
    for a, n in RANGES:
        for b in r[a:a + n]:
            h = ((h ^ b) * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return h


def main():
    exe, overlay, base = load_overlay()
    common = None
    patches = []
    cases = []
    fixtures = []
    for a0 in (0, 1, 2, 10, 0xFFFFFFFF):
        fixtures.append(('91DE8', 0x80191DE8, (a0,), dict(a0=a0, bank=0x12340000)))
    for dest, bank in ((0x8019C1F8, 0x80150000), (0x80140000, 0x80150000),
                       (0x8019C1F8, 0), (0x80140000, 0xABCDEF00)):
        fixtures.append(('9BF8C', 0x8019BF8C, (dest,), dict(dest=dest, bank=bank)))
    add_cases = (
        (0, 0x11111111, 0x22222222),
        (-1, 0x01010101, 0x02020202),
        (1, 0x1000, 0x10),
        (2, 0xFFFFFFF0, 0x20),
        (5, 0xABCDEF00, 0x11111111),
    )
    for n, dest_word, src_word in add_cases:
        fixtures.append(('93AB0', 0x80193AB0, (),
                         dict(n=n, dest_word=dest_word, src_word=src_word)))
    for intensity in (0, 1, 128, 255, 0x1FF):
        fixtures.append(('941A4', 0x801941A4, (intensity,), dict(a0=intensity)))
    for level in (-300, -255, -254, -1, 0, 1, 16, 254, 255, 256, 0x7FFF, 0xFFFF8000):
        fixtures.append(('94108', 0x80194108, (level & 0xFFFFFFFF,), dict(a0=level & 0xFFFFFFFF)))
    for flag, ctx in ((0, 0x80142000), (1, 0x80142000), (1, 0x8019C1F8), (2, 0x80142000)):
        fixtures.append(('92740', 0x80192740, (), dict(flag=flag, ctx=ctx, c02c=0)))
    fixtures.append(('92740', 0x80192740, (), dict(flag=0, ctx=0x80142000, c02c=10)))
    for level in (-300, -8, -1, 0, 1, 8, 247, 255, 300):
        fixtures.append(('93B5C', 0x80193B5C, (level & 0xFFFFFFFF,),
                         dict(a0=level & 0xFFFFFFFF, ctx=0x80142000)))
    fixtures.append(('93B5C', 0x80193B5C, (1,), dict(a0=1, ctx=0x8019C1F8)))
    fixtures.append(('93B5C', 0x80193B5C, (1,), dict(a0=1, ctx=0x80142000, slot0=1)))
    fixtures.append(('93B5C', 0x80193B5C, (0xFFFFFFF8,), dict(a0=0xFFFFFFF8, ctx=0x80142000, slot0=1)))
    for kind, entry, args, extra in fixtures:
        r = bytearray(0x200000)
        r[0x10000:0x10000 + len(exe) - 0x800] = exe[0x800:]
        r[base & 0x1FFFFF:(base & 0x1FFFFF) + len(overlay)] = overlay
        for a, n in RANGES:
            r[a:a + n] = bytes(n)

        def sw(a, v):
            struct.pack_into('<I', r, a & 0x1FFFFF, v & 0xFFFFFFFF)

        sw(0xB0E4C, extra.get('bank', 0))
        sw(0x19C1F8, 0xDEADBEEF)
        sw(0x140000, 0xFEEDFACE)
        if kind == '93AB0':
            struct.pack_into('<h', r, 0x19C058, extra['n'])
            dest_word = extra['dest_word'] & 0xFFFFFFFF
            src_word = extra['src_word'] & 0xFFFFFFFF
            for i in range(0, 0xA8, 4):
                sw(0x19CAA8 + i, dest_word + i)
            for i in range(0, 0xA0, 4):
                sw(0x1EA268 + i, src_word + (i << 8))
        if kind in ('941A4', '94108'):
            sw(0x19C9C0, 0x80142000)
            sw(0x142000, 0x80142110)
            sw(0x142004, 0x80142200)
            sw(0x142200, 0x12AABBCC)
            for i in range(0, 0xC0, 4):
                sw(0x142110 + i, 0xA5A50000 + i)
        if kind in ('92740', '93B5C'):
            r[0x19C1F0] = extra.get('flag', 0) & 0xFF
            struct.pack_into('<h', r, 0x19C02C, extra.get('c02c', 0))
            ctx = extra.get('ctx', 0x80142000)
            sw(0x19C9C0, ctx)
            sw(0x19C1FC, 0x80144000)
            sw(0x142004, 0x80144000)
            sw(0x147FFC, 0x10AABBCC)
            sw(0x144028, 0x20CCDDEE)
            sw(0x1EA598, 0xAA000001)
            sw(0x1EA5BC, 0xBB000002)
            sw(0x19CA80, 0xCC000011)
            sw(0x19CA88, 0xCC000022)
            struct.pack_into('<h', r, 0x19CC52, 3)
            if extra.get('slot0'):
                r[0x1EA394] = 1
                sw(0x142000, 0x80142110)
                struct.pack_into('<hhh', r, 0x1EA370, 80, 0, 40)
                struct.pack_into('<h', r, 0x19CAAA, 0)
                struct.pack_into('<h', r, 0x19CAAE, 0)
                struct.pack_into('<h', r, 0x19CAB2, 0)
                for i in range(0, 0xC0, 4):
                    sw(0x142110 + i, 0xA5A50000 + i)
        initial = {a + i: struct.unpack_from('<I', r, a + i)[0]
                   for a, n in RANGES for i in range(0, n, 4)}
        if common is None:
            common = {a: v for a, v in initial.items() if v}
        first = len(patches)
        patches.extend((a, v) for a, v in initial.items() if v != common.get(a, 0))
        regs = execute(r, entry, args, instruction_budget=50000)
        arg = extra.get('a0', extra.get('dest', extra.get('n', extra.get('ctx', extra.get('flag', 0)))))
        cases.append((kind, first, len(patches), fingerprint(r), arg, regs[2] & 0xFFFFFFFF))
        print(kind, extra, hex(cases[-1][3]), hex(cases[-1][5]), flush=True)
    out = ['/* Generated by pe_m0000i_leaves_oracle.py; original overlay instruction execution. */']
    out.append(f'static const char DAY1_m0000i_91de8_sha[] = "{SHA_91DE8}";')
    out.append(f'static const char DAY1_m0000i_9bf8c_sha[] = "{SHA_9BF8C}";')
    out.append(f'static const char DAY1_m0000i_93ab0_sha[] = "{SHA_93AB0}";')
    out.append(f'static const char DAY1_m0000i_941a4_sha[] = "{SHA_941A4}";')
    out.append(f'static const char DAY1_m0000i_94108_sha[] = "{SHA_94108}";')
    out.append(f'static const char DAY1_m0000i_92740_sha[] = "{SHA_92740}";')
    out.append(f'static const char DAY1_m0000i_93b5c_sha[] = "{SHA_93B5C}";')
    for name, rows in (('ranges', RANGES), ('common', sorted(common.items())),
                       ('patches', patches)):
        out.append(f'static const uint32_t DAY1_m0000i_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a, b in rows)
        out.append('};')
    out.append('static const struct { unsigned kind,first,end; uint32_t arg,ret; uint64_t hash; } DAY1_m0000i_cases[]={')
    kind_id = {'91DE8': 0, '9BF8C': 1, '93AB0': 2, '941A4': 3, '94108': 4, '92740': 5, '93B5C': 6}
    out.extend(f'    {{{kind_id[k]},{a},{b},0x{arg & 0xFFFFFFFF:X}u,0x{ret:X}u,UINT64_C(0x{h:016X})}},'
               for k, a, b, h, arg, ret in cases)
    out.append('};')
    if '--write-header' in sys.argv:
        (ROOT / 'pc_port/tests/retail_m0000i_leaves_cases.h').write_text('\n'.join(out) + '\n')
    print('PASS:', len(cases), 'original M0000I leaf cases')
    print('91DE8', SHA_91DE8)
    print('9BF8C', SHA_9BF8C)
    print('93AB0', SHA_93AB0)
    print('941A4', SHA_941A4)
    print('94108', SHA_94108)
    print('92740', SHA_92740)
    print('93B5C', SHA_93B5C)


if __name__ == '__main__':
    main()

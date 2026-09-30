#!/usr/bin/env python3
"""Original opcode DD and following wait, including operand aliasing."""
import hashlib
import itertools
import struct
import sys
from pe_battle_hud_oracle import ROOT, execute

RANGES = ((0x910A0, 0x400), (0x9589C, 0x804), (0x9CE00, 4),
          (0x9D1A0, 4), (0x9D254, 4), (0x9D2F0, 4), (0x9D300, 4),
          (0x9DF70, 32), (0xA77F0, 32), (0xB6A80, 32), (0x140000, 0x400))


def fixture(exe, pattern, radius, angle):
    ram = bytearray(0x200000)
    ram[0x10000:0x10000 + len(exe) - 0x800] = exe[0x800:]
    for a, n in RANGES:
        if a not in (0x910A0, 0x9589C):
            ram[a:a+n] = bytes(n)

    def w(a, v):
        struct.pack_into('<I', ram, a, v & 0xFFFFFFFF)

    w(0x9D2F0, 0x80140000)
    w(0x9D300, 0x80140280)
    w(0x140028, 0xFFF08001)
    w(0x140030, 0x7FFFFF00)
    w(0x140280, 0x80140300)
    w(0x140288, 0x80)
    w(0x140290, 1)
    modes = [(pattern + i) % 5 for i in range(4)] if pattern < 5 else [1]*4
    slots = list(range(4)) if pattern < 5 else {
        5: [0, 1, 2, 2], 6: [0, 1, 0, 1], 7: [0, 1, 1, 0]}[pattern]
    word = 0xDD | (4 << 13)
    for i, mode in enumerate(modes):
        word |= mode << (17 + 3*i)
        ptr = 0x140308 + 4*i if mode == 0 else (
            (0x1400AC, 0xA77F0, 0x9DF70, 0xB6A80)[mode-1] + slots[i]*4)
        w(0x140308 + i*4, slots[i])
        if i < 2:
            w(ptr, (radius, angle)[i])
        elif pattern < 5:
            w(ptr, 0x12345678 + i)
    w(0x140300, word)
    w(0x140318, 2 | (1 << 13))
    w(0x14031C, 0)
    w(0x140320, 3)
    return ram


def fingerprint(ram):
    value = 14695981039346656037
    for a, n in RANGES:
        for b in ram[a:a+n]:
            value = ((value ^ b) * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return value


def main():
    exe = (ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    assert struct.unpack_from('<I', exe, 0x91414-0x10000+0x800)[0] == 0x8001A214
    angles = (0, 1, 0x3FF, 0x400, 0x401, 0x7FF, 0x800, 0x801,
              0xBFF, 0xC00, 0xFFF, 0x1000, 0x1400, -1, -32768, 32767, 0xFFFF8001)
    cases = list(itertools.product(range(5), (0, 1, 0x2008000, 0x80000000, 0x7FFFFFFF, -1), angles))
    cases += list(itertools.product(range(5, 8), (0x2008000,), angles))
    patches, rows, seen = [], [], set()
    base = None
    covered = {a+i for a, n in RANGES for i in range(n)}
    for pattern, radius, angle in cases:
        ram = fixture(exe, pattern, radius, angle)
        initial = {a+i: struct.unpack_from('<I', ram, a+i)[0]
                   for a, n in RANGES for i in range(0, n, 4)}
        if base is None:
            base = initial
        first = len(patches)
        patches.extend((a, v) for a, v in initial.items() if v != base[a])
        before = bytes(ram)
        execute(ram, 0x80017018, visited_pcs=seen)
        assert struct.unpack_from('<I', ram, 0x140280)[0] == 0x80140324
        assert all(a in covered for a in range(0x1FE000) if ram[a] != before[a])
        rows.append((first, len(patches), fingerprint(ram)))
    assert {0x8001A214, 0x80077DC4, 0x80077CF4, 0x8003708C} <= seen
    out = ['/* Generated complete original polar-coordinate VM cases. */']
    for name, data in (('ranges', RANGES), ('common', [(a, v) for a, v in base.items() if v]), ('patches', patches)):
        out.append(f'static const uint32_t polar_vm_{name}[][2]={{')
        out.extend(f' {{0x{a:X}u,0x{v:X}u}},' for a, v in data)
        out.append('};')
    out.append('static const struct { unsigned first,end; uint64_t hash; } polar_vm_cases[]={')
    out.extend(f' {{{a},{b},UINT64_C(0x{h:X})}},' for a, b, h in rows)
    out.append('};')
    if '--write-header' in sys.argv:
        (ROOT/'pc_port/tests/retail_polar_vm_cases.h').write_text('\n'.join(out)+'\n')
    print('PASS:', len(rows), 'complete original polar-coordinate VM cases')


if __name__ == '__main__':
    main()

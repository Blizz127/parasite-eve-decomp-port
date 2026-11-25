#!/usr/bin/env python3
"""Black-box vectors for libpad PadInfoMode (0x80082680) by seeded fuzzing.

The original code from the user's EXE runs under the test-only interpreter
with the original record hook (0x80084B20).  For each case a xorshift32
generator (seed per case) fills the two controller records
0x800A5B70..0x800A5D50 and a table zone 0x80150000..0x80151000.  Pattern 0
writes record words that are pointers into the zone; pattern 1 writes small
bytes; pattern 2 mixes both per word; pattern 3 writes bytes 0x00/0xFF (byte i
is 0xFF when bit 8*i of the generator word is set); pattern 4 is pattern 3
then byte +0xE4 of both records = (0,1,2,3,4,0x80,0xFF)[seed % 7].  The checker regenerates the same RAM
from the seed (same algorithm in C), so only (seed, pattern, args, result) is
stored.  Usage: pe_pad_info_vectors.py OUT.txt
"""
import struct
import sys

import pe_irq_callback_vectors as v

INFO = 0x80082680
REC_LO, REC_HI = 0x800A5B70, 0x800A5D50
ZONE_LO, ZONE_HI = 0x80150000, 0x80151000


def xs32(x):
    x ^= (x << 13) & 0xFFFFFFFF
    x ^= x >> 17
    x ^= (x << 5) & 0xFFFFFFFF
    return x & 0xFFFFFFFF


def fill(ram, seed, pattern):
    x = seed or 1
    for a in range(REC_LO, REC_HI, 4):
        x = xs32(x)
        use_ptr = pattern == 0 or (pattern == 2 and (x & 1))
        if pattern in (3, 4):   # bytes 0x00/0xFF chosen by bits 0, 8, 16, 24
            val = sum(0xFF << (8 * i) for i in range(4) if (x >> (8 * i)) & 1)
        elif use_ptr:
            val = ZONE_LO + (x % (ZONE_HI - ZONE_LO)) & ~3
        else:
            val = x & 0x07070707
        struct.pack_into('<I', ram, a & 0x1FFFFF, val & 0xFFFFFFFF)
    for a in range(ZONE_LO, ZONE_HI, 4):
        x = xs32(x)
        struct.pack_into('<I', ram, a & 0x1FFFFF, x & 0x0F0F0F0F)
    if pattern == 4:   # as 3, then byte +0xE4 of both records from the seed
        e4 = (0, 1, 2, 3, 4, 0x80, 0xFF)[(seed or 1) % 7]
        for rec in (0x800A5B70, 0x800A5C60):
            ram[(rec + 0xE4) & 0x1FFFFF] = e4


def main():
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out = []
    faults = 0
    for seed in range(1, 61):
        for pattern in (0, 1, 2):
            for port in (0x00, 0x10):
                for term in (0, 1, 2, 3, 4, 5, 6, 7, 8, -1, 100):
                    for offs in ((0, 1, 2, 3, -1, -2, 4, 7, 10) if term == 4 or seed <= 20
                                 else (0, 1, 2, 3, -1)):
                        v.EVENTS.clear()
                        v.RAM_WRITES.clear()
                        ram = bytearray(base)
                        struct.pack_into('<I', ram, 0x9B738, 0x80084B20)
                        fill(ram, seed * 7919 + pattern, pattern)
                        try:
                            v.call(ram, v.Bios(), INFO, [port, term & 0xFFFFFFFF, offs & 0xFFFFFFFF])
                            ret = [e[1] for e in v.EVENTS if e[0] == 'ret'][-1]
                            writes = len(v.RAM_WRITES)   # guest stores outside the stack
                        except AssertionError:
                            faults += 1
                            continue
                        out.append('%d %d %02X %d %d %08X %d' % (seed * 7919 + pattern, pattern,
                                                                  port, term, offs, ret, writes))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'faults', faults)


if __name__ == '__main__':
    main()

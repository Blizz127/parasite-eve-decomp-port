#!/usr/bin/env python3
"""Black-box vectors for libpad PadGetState (0x800825C0).

Runs the original code from the user's EXE under the test-only interpreter
(the store-traced execute() of pe_irq_callback_vectors.py).  The record hook
D_8009B738 is set to the original record function 0x80084B20, so the port's
controller record is at 0x800A5B70 + (port & 0xF0 ? 0xF0 : 0).  Each case
sets record fields, calls PadGetState(port) and records the return value and
every guest word written.  Field choice was guided by the lead's knowledge of
the record layout; the clean implementation is written from these vectors
only.  Output (text) stays outside the repo.
Usage: pe_pad_vectors.py OUT.txt
"""
import itertools
import struct
import sys

import pe_irq_callback_vectors as v

GETSTATE = 0x800825C0
REC = {0: 0x800A5B70, 0x10: 0x800A5C60}


def run(exe, port, fields):
    v.EVENTS.clear()
    v.RAM_WRITES.clear()
    ram = v.fixture(exe, 0, 0, 0)
    struct.pack_into('<I', ram, 0x9B738, 0x80084B20)
    for addr, size, val in fields:
        struct.pack_into({1: '<B', 4: '<I'}[size], ram, addr & 0x1FFFFF, val)
    before = bytes(ram)
    v.call(ram, v.Bios(), GETSTATE, [port])
    ret = [e[1] for e in v.EVENTS if e[0] == 'ret'][-1]
    writes = [(0x80000000 + a, ram[a:a + 4].hex()) for a in range(0x10000, 0x1F0000, 4)
              if ram[a:a + 4] != before[a:a + 4]]
    return ret, writes


def main():
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    out = []
    for port in (0x00, 0x10, 0x01, 0x11, 0x20):
        rec = REC[0x10 if port & 0xF0 else 0]
        for w34, self10, b38, cur, st in itertools.product(
                (0, 0x00010000, 0x0000FFFF), (0, 1), (0, 1, 0x80), (0, 1, 0xFF), range(9)):
            fields = [(rec + 0x34, 4, w34), (rec + 0x10, 4, rec if self10 else rec + 0x40),
                      (rec + 0x38, 1, b38), (rec + 0x30, 4, 0x80150000), (0x80150000, 1, cur),
                      (rec + 0x49, 1, st)]
            ret, writes = run(exe, port, fields)
            out.append('CASE %02X %s -> RET %08X WRITES %d' % (
                port, ' '.join('%08X/%d=%X' % f for f in fields), ret, len(writes)))
    # Discriminating cases: other +0x34 words and other link values.
    for port in (0x00, 0x10):
        rec = REC[0x10 if port & 0xF0 else 0]
        for w34, link, b38, cur, st in itertools.product(
                (0x00020000, 0xFFFF0000, 0x00018000, 0x00008000, 0x80000000),
                (rec, rec + 0x80, 0, 0x80150000), (0, 1), (0,), (2, 3, 6)):
            fields = [(rec + 0x34, 4, w34), (rec + 0x10, 4, link), (rec + 0x38, 1, b38),
                      (rec + 0x30, 4, 0x80150000), (0x80150000, 1, cur), (rec + 0x49, 1, st)]
            ret, writes = run(exe, port, fields)
            out.append('CASE %02X %s -> RET %08X WRITES %d' % (
                port, ' '.join('%08X/%d=%X' % f for f in fields), ret, len(writes)))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'with writes', sum(1 for o in out if not o.endswith('WRITES 0')))


if __name__ == '__main__':
    main()

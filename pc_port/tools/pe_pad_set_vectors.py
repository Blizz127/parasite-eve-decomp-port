#!/usr/bin/env python3
"""Black-box vectors for the libpad setters PadSetAct (0x80082974),
PadSetActAlign (0x800828F4) and PadSetMainMode (0x8008292C).

Same seeded memory as pe_pad_info_vectors.py (records + table zone from a
xorshift32 seed and pattern), original record hook 0x80084B20 and busy hook
0x80084F8C.  For each case the line records the busy hook's own result on the
record (the original 0x80084F8C run separately on the same memory), the call's
return value and every guest word written with its final value.
Usage: pe_pad_set_vectors.py OUT.txt
"""
import struct
import sys

import pe_irq_callback_vectors as v
import pe_pad_info_vectors as pi

ENTRIES = {'ACT': 0x80082974, 'ALIGN': 0x800828F4, 'MAIN': 0x8008292C}
REC = {0: 0x800A5B70, 0x10: 0x800A5C60}


def prep(base, seed, pattern):
    ram = bytearray(base)
    struct.pack_into('<I', ram, 0x9B738, 0x80084B20)
    struct.pack_into('<I', ram, 0x9B740, 0x80084F8C)
    pi.fill(ram, seed, pattern)
    return ram


def run(ram, entry, args):
    v.EVENTS.clear()
    v.RAM_WRITES.clear()
    v.call(ram, v.Bios(), entry, args)
    ret = [e[1] for e in v.EVENTS if e[0] == 'ret'][-1]
    words = sorted({a & ~3 for a in v.RAM_WRITES})
    return ret, [(a, struct.unpack_from('<I', ram, a & 0x1FFFFF)[0]) for a in words]


def main():
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out = []
    faults = 0
    for seed in range(1, 71):
        for pattern in ((0, 1, 2, 3, 4) if seed <= 40 else (3, 4)):
            s = seed * 7919 + pattern
            for port in (0x00, 0x10, 0x01):
                rec = REC[0x10 if port & 0xF0 else 0]
                busy = run(prep(base, s, pattern), 0x80084F8C, [rec])[0]
                calls = [('ACT', [port, 0x80150100, n]) for n in (0, 1, 6, 255, 256, 0x1FF)]
                calls += [('ALIGN', [port, d]) for d in (0x80150200, 0x80150300)]
                calls += [('MAIN', [port, m, l]) for m in (0, 1, 2, 3, 4, 0x80, 0xFF)
                          for l in (0, 1, 3, 0x80)]
                for kind, args in calls:
                    try:
                        ret, writes = run(prep(base, s, pattern), ENTRIES[kind], args)
                    except AssertionError:
                        faults += 1
                        continue
                    out.append('%d %d %s %s BUSY %08X RET %08X W %s' % (
                        s, pattern, kind, ' '.join('%X' % a for a in args), busy, ret,
                        ' '.join('%08X=%08X' % w for w in writes) or '-'))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'faults', faults)


if __name__ == '__main__':
    main()

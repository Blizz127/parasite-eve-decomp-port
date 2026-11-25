#!/usr/bin/env python3
"""Host GTE model vs the test-only interpreter, one cop2 command at a time.

For each command word and seed, a canonical GTE state is generated (every
register holds a value hardware would read back unchanged; psx-spx read
widths, see canon_data / canon_ctrl). The interpreter runs the single
instruction from that state. Each output line holds the command, the initial
control and data registers (C0/D0), the interpreter's final raw registers
(C1/D1) and whether the interpreter's FLAG computation is valid for the
command (F 1/0). pe_gte_cmd_diff.c runs the same command on the host model
and compares registers under the same read widths.

Edge seeding: matrix halfwords, vector components and IR values are drawn
partly from +-0x7FFF/0x8000/0xFFFF/0x1000/0/1, and translation words partly
from large values, so saturation and MAC overflow are exercised.

Usage: pe_gte_cmd_diff.py OUT.txt CMD [CMD ...]   (CMD hex, e.g. 486012)
"""
import struct
import sys

import pe_irq_callback_vectors as v

_hud = v._hud
_src = v.inspect.getsource(_hud.execute).replace(
    'final_gte.update(data=cop_data[:], control=cop_control[:])',
    'final_gte.update(data=cop_data[:], control=cop_control[:], flags_valid=flags_valid)')
_ns = dict(vars(_hud))
exec(compile(_src, _hud.__file__, 'exec'), _ns)
execute = _ns['execute']

CODE = 0x180000
E16 = (0, 1, 0xFFFF, 0x7FFF, 0x8000, 0x8001, 0x1000, 0xF000, 0x0800, 0xFFFE)
E32 = (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x00010000, 0x7FFF0000)


def xs32(x):
    x ^= (x << 13) & 0xFFFFFFFF
    x ^= x >> 17
    x ^= (x << 5) & 0xFFFFFFFF
    return x & 0xFFFFFFFF


def sx16(v):
    v &= 0xFFFF
    return (v - 0x10000 if v & 0x8000 else v) & 0xFFFFFFFF


def lzcr(v):
    bits = v ^ 0xFFFFFFFF if v & 0x80000000 else v
    return 32 - bits.bit_length()


def irgb(d):
    out = 0
    for i, r in enumerate((9, 10, 11)):
        c = (d[r] ^ 0x80000000) - 0x80000000
        c = max(0, min(0x1F, c >> 7))
        out |= c << (5 * i)
    return out


def canon_data(d):
    """Hardware mfc2 readback of a data register file (psx-spx)."""
    o = list(d)
    for r in (1, 3, 5, 8, 9, 10, 11):
        o[r] = sx16(d[r])
    for r in (7, 16, 17, 18, 19):
        o[r] = d[r] & 0xFFFF
    o[15] = o[14]
    o[28] = o[29] = irgb(o)
    o[31] = lzcr(d[30])
    return o


def canon_ctrl(c):
    """Hardware cfc2 readback of a control register file (psx-spx)."""
    o = list(c)
    for r in (4, 12, 20, 26, 27, 29, 30):
        o[r] = sx16(c[r])
    f = c[31] & 0x7FFFF000
    o[31] = f | (0x80000000 if f & 0x7F87E000 else 0)
    return o


def seeded(x, half):
    x = xs32(x)
    if x % 3 == 0:
        e = E16 if half else E32
        return e[(x >> 8) % len(e)], x
    if half:
        return x >> 16, x
    return x, x


def state(seed):
    x = seed * 2654435761 & 0xFFFFFFFF or 1
    c, d = [0] * 32, [0] * 32
    for r in range(32):
        lo, x = seeded(x, True)
        hi, x = seeded(x, True)
        c[r] = (hi << 16 | lo) & 0xFFFFFFFF
        if r in (5, 6, 7, 13, 14, 15, 21, 22, 23, 24, 25, 28):
            c[r], x = seeded(x, False)
        lo, x = seeded(x, True)
        hi, x = seeded(x, True)
        d[r] = (hi << 16 | lo) & 0xFFFFFFFF
        if r in (6, 20, 21, 22, 23, 24, 25, 26, 27, 30):
            d[r], x = seeded(x, False)
    return canon_ctrl(c), canon_data(d)


def main():
    out = sys.argv[1]
    cmds = [int(a, 16) for a in sys.argv[2:]]
    ram0 = bytearray(0x200000)
    lines = []
    for cmd in cmds:
        for seed in range(1, 4001):
            c0, d0 = state(seed * 7919 + cmd)
            ram = bytearray(ram0)
            struct.pack_into('<III', ram, CODE, 0x4A000000 | (cmd & 0x1FFFFFF), 0x03E00008, 0)
            fg = {}
            execute(ram, 0x80000000 | CODE, (), stop_at={0x80000000 | (CODE + 4)},
                    initial_cop_control=dict(enumerate(c0)), initial_cop_data=dict(enumerate(d0)),
                    instruction_budget=10, final_gte=fg)
            lines.append('%06X C0 %s D0 %s C1 %s D1 %s F %d' % (
                cmd, ' '.join('%X' % a for a in c0), ' '.join('%X' % a for a in d0),
                ' '.join('%X' % (a & 0xFFFFFFFF) for a in fg['control']),
                ' '.join('%X' % (a & 0xFFFFFFFF) for a in fg['data']),
                int(fg['flags_valid'])))
    open(out, 'w').write('\n'.join(lines) + '\n')
    print('cases', len(lines))


if __name__ == '__main__':
    main()

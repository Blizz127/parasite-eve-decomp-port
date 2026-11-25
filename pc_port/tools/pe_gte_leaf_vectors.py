#!/usr/bin/env python3
"""Black-box vectors for small libgte leaves (original code from the user's
EXE under the test-only interpreter).

Each case:
- loads the EXE text;
- fills a matrix zone (0x80100000-0x80101FFF) and a vector zone
  (0x80102000-0x80102FFF) with seeded words;
- seeds all 32 GTE control registers and all 32 data registers with seeded
  words;
- calls one entry with seeded arguments. Pointer arguments are aligned
  addresses in the zones; integer arguments mix small, edge and full 32-bit
  values.

Each line records:
- the return value;
- the GTE control registers before (C0) and after (C1) the call, raw as
  written by ctc2 (the interpreter keeps raw ctc2 values);
- whether any data register changed (D 0/1);
- every data load in order (address, size, value) and every store in order.

Pattern 1 (ScaleMatrix 0x80078CC4 only): the matrix halfwords and scale
words are set from edge lists (0, 1, 0x7FFF, 0x8000, 0xFFFF, ... and
0, 1, 0x1000, 0xFFFFF000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x10000).

Batch "stack" (PushMatrix 0x80078A94, PopMatrix 0x80078B38): the stack
index word 0x800963E8 is set from a list of values (multiples of 32 from
-64 to 704, some non-multiples, small negatives) and then from seeds (in-range multiples of 32, or values >= 0x400 for Push), and the
stack area 0x800963EC-0x8009666B from seeded words. A call to 0x80071A74
(the printf stub) is recorded as CALL a0 a1 and returns 0. The word
0x800963DC is not logged in cases that make that call: the original saves
its return address there around the call (a guest code address with no
meaning on the host). Other accesses to it (a stack frame overlapping it)
are logged.

Usage: pe_gte_leaf_vectors.py OUT.txt [ENTRY ... | stack]
"""
import struct
import sys

import pe_pad_leaf_vectors as pl

v = pl.v
ENTRIES = (0x80078E04, 0x80078E34, 0x80078E64, 0x80078E94, 0x80078FC4,
           0x80078FE4, 0x80079004, 0x80079024, 0x80078CC4)
# Arguments: 'm' matrix pointer, 'v' vector pointer, 'i' integer.
ARGS = {0x80078E04: 'm', 0x80078E34: 'm', 0x80078E64: 'm', 0x80078E94: 'm',
        0x80078FC4: 'iii', 0x80078FE4: 'iii', 0x80079004: 'ii',
        0x80079024: 'i', 0x80078CC4: 'mv'}
STACK_ENTRIES = (0x80078A94, 0x80078B38)
PRINTF = 0x80071A74
RA_SAVE = 0x963DC
SP_INDEX = 0x963E8
SP_AREA = (0x963EC, 0x96670)
SP_VALUES = tuple(range(-64, 708, 32)) + (4, 8, 28, 36, 600, 636, 639, 641, 644, 0x10000,
                                          -4, -8, -36, -40)
MZONE = (0x80100000, 0x2000)
VZONE = (0x80102000, 0x1000)
EDGE32 = (0, 1, 2, 0x7F, 0x80, 0xFF, 0x7FFF, 0x8000, 0xFFFF, 0x10000,
          0x0FFFFFFF, 0x10000000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF,
          0xFFFFFFFE, 0xFFFF8000, 0xFFFFF000)
EDGE16 = (0, 1, 0x7FF, 0x800, 0xFFF, 0x1000, 0x7FFF, 0x8000, 0x8001, 0xFFFF)
SCALE = (0, 1, 0x1000, 0xFFFFF000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x10000)


def xs32(x):
    x ^= (x << 13) & 0xFFFFFFFF
    x ^= x >> 17
    x ^= (x << 5) & 0xFFFFFFFF
    return x & 0xFFFFFFFF


def num(x):
    k = x % 5
    if k == 4:
        return EDGE32[(x >> 8) % len(EDGE32)]
    return (x >> 8) & (0xF, 0xFFF, 0xFFFF, 0xFFFFFFFF)[k]


def prep(base, seed):
    ram = bytearray(base)
    x = seed * 2654435761 & 0xFFFFFFFF or 1
    for a in range(MZONE[0] & 0x1FFFFF, (MZONE[0] & 0x1FFFFF) + MZONE[1] + VZONE[1], 4):
        x = xs32(x)
        struct.pack_into('<I', ram, a, x)
    ctrl, data = {}, {}
    for i in range(32):
        x = xs32(x)
        ctrl[i] = x
        x = xs32(x)
        data[i] = x
    return ram, x, ctrl, data


def args_for(entry, x):
    out = []
    for kind in ARGS[entry]:
        x = xs32(x)
        if kind == 'm':
            out.append(MZONE[0] + (x % (MZONE[1] - 0x40)) // 4 * 4)
        elif kind == 'v':
            out.append(VZONE[0] + (x % (VZONE[1] - 0x10)) // 4 * 4)
        else:
            out.append(num(x))
    return out, x


class Stop:
    """stop_at set that remembers which stop address was reached."""
    def __init__(self, pcs):
        self.pcs, self.hit = pcs, None

    def __contains__(self, pc):
        if pc in self.pcs:
            self.hit = pc
            return True
        return False


def run(ram, entry, args, ctrl, data):
    pl.LOADS.clear()
    pl.STORES.clear()
    pl.CUR[0] = entry
    calls = []
    data0 = [data[i] & 0xFFFFFFFF for i in range(32)]
    regs = {i: a for i, a in enumerate(args, 4)}
    regs[31] = v.RET
    pc = entry
    for _ in range(4):
        fg = {}
        st = Stop((v.RET, PRINTF))
        r = v.execute(ram, pc, (), stop_at=st, initial_regs=regs,
                      initial_cop_control=ctrl, initial_cop_data=data,
                      instruction_budget=10000, final_gte=fg)
        if st.hit != PRINTF:
            break
        calls.append((r[4] & 0xFFFFFFFF, r[5] & 0xFFFFFFFF))
        r[2] = 0
        regs = dict(enumerate(r))
        ctrl = dict(enumerate(fg['control']))
        data = dict(enumerate(fg['data']))
        pc = r[31]
    else:
        raise AssertionError('too many calls')
    changed = int(fg['data'] != data0)
    # The return-address save around the printf call is not logged; any other
    # access to that word (a frame overlapping it) is.
    keep = lambda e: not (calls and RA_SAVE <= e[0] - 0x80000000 < RA_SAVE + 4)
    return (r[2] & 0xFFFFFFFF, fg['control'], changed,
            [l for l in pl.LOADS if keep(l)], [t for t in pl.STORES if keep(t)], calls)


def line(entry, s, pattern, args, ctrl, res):
    ret, c1, changed, loads, stores, calls = res
    text = '%08X %d %d A %s RET %08X C0 %s C1 %s D %d L %s S %s' % (
        entry, s, pattern, ' '.join('%X' % a for a in args) or '-', ret,
        ' '.join('%X' % ctrl[i] for i in range(32)),
        ' '.join('%X' % c for c in c1), changed,
        ' '.join('%08X:%d=%X' % l for l in loads) or '-',
        ' '.join('%08X:%d=%X' % t for t in stores) or '-')
    if entry in STACK_ENTRIES:
        text += ' CALL ' + (' '.join('%08X,%08X' % c for c in calls) or '-')
    return text


def main():
    stack = sys.argv[2:] == ['stack']
    entries = STACK_ENTRIES if stack else [int(a, 16) for a in sys.argv[2:]] or ENTRIES
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out = []
    for entry in entries:
        if entry in STACK_ENTRIES:
            i = 0
            for sp in SP_VALUES + tuple(range(1000)):
                s = 0xA0000 + i
                i += 1
                ram, x, ctrl, data = prep(base, s)
                for a in range(SP_AREA[0], SP_AREA[1], 4):
                    x = xs32(x)
                    struct.pack_into('<I', ram, a, x)
                if i > len(SP_VALUES):      # seeded index: in range, or large for Push
                    x = xs32(x)
                    sp = (x % 21) * 32 if entry == 0x80078B38 or x & 1 else x & 0x7FFFFFFF | 0x400
                struct.pack_into('<I', ram, SP_INDEX, sp & 0xFFFFFFFF)
                out.append(line(entry, s, 2, [], ctrl, run(ram, entry, [], ctrl, data)))
            continue
        for seed in range(1, 1501):
            s = seed * 7919
            ram, x, ctrl, data = prep(base, s)
            args, x = args_for(entry, x)
            out.append(line(entry, s, 0, args, ctrl, run(ram, entry, args, ctrl, data)))
        if entry == 0x80078CC4:
            i = 0
            for h in EDGE16:
                for sc in SCALE:
                    for col in range(3):
                        s = 0x80000 + i
                        i += 1
                        ram, x, ctrl, data = prep(base, s)
                        args, x = args_for(entry, x)
                        m, vec = args[0] & 0x1FFFFF, args[1] & 0x1FFFFF
                        for k in range(10):     # nine cells plus the pad
                            x = xs32(x)
                            struct.pack_into('<H', ram, m + 2 * k,
                                             h if k % 3 == col or x & 1 else EDGE16[x % len(EDGE16)])
                        struct.pack_into('<I', ram, vec + 4 * col, sc)
                        out.append(line(entry, s, 1, args, ctrl, run(ram, entry, args, ctrl, data)))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out))


if __name__ == '__main__':
    main()

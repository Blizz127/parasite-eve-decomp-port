#!/usr/bin/env python3
"""List every reference to given EXE addresses in the retail program.

Scans the EXE text for jal/j to each address, aligned data words equal to it,
and lui + addiu/ori pairs (within 32 instructions) that build it; then scans
every extracted PE.IMG overlay blob for aligned jal/j/data words and the same
lui pairs.  Overlay hits are
reported with their offset so they can be checked against the overlay's
code/data split.  Used to show an SDK leaf is dead before it is unlinked
(gen_decomp_ports.py DEAD_SDK_UNLINKED).

Usage: sdk_ref_scan.py 0x8007DEC0 [0x... ...]
"""
import glob
import os
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def pairs(words, t):
    """Offsets (word index) of lui rt,%hi followed within 32 words by
    addiu/ori rd,rt,%lo building t."""
    out = []
    hi_a, hi_o = ((t + 0x8000) >> 16) & 0xFFFF, (t >> 16) & 0xFFFF
    lo = t & 0xFFFF
    for i, w in enumerate(words):
        if (w & 0xFFE00000) != 0x3C000000:
            continue
        imm, rt = w & 0xFFFF, (w >> 16) & 31
        for k in range(1, 33):
            if i + k >= len(words):
                break
            x = words[i + k]
            op = x >> 26
            if ((x >> 21) & 31) == rt and (x & 0xFFFF) == lo and (
                    (op == 9 and imm == hi_a) or (op == 13 and imm == hi_o)):
                out.append(i)
                break
    return out


def main():
    targets = [int(a, 16) for a in sys.argv[1:]]
    exe = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    t_addr, t_size = struct.unpack_from('<II', exe, 0x18)
    words = struct.unpack_from('<%dI' % (t_size // 4), exe, 0x800)
    for t in targets:
        jal, j = 0x0C000000 | ((t >> 2) & 0x3FFFFFF), 0x08000000 | ((t >> 2) & 0x3FFFFFF)
        hi, lo = (t + 0x8000) >> 16 & 0xFFFF, t & 0xFFFF
        refs = []
        for i, w in enumerate(words):
            a = t_addr + i * 4
            if w in (jal, j):
                refs.append('exe call/jump @%08X' % a)
            elif w == t:
                refs.append('exe data word @%08X' % a)

        refs += ['exe lui pair @%08X' % (t_addr + i * 4) for i in pairs(words, t)]
        pats = {struct.pack('<I', jal): 'jal', struct.pack('<I', j): 'j', struct.pack('<I', t): 'word'}
        for f in sorted(glob.glob(str(ROOT / 'build/extracted/disc1/peimg/*'))):
            d = open(f, 'rb').read()
            for p, kind in pats.items():
                i = d.find(p)
                while i != -1:
                    if i % 4 == 0:
                        refs.append('overlay %s %s @+0x%X' % (os.path.basename(f), kind, i))
                    i = d.find(p, i + 1)
            ow = struct.unpack_from('<%dI' % (len(d) // 4), d)
            refs += ['overlay %s lui pair @+0x%X' % (os.path.basename(f), i * 4) for i in pairs(ow, t)]
        print('%08X: %s' % (t, '; '.join(refs) if refs else 'no references'))


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Independent Phase 6E-B32 oracle for func_800614AC.

The 36 words below are an independent transcription.  They are checked
against the SHA-verified executable before this delay-slot-aware interpreter
executes the transcription.  Production C is never called.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x800614AC
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000

W = exe_words(0x800614AC, 36)

def u32(x): return x & 0xFFFFFFFF
def s32(x):
    x = u32(x)
    return x - 0x100000000 if x & 0x80000000 else x

class Oracle:
    def __init__(self, exe):
        data = open(exe, "rb").read()
        got = hashlib.sha1(data).hexdigest()
        if got != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {got} != {SHA1}")
        taddr = struct.unpack_from("<I", data, 0x18)[0]
        for i, want in enumerate(W):
            got = struct.unpack_from("<I", data, BASE + i * 4 - taddr + 0x800)[0]
            if got != want:
                raise SystemExit(f"FATAL: word {i} @ {BASE+i*4:08X}: "
                                 f"{got:08X} != {want:08X}")
        print(f"exe SHA-1 {SHA1} OK; func_800614AC: {len(W)} words verified")
        self.ram = bytearray(0x200000)
        self.r = [0] * 32
        self.r[28] = GP
        self.pc = 0
        self.steps = 0
        self.reads = []
        self.writes = []

    def off(self, a, n):
        if not (RAM_BASE <= a and a + n <= RAM_END):
            raise SystemExit(f"FATAL: guest access {a:08X}/{n}")
        return a - RAM_BASE

    def store(self, a, n, v):
        o = self.off(a, n)
        v = u32(v) & ((1 << (8 * n)) - 1)
        self.ram[o:o+n] = v.to_bytes(n, "little")
        self.writes.append((a, n, v))

    def one(self, w, addr):
        op = (w >> 26) & 63
        rs, rt, rd = (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        sa, fn = (w >> 6) & 31, w & 63
        imm = w & 0xFFFF
        simm = imm if imm < 0x8000 else imm - 0x10000
        r = self.r
        if w == 0:
            return
        if op == 0:
            if fn == 0: r[rd] = u32(r[rt] << sa)
            elif fn == 3: r[rd] = u32(s32(r[rt]) >> sa)
            elif fn == 8: raise SystemExit("FATAL: unexpected jr")
            elif fn == 0x21: r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x25: r[rd] = u32(r[rs] | r[rt])
            elif fn == 0x24: r[rd] = u32(r[rs] & r[rt])
            else: raise SystemExit(f"FATAL: SPECIAL {fn:02X} @{addr:08X}")
        elif op == 9: r[rt] = u32(r[rs] + simm)
        elif op == 0x0A: r[rt] = int(s32(r[rs]) < simm)
        elif op == 0x0F: r[rt] = u32(imm << 16)
        elif op == 0x0D: r[rt] = u32(r[rs] | imm)
        elif op == 0x0C: r[rt] = u32(r[rs] & imm)
        elif op == 0x2B: self.store(u32(r[rs] + simm), 4, r[rt])
        else: raise SystemExit(f"FATAL: opcode {op:02X} @{addr:08X}")
        r[0] = 0

    def run(self, arg):
        self.r[4] = u32(arg)
        while True:
            if self.steps > 100: raise SystemExit("FATAL: oracle did not terminate")
            self.steps += 1
            addr = BASE + self.pc * 4
            w = W[self.pc]
            op = (w >> 26) & 63
            rs, rt = (w >> 21) & 31, (w >> 16) & 31
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            fn = w & 63
            delay = W[self.pc + 1]
            def d(): self.one(delay, addr + 4)
            if op in (4, 5):
                take = (self.r[rs] == self.r[rt]) if op == 4 else (self.r[rs] != self.r[rt])
                d(); self.pc = ((addr + 4 + (simm << 2) - BASE) // 4) if take else self.pc + 2
                continue
            if op == 2:
                target = ((w & 0x03FFFFFF) << 2) | (addr & 0xF0000000)
                d(); self.pc = (target - BASE) // 4; continue
            if op == 0 and fn == 8:
                d(); return self.r[2]
            self.one(w, addr)
            self.pc += 1

def main():
    if len(sys.argv) != 2: raise SystemExit("usage: b32_oracle.py executable")
    cases = [(0x00404040, 0x00404040), (0x00FFFFFF, 0x00FFFFFF),
             (0x00FF0000, 0x00007F7F), (0x00FFFF00, 0x007F7FFF),
             (0x12345678, 0x00675645)]
    for arg, expected in cases:
        o = Oracle(sys.argv[1]); got = o.run(arg)
        assert got == expected, f"{arg:08X}: {got:08X} != {expected:08X}"
        assert o.writes == [(0x8009D14C, 4, arg & 0x00FFFFFF),
                            (0x8009D150, 4, expected)]
    print("PASS: B32 independent transcription, delay slots, packed-color arithmetic, writes, and returns")

if __name__ == "__main__": main()

#!/usr/bin/env python3
"""Independent Phase 6E-B33 oracle for func_8005E884.

The 4 words below are an independent transcription.  They are checked
against the SHA-verified executable before this delay-slot-aware interpreter
executes the transcription.  Production C is never called.
"""
import hashlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8005E884
RAM_BASE = 0x80000000
RAM_END = 0x80200000

W = [
    0x3C02800B,  # lui $v0, 0x800B
    0x80420DB1,  # lb  $v0, 0x0DB1($v0)
    0x03E00008,  # jr  $ra
    0x00000000,  # nop
]

def u32(x): return x & 0xFFFFFFFF
def s32(x):
    x = u32(x)
    return x - 0x100000000 if x & 0x80000000 else x

def s8(x):
    x = x & 0xFF
    return x - 0x100 if x & 0x80 else x

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
        print(f"exe SHA-1 {SHA1} OK; func_8005E884: {len(W)} words verified")
        self.ram = bytearray(0x200000)
        self.r = [0] * 32
        self.pc = 0
        self.steps = 0
        self.reads = []
        self.writes = []

    def off(self, a, n):
        if not (RAM_BASE <= a and a + n <= RAM_END):
            raise SystemExit(f"FATAL: guest access {a:08X}/{n}")
        return a - RAM_BASE

    def load_s8(self, a):
        o = self.off(a, 1)
        v = self.ram[o]
        self.reads.append((a, 1, v))
        return s8(v)

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
        elif op == 0x20:  # lb
            addr = u32(r[rs] + simm)
            r[rt] = u32(self.load_s8(addr))
        else: raise SystemExit(f"FATAL: opcode {op:02X} @{addr:08X}")
        r[0] = 0

    def run(self):
        while True:
            if self.steps > 100: raise SystemExit("FATAL: oracle did not terminate")
            self.steps += 1
            addr = BASE + self.pc * 4
            w = W[self.pc]
            op = (w >> 26) & 63
            fn = w & 63
            delay = W[self.pc + 1]
            def d(): self.one(delay, addr + 4)
            if op == 2:
                target = ((w & 0x03FFFFFF) << 2) | (addr & 0xF0000000)
                d(); self.pc = (target - BASE) // 4; continue
            if op == 0 and fn == 8:
                d(); return self.r[2]
            self.one(w, addr)
            self.pc += 1

def main():
    if len(sys.argv) != 2: raise SystemExit("usage: b33_oracle.py executable")

    # Test 1: unwritten BSS (0x00) → return 0
    o = Oracle(sys.argv[1])
    got = o.run()
    assert got == 0, f"unwritten BSS: {got} != 0"
    assert o.reads == [(0x800B0DB1, 1, 0x00)], f"unexpected reads: {o.reads}"
    assert o.writes == [], f"unexpected writes: {o.writes}"

    # Test 2: store 42 → return 42
    o = Oracle(sys.argv[1])
    o.ram[0x800B0DB1 - 0x80000000] = 42
    got = o.run()
    assert got == 42, f"positive byte: {got} != 42"

    # Test 3: store 0x80 → return -128 (signed)
    o = Oracle(sys.argv[1])
    o.ram[0x800B0DB1 - 0x80000000] = 0x80
    got = o.run()
    assert got == u32(-128), f"signed 0x80: {got:08X} != {u32(-128):08X}"

    # Test 4: store 0xFF → return -1 (signed)
    o = Oracle(sys.argv[1])
    o.ram[0x800B0DB1 - 0x80000000] = 0xFF
    got = o.run()
    assert got == u32(-1), f"signed 0xFF: {got:08X} != {u32(-1):08X}"

    # Test 5: store 0x7F → return 127
    o = Oracle(sys.argv[1])
    o.ram[0x800B0DB1 - 0x80000000] = 0x7F
    got = o.run()
    assert got == 127, f"signed 0x7F: {got} != 127"

    # Test 6: verify exactly one read, no writes, correct address
    o = Oracle(sys.argv[1])
    o.ram[0x800B0DB1 - 0x80000000] = 55
    got = o.run()
    assert got == 55
    assert len(o.reads) == 1, f"expected 1 read, got {len(o.reads)}"
    assert o.reads[0] == (0x800B0DB1, 1, 55), f"read wrong: {o.reads[0]}"
    assert o.writes == [], f"leaf must not write"

    print("PASS: B33 independent transcription, delay slots, signed-byte load, "
          "no writes, reads-only D_800B0DB1")

if __name__ == "__main__": main()

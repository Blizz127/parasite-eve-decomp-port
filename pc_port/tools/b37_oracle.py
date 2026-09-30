#!/usr/bin/env python3
"""Independent Phase 6E-B37 oracle for func_8006A2E8.

The 5 words below are an independent transcription.  They are checked
against the SHA-verified executable before this delay-slot-aware interpreter
executes the transcription.  Production C is never called.
"""
import hashlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8006A2E8
RAM_BASE = 0x80000000
RAM_END = 0x80200000

W = [
    0x2CA20010,  # sltiu $v0, $a1, 0x10
    0x10400008,  # beq   $v0, $zero, exit
    0x00000000,  # nop
    0x00A01021,  # addu  $v0, $a1, $zero
    0x3C01800C,  # lui   $at, 0x800C
    0xA422CE9E,  # sh    $v0, -0x3162($at)  -> 0x800BCE9E
    0x3C01800C,  # lui   $at, 0x800C
    0xA422CE8A,  # sh    $v0, -0x3176($at)  -> 0x800BCE8A
    0x3C01800B,  # lui   $at, 0x800B
    0xA0220DB1,  # sb    $v0, 0x0DB1($at)   -> 0x800B0DB1
    0x03E00008,  # jr    $ra
    0x00001021,  # addu  $v0, $zero, $zero  (delay slot)
]

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
        print(f"exe SHA-1 {SHA1} OK; func_8006A2E8: {len(W)} words verified")
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
        elif op == 0x0B: r[rt] = 1 if r[rs] < u32(simm) else 0  # sltiu
        elif op == 0x0F: r[rt] = u32(imm << 16)
        elif op == 0x0D: r[rt] = u32(r[rs] | imm)
        elif op == 0x29:  # sh
            a = u32(r[rs] + simm)
            self.store(a, 2, r[rt] & 0xFFFF)
        elif op == 0x28:  # sb
            a = u32(r[rs] + simm)
            self.store(a, 1, r[rt] & 0xFF)
        else: raise SystemExit(f"FATAL: opcode {op:02X} @{addr:08X}")
        r[0] = 0

    def run(self, a0, a1):
        self.r[4] = u32(a0)
        self.r[5] = u32(a1)
        while True:
            if self.steps > 100: raise SystemExit("FATAL: oracle did not terminate")
            self.steps += 1
            addr = BASE + self.pc * 4
            w = W[self.pc]
            op = (w >> 26) & 63
            rs = (w >> 21) & 31
            rt = (w >> 16) & 31
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            fn = w & 63
            delay = W[self.pc + 1]
            def d(): self.one(delay, addr + 4)
            if op == 4:  # beq
                take = (self.r[rs] == self.r[rt])
                d()  # delay slot executes before branch effect
                if take:
                    self.pc = (addr + 4 + (simm << 2) - BASE) // 4
                else:
                    self.pc += 2
                continue
            if op == 0 and fn == 8:  # jr $ra
                d()  # delay slot: addu $v0,$zero,$zero
                return self.r[2]
            self.one(w, addr)
            self.pc += 1

def main():
    if len(sys.argv) != 2: raise SystemExit("usage: b37_oracle.py executable")

    # Note: jr $ra delay slot is `addu $v0,$zero,$zero` → return always 0.
    # The stores use $v0 = $a1 (set by addu at word 3), but the return
    # register is overwritten in the jr delay slot.

    # Test 1: a1=8 < 16 → store 8, return 0 (delay slot overwrites $v0)
    o = Oracle(sys.argv[1])
    got = o.run(0, 8)
    assert got == 0, f"return: {got} != 0"
    assert o.writes == [(0x800BCE9E, 2, 8), (0x800BCE8A, 2, 8),
                        (0x800B0DB1, 1, 8)], f"writes: {o.writes}"
    print("  S1: a1=8 → stores(8,8,8), ret=0 OK")

    # Test 2: a1=15 < 16 → store 15, return 0
    o = Oracle(sys.argv[1])
    got = o.run(0, 15)
    assert got == 0, f"return: {got} != 0"
    assert o.writes == [(0x800BCE9E, 2, 15), (0x800BCE8A, 2, 15),
                        (0x800B0DB1, 1, 15)], f"writes: {o.writes}"
    print("  S2: a1=15 → stores(15,15,15), ret=0 OK")

    # Test 3: a1=16 >= 16 → no stores, return 0
    o = Oracle(sys.argv[1])
    got = o.run(0, 16)
    assert got == 0, f"return: {got} != 0"
    assert o.writes == [], f"writes: {o.writes}"
    print("  S3: a1=16 → no stores, ret=0 OK")

    # Test 4: a1=0 → store 0, return 0
    o = Oracle(sys.argv[1])
    got = o.run(99, 0)
    assert got == 0, f"return: {got} != 0"
    assert o.writes == [(0x800BCE9E, 2, 0), (0x800BCE8A, 2, 0),
                        (0x800B0DB1, 1, 0)], f"writes: {o.writes}"
    print("  S4: a1=0 → stores(0,0,0), ret=0 OK")

    # Test 5: a0 is ignored
    o = Oracle(sys.argv[1])
    o.run(0xFFFFFFFF, 5)
    assert o.writes[0][2] == 5, f"a0 leaked into writes"
    print("  S5: a0 ignored OK")

    # Test 6: a1=255 >= 16 → no stores
    o = Oracle(sys.argv[1])
    got = o.run(0, 255)
    assert got == 0, f"return: {got} != 0"
    assert o.writes == [], f"writes: {o.writes}"
    print("  S6: a1=255 → no stores, ret=0 OK")

    print("PASS: B37 independent transcription, delay slots, conditional stores, "
          "threshold, return, a0-ignored")

if __name__ == "__main__": main()

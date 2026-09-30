#!/usr/bin/env python3
"""Independent Phase 6E-B35 oracle for func_800649D0.

The 30 words below are an independent transcription.  They are checked
against the SHA-verified executable before this delay-slot-aware interpreter
executes the transcription.  Production C is never called.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x800649D0
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000

W = exe_words(0x800649D0, 30)
    # addiu $sp, $sp, -0x18
    # sw    $ra, 0x10($sp)
    # sw    $a0, 0x3FC($gp)
    # bne   $a0, $zero, exit
    # nop
    # lui   $a0, 0x800A
    # addiu $a0, $a0, 0x3060
    # jal   func_80071A24
    # addiu $a1, $zero, 0x120
    # addiu $v0, $zero, -1
    # lui   $at, 0x800A
    # sb    $v0, 0x3078($at)
    # lui   $at, 0x800A
    # sb    $v0, 0x30A0($at)
    # lui   $at, 0x800A
    # sb    $v0, 0x30B0($at)
    # lui   $at, 0x800A
    # sb    $v0, 0x30B8($at)
    # lui   $at, 0x800A
    # sb    $v0, 0x30C0($at)
    # lui   $at, 0x800A
    # sb    $v0, 0x30C4($at)
    # lui   $at, 0x800A
    # sb    $v0, 0x3124($at)
    # lui   $at, 0x800A
    # sb    $v0, 0x3134($at)
    # lw    $ra, 0x10($sp)
    # addiu $sp, $sp, 0x18
    # jr    $ra
    # nop

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
        print(f"exe SHA-1 {SHA1} OK; func_800649D0: {len(W)} words verified")
        self.ram = bytearray(0x200000)
        self.r = [0] * 32
        self.r[28] = GP
        self.r[29] = 0x801FFF00  # sp
        self.pc = 0
        self.steps = 0
        self.reads = []
        self.writes = []
        self.calls = []

    def off(self, a, n):
        if not (RAM_BASE <= a and a + n <= RAM_END):
            raise SystemExit(f"FATAL: guest access {a:08X}/{n}")
        return a - RAM_BASE

    def store(self, a, n, v):
        o = self.off(a, n)
        v = u32(v) & ((1 << (8 * n)) - 1)
        self.ram[o:o+n] = v.to_bytes(n, "little")
        self.writes.append((a, n, v))

    def load_u32(self, a):
        o = self.off(a, 4)
        v = struct.unpack_from("<I", self.ram, o)[0]
        self.reads.append((a, 4, v))
        return v

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
        elif op == 0x0F: r[rt] = u32(imm << 16)
        elif op == 0x0D: r[rt] = u32(r[rs] | imm)
        elif op == 0x23:  # lw
            a = u32(r[rs] + simm)
            r[rt] = self.load_u32(a)
        elif op == 0x2B:  # sw
            a = u32(r[rs] + simm)
            self.store(a, 4, r[rt])
        elif op == 0x28:  # sb
            a = u32(r[rs] + simm)
            self.store(a, 1, r[rt] & 0xFF)
        else: raise SystemExit(f"FATAL: opcode {op:02X} @{addr:08X}")
        r[0] = 0

    def run(self, a0):
        self.r[4] = u32(a0)
        while True:
            if self.steps > 200: raise SystemExit("FATAL: oracle did not terminate")
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
            # bne rs,rt,target
            if op == 5:
                take = (self.r[rs] != self.r[rt])
                d()
                if take:
                    target_pc = (addr + 4 + (simm << 2) - BASE) // 4
                    self.pc = target_pc
                else:
                    self.pc += 2
                continue
            if op == 3:  # jal
                target = ((w & 0x03FFFFFF) << 2) | (addr & 0xF0000000)
                d()  # delay slot executes before callee sees args
                # func_80071A24: bzero — model by zeroing the range
                dst = self.r[4]
                length = self.r[5]
                if dst >= RAM_BASE and dst + length <= RAM_END:
                    for off in range(0, length, 4):
                        self.store(dst + off, 4, 0)
                    # Also zero individual bytes if length not multiple of 4
                    for off in range(length - (length % 4), length):
                        self.store(dst + off, 1, 0)
                self.calls.append(("func_80071A24", dst, length))
                self.r[2] = dst
                self.r[31] = u32(addr + 8)
                self.pc = ((addr + 8) - BASE) // 4
                continue
            if op == 0 and fn == 8:
                d()
                return
            self.one(w, addr)
            self.pc += 1

def main():
    if len(sys.argv) != 2: raise SystemExit("usage: b35_oracle.py executable")

    # Test 1: a0=0 → bzero + 8 stores
    o = Oracle(sys.argv[1])
    o.ram[0x800A3060 - 0x80000000:0x800A3060 - 0x80000000 + 0x120] = b'\xAA' * 0x120
    o.run(0)
    # D_8009D16C = 0
    state = struct.unpack_from("<I", o.ram, 0x8009D16C - 0x80000000)[0]
    assert state == 0, f"state: {state} != 0"
    # bzero range cleared (8 bytes later overwritten with 0xFF)
    ff_addrs = {0x800A3078, 0x800A30A0, 0x800A30B0, 0x800A30B8,
                0x800A30C0, 0x800A30C4, 0x800A3124, 0x800A3134}
    for off in range(0, 0x120):
        a = 0x800A3060 + off
        v = o.ram[a - 0x80000000]
        expected = 0xFF if a in ff_addrs else 0
        assert v == expected, f"miss @ {a:08X}: {v:#x} != {expected:#x}"
    # 8 bytes at 0xFF
    for addr in [0x800A3078, 0x800A30A0, 0x800A30B0, 0x800A30B8,
                 0x800A30C0, 0x800A30C4, 0x800A3124, 0x800A3134]:
        v = o.ram[addr - 0x80000000]
        assert v == 0xFF, f"byte @{addr:08X}: {v:#x} != 0xFF"
    print("  S1: a0=0 bzero+stores OK")

    # Test 2: a0=1 → only state store
    o = Oracle(sys.argv[1])
    for i in range(0, 0x200000, 4):
        struct.pack_into("<I", o.ram, i, 0xA5A5A5A5)
    o.run(1)
    state = struct.unpack_from("<I", o.ram, 0x8009D16C - 0x80000000)[0]
    assert state == 1, f"state: {state} != 1"
    # No other writes — check a few spots
    for addr in [0x800A3060, 0x800A3078, 0x800A3134]:
        v = struct.unpack_from("<I", o.ram, addr - 0x80000000)[0]
        assert v == 0xA5A5A5A5, f"unexpected write @{addr:08X}: {v:#x}"
    print("  S2: a0=1 state-only OK")

    # Test 3: verify bzero call arguments
    o = Oracle(sys.argv[1])
    o.run(0)
    assert o.calls == [("func_80071A24", 0x800A3060, 0x120)], f"calls: {o.calls}"
    print("  S3: bzero args OK")

    # Test 4: guard bytes outside range untouched
    o = Oracle(sys.argv[1])
    for i in range(0, 0x200000, 4):
        struct.pack_into("<I", o.ram, i, 0xA5A5A5A5)
    o.run(0)
    below = struct.unpack_from("<I", o.ram, 0x800A305C - 0x80000000)[0]
    above = struct.unpack_from("<I", o.ram, 0x800A3180 - 0x80000000)[0]
    assert below == 0xA5A5A5A5, f"guard below: {below:#x}"
    assert above == 0xA5A5A5A5, f"guard above: {above:#x}"
    print("  S4: guard bytes OK")

    print("PASS: B35 independent transcription, delay slots, bzero path, "
          "nonzero path, 0xFF stores, guards")

if __name__ == "__main__": main()

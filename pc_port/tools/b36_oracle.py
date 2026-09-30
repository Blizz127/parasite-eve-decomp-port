#!/usr/bin/env python3
"""Independent Phase 6E-B36 oracle for func_80052790.

The 9 words below are an independent transcription.  They are checked
against the SHA-verified executable before this delay-slot-aware interpreter
executes the transcription.  Production C is never called.
"""
import hashlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80052790
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000

W = [
    0x27BDFFE8,  # addiu $sp, $sp, -0x18
    0xAF8402B0,  # sw    $a0, 0x2B0($gp)
    0xAFBF0010,  # sw    $ra, 0x10($sp)
    0x0C0219CA,  # jal   func_80086728
    0x2C840001,  # sltiu $a0, $a0, 1     (delay slot)
    0x8FBF0010,  # lw    $ra, 0x10($sp)
    0x27BD0018,  # addiu $sp, $sp, 0x18
    0x03E00008,  # jr    $ra
    0x00000000,  # nop
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
        print(f"exe SHA-1 {SHA1} OK; func_80052790: {len(W)} words verified")
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
        elif op == 0x0C: r[rt] = u32(r[rs] & imm)
        elif op == 0x0B: r[rt] = 1 if r[rs] < u32(simm) else 0  # sltiu
        elif op == 0x23:  # lw
            a = u32(r[rs] + simm)
            o = self.off(a, 4)
            r[rt] = struct.unpack_from("<I", self.ram, o)[0]
            self.reads.append((a, 4, r[rt]))
        elif op == 0x2B:  # sw
            a = u32(r[rs] + simm)
            self.store(a, 4, r[rt])
        else: raise SystemExit(f"FATAL: opcode {op:02X} @{addr:08X}")
        r[0] = 0

    def run(self, a0):
        self.r[4] = u32(a0)
        while True:
            if self.steps > 100: raise SystemExit("FATAL: oracle did not terminate")
            self.steps += 1
            addr = BASE + self.pc * 4
            w = W[self.pc]
            op = (w >> 26) & 63
            fn = w & 63
            delay = W[self.pc + 1]
            def d(): self.one(delay, addr + 4)
            if op == 3:  # jal
                target = ((w & 0x03FFFFFF) << 2) | (addr & 0xF0000000)
                d()  # delay slot (sltiu) executes before callee
                bool_arg = self.r[4]
                self.calls.append(("func_80086728", bool_arg))
                self.r[2] = 0
                self.r[31] = u32(addr + 8)
                self.pc = ((addr + 8) - BASE) // 4
                continue
            if op == 0 and fn == 8:
                d()
                return
            self.one(w, addr)
            self.pc += 1

def main():
    if len(sys.argv) != 2: raise SystemExit("usage: b36_oracle.py executable")

    # Test 1: a0=0 → store 0, boolean = (0 < 1) = 1
    o = Oracle(sys.argv[1])
    o.run(0)
    state = struct.unpack_from("<I", o.ram, GP + 0x2B0 - RAM_BASE)[0]
    assert state == 0, f"state: {state} != 0"
    assert o.calls == [("func_80086728", 1)], f"calls: {o.calls}"
    print("  S1: a0=0 → state=0, bool=1 OK")

    # Test 2: a0=1 → store 1, boolean = (1 < 1) = 0
    o = Oracle(sys.argv[1])
    o.run(1)
    state = struct.unpack_from("<I", o.ram, GP + 0x2B0 - RAM_BASE)[0]
    assert state == 1, f"state: {state} != 1"
    assert o.calls == [("func_80086728", 0)], f"calls: {o.calls}"
    print("  S2: a0=1 → state=1, bool=0 OK")

    # Test 3: a0=42 → store 42, boolean = (42 < 1) = 0
    o = Oracle(sys.argv[1])
    o.run(42)
    state = struct.unpack_from("<I", o.ram, GP + 0x2B0 - RAM_BASE)[0]
    assert state == 42, f"state: {state} != 42"
    assert o.calls == [("func_80086728", 0)], f"calls: {o.calls}"
    print("  S3: a0=42 → state=42, bool=0 OK")

    # Test 4: verify exact write footprint
    o = Oracle(sys.argv[1])
    o.run(5)
    state_writes = [w for w in o.writes if w[0] == GP + 0x2B0]
    assert len(state_writes) == 1, f"expected 1 state write, got {len(state_writes)}"
    assert state_writes[0] == (GP + 0x2B0, 4, 5), f"state write wrong: {state_writes[0]}"
    # Only the state write + stack (ra save/restore)
    guest_writes = [w for w in o.writes if w[0] >= RAM_BASE and w[0] < RAM_END
                    and not (0x801FFF00 - 0x18 <= w[0] < 0x801FFF00)]
    assert len(guest_writes) == 1, f"expected 1 guest write, got {len(guest_writes)}"
    print("  S4: exact write footprint OK")

    print("PASS: B36 independent transcription, delay slots, state store, "
          "boolean conversion, call args, footprint")

if __name__ == "__main__": main()

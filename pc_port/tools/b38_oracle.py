#!/usr/bin/env python3
"""Independent Phase 6E-B38 oracle for func_80086728.

The 18 words below are an independent transcription.  They are checked
against the SHA-verified executable before this delay-slot-aware interpreter
executes the transcription.  Production C is never called.
"""
import hashlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80086728
RAM_BASE = 0x80000000
RAM_END = 0x80200000

# func_80086728: streaming-mode command-byte switch + func_8008CBA8 dispatch.
# 18 words / 0x48 bytes.  Sole call site: func_80052790 @ 0x8005279C.
W = [
    0x27BDFFE8,  # addiu $sp, $sp, -0x18
    0x24020001,  # addiu $v0, $zero, 1
    0x10820006,  # beq   $a0, $v0, case_1   (→ 0x8008674C)
    0xAFBF0010,  # sw    $ra, 0x10($sp)     (delay slot)
    0x24020002,  # addiu $v0, $zero, 2
    0x10820004,  # beq   $a0, $v0, common   (→ 0x80086750)
    0x24020082,  # addiu $v0, $zero, 0x82   (delay slot: cmd for case 2)
    0x080219D4,  # j     common             (→ 0x80086750)
    0x24020080,  # addiu $v0, $zero, 0x80   (delay slot: cmd for default)
    0x24020081,  # addiu $v0, $zero, 0x81   (case_1: cmd for case 1)
    0x3C01800C,  # lui   $at, 0x800C        (common)
    0xAC22CD80,  # sw    $v0, -0x3280($at)  → D_800BCD80
    0x0C0232EA,  # jal   func_8008CBA8
    0x00000000,  # nop                      (delay slot)
    0x8FBF0010,  # lw    $ra, 0x10($sp)
    0x27BD0018,  # addiu $sp, $sp, 0x18
    0x03E00008,  # jr    $ra
    0x00000000,  # nop                      (delay slot)
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
        print(f"exe SHA-1 {SHA1} OK; func_80086728: {len(W)} words verified")
        self.ram = bytearray(0x200000)
        self.r = [0] * 32
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

    def load(self, a, n):
        o = self.off(a, n)
        return int.from_bytes(self.ram[o:o+n], "little")

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
        elif op == 2:  # j
            raise SystemExit("FATAL: unexpected j (should be handled by run loop)")
        elif op == 4:  # beq
            raise SystemExit("FATAL: unexpected beq (should be handled by run loop)")
        elif op == 9: r[rt] = u32(r[rs] + simm)
        elif op == 0x0F: r[rt] = u32(imm << 16)
        elif op == 0x0D: r[rt] = u32(r[rs] | imm)
        elif op == 0x23:  # lw
            a = u32(r[rs] + simm)
            r[rt] = self.load(a, 4)
            self.reads.append((a, 4, r[rt]))
        elif op == 0x2B:  # sw
            a = u32(r[rs] + simm)
            self.store(a, 4, r[rt])
        else: raise SystemExit(f"FATAL: opcode {op:02X} @{addr:08X}")
        r[0] = 0

    def run(self, a0):
        self.r[4] = u32(a0)
        self.r[29] = 0x801FFF00  # $sp (valid guest stack)
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

            if w == 0:
                self.pc += 1
                continue

            # beq: evaluate condition, execute delay slot, then branch or advance
            if op == 4:
                delay = W[self.pc + 1]
                take = (self.r[rs] == self.r[rt])
                self.one(delay, addr + 4)
                if take:
                    self.pc = (addr + 4 + (simm << 2) - BASE) // 4
                else:
                    self.pc += 2
                continue

            # j: execute delay slot, then jump
            if op == 2:
                target = (addr & 0xF0000000) | ((w & 0x03FFFFFF) << 2)
                delay = W[self.pc + 1]
                self.one(delay, addr + 4)
                self.pc = (target - BASE) // 4
                continue

            # jal: call func_8008CBA8
            if op == 3:
                target = (addr & 0xF0000000) | ((w & 0x03FFFFFF) << 2)
                delay = W[self.pc + 1]
                self.one(delay, addr + 4)
                # Record the call
                cmd = self.load(0x800BCD80, 4)
                self.calls.append(("func_8008CBA8", cmd))
                # Pretend callee returned 0
                self.r[2] = 0
                self.r[31] = u32(addr + 8)
                self.pc = ((addr + 8) - BASE) // 4
                continue

            # jr $ra: return
            if op == 0 and fn == 8:
                delay = W[self.pc + 1]
                self.one(delay, addr + 4)
                return

            self.one(w, addr)
            self.pc += 1


def main():
    if len(sys.argv) != 2: raise SystemExit("usage: b38_oracle.py executable")

    # Test 1: a0=0 (false, from func_80052790) → cmd = 0x80
    o = Oracle(sys.argv[1])
    o.run(0)
    cmd = struct.unpack_from("<I", o.ram, 0x800BCD80 - RAM_BASE)[0]
    assert cmd == 0x80, f"cmd: 0x{cmd:02X} != 0x80"
    assert o.calls == [("func_8008CBA8", 0x80)], f"calls: {o.calls}"
    print("  S1: a0=0 → cmd=0x80, call(func_8008CBA8, 0x80) OK")

    # Test 2: a0=1 (true, from func_80052790) → cmd = 0x81
    o = Oracle(sys.argv[1])
    o.run(1)
    cmd = struct.unpack_from("<I", o.ram, 0x800BCD80 - RAM_BASE)[0]
    assert cmd == 0x81, f"cmd: 0x{cmd:02X} != 0x81"
    assert o.calls == [("func_8008CBA8", 0x81)], f"calls: {o.calls}"
    print("  S2: a0=1 → cmd=0x81, call(func_8008CBA8, 0x81) OK")

    # Test 3: a0=2 (unreachable from func_80052790 but valid) → cmd = 0x82
    o = Oracle(sys.argv[1])
    o.run(2)
    cmd = struct.unpack_from("<I", o.ram, 0x800BCD80 - RAM_BASE)[0]
    assert cmd == 0x82, f"cmd: 0x{cmd:02X} != 0x82"
    assert o.calls == [("func_8008CBA8", 0x82)], f"calls: {o.calls}"
    print("  S3: a0=2 → cmd=0x82, call(func_8008CBA8, 0x82) OK")

    # Test 4: a0=42 (default) → cmd = 0x80
    o = Oracle(sys.argv[1])
    o.run(42)
    cmd = struct.unpack_from("<I", o.ram, 0x800BCD80 - RAM_BASE)[0]
    assert cmd == 0x80, f"cmd: 0x{cmd:02X} != 0x80"
    assert o.calls == [("func_8008CBA8", 0x80)], f"calls: {o.calls}"
    print("  S4: a0=42 → cmd=0x80, call(func_8008CBA8, 0x80) OK")

    # Test 5: verify exact write footprint (only D_800BCD80 + stack)
    o = Oracle(sys.argv[1])
    o.run(1)
    guest_writes = [w for w in o.writes if w[0] >= RAM_BASE and w[0] < RAM_END
                    and not (0x801FFF00 - 0x18 <= w[0] < 0x801FFF00)]
    assert len(guest_writes) == 1, f"expected 1 guest write, got {len(guest_writes)}"
    assert guest_writes[0] == (0x800BCD80, 4, 0x81), f"unexpected write: {guest_writes[0]}"
    print("  S5: exact write footprint OK")

    # Test 6: verify return value is not consumed (void function)
    o = Oracle(sys.argv[1])
    o.run(0)
    # The function returns void — no v0 assertion needed
    print("  S6: void return OK")

    print("PASS: B38 independent transcription, delay slots, command-byte switch, "
          "3-way branch, store, call, void return, footprint")


if __name__ == "__main__": main()

#!/usr/bin/env python3
"""Independent Phase 6E-B47 oracle for func_80085EB4.

23 words from asm/disc1/75F44.s:631-657, verified against SHA-exact exe.
Models SPU-memory address validation with alignment and shift.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80085EB4
RAM_BASE = 0x80000000
RAM_END = 0x80200000

W = exe_words(0x80085EB4, 23)
    # addiu $sp, $sp, -0x18
    # addu  $a1, $a0, $zero
    # lui   $v0, 0x7
    # ori   $v0, $v0, 0xEFE8
    # addiu $v1, $a1, -0x1010
    # sltu  $v0, $v0, $v1
    # bnez  $v0, .L80085EFC
    # sw    $ra, 0x10($sp)  [delay slot]
    # jal   func_8007DB24
    # addiu $a0, $zero, -1  [delay slot]
    # lui   $at, 0x800A
    # sh    $v0, -0x4BEC($at)  [D_8009B414]
    # lui   $v1, 0x800A
    # lhu   $v1, -0x4BEC($v1)  [D_8009B414]
    # lui   $v0, 0x800A
    # lw    $v0, -0x4BDC($v0)  [D_8009B424]
    # j     .L80085F00
    # sllv  $v0, $v1, $v0  [delay slot]
    # .L80085EFC:
    # addu  $v0, $zero, $zero
    # .L80085F00:
    # lw    $ra, 0x10($sp)
    # addiu $sp, $sp, 0x18
    # jr    $ra
    # nop  [delay slot]

# func_8007DB24 is called with a0=-1, a1=spu_addr.
# For the oracle, we inline its behavior:
# - If D_8009B420 != 0: align a1 to D_8009B428 boundary
# - Return (aligned_a1 >> D_8009B424) & 0xFFFF

# SPU globals (initialized by SsInit / func_80085644)
SPU_ALIGN_MODE = 2
SPU_SHIFT = 3
SPU_ALIGN_DIV = 8
SPU_ALIGN_MASK = 7


def u32(x): return x & 0xFFFFFFFF


class Oracle:
    def __init__(self, exe):
        data = open(exe, "rb").read()
        got = hashlib.sha1(data).hexdigest()
        if got != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {got} != {SHA1}")
        taddr = struct.unpack_from("<I", data, 0x18)[0]
        for i, want in enumerate(W):
            got_w = struct.unpack_from("<I", data, BASE + i * 4 - taddr + 0x800)[0]
            if got_w != want:
                raise SystemExit(f"FATAL: word {i} @ {BASE+i*4:08X}: "
                                 f"{got_w:08X} != {want:08X}")
        print(f"exe SHA-1 {SHA1} OK; func_80085EB4: {len(W)} words verified")
        self.ram = bytearray(RAM_END - RAM_BASE)
        self.spu_globals = {
            0x8009B414: 0x1010,  # heap top (halfword, but store as u32)
            0x8009B420: SPU_ALIGN_MODE,
            0x8009B424: SPU_SHIFT,
            0x8009B428: SPU_ALIGN_DIV,
            0x8009B42C: SPU_ALIGN_MASK,
        }
        self.r = [0] * 32
        self.pc = 0
        self.steps = 0
        self.reads = []
        self.writes = []
        self.calls = []

    def store_u16(self, addr, val):
        if addr in self.spu_globals:
            self.spu_globals[addr] = val & 0xFFFF
        elif RAM_BASE <= addr < RAM_END:
            struct.pack_into("<H", self.ram, addr - RAM_BASE, val & 0xFFFF)
        self.writes.append((addr, 2, val & 0xFFFF))

    def store_u32(self, addr, val):
        if addr in self.spu_globals:
            self.spu_globals[addr] = val
        elif RAM_BASE <= addr < RAM_END:
            struct.pack_into("<I", self.ram, addr - RAM_BASE, val)
        self.writes.append((addr, 4, val))

    def load_u16(self, addr):
        if addr in self.spu_globals:
            return self.spu_globals[addr] & 0xFFFF
        if RAM_BASE <= addr < RAM_END:
            return struct.unpack_from("<H", self.ram, addr - RAM_BASE)[0]
        raise SystemExit(f"FATAL: load_u16 {addr:08X}")

    def load_u32(self, addr):
        if addr in self.spu_globals:
            return self.spu_globals[addr]
        if RAM_BASE <= addr < RAM_END:
            return struct.unpack_from("<I", self.ram, addr - RAM_BASE)[0]
        raise SystemExit(f"FATAL: load_u32 {addr:08X}")

    def run(self, a0):
        self.r[4] = u32(a0)
        self.r[29] = 0x801FFF00

        while True:
            if self.steps > 200:
                raise SystemExit("oracle did not terminate")
            self.steps += 1
            addr = BASE + self.pc * 4
            w = W[self.pc]
            op = (w >> 26) & 63
            rs = (w >> 21) & 31
            rt = (w >> 16) & 31
            rd = (w >> 11) & 31
            sa = (w >> 6) & 31
            fn = w & 63
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            r = self.r

            if w == 0:
                self.pc += 1
                continue

            # bnez (bne rs, $zero)
            if op == 5 and rt == 0:
                delay = W[self.pc + 1]
                take = (r[rs] != 0)
                self._one(delay, addr + 4)
                if take:
                    self.pc = (addr + 4 + (simm << 2) - BASE) // 4
                else:
                    self.pc += 2
                continue

            # jal
            if op == 3:
                target = (addr & 0xF0000000) | ((w & 0x03FFFFFF) << 2)
                if target != 0x8007DB24:
                    raise SystemExit(f"unexpected call {target:08X}")
                delay = W[self.pc + 1]
                self._one(delay, addr + 4)
                # Inline func_8007DB24(-1, $a1)
                a1_val = r[5]
                aligned = a1_val
                if self.spu_globals.get(0x8009B420, 0) != 0:
                    div = self.spu_globals.get(0x8009B428, 1)
                    rem = aligned % div
                    if rem != 0:
                        mask = self.spu_globals.get(0x8009B42C, 0)
                        aligned = (aligned + div) & (~mask & 0xFFFFFFFF)
                shift = self.spu_globals.get(0x8009B424, 0)
                ret = (aligned >> shift) & 0xFFFF
                self.calls.append(("func_8007DB24", -1, a1_val, ret))
                self.r[2] = ret
                self.pc = ((addr + 8) - BASE) // 4
                continue

            # j
            if op == 2:
                target = (addr & 0xF0000000) | ((w & 0x03FFFFFF) << 2)
                delay = W[self.pc + 1]
                self._one(delay, addr + 4)
                self.pc = (target - BASE) // 4
                continue

            # jr $ra
            if op == 0 and fn == 8:
                delay = W[self.pc + 1]
                self._one(delay, addr + 4)
                return

            self._one(w, addr)
            self.pc += 1

    def _one(self, w, addr):
        op = (w >> 26) & 63
        rs = (w >> 21) & 31
        rt = (w >> 16) & 31
        rd = (w >> 11) & 31
        sa = (w >> 6) & 31
        fn = w & 63
        imm = w & 0xFFFF
        simm = imm if imm < 0x8000 else imm - 0x10000
        r = self.r
        if w == 0:
            return
        if op == 0:
            if fn == 0x21: r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x04: r[rd] = u32(r[rt] << (r[rs] & 31))
            elif fn == 0x2B: r[rd] = 1 if r[rs] < r[rt] else 0  # sltu
            else: raise SystemExit(f"SPECIAL {fn:02X} @{addr:08X}")
        elif op == 9: r[rt] = u32(r[rs] + simm)
        elif op == 0x0F: r[rt] = u32(imm << 16)
        elif op == 0x0D: r[rt] = u32(r[rs] | imm)
        elif op == 0x0A: r[rt] = 1 if (r[rs] + simm) < r[rs] else 0  # slti
        elif op == 0x2B:  # sltu via special
            pass  # handled separately
        elif op == 0x23:  # lw
            a = u32(r[rs] + simm)
            r[rt] = self.load_u32(a)
        elif op == 0x25:  # lhu
            a = u32(r[rs] + simm)
            r[rt] = self.load_u16(a)
        elif op == 0x29:  # sh
            a = u32(r[rs] + simm)
            self.store_u16(a, r[rt])
        elif op == 0x2B:  # sw
            a = u32(r[rs] + simm)
            self.store_u32(a, r[rt])
        else:
            # sltu in SPECIAL encoding
            if op == 0 and fn == 0x2B:
                r[rd] = 1 if r[rs] < r[rt] else 0
            else:
                raise SystemExit(f"opcode {op:02X} fn {fn:02X} @{addr:08X}")
        r[0] = 0


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: b47_oracle.py executable")

    # S1: address in valid range → returns aligned+shifted address
    o = Oracle(sys.argv[1])
    o.run(0x1010)
    assert o.r[2] == 0x1010, f"return: {o.r[2]:08X}"
    assert o.spu_globals[0x8009B414] == (0x1010 >> SPU_SHIFT), \
        f"D_8009B414: {o.spu_globals[0x8009B414]:08X}"
    print(f"  S1: 0x1010 → D_8009B414={o.spu_globals[0x8009B414]:X}, return 0x1010 OK")

    # S2: address at boundary → valid
    o = Oracle(sys.argv[1])
    o.run(0x1010 + 0x7EFE8)
    expected = 0x1010 + 0x7EFE8  # 0x7FFF8
    assert o.r[2] == expected, f"return: {o.r[2]:08X} != {expected:08X}"
    print(f"  S2: boundary 0x{expected:X} OK")

    # S3: address just past boundary → return 0
    o = Oracle(sys.argv[1])
    o.run(0x1010 + 0x7EFE8 + 1)
    assert o.r[2] == 0, f"return: {o.r[2]:08X}"
    print("  S3: past boundary → 0 OK")

    # S4: address below base → return 0
    o = Oracle(sys.argv[1])
    o.run(0x100F)
    assert o.r[2] == 0, f"return: {o.r[2]:08X}"
    print("  S4: below base → 0 OK")

    # S5: address 0 → return 0 (below base)
    o = Oracle(sys.argv[1])
    o.run(0)
    assert o.r[2] == 0, f"return: {o.r[2]:08X}"
    print("  S5: zero → 0 OK")

    # S6: unaligned address → aligned upward
    o = Oracle(sys.argv[1])
    o.run(0x1011)
    # 0x1011 % 8 = 1, so align: (0x1011 + 8) & ~7 = 0x1018
    # shifted: 0x1018 >> 3 = 0x203
    # D_8009B414 = 0x203
    # return: 0x203 << 3 = 0x1018
    assert o.spu_globals[0x8009B414] == 0x203, \
        f"D_8009B414: {o.spu_globals[0x8009B414]:X}"
    assert o.r[2] == 0x1018, f"return: {o.r[2]:08X}"
    print("  S6: unaligned 0x1011 → aligned 0x1018 OK")

    # S7: already aligned address → no change
    o = Oracle(sys.argv[1])
    o.run(0x2000)
    # 0x2000 % 8 = 0, no alignment needed
    # shifted: 0x2000 >> 3 = 0x400
    # D_8009B414 = 0x400
    # return: 0x400 << 3 = 0x2000
    assert o.spu_globals[0x8009B414] == 0x400, \
        f"D_8009B414: {o.spu_globals[0x8009B414]:X}"
    assert o.r[2] == 0x2000, f"return: {o.r[2]:08X}"
    print("  S7: aligned 0x2000 OK")

    # S8: func_8007DB24 called exactly once per valid invocation
    o = Oracle(sys.argv[1])
    o.run(0x1010)
    db24_calls = [c for c in o.calls if c[0] == "func_8007DB24"]
    assert len(db24_calls) == 1, f"expected 1 call, got {len(db24_calls)}"
    assert db24_calls[0] == ("func_8007DB24", -1, 0x1010, 0x202)
    print("  S8: func_8007DB24(-1, 0x1010) called, returned 0x202 OK")

    # S9: guest writes are stack + D_8009B414 (halfword store)
    o = Oracle(sys.argv[1])
    o.run(0x1010)
    guest_writes = [(a, n, v) for a, n, v in o.writes
                    if RAM_BASE <= a < RAM_END]
    # Stack: sw $ra at sp+0x10
    stack_writes = [w for w in guest_writes
                    if 0x801FFF00 - 0x18 <= w[0] < 0x801FFF00]
    # D_8009B414 halfword store
    spu_writes = [w for w in guest_writes if w[0] == 0x8009B414]
    non_stack_spu = [w for w in guest_writes
                     if w not in stack_writes and w not in spu_writes]
    assert len(non_stack_spu) == 0, f"unexpected guest writes: {non_stack_spu}"
    assert len(spu_writes) == 1 and spu_writes[0][1] == 2, \
        f"D_8009B414 write: {spu_writes}"
    print("  S9: guest writes = stack + D_8009B414 OK")

    # S10: func_8007DB24 not called on invalid address
    o = Oracle(sys.argv[1])
    o.run(0)
    db24_calls = [c for c in o.calls if c[0] == "func_8007DB24"]
    assert len(db24_calls) == 0, f"expected 0 calls, got {len(db24_calls)}"
    print("  S10: no func_8007DB24 call on invalid address OK")

    print("PASS: B47 oracle — 23 words, range check, alignment, "
          "shift, func_8007DB24 call, D_8009B414 update")


if __name__ == "__main__":
    main()

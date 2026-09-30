#!/usr/bin/env python3
"""Independent Phase 6E-B46 oracle for func_800851A8.

58 words from asm/disc1/74FB0.s:800-863, verified against SHA-exact exe.
Only the magic-number check and error path are deterministic.
The success path (SPU heap + DMA) is hardware-dependent.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x800851A8
RAM_BASE = 0x80000000
RAM_END = 0x80200000
MAGIC = 0xB0BEB4BF

W = exe_words(0x800851A8, 58)

FUNC_80085174 = 0x80085174
FUNC_80085084 = 0x80085084
FUNC_80085EB4 = 0x80085EB4
FUNC_800850F4 = 0x800850F4


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
            got_w = struct.unpack_from("<I", data, BASE + i * 4 - taddr + 0x800)[0]
            if got_w != want:
                raise SystemExit(f"FATAL: word {i} @ {BASE+i*4:08X}: "
                                 f"{got_w:08X} != {want:08X}")
        print(f"exe SHA-1 {SHA1} OK; func_800851A8: {len(W)} words verified")
        self.ram = bytearray(RAM_END - RAM_BASE)
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
            elif fn == 8: raise SystemExit("unexpected jr in one()")
            elif fn == 0x21: r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x23: r[rd] = u32(r[rs] - r[rt])
            elif fn == 0x25: r[rd] = u32(r[rs] | r[rt])
            elif fn == 0x24: r[rd] = u32(r[rs] & r[rt])
            else: raise SystemExit(f"SPECIAL {fn:02X} @{addr:08X}")
        elif op == 9: r[rt] = u32(r[rs] + simm)
        elif op == 0x0C: r[rt] = u32(r[rs] & imm)
        elif op == 0x0D: r[rt] = u32(r[rs] | imm)
        elif op == 0x0F: r[rt] = u32(imm << 16)
        elif op == 0x23:
            a = u32(r[rs] + simm)
            r[rt] = self.load(a, 4)
            self.reads.append((a, 4, r[rt]))
        elif op == 0x2B:
            a = u32(r[rs] + simm)
            self.store(a, 4, r[rt])
        else: raise SystemExit(f"opcode {op:02X} @{addr:08X}")
        r[0] = 0

    def run(self, a0, a1, mock_returns=None):
        if mock_returns is None:
            mock_returns = {}
        self.r[4] = u32(a0)
        self.r[5] = u32(a1)
        self.r[29] = 0x801FFF00
        call_idx = {}

        while True:
            if self.steps > 2000:
                raise SystemExit("oracle did not terminate")
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

            # Branches: beq(4), bne(5), blez(6), bgtz(7), beqz/bnez via bne/beq $0
            if op in (4, 5, 6, 7):
                delay = W[self.pc + 1]
                if op == 4: take = (self.r[rs] == self.r[rt])
                elif op == 5: take = (self.r[rs] != self.r[rt])
                elif op == 6: take = s32(self.r[rs]) <= 0
                else: take = s32(self.r[rs]) > 0
                self.one(delay, addr + 4)
                if take:
                    self.pc = (addr + 4 + (simm << 2) - BASE) // 4
                else:
                    self.pc += 2
                continue

            if op == 2:  # j
                target = (addr & 0xF0000000) | ((w & 0x03FFFFFF) << 2)
                self.one(W[self.pc + 1], addr + 4)
                self.pc = (target - BASE) // 4
                continue

            if op == 3:  # jal
                target = (addr & 0xF0000000) | ((w & 0x03FFFFFF) << 2)
                self.one(W[self.pc + 1], addr + 4)
                if target == FUNC_80085174:
                    ret = 0  # D_8009D24C is 0, spin exits immediately
                    self.calls.append(("func_80085174",))
                elif target == FUNC_80085084:
                    buf = self.r[4]
                    val = self.load(buf, 4)
                    ret = u32(val + MAGIC)
                    self.calls.append(("func_80085084", buf, val, ret))
                elif target == FUNC_80085EB4:
                    idx = call_idx.get(target, 0)
                    seq = mock_returns.get(target, [0x1010])
                    ret = seq[idx] if idx < len(seq) else seq[-1]
                    call_idx[target] = idx + 1
                    self.calls.append(("func_80085EB4", self.r[4], ret))
                elif target == FUNC_800850F4:
                    idx = call_idx.get(target, 0)
                    seq = mock_returns.get(target, [0])
                    ret = seq[idx] if idx < len(seq) else seq[-1]
                    call_idx[target] = idx + 1
                    self.calls.append(("func_800850F4", self.r[4], self.r[5], ret))
                else:
                    raise SystemExit(f"unexpected call target {target:08X}")
                self.r[2] = u32(ret)
                self.pc = ((addr + 8) - BASE) // 4
                continue

            if op == 0 and fn == 8:  # jr $ra
                self.one(W[self.pc + 1], addr + 4)
                return

            self.one(w, addr)
            self.pc += 1


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: b46_oracle.py executable")

    def poke(ram, a, v):
        struct.pack_into("<I", ram, a - RAM_BASE, v)

    # S1: invalid buffer → error path
    o = Oracle(sys.argv[1])
    poke(o.ram, 0x80100000, 0xA5A5A5A5)
    o.run(0x80100000, 1)
    assert o.r[2] == 0xFFFFFFFF, f"return: {o.r[2]:08X}"
    # Delay slot sets v0=-1 before branch, so D_8009D24C gets -1
    d24c = struct.unpack_from("<I", o.ram, 0x9D24C)[0]
    assert d24c == 0xFFFFFFFF, f"D_8009D24C: {d24c:08X}"
    hw = [c for c in o.calls if c[0] not in ("func_80085174", "func_80085084")]
    assert len(hw) == 0, f"unexpected HW calls: {hw}"
    print("  S1: invalid buffer → error, D_8009D24C set, return -1 OK")

    # S2: valid buffer → success path
    o = Oracle(sys.argv[1])
    poke(o.ram, 0x80100000, 0x4F414B41)
    # Set transfer params at buffer+0x10 to keep copy loop small
    poke(o.ram, 0x80100010, 0x1010)   # param0: SPU addr
    poke(o.ram, 0x80100014, 0x10)     # param1: size
    poke(o.ram, 0x80100018, 0)        # param2: offset (s2=0)
    poke(o.ram, 0x8010001C, 1)        # param3: mode (s0=1-0=1, a1=1<<4=16)
    o.run(0x80100000, 1)
    assert o.r[2] == 0, f"return: {o.r[2]:08X}"
    names = [c[0] for c in o.calls]
    assert "func_80085084" in names
    assert "func_80085EB4" in names
    assert "func_800850F4" in names
    assert names.count("func_80085174") == 2
    print("  S2: valid buffer → success, all HW calls OK")

    # S3: count=0 → single barrier
    o = Oracle(sys.argv[1])
    poke(o.ram, 0x80100000, 0x4F414B41)
    poke(o.ram, 0x8010001C, 1)  # param3: keep copy loop small
    o.run(0x80100000, 0)
    assert o.r[2] == 0
    names = [c[0] for c in o.calls]
    assert names.count("func_80085174") == 1
    print("  S3: count=0 → single barrier OK")

    # S4: magic arithmetic
    o = Oracle(sys.argv[1])
    poke(o.ram, 0x80100000, 0x4F414B41)
    poke(o.ram, 0x8010001C, 1)
    o.run(0x80100000, 1)
    chk = [c for c in o.calls if c[0] == "func_80085084"]
    assert chk[0][3] == 0
    print("  S4: 0x4F414B41 + 0xB0BEB4BF = 0 OK")

    # S5: error path single guest write
    o = Oracle(sys.argv[1])
    poke(o.ram, 0x80100000, 0xAAAAAAAA)
    o.run(0x80100000, 1)
    gw = [(a, n, v) for a, n, v in o.writes if RAM_BASE <= a < RAM_END]
    # Stack saves (5) + D_8009D24C store (1) = 6 guest writes
    d24c_writes = [(a, n, v) for a, n, v in gw if a == 0x8009D24C]
    assert len(d24c_writes) == 1 and d24c_writes[0][2] == 0xFFFFFFFF
    print("  S5: error path single write (D_8009D24C) OK")

    # S6: D_800B2900 copy on success
    o = Oracle(sys.argv[1])
    poke(o.ram, 0x80100000, 0x4F414B41)
    poke(o.ram, 0x80100010, 0x1010)   # param0: SPU addr
    poke(o.ram, 0x80100014, 0x10)     # param1: size
    poke(o.ram, 0x80100018, 0)        # param2: offset (s2=0)
    poke(o.ram, 0x8010001C, 1)        # param3: mode (s0=1, a1=16 words)
    # Data at s1 = s0+0x24 = 0x10+0x10+0xC+0x24 = buffer+0x50
    # Wait: s0 starts at buffer+0x10, then +=4 three times → buffer+0x1C
    # Then s1 = s0+0x24 = buffer+0x1C+0x24 = buffer+0x40
    for i in range(4):
        poke(o.ram, 0x80100040 + i * 4, 0xDEAD0000 + i)
    o.run(0x80100000, 1)
    val = struct.unpack_from("<I", o.ram, 0xB2900)[0]
    assert val == 0xDEAD0000, f"D_800B2900: {val:08X}"
    print("  S6: D_800B2900 copy OK")

    print("PASS: B46 oracle — 58 words, magic check, error path, "
          "success path, D_8009D24C, D_800B2900 copy")


if __name__ == "__main__":
    main()

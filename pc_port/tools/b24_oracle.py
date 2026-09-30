#!/usr/bin/env python3
"""Independent B24 oracle for func_8005BCBC (resource-state selector).

Phase 6E-B24.  Does NOT call or depend on the production C port
implementation.

Loads the verified retail executable (PS-X EXE, SHA-1 checked, supplied at
runtime — never embedded or committed), cross-checks all 21 instruction
words of func_8005BCBC against the transcribed retail body, then EXECUTES
those words with a tiny delay-slot-aware MIPS-I interpreter that shares one
register file between branch and delay-slot execution and logs every guest
read and write with width and order.

Contract re-derived from the raw words:
  * sw $a0 -> $gp+0x358 (D_8009D0C8) is ALWAYS the first store
  * a0 != 0: byte6 = lbu(a0+6) selects between 0x800C20A4 (byte6 != 9)
    and 0x800C20B4 (byte6 == 9)
  * a0 == 0: word D_8009D218 ($gp+0x4A8) selects between 0x800C0DE0
    (flag == 0) and 0x800C0DF0 (flag != 0)
  * final stores $gp+0x350 (D_8009D0C0) = selected base and
    $gp+0x354 (D_8009D0C4) = 8; return $v0 = 8 on every path

Usage:
  b24_oracle.py /path/to/disc1.candidate.exe

Exit 0 on PASS, non-zero on mismatch.
"""

import hashlib
import struct
import sys
from pe_exe_words import exe_hex_list

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

MASK = 0xFFFFFFFF
GP = 0x8009CD70
FUNC_ADDR = 0x8005BCBC

# Transcribed retail body, asm/disc1/4C4BC.s:12-32 (little-endian byte
# order in the comment column of the split).
_BODY_LE_HEX = exe_hex_list(0x8005BCBC, 21, 'le', True)
    # sw   $a0, 0x358($gp)
    # beqz $a0, +9
    #  addiu $v0, $zero, 9        (delay)
    # lui  $a1, 0x800C
    # addiu $a1, $a1, 0x20A4      -> 0x800C20A4
    # lbu  $v1, 6($a0)
    # nop
    # bne  $v1, $v0, +9
    #  addiu $v0, $zero, 8        (delay)
    # j    0x8005BD00
    #  addiu $a1, $a1, 0x10       (delay)
    # lw   $v0, 0x4A8($gp)
    # lui  $a1, 0x800C
    # addiu $a1, $a1, 0x0DE0      -> 0x800C0DE0
    # beqz $v0, +2
    #  addiu $v0, $zero, 8        (delay)
    # addiu $a1, $a1, 0x10
    # sw   $a1, 0x350($gp)
    # sw   $v0, 0x354($gp)
    # jr   $ra
    # nop
EXPECTED_WORDS = [struct.unpack("<I", bytes.fromhex(h))[0]
                  for h in _BODY_LE_HEX]

GA_C8 = 0x8009D0C8   # $gp+0x358
GA_C0 = 0x8009D0C0   # $gp+0x350
GA_C4 = 0x8009D0C4   # $gp+0x354
GA_FLAG = 0x8009D218  # $gp+0x4A8


def u32(x):
    return x & MASK


def s32(x):
    x &= MASK
    return x - 0x100000000 if x >= 0x80000000 else x


class Machine:
    def __init__(self, path):
        with open(path, "rb") as f:
            self.data = f.read()
        sha1 = hashlib.sha1(self.data).hexdigest()
        if sha1 != RETAIL_SHA1:
            raise SystemExit(
                f"FATAL: {path} SHA-1 {sha1} != retail {RETAIL_SHA1}")
        if self.data[:8] != b"PS-X EXE":
            raise SystemExit("FATAL: not a PS-X EXE")
        self.ram = bytearray(0x200000)
        taddr = struct.unpack_from("<I", self.data, 0x18)[0]
        base = taddr - 0x80000000
        size = min(len(self.data) - 0x800, 0x200000 - base)
        self.ram[base:base + size] = self.data[0x800:0x800 + size]

        # Cross-check every modeled word against the exe.
        off = 0x800 + (FUNC_ADDR - taddr)
        for i, exp in enumerate(EXPECTED_WORDS):
            got = struct.unpack_from("<I", self.data, off + i * 4)[0]
            if got != exp:
                raise SystemExit(
                    f"FATAL: word {i} @ {FUNC_ADDR + i*4:08X}: "
                    f"exe {got:08X} != transcribed {exp:08X}")
        self.reads = []
        self.writes = []

    # ── guest memory (little-endian, logged) ────────────────────────────
    def ld8(self, a):
        v = self.ram[a & 0x1FFFFF]
        self.reads.append((a, 1, v))
        return v

    def ld32(self, a):
        v = struct.unpack_from("<I", self.ram, a & 0x1FFFFF)[0]
        self.reads.append((a, 4, v))
        return v

    def st32(self, a, v):
        struct.pack_into("<I", self.ram, a & 0x1FFFFF, v & MASK)
        self.writes.append((a, 4, v & MASK))

    def st8(self, a, v):
        self.ram[a & 0x1FFFFF] = v & 0xFF
        self.writes.append((a, 1, v & 0xFF))

    # ── delay-slot-aware execution of the 21 retail words ──────────────
    def call(self, a0):
        words = EXPECTED_WORDS
        regs = {i: 0 for i in range(32)}
        regs[28] = GP
        regs[29] = 0x801F0000
        regs[31] = 0xDEADBEEF
        regs[4] = a0 & MASK
        n = len(words)
        pc = 0
        while pc < n:
            w = words[pc]
            ins_addr = FUNC_ADDR + pc * 4
            op = w >> 26
            rs = (w >> 21) & 0x1F
            rt = (w >> 16) & 0x1F
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            addr_field = w & 0x3FFFFFF
            func = w & 0x3F
            delay = words[pc + 1] if pc + 1 < n else None

            def run(word):
                """Execute one instruction on the SHARED register file."""
                if word == 0:
                    return  # nop
                o = word >> 26
                r_s = (word >> 21) & 0x1F
                r_t = (word >> 16) & 0x1F
                im = word & 0xFFFF
                si = im if im < 0x8000 else im - 0x10000
                if o == 0x09:
                    regs[r_t] = u32(regs[r_s] + si)
                elif o == 0x0F:
                    regs[r_t] = im << 16
                elif o == 0x23:
                    regs[r_t] = u32(self.ld32(u32(regs[r_s] + si)))
                elif o == 0x24:
                    regs[r_t] = self.ld8(u32(regs[r_s] + si))
                elif o == 0x2B:
                    self.st32(u32(regs[r_s] + si), regs[r_t])
                elif o == 0x00:
                    raise SystemExit(f"unexpected SPECIAL {word:08X}")
                else:
                    raise SystemExit(f"unexpected delay op {o:02X}")

            def do_delay():
                if delay is not None:
                    run(delay)

            if op == 0x00 and func == 0x08:      # jr
                do_delay()
                return regs[2]
            elif op == 0x00 and w == 0:          # nop
                pc += 1
                continue
            elif op == 0x02:                     # j
                target = (addr_field << 2) | (ins_addr & 0xF0000000)
                do_delay()
                pc = (target - FUNC_ADDR) // 4
                continue
            elif op == 0x04:                     # beq / beqz
                # Condition sampled at issue, BEFORE the delay slot runs.
                take = regs[rs] == regs[rt]
                do_delay()
                if take:
                    pc = ((ins_addr + 4 + (simm << 2)) - FUNC_ADDR) // 4
                    continue
                pc += 2                          # slot already executed
                continue
            elif op == 0x05:                     # bne
                take = regs[rs] != regs[rt]
                do_delay()
                if take:
                    pc = ((ins_addr + 4 + (simm << 2)) - FUNC_ADDR) // 4
                    continue
                pc += 2                          # slot already executed
                continue
            elif op == 0x09:
                regs[rt] = u32(regs[rs] + simm)
            elif op == 0x0F:
                regs[rt] = imm << 16
            elif op == 0x23:
                regs[rt] = u32(self.ld32(u32(regs[rs] + simm)))
            elif op == 0x24:
                regs[rt] = self.ld8(u32(regs[rs] + simm))
            elif op == 0x2B:
                self.st32(u32(regs[rs] + simm), regs[rt])
            else:
                raise SystemExit(f"unimpl op {op:02X} @{ins_addr:08X}")
            pc += 1
        raise SystemExit("fell off the end without jr $ra")


def scenario(path, name, seed, a0, exp_writes, exp_reads, exp_ret):
    m = Machine(path)
    for addr, width, val in seed:
        if width == 4:
            struct.pack_into("<I", m.ram, addr & 0x1FFFFF, val)
        else:
            m.ram[addr & 0x1FFFFF] = val
    m.reads.clear()
    m.writes.clear()
    ret = m.call(a0)
    ok = True
    if m.writes != exp_writes:
        print(f"  FAIL[{name}] writes:")
        print(f"    got  {m.writes}")
        print(f"    want {exp_writes}")
        ok = False
    if m.reads != exp_reads:
        print(f"  FAIL[{name}] reads:")
        print(f"    got  {m.reads}")
        print(f"    want {exp_reads}")
        ok = False
    if ret != exp_ret:
        print(f"  FAIL[{name}] return {ret}, want {exp_ret}")
        ok = False
    if ok:
        print(f"  PASS[{name}] ret={ret} writes={len(m.writes)} "
              f"reads={len(m.reads)} (ROM order exact)")
    return ok


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: b24_oracle.py /path/to/disc1.candidate.exe")
    path = sys.argv[1]

    # Word cross-check happens inside Machine() on every scenario run.
    ok = True

    # S1 — dispatcher path: a0 = 0, D_8009D218 = 1 (func_8005BC98 set it).
    ok &= scenario(path, "dispatcher a0=0 flag=1",
                   [(GA_FLAG, 4, 1)], 0,
                   [(GA_C8, 4, 0), (GA_C0, 4, 0x800C0DF0), (GA_C4, 4, 8)],
                   [(GA_FLAG, 4, 1)], 8)

    # S2 — a0 = 0, flag clear: low NULL buffer base, no +0x10.
    ok &= scenario(path, "a0=0 flag=0",
                   [(GA_FLAG, 4, 0)], 0,
                   [(GA_C8, 4, 0), (GA_C0, 4, 0x800C0DE0), (GA_C4, 4, 8)],
                   [(GA_FLAG, 4, 0)], 8)

    # S3 — record pointer, byte6 == 9: high record base + 0x10.
    rec = 0x800A0040
    ok &= scenario(path, "record byte6=9",
                   [(rec + 6, 1, 9)], rec,
                   [(GA_C8, 4, rec), (GA_C0, 4, 0x800C20B4), (GA_C4, 4, 8)],
                   [(rec + 6, 1, 9)], 8)

    # S4 — record pointer, byte6 != 9: high record base unchanged.
    ok &= scenario(path, "record byte6!=9",
                   [(rec + 6, 1, 0x13)], rec,
                   [(GA_C8, 4, rec), (GA_C0, 4, 0x800C20A4), (GA_C4, 4, 8)],
                   [(rec + 6, 1, 0x13)], 8)

    if ok:
        print("B24 ORACLE: PASS")
        sys.exit(0)
    print("B24 ORACLE: FAIL")
    sys.exit(1)


if __name__ == "__main__":
    main()

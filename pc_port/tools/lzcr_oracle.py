#!/usr/bin/env python3
"""Independent retail oracle for func_8003EAC8 (GTE LZCS/LZCR registration leaf).

Phase 6E-B4.  Does NOT reimplement the C port's formula: it loads the real
retail executable bytes (PS-X EXE, SHA-1 checked, supplied at runtime — never
embedded or committed), verifies the 15 instruction words of func_8003EAC8
against the transcribed disassembly, and then EXECUTES those words with a tiny
MIPS interpreter (delay slots honored) whose cop2 models only the two GTE data
registers this function touches: LZCS (reg 30, write) and LZCR (reg 31, read).

GTE LZCS/LZCR semantics (PlayStation cop2, no$psx spec): writing LZCS latches
the source; LZCR reads back the count of leading bits equal to the sign bit —
leading zeros for nonnegative input (0 -> 32), leading ones for negative input
(0xFFFFFFFF -> 32).  A second, structurally different implementation of that
count is cross-checked over the full input set plus random fuzz.

No local GTE-capable emulator exists in this environment; the cross-checks are:
  1. the executed words are the verified retail exe bytes,
  2. dual LZCR implementations agree,
  3. documented GTE semantics as above.

Usage:
  lzcr_oracle.py /path/to/disc1.candidate.exe

Output: one line per input, byte-identical to the C port's --lzcr-oracle-dump.
"""

import hashlib
import random
import struct
import sys

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

FUNC_ADDR = 0x8003EAC8
FUNC_WORDS = 15               # 0x3C bytes
GA_TABLE = 0x800A76F0
SP_INIT = 0x801FFFF0
RA_SENTINEL = 0xDEADBEEC

MASK = 0xFFFFFFFF

# The transcribed retail body (asm/disc1/2EF54.s:255-271, live split per
# configs/USA/disc1.yaml).  Verified word-for-word against the exe below.
EXPECTED_WORDS = [
    0xAFA40000,  # sw   $a0, 0($sp)
    0x4884F000,  # mtc2 $a0, $30   (LZCS)
    0x3C028000,  # lui  $v0, 0x8000
    0x10820005,  # beq  $a0, $v0, .L8003EAEC
    0x2403001F,  # addiu $v1, $zero, 0x1F   (branch delay slot)
    0xEBBF0000,  # swc2 $31, 0($sp)  (LZCR)
    0x8FA20000,  # lw   $v0, 0($sp)
    0x00000000,  # nop
    0x00621823,  # subu $v1, $v1, $v0
    0x00031080,  # sll  $v0, $v1, 2
    0x3C01800A,  # lui  $at, 0x800A
    0x00220821,  # addu $at, $at, $v0
    0xAC2576F0,  # sw   $a1, 0x76F0($at)
    0x03E00008,  # jr   $ra
    0x00000000,  # nop               (jump delay slot)
]

INPUTS = [
    0x00000000, 0x00000001, 0x00000002, 0x00000003,
    0x00000008, 0x00008000, 0x40000000, 0x7FFFFFFF,
    0x80000000, 0x80000001, 0xC0000000, 0xFFFFFFFF,
] + [1 << k for k in range(32) if (1 << k) not in
     {0x00000001, 0x00000002, 0x00000008, 0x00008000,
      0x40000000, 0x80000000}]


def u32(x):
    return x & MASK


def s32(x):
    x &= MASK
    return x - 0x100000000 if x >= 0x80000000 else x


def lzcr_a(v):
    """GTE LZCR, method A: walk bits from the top while equal to the sign."""
    v &= MASK
    sign = v & 0x80000000
    n = 0
    while n < 32 and (v & 0x80000000) == sign:
        n += 1
        v = u32(v << 1)
    return n


def lzcr_b(v):
    """GTE LZCR, method B: xor with sign-replica, then count leading zeros."""
    v &= MASK
    xr = v ^ (MASK if v & 0x80000000 else 0)   # leading sign bits -> zeros
    if xr == 0:
        return 32
    return 32 - xr.bit_length()


class Machine:
    """Tiny MIPS-I interpreter for exactly this function, over a 2 MiB guest
    RAM image seeded with the retail exe bytes at their taddr."""

    def __init__(self, path):
        with open(path, "rb") as f:
            data = f.read()
        sha1 = hashlib.sha1(data).hexdigest()
        if sha1 != RETAIL_SHA1:
            raise SystemExit(
                f"FATAL: {path} SHA-1 {sha1} != retail {RETAIL_SHA1}")
        if data[:8] != b"PS-X EXE":
            raise SystemExit("FATAL: not a PS-X EXE")
        self.taddr = struct.unpack_from("<I", data, 0x18)[0]
        tsize = struct.unpack_from("<I", data, 0x1C)[0]
        if len(data) != 0x800 + tsize:
            raise SystemExit("FATAL: exe size mismatch")
        self.ram = bytearray(2 * 1024 * 1024)          # 0x80000000-based
        off = self.taddr - 0x80000000
        self.ram[off:off + tsize] = data[0x800:0x800 + tsize]

        # Cross-check the transcription against the retail bytes.
        for i, want in enumerate(EXPECTED_WORDS):
            got = self.load32(FUNC_ADDR + 4 * i)
            if got != want:
                raise SystemExit(
                    f"FATAL: exe word {i} @0x{FUNC_ADDR + 4 * i:08X} = "
                    f"0x{got:08X}, transcription said 0x{want:08X}")

    def load32(self, addr):
        off = addr - 0x80000000
        if off < 0 or off + 4 > len(self.ram):
            raise SystemExit(f"FATAL: lw outside guest RAM: 0x{addr:08X}")
        return struct.unpack_from("<I", self.ram, off)[0]

    def store32(self, addr, val):
        off = addr - 0x80000000
        if off < 0 or off + 4 > len(self.ram):
            raise SystemExit(f"FATAL: sw outside guest RAM: 0x{addr:08X}")
        struct.pack_into("<I", self.ram, off, val & MASK)

    def run_func(self, a0, a1):
        """Execute the 15 retail words with correct delay slots.  Returns
        (lzcr_used, idx_final, dest_addr, value_stored)."""
        r = [0] * 32
        r[4] = a0 & MASK            # $a0
        r[5] = a1 & MASK            # $a1
        r[29] = SP_INIT             # $sp
        r[31] = RA_SENTINEL         # $ra
        lzcr_reg = [None]           # GTE LZCR latch (reg 31 readback)
        trace = {"lzcr": None, "dest": None, "stored": None}

        def fetch(pc):
            return self.load32(pc)

        pc = FUNC_ADDR
        pending_branch = None       # target to apply after the delay slot
        steps = 0
        while True:
            steps += 1
            if steps > 64:
                raise SystemExit("FATAL: interpreter did not return")
            w = fetch(pc)
            op = (w >> 26) & 0x3F
            rs = (w >> 21) & 0x1F
            rt = (w >> 16) & 0x1F
            rd = (w >> 11) & 0x1F
            sa = (w >> 6) & 0x1F
            imm = w & 0xFFFF
            simm = imm - 0x10000 if imm >= 0x8000 else imm
            next_pc = u32(pc + 4)

            if w == 0:
                pass                                    # nop / sll $0
            elif op == 0x00:                            # SPECIAL
                fn = w & 0x3F
                if fn == 0x00:                          # sll
                    r[rd] = u32(r[rt] << sa)
                elif fn == 0x21:                        # addu
                    r[rd] = u32(r[rs] + r[rt])
                elif fn == 0x23:                        # subu
                    r[rd] = u32(r[rs] - r[rt])
                elif fn == 0x08:                        # jr
                    pending_branch = r[rs]
                else:
                    raise SystemExit(f"FATAL: SPECIAL fn 0x{fn:02X} @0x{pc:08X}")
            elif op == 0x09:                            # addiu
                r[rt] = u32(r[rs] + simm)
            elif op == 0x0F:                            # lui
                r[rt] = u32(imm << 16)
            elif op == 0x04:                            # beq
                if r[rs] == r[rt]:
                    pending_branch = u32(pc + 4 + (simm << 2))
            elif op == 0x23:                            # lw
                r[rt] = self.load32(u32(r[rs] + simm))
            elif op == 0x2B:                            # sw
                addr = u32(r[rs] + simm)
                self.store32(addr, r[rt])
                trace["dest"] = addr                    # last sw wins
                trace["stored"] = r[rt]
            elif op == 0x12:                            # cop2
                cop_rs = rs
                if cop_rs == 0x04:                      # mtc2
                    if rt != 4 or rd != 30:
                        raise SystemExit("FATAL: unexpected mtc2")
                    lzcr_reg[0] = lzcr_a(r[4])
                    trace["lzcr"] = lzcr_reg[0]
                else:
                    raise SystemExit(f"FATAL: cop2 rs {cop_rs} @0x{pc:08X}")
            elif op == 0x3A:                            # swc2
                # swc2: the rt field encodes the cop2 data register (31=LZCR)
                cop_reg = (w >> 16) & 0x1F
                if cop_reg != 31:
                    raise SystemExit("FATAL: unexpected swc2 reg")
                addr = u32(r[rs] + simm)
                self.store32(addr, lzcr_reg[0])
            else:
                raise SystemExit(f"FATAL: opcode 0x{op:02X} @0x{pc:08X}")

            r[0] = 0
            if pending_branch is not None:
                # This instruction was the branch: run its delay slot, then
                # transfer.  (Delay slots here are addiu/nop only — no
                # further control transfer, see _step_delay.)
                target = pending_branch
                pending_branch = None
                self._step_delay(fetch(next_pc), r, lzcr_reg, trace)
                if target == RA_SENTINEL:
                    break
                pc = target
            else:
                pc = next_pc

        idx = s32(r[3])
        return trace["lzcr"], idx, trace["dest"], trace["stored"]

    def _step_delay(self, w, r, lzcr_reg, trace):
        """Execute one delay-slot instruction (no further control transfer
        occurs inside this function's delay slots: addiu / nop only)."""
        if w == 0:
            return
        op = (w >> 26) & 0x3F
        rs = (w >> 21) & 0x1F
        rt = (w >> 16) & 0x1F
        simm = w & 0xFFFF
        if simm >= 0x8000:
            simm -= 0x10000
        if op == 0x09:                                  # addiu
            r[rt] = u32(r[rs] + simm)
        else:
            raise SystemExit(f"FATAL: delay-slot opcode 0x{op:02X}")


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: lzcr_oracle.py /path/to/disc1.candidate.exe")

    # Dual-implementation LZCR cross-check over inputs + fuzz.
    for v in INPUTS:
        if lzcr_a(v) != lzcr_b(v):
            raise SystemExit(f"FATAL: LZCR models disagree on 0x{v:08X}")
    rng = random.Random(0xEAC8)
    for _ in range(100000):
        v = rng.getrandbits(32)
        if lzcr_a(v) != lzcr_b(v):
            raise SystemExit(f"FATAL: LZCR models disagree on 0x{v:08X}")

    m = Machine(sys.argv[1])
    for i, a0 in enumerate(INPUTS):
        sentinel = 0xEAC80000 | i
        lzcr, idx, dest, stored = m.run_func(a0, sentinel)
        if lzcr != lzcr_a(a0):
            raise SystemExit(f"FATAL: GTE model mismatch for 0x{a0:08X}")
        # Independent destination check: idx must address dest via the
        # sll/addu chain, wrapping mod 2^32.
        want_dest = u32(GA_TABLE + u32(idx << 2))
        if dest != want_dest:
            raise SystemExit(
                f"FATAL: dest 0x{dest:08X} != model 0x{want_dest:08X}")
        if stored != sentinel:
            raise SystemExit("FATAL: stored value != a1")
        print(f"EAC8 a0=0x{a0:08X} lzcr={lzcr} idx={idx} "
              f"dest=0x{dest:08X} stored=0x{stored:08X}")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Independent MIPS-interpreter oracle for retail func_8005DE88.

Phase 6E-B22.  Does NOT call or depend on the production C port implementation.
Loads the verified retail executable (PS-X EXE, SHA-1 checked, supplied at
runtime — never embedded or committed), verifies all 23 instruction words of
func_8005DE88 against the transcribed disassembly, and then EXECUTES those
words with a tiny MIPS interpreter (delay slots honored, branches tracked).

The exact write sequence (address, value) is recorded and validated.

Usage:
  b22_5de88_oracle.py /path/to/disc1.candidate.exe

Output: one line summary, then the 27-entry write sequence.
"""

import hashlib
import struct
import sys
from pe_exe_words import exe_hex_list

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

FUNC_ADDR = 0x8005DE88
FUNC_WORDS = 23              # 0x5C bytes

MASK = 0xFFFFFFFF

# func_8005DE88 transcribed retail body (asm/disc1/4CC98.s:1972-1998).
# Hex column is little-endian memory byte order; each entry is the uint32
# value formed by those bytes at their word-aligned guest address.
_EXPECTED_LE_HEX = exe_hex_list(0x8005DE88, 23, 'le', True)
    # 4E688: lui   $v1, %hi(D_800A2090)
    # 4E68C: addiu $v1, $v1, %lo(D_800A2090)
    # 4E690: addiu $a0, $v1, 0xF0
    # 4E694: sltu  $v0, $v1, $a0
    # 4E698: beqz  $v0, .L8005DEB4
    # 4E69C: addiu $v0, $v1, 0xC   (delay slot)
    # 4E6A0: sw    $v0, 0x0($v1)    (loop top)
    # 4E6A4: addu  $v1, $v0, $zero
    # 4E6A8: sltu  $v0, $v1, $a0
    # 4E6AC: bnez  $v0, .L8005DEA0
    # 4E6B0: addiu $v0, $v1, 0xC   (delay slot)
    # 4E6B4: lui   $v0, %hi(D_800A2174)
    # 4E6B8: addiu $v0, $v0, %lo(D_800A2174)
    # 4E6BC: sw    $zero, 0x0($v0)
    # 4E6C0: addiu $v0, $v0, -0xE4
    # 4E6C4: sw    $v0, 0x36C($gp)
    # 4E6C8: sw    $zero, 0x374($gp)
    # 4E6CC: sw    $zero, 0x370($gp)
    # 4E6D0: sw    $zero, 0x378($gp)
    # 4E6D4: sw    $zero, 0x37C($gp)
    # 4E6D8: sw    $zero, 0x380($gp)
    # 4E6DC: jr    $ra
    # 4E6E0: nop
EXPECTED_WORDS = [struct.unpack("<I", bytes.fromhex(h))[0]
                  for h in _EXPECTED_LE_HEX]


def u32(x):
    return x & MASK


def s32(x):
    x &= MASK
    return x - 0x100000000 if x >= 0x80000000 else x


class Machine:
    """Tiny MIPS-I interpreter over 2 MiB guest RAM seeded with the retail exe
    bytes at their taddr.  Executes func_8005DE88 only; records every sw in
    order until the jr $ra returns."""

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

    def run(self):
        """Execute func_8005DE88, recording every sw (address, value)."""
        r = [0] * 32
        r[28] = 0x8009CD70          # $gp (retail value)
        r[31] = 0xDEAD0000          # $ra sentinel (upper half distinguishable)
        writes = []

        pc = FUNC_ADDR
        steps = 0
        while True:
            steps += 1
            if steps > 128:
                raise SystemExit("FATAL: interpreter did not return")
            w = self.load32(pc)
            op = (w >> 26) & 0x3F
            rs = (w >> 21) & 0x1F
            rt = (w >> 16) & 0x1F
            rd = (w >> 11) & 0x1F
            sa = (w >> 6) & 0x1F
            imm = w & 0xFFFF
            simm = imm - 0x10000 if imm >= 0x8000 else imm
            next_pc = u32(pc + 4)
            jump_target = None
            branch_taken = False
            branch_reason = None

            if w == 0:
                pass                                    # nop
            elif op == 0x00:                            # SPECIAL
                fn = w & 0x3F
                if fn == 0x21:                          # addu
                    r[rd] = u32(r[rs] + r[rt])
                elif fn == 0x2B:                        # sltu
                    r[rd] = 1 if r[rs] < r[rt] else 0
                elif fn == 0x08:                        # jr
                    jump_target = r[rs]
                else:
                    raise SystemExit(
                        f"FATAL: SPECIAL fn 0x{fn:02X} @0x{pc:08X}")
            elif op == 0x04:                            # BEQ (beqz is BEQ with rt=0)
                if r[rs] == r[rt]:
                    branch_taken = True
            elif op == 0x05:                            # BNE (bnez is BNE with rt=0)
                if r[rs] != r[rt]:
                    branch_taken = True
            elif op == 0x09:                            # ADDIU
                r[rt] = u32(r[rs] + simm)
            elif op == 0x0F:                            # LUI
                r[rt] = u32(imm << 16)
            elif op == 0x2B:                            # SW
                addr = u32(r[rs] + simm)
                val = r[rt]
                self.store32(addr, val)
                writes.append((addr, val))
            else:
                raise SystemExit(
                    f"FATAL: opcode 0x{op:02X} @0x{pc:08X}")

            r[0] = 0

            if jump_target is not None:
                # Execute delay slot
                dw = self.load32(next_pc)
                if dw != 0:
                    self._step_delay(dw, r, writes)
                if jump_target == r[31]:  # jr $ra sentinel
                    break
                pc = jump_target
                continue

            if branch_taken:
                # Execute delay slot, then branch
                dw = self.load32(next_pc)
                if dw != 0:
                    self._step_delay(dw, r, writes)
                target = u32(pc + 4 + (simm << 2))
                pc = target
                continue

            pc = next_pc

        return writes

    def _step_delay(self, dw, r, writes):
        """Execute one delay-slot instruction."""
        op = (dw >> 26) & 0x3F
        rs = (dw >> 21) & 0x1F
        rt = (dw >> 16) & 0x1F
        rd = (dw >> 11) & 0x1F
        sa = (dw >> 6) & 0x1F
        imm = dw & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm

        if dw == 0:
            return                                    # nop
        elif op == 0x00:                              # SPECIAL
            fn = dw & 0x3F
            if fn == 0x21:                            # addu
                r[rd] = u32(r[rs] + r[rt])
            else:
                raise SystemExit(
                    f"FATAL: delay SPECIAL fn 0x{fn:02X}")
        elif op == 0x09:                              # addiu
            r[rt] = u32(r[rs] + simm)
        elif op == 0x2B:                              # sw
            addr = u32(r[rs] + simm)
            val = r[rt]
            self.store32(addr, val)
            writes.append((addr, val))
        else:
            raise SystemExit(
                f"FATAL: delay-slot opcode 0x{op:02X}")


def main():
    if len(sys.argv) != 2:
        raise SystemExit(
            "usage: b22_5de88_oracle.py /path/to/disc1.candidate.exe")

    m = Machine(sys.argv[1])
    writes = m.run()

    assert len(writes) == 27, \
        f"expected 27 writes, got {len(writes)}"
    print("writes", len(writes), "first", writes[0], "last", writes[-1])

    # Verify canonical write sequence from retail transcription
    BASE = 0x800A2090
    END = 0x800A2180
    STATE = 0x800A2174
    GP = 0x8009CD70

    expected = [(node, node + 0xC) for node in range(BASE, END, 0xC)]
    expected.append((STATE, 0))
    expected.extend(((GP + 0x36C, BASE), (GP + 0x374, 0),
                     (GP + 0x370, 0), (GP + 0x378, 0),
                     (GP + 0x37C, 0), (GP + 0x380, 0)))

    assert list(writes) == expected, "5DE88 write order mismatch"
    print("B22 ORACLE: PASS (27 writes, order exact)")


if __name__ == "__main__":
    main()

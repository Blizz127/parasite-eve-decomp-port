#!/usr/bin/env python3
"""Independent retail oracle for func_800527C8 bootstrap dispatcher call order.

Phase 6E-B17.  Does NOT call or depend on the production C port implementation.
Loads the verified retail executable (PS-X EXE, SHA-1 checked, supplied at
runtime — never embedded or committed), verifies all 49 instruction words of
func_800527C8 against the transcribed disassembly, and then EXECUTES those
words with a tiny MIPS interpreter (delay slots honored) that tracks every
jal target in execution order without executing the callees (they return
immediately via the link register sentinel).

The unresolved callees are reported as the canonical dispatch sequence
that the C port must observe through the centralized bootstrap boundary.
As of B25, func_8005D6F4 is translated, leaving two unresolved dispatcher
callees (func_80051CC4, func_80042C78).

Usage:
  dispatcher_oracle.py /path/to/disc1.candidate.exe

Output: one line per jal, then the two-entry unresolved dispatch sequence.
"""

import hashlib
import struct
import sys
from pe_exe_words import exe_hex_list

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

FUNC_ADDR = 0x800527C8
FUNC_WORDS = 49              # 0xC4 bytes
SP_INIT = 0x801FFFF0
RA_SENTINEL = 0xDEADBEEC

MASK = 0xFFFFFFFF

# func_800527C8 transcribed retail body (asm/disc1/42FC8.s:12-62).
# Hex column is little-endian memory byte order; each entry is the uint32
# value formed by those bytes at their word-aligned guest address.
_EXPECTED_LE_HEX = exe_hex_list(0x800527C8, 49, 'le', True)
EXPECTED_WORDS = [struct.unpack("<I", bytes.fromhex(h))[0]
                  for h in _EXPECTED_LE_HEX]

# Known translated callee addresses through B25.  func_800371A4 is REAL
# (Phase 6E-B7); B18/B19/B20/B21/B22/B23/B24/B25 rungs are also translated.
TRANSLATED_CALLEES = {
    0x8005B890,   # func_8005B890  (B17 leaf)
    0x8005BC98,   # func_8005BC98  (B17 leaf, called twice)
    0x8004F808,   # func_8004F808  (B17 leaf)
    0x800528F0,   # func_800528F0  (B18)
    0x8005E588,   # func_8005E588  (B19)
    0x80062568,   # func_80062568  (B20)
    0x80064964,   # func_80064964  (B21 A(28h) bzero + flags)
    0x8005DE88,   # func_8005DE88  (B22 resource-list/state initializer)
    0x80042B38,   # func_80042B38  (B17 leaf)
    0x80051084,   # func_80051084  (B17 leaf)
    0x80052C6C,   # func_80052C6C  (B23 resource-table search + init, with coupled callees)
    0x8005BCBC,   # func_8005BCBC  (B24 resource-state pointer/count selector)
    0x8005D6F4,   # func_8005D6F4  (B25 resource-buffer + display-state init)
    0x800371A4,   # func_800371A4  (B7, REAL)
}


def u32(x):
    return x & MASK


def s32(x):
    x &= MASK
    return x - 0x100000000 if x >= 0x80000000 else x


class Machine:
    """Tiny MIPS-I interpreter over 2 MiB guest RAM seeded with the retail exe
    bytes at their taddr.  Executes func_800527C8 only; jal targets are
    recorded but NOT followed — the interpreter returns via $ra sentinel."""

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
        """Execute func_800527C8, recording every jal target.  Returns the
        ordered list of (target, is_translated) tuples."""
        r = [0] * 32
        r[29] = SP_INIT             # $sp
        r[31] = RA_SENTINEL         # $ra
        r[28] = 0x8009CD70          # $gp (retail value)
        jal_log = []

        pc = FUNC_ADDR
        pending_branch = None
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

            if w == 0:
                pass                                    # nop
            elif op == 0x00:                            # SPECIAL
                fn = w & 0x3F
                if fn == 0x00:                          # sll
                    r[rd] = u32(r[rt] << sa)
                elif fn == 0x21:                        # addu
                    r[rd] = u32(r[rs] + r[rt])
                elif fn == 0x08:                        # jr
                    jump_target = r[rs]
                elif fn == 0x25:                        # or
                    r[rd] = u32(r[rs] | r[rt])
                else:
                    raise SystemExit(
                        f"FATAL: SPECIAL fn 0x{fn:02X} @0x{pc:08X}")
            elif op == 0x03:                            # JAL
                target = (pc & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
                r[31] = u32(pc + 8)                     # link register
                jump_target = target
            elif op == 0x09:                            # addiu
                r[rt] = u32(r[rs] + simm)
            elif op == 0x0F:                            # lui
                r[rt] = u32(imm << 16)
            elif op == 0x23:                            # lw
                r[rt] = self.load32(u32(r[rs] + simm))
            elif op == 0x2B:                            # sw
                self.store32(u32(r[rs] + simm), r[rt])
            else:
                raise SystemExit(
                    f"FATAL: opcode 0x{op:02X} @0x{pc:08X}")

            r[0] = 0
            if jump_target is not None:
                # Delay slot execution
                dw = self.load32(next_pc)
                if dw != 0:
                    self._step_delay(dw, r)
                if jump_target == RA_SENTINEL:
                    break
                # Is this jump_target a jal or a jr?
                if op == 0x03:  # JAL: record, then pretend callee returned
                    is_xlated = jump_target in TRANSLATED_CALLEES
                    jal_log.append((jump_target, is_xlated))
                    # Return to the link register (next instruction after
                    # the delay slot).  Callee execution is not modeled.
                    pc = r[31]
                    if pc == RA_SENTINEL:
                        break
                    continue
                elif op == 0x00 and (w & 0x3F) == 0x08:  # JR: actual return
                    pass  # already handled by RA_SENTINEL check
                else:
                    # branch in delay slot (not used in this function)
                    pc = jump_target
            else:
                pc = next_pc

        return jal_log

    def _step_delay(self, dw, r):
        """Execute one delay-slot instruction (no further control transfer
        occurs in func_800527C8's delay slots)."""
        op = (dw >> 26) & 0x3F
        rs = (dw >> 21) & 0x1F
        rt = (dw >> 16) & 0x1F
        rd = (dw >> 11) & 0x1F
        imm = dw & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm

        if dw == 0:
            return                                    # nop
        elif op == 0x00:                              # SPECIAL
            fn = dw & 0x3F
            if fn == 0x21:                            # addu
                r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x25:                          # or
                r[rd] = u32(r[rs] | r[rt])
            else:
                raise SystemExit(
                    f"FATAL: delay SPECIAL fn 0x{fn:02X}")
        elif op == 0x09:                              # addiu
            r[rt] = u32(r[rs] + simm)
        elif op == 0x2B:                              # sw
            self.store32(u32(r[rs] + simm), r[rt])
        elif op == 0x0F:                              # lui
            r[rt] = u32(imm << 16)
        elif op == 0x23:                              # lw
            r[rt] = self.load32(u32(r[rs] + simm))
        else:
            raise SystemExit(
                f"FATAL: delay-slot opcode 0x{op:02X}")


def addr_name(addr):
    """Map a guest address back to the canonical function name."""
    return f"func_{addr:08X}"


def main():
    if len(sys.argv) != 2:
        raise SystemExit(
            "usage: dispatcher_oracle.py /path/to/disc1.candidate.exe")

    m = Machine(sys.argv[1])
    jal_log = m.run()

    assert len(jal_log) == 17, \
        f"expected 17 jal targets, got {len(jal_log)}"

    print("func_800527C8 jal sequence (retail execution order):")
    unresolved = []
    for i, (target, is_xlated) in enumerate(jal_log, 1):
        tag = "TRANSLATED" if is_xlated else "UNRESOLVED"
        print(f"  {i:2d}. {addr_name(target)}  ({tag})")
        if not is_xlated:
            unresolved.append(target)

    print()
    print(f"Unresolved callee count: {len(unresolved)}")
    assert len(unresolved) == 2, \
        f"expected 2 unresolved callees, got {len(unresolved)}"

    expected_unresolved = [
        0x80051CC4, 0x80042C78,
    ]
    for i, (got, want) in enumerate(zip(unresolved, expected_unresolved)):
        assert got == want, \
            f"dispatch seq[{i}]: got {addr_name(got)}, want {addr_name(want)}"

    print("Dispatch sequence (2 unresolved, retail order):")
    for i, addr in enumerate(unresolved):
        print(f"  [{i}] {addr_name(addr)}")
    print()
    print("DISPATCHER ORACLE: PASS (17 total, 2 unresolved, order exact)")


if __name__ == "__main__":
    main()

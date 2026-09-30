#!/usr/bin/env python3
"""Independent retail oracle for func_80073D24 (VBlank callback slot setter).

Phase 6E-B6.  Does NOT reimplement the C port's state machine: it loads the
verified retail executable (PS-X EXE, SHA-1 checked, supplied at runtime —
never embedded or committed), verifies the transcribed words of the three
functions that define the contract, and then EXECUTES the two semantics-
bearing leaf functions with a tiny MIPS interpreter (delay slots honored)
over a 2 MiB guest RAM seeded with the retail exe bytes at their taddr:

  func_80073D24  (13 words) — jump-table wrapper, forces slot 4, forwards
                              the callee's return.  Verified word-for-word;
                              its only semantics are "slot := 4" and
                              "call table->field_0x14", so the oracle
                              composes it structurally around func_80074478.
  func_80074478  (11 words) — the installed field_0x14 handler: read prev at
                              D_8009568C + (slot<<2), store if different,
                              return prev.  Interpreted.
  func_8007440C  (26 words) — the dispatcher: D_800956AC++, then slots 0..7
                              in order, jalr each non-null handler with no
                              arguments.  Interpreted; a jalr that leaves
                              the function body records a handler "visit".

The field_0x14 install chain (ResetCallback → func_80073E28 → delay-slot
store of func_800743B4's return value) is verified against the exe bytes:
the store word @0x80073ED0 and the lui/addiu pair @0x800743F4/F8 that
materializes 0x80074478 as the return value.

Usage:
  callback_oracle.py /path/to/disc1.candidate.exe

Output: one line per scripted operation, byte-identical to the C port's
--callback-oracle-dump.
"""

import hashlib
import struct
import sys
from pe_exe_words import exe_hex_text

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
MASK = 0xFFFFFFFF

GA_WRAPPER   = 0x80073D24   # func_80073D24
GA_SETTER    = 0x80074478   # func_80074478
GA_DISPATCH  = 0x8007440C   # func_8007440C
GA_TABLE     = 0x8009568C   # D_8009568C: 8 handler slots
GA_COUNTER   = 0x800956AC   # D_800956AC: dispatch counter
GA_JUMPTAB   = 0x8009564C   # D_8009564C: libetc jump table
GA_PTR_JT    = 0x8009566C   # D_8009566C -> GA_JUMPTAB
SP_INIT      = 0x801FFFF0
RA_SENTINEL  = 0xDEADBEEC

# Transcribed retail bodies (asm/disc1/5F3E4.s:6153-6169 and
# asm/disc1/645F8.s — the .s hex column is raw memory byte order, converted
# below).  Verified word-for-word against the exe at load time.
def words(hexcol):
    return [int.from_bytes(bytes.fromhex(h), "little") for h in hexcol.split()]

WRAPPER_WORDS = words(exe_hex_text(0x80073D24, 13, 'le', True))

SETTER_WORDS = words(exe_hex_text(0x80074478, 11, 'le', True))

DISPATCH_WORDS = words(exe_hex_text(0x8007440C, 27, 'le', True))

# Extra bytes proving the field_0x14 install chain:
#   D_8009566C word          -> 0x8009564C (jump table base)
#   D_8009564C+0x14 word     -> 0 (patched at runtime by ResetCallback)
#   @0x80073ED0 (func_80073E28 delay slot): sw $v0, 0x14($v1)
#   @0x800743F4/F8 (func_800743B4 tail):  lui/addiu materializing 0x80074478
PROOF_WORDS = [
    (0x8009566C, words("4C560980")[0]),
    (0x80095660, words("00000000")[0]),
    (0x80073ED0, words("140062AC")[0]),
    (0x800743F4, words("0780023C")[0]),
    (0x800743F8, words("78444224")[0]),
]


def u32(x):
    return x & MASK


class Machine:
    """Tiny MIPS-I interpreter over the 2 MiB guest RAM seeded with the
    retail exe image.  Supports exactly the opcode subset used by the two
    interpreted leaf functions."""

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

        for base, expected in ((GA_WRAPPER, WRAPPER_WORDS),
                               (GA_SETTER, SETTER_WORDS),
                               (GA_DISPATCH, DISPATCH_WORDS)):
            for i, want in enumerate(expected):
                got = self.load32(base + 4 * i)
                if got != want:
                    raise SystemExit(
                        f"FATAL: exe word {i} @0x{base + 4 * i:08X} = "
                        f"0x{got:08X}, transcription said 0x{want:08X}")
        for addr, want in PROOF_WORDS:
            got = self.load32(addr)
            if got != want:
                raise SystemExit(
                    f"FATAL: proof word @0x{addr:08X} = 0x{got:08X}, "
                    f"want 0x{want:08X}")
        # Model the post-ResetCallback install: field_0x14 = func_80074478.
        self.store32(GA_JUMPTAB + 0x14, GA_SETTER)
        # Fresh state: 8 slots + counter zero (image bytes already are).
        for a in (GA_TABLE, GA_COUNTER):
            if self.load32(a) != 0:
                raise SystemExit(f"FATAL: initial state @0x{a:08X} != 0")

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

    def run(self, entry, a0=0, a1=0, visits=None, step_cap=512):
        """Execute the retail function body at `entry` with correct delay
        slots.  A jalr that targets an address outside the body records a
        handler visit and continues at the link register (dispatcher case).
        Returns $v0."""
        r = [0] * 32
        r[4] = a0 & MASK
        r[5] = a1 & MASK
        r[29] = SP_INIT
        r[31] = RA_SENTINEL
        body_lo, body_hi = entry, entry + 0x200
        pc = entry
        pending = None
        steps = 0
        while True:
            steps += 1
            if steps > step_cap:
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
                pass                                        # nop
            elif op == 0x00:                                # SPECIAL
                fn = w & 0x3F
                if fn == 0x00:                              # sll
                    r[rd] = u32(r[rt] << sa)
                elif fn == 0x21:                            # addu
                    r[rd] = u32(r[rs] + r[rt])
                elif fn == 0x08:                            # jr
                    jump_target = r[rs]
                elif fn == 0x09:                            # jalr
                    r[rd] = u32(pc + 8)                     # rd = $ra here
                    jump_target = r[rs]
                else:
                    raise SystemExit(
                        f"FATAL: SPECIAL fn 0x{fn:02X} @0x{pc:08X}")
            elif op == 0x09:                                # addiu
                r[rt] = u32(r[rs] + simm)
            elif op == 0x0F:                                # lui
                r[rt] = u32(imm << 16)
            elif op == 0x04:                                # beq
                if r[rs] == r[rt]:
                    jump_target = u32(pc + 4 + (simm << 2))
            elif op == 0x05:                                # bne
                if r[rs] != r[rt]:
                    jump_target = u32(pc + 4 + (simm << 2))
            elif op == 0x0A:                                # slti
                s = r[rs] - 0x100000000 if r[rs] >= 0x80000000 else r[rs]
                r[rt] = 1 if s < simm else 0
            elif op == 0x23:                                # lw
                r[rt] = self.load32(u32(r[rs] + simm))
            elif op == 0x2B:                                # sw
                self.store32(u32(r[rs] + simm), r[rt])
            else:
                raise SystemExit(f"FATAL: opcode 0x{op:02X} @0x{pc:08X}")

            r[0] = 0
            if jump_target is not None:
                # Execute the delay slot (these bodies use addiu/nop only)
                # before transferring control.
                dw = self.load32(next_pc)
                if dw != 0:
                    dop = (dw >> 26) & 0x3F
                    if dop == 0x09:                         # addiu
                        drs = (dw >> 21) & 0x1F
                        drt = (dw >> 16) & 0x1F
                        dimm = dw & 0xFFFF
                        if dimm >= 0x8000:
                            dimm -= 0x10000
                        r[drt] = u32(r[drs] + dimm)
                        r[0] = 0
                    else:
                        raise SystemExit(
                            f"FATAL: delay-slot opcode 0x{dop:02X} "
                            f"@0x{next_pc:08X}")
                if jump_target == RA_SENTINEL:
                    return r[2]
                if not (body_lo <= jump_target < body_hi):
                    # jalr to a slot handler: record the visit, return
                    # into the dispatcher via the link register.
                    if visits is None:
                        raise SystemExit(
                            f"FATAL: unexpected call target "
                            f"0x{jump_target:08X}")
                    visits.append(jump_target)
                    pc = r[31]
                    if pc == RA_SENTINEL:
                        return r[2]
                    continue
                pc = jump_target
            else:
                pc = next_pc

    # ── Contract operations ────────────────────────────────────────────

    def set_slot(self, slot, handler):
        """Interpreted func_80074478(slot, handler) -> prev."""
        return self.run(GA_SETTER, a0=slot, a1=handler)

    def wrapper_set(self, handler):
        """func_80073D24(handler): verified transcription forces slot 4 and
        calls table->field_0x14 (installed as func_80074478); the wrapper
        forwards the return untouched (jr $ra, nop delay slot)."""
        field = self.load32(self.load32(GA_PTR_JT) + 0x14)
        if field != GA_SETTER:
            raise SystemExit("FATAL: field_0x14 not installed as setter")
        return self.run(GA_SETTER, a0=4, a1=handler)

    def dispatch(self):
        """Interpreted func_8007440C: counter++, slots 0..7 in order."""
        visits = []
        self.run(GA_DISPATCH, visits=visits)
        return self.load32(GA_COUNTER), visits


def fmt_visits(visits):
    return ",".join(f"0x{v:08X}" for v in visits) if visits else "-"


def main():
    if len(sys.argv) != 2:
        raise SystemExit(
            "usage: callback_oracle.py /path/to/disc1.candidate.exe")

    m = Machine(sys.argv[1])

    # Fixed operation script — mirrored op-for-op by the C port's
    # --callback-oracle-dump.  Guest handler addresses 0x80010000/4/8 stand
    # for arbitrary bound callbacks; 0x8003E91C is the real boot callback.
    def wset(handler):
        prev = m.wrapper_set(handler)
        print(f"wset handler=0x{handler:08X} prev=0x{prev:08X} "
              f"slot4=0x{m.load32(GA_TABLE + 16):08X}")

    def sset(slot, handler):
        prev = m.set_slot(slot, handler)
        print(f"sset slot={slot} handler=0x{handler:08X} prev=0x{prev:08X}")

    def dispatch():
        counter, visits = m.dispatch()
        print(f"dispatch counter={counter} visits={fmt_visits(visits)}")

    wset(0x00000000)            # func_8003E680's first call: clear slot 4
    wset(0x8003E91C)            # second call: install the boot callback
    wset(0x8003E91C)            # repeated identical install: no store
    wset(0x80010000)            # replacement: returns previous handler
    wset(0x00000000)            # removal: returns replaced handler
    wset(0x00000000)            # repeated removal: no store, prev 0
    sset(0, 0x80010000)         # other slots via the raw setter
    sset(2, 0x80010004)
    sset(7, 0x80010008)
    dispatch()                  # counter=1, visits slots 0,2,7
    wset(0x8003E91C)            # re-install slot 4
    dispatch()                  # counter=2, visits 0,2,4,7 in slot order
    sset(8, 0x11111111)         # exact retail address arithmetic: slot 8
                                # aliases the dispatch counter @0x800956AC
    slots = [m.load32(GA_TABLE + 4 * i) for i in range(8)]
    print("snapshot slots=" + ",".join(f"0x{s:08X}" for s in slots) +
          f" counter=0x{m.load32(GA_COUNTER):08X}")


if __name__ == "__main__":
    main()

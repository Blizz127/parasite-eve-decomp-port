#!/usr/bin/env python3
"""Independent B23 oracle for func_80052C6C (resource-table search + init).

Phase 6E-B23, CORRECTED during the Phase 6E-B26 mandatory interpreter
audit.  Does NOT call or depend on the production C port implementation.

Why this file was corrected (separate commit, history preserved):
  The original committed oracle carried TWO defects that the B26 audit
  proved material:
  1. Word transcription errors in W_80052C6C, never cross-checked
     against the executable:
       0x80052CC8 was 0x06004290 (byte-swapped residue); the retail
       word is 0x90420006 (lbu $v0, 6($v0)) — the search-loop record
       byte load.
       0x80052DC8 was 0x14C0FF44 (a branch to an out-of-function
       address); the retail word is 0x1440FFCB (bne $v0, $zero, -53)
       — the main loop's i<9 back-edge.  The corrupted word removed
       the loop, so the old oracle executed the main body once.
  2. Interpreter semantics errors shared with the old b24 pattern:
       branch conditions were evaluated AFTER the delay slot executed
       (the slot could clobber the tested register), and not-taken
       branches re-executed their delay slot as an ordinary
       instruction (double execution), and each callee ran on a fresh
       register file with no argument propagation.
  Under the corrected words + corrected MIPS-I semantics the scenario
  below produces: D_8009D03C = 2, db44 called 11 times (a0 sequence
  0,1 | 1,2,3 repeated three times), and ALL NINE output records
  receive byte[9]=0 and halfword[18]=999 plus the 32-byte record copy
  from the seeded source records.  The old "rec0-only" assertion was
  an artifact of the defects above and is retired here.

The interpreter now models MIPS-I faithfully for the modeled bodies:
  * one shared active register file for caller and callees (arguments
    propagate in $a0.., callee clobbers are visible to the caller)
  * every delay slot executes EXACTLY ONCE, taken or not
  * branch conditions sampled at issue, before the slot runs
  * next PC = branch target when taken, PC+8 when not taken
  * exact little-endian memory, 32-bit wraparound, sltu/slti/sltiu,
    sll/srl/sra, mult/mfhi, lwl/lwr/swl/swr
  * $gp-relative stores resolved with retail $gp 0x8009CD70
  * every guest read and write logged with width and order

Before the scenario runs, the interpreter must pass its self-tests
(taken/not-taken slot single execution, non-idempotent register and
memory slot effects, next-PC behavior, jal/jr slots, shared register
file).

Usage:
  b23_oracle.py /path/to/disc1.candidate.exe

Exit 0 on PASS, non-zero on mismatch.
"""

import hashlib
import struct
import sys
from pe_exe_words import exe_words

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

MASK = 0xFFFFFFFF
GP = 0x8009CD70
SP_INIT = 0x801F0000          # oracle entry stack pointer
RA_SENTINEL = 0xBADF00D

# ── Retail instruction words ──────────────────────────────────────────────
# W_80052C6C carries the audit-corrected words at indices 23 and 87;
# every word below is cross-checked against the SHA-verified executable
# at load time and the oracle aborts on any mismatch.

W_80052C6C = exe_words(0x80052C6C, 113)

W_80052E30 = exe_words(0x80052E30, 32)

W_80052EB0 = [0xAF8402DC, 0xAF8502E4, 0x03E00008]

W_80052F0C = [0x8F8202D8, 0x3C03800C, 0x24630E48, 0x00431026, 0x03E00008]

W_80052F70 = exe_words(0x80052F70, 23)

W_8005DB44 = exe_words(0x8005DB44, 18)

W_80051E58 = [0x8F8202A8, 0x03E00008]

FUNCS = {
    0x80052C6C: (W_80052C6C, "func_80052C6C"),
    0x80052E30: (W_80052E30, "func_80052E30"),
    0x80052EB0: (W_80052EB0, "func_80052EB0"),
    0x80052F0C: (W_80052F0C, "func_80052F0C"),
    0x80052F70: (W_80052F70, "func_80052F70"),
    0x8005DB44: (W_8005DB44, "func_8005DB44"),
    0x80051E58: (W_80051E58, "func_80051E58"),
}

# Canonical MIPS-I unaligned-load/store merge tables.
LWL_MASK = (0x00FFFFFF, 0x0000FFFF, 0x000000FF, 0x00000000)
LWL_SHIFT = (24, 16, 8, 0)
LWR_MASK = (0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00)
LWR_SHIFT = (0, 8, 16, 24)
SWL_MASK = (0xFFFFFF00, 0xFFFF0000, 0xFF000000, 0x00000000)
SWL_SHIFT = (24, 16, 8, 0)
SWR_MASK = (0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF)
SWR_SHIFT = (0, 8, 16, 24)


def u32(x):
    return x & MASK


def s32(x):
    x &= MASK
    return x - 0x100000000 if x >= 0x80000000 else x


class Oracle:
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
        self.taddr = struct.unpack_from("<I", self.data, 0x18)[0]
        guest_base = self.taddr - 0x80000000
        size = min(len(self.data) - 0x800, 0x200000 - guest_base)
        self.ram[guest_base:guest_base + size] = \
            self.data[0x800:0x800 + size]

        # Cross-check EVERY modeled word against the executable.
        for addr, (words, name) in FUNCS.items():
            off = 0x800 + (addr - self.taddr)
            for i, exp in enumerate(words):
                got = struct.unpack_from("<I", self.data, off + i * 4)[0]
                if got != exp:
                    raise SystemExit(
                        f"FATAL: {name} word {i} @ {addr + i*4:08X}: "
                        f"exe {got:08X} != transcribed {exp:08X}")

        self.regs = {i: 0 for i in range(32)}
        self.regs[28] = GP
        self.regs[29] = SP_INIT
        self.regs[31] = RA_SENTINEL
        self.hi = 0
        self.lo = 0
        self.reads = []
        self.writes = []
        self.calls = []
        self.db44_args = []
        self.steps = 0

    # ── logged little-endian guest memory ──────────────────────────────
    def ld8(self, a):
        v = self.ram[a & 0x1FFFFF]
        self.reads.append((a, 1, v))
        return v

    def ld16(self, a):
        v = struct.unpack_from("<H", self.ram, a & 0x1FFFFF)[0]
        self.reads.append((a, 2, v))
        return v

    def ld32(self, a):
        v = struct.unpack_from("<I", self.ram, a & 0x1FFFFF)[0]
        self.reads.append((a, 4, v))
        return v

    def st8(self, a, v):
        self.ram[a & 0x1FFFFF] = v & 0xFF
        self.writes.append((a, 1, v & 0xFF))

    def st16(self, a, v):
        struct.pack_into("<H", self.ram, a & 0x1FFFFF, v & 0xFFFF)
        self.writes.append((a, 2, v & 0xFFFF))

    def st32(self, a, v):
        struct.pack_into("<I", self.ram, a & 0x1FFFFF, v & MASK)
        self.writes.append((a, 4, v & MASK))

    # ── corrected MIPS-I core ─────────────────────────────────────────
    def run_one(self, word, ins_addr):
        """Execute one non-branch instruction on the shared regs."""
        if word == 0:
            return
        regs = self.regs
        op = word >> 26
        rs = (word >> 21) & 0x1F
        rt = (word >> 16) & 0x1F
        rd = (word >> 11) & 0x1F
        sa = (word >> 6) & 0x1F
        func = word & 0x3F
        imm = word & 0xFFFF
        simm = imm if imm < 0x8000 else imm - 0x10000

        if op == 0x00:
            if func == 0x21:
                regs[rd] = u32(regs[rs] + regs[rt])
            elif func == 0x23:
                regs[rd] = u32(regs[rs] - regs[rt])
            elif func == 0x2B:
                regs[rd] = 1 if u32(regs[rs]) < u32(regs[rt]) else 0
            elif func == 0x00:
                regs[rd] = u32(regs[rt] << sa)
            elif func == 0x02:
                regs[rd] = u32(regs[rt]) >> sa
            elif func == 0x03:
                regs[rd] = u32(s32(regs[rt]) >> sa)
            elif func == 0x18:
                p = s32(regs[rs]) * s32(regs[rt])
                self.lo = p & MASK
                self.hi = (p >> 32) & MASK
            elif func == 0x10:
                regs[rd] = self.hi
            elif func == 0x12:
                regs[rd] = self.lo
            else:
                raise SystemExit(
                    f"unimpl SPECIAL {func:02X} @{ins_addr:08X}")
        elif op in (0x08, 0x09):
            regs[rt] = u32(regs[rs] + simm)
        elif op == 0x0A:
            regs[rt] = 1 if s32(regs[rs]) < simm else 0
        elif op == 0x0B:
            regs[rt] = 1 if u32(regs[rs]) < u32(simm & MASK) else 0
        elif op == 0x0C:
            regs[rt] = u32(regs[rs] & imm)
        elif op == 0x0D:
            regs[rt] = u32(regs[rs] | imm)
        elif op == 0x0F:
            regs[rt] = imm << 16
        elif op == 0x23:
            regs[rt] = u32(self.ld32(u32(regs[rs] + simm)))
        elif op == 0x24:
            regs[rt] = self.ld8(u32(regs[rs] + simm))
        elif op == 0x25:
            regs[rt] = self.ld16(u32(regs[rs] + simm))
        elif op == 0x22:                                # lwl
            a = u32(regs[rs] + simm)
            n = a & 3
            wd = self.ld32(a & ~3)
            regs[rt] = ((regs[rt] & LWL_MASK[n])
                        | u32(wd << LWL_SHIFT[n]))
        elif op == 0x26:                                # lwr
            a = u32(regs[rs] + simm)
            n = a & 3
            wd = self.ld32(a & ~3)
            regs[rt] = ((regs[rt] & LWR_MASK[n])
                        | (wd >> LWR_SHIFT[n]))
        elif op == 0x28:
            self.st8(u32(regs[rs] + simm), regs[rt])
        elif op == 0x29:
            self.st16(u32(regs[rs] + simm), regs[rt])
        elif op == 0x2B:
            self.st32(u32(regs[rs] + simm), regs[rt])
        elif op == 0x2A:                                # swl
            a = u32(regs[rs] + simm)
            n = a & 3
            cur = self.ld32(a & ~3)
            self.st32(a & ~3, ((cur & SWL_MASK[n])
                               | (regs[rt] >> SWL_SHIFT[n])))
        elif op == 0x2E:                                # swr
            a = u32(regs[rs] + simm)
            n = a & 3
            cur = self.ld32(a & ~3)
            self.st32(a & ~3, ((cur & SWR_MASK[n])
                               | u32(regs[rt] << SWR_SHIFT[n])))
        else:
            raise SystemExit(f"unimpl op {op:02X} @{ins_addr:08X}")

    def exec_func(self, addr):
        """Execute FUNCS[addr] on the SHARED register file."""
        words, name = FUNCS[addr]
        self.calls.append((addr, name))
        if addr == 0x8005DB44:
            self.db44_args.append(self.regs[4])
        n = len(words)
        pc = 0
        while pc < n:
            self.steps += 1
            if self.steps > 2_000_000:
                raise SystemExit("execution did not terminate (step limit)")
            w = words[pc]
            ins_addr = addr + pc * 4
            op = w >> 26
            rs = (w >> 21) & 0x1F
            rt = (w >> 16) & 0x1F
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            af = w & 0x3FFFFFF
            func = w & 0x3F
            delay = words[pc + 1] if pc + 1 < n else None

            def do_delay():
                # Delay slot executes EXACTLY ONCE on the shared file.
                if delay is not None:
                    self.run_one(delay, ins_addr + 4)

            if op == 0x00 and func == 0x08:            # jr
                do_delay()
                return
            elif op == 0x02:                           # j
                target = (af << 2) | (ins_addr & 0xF0000000)
                do_delay()
                pc = (target - addr) // 4
            elif op == 0x03:                           # jal
                target = (af << 2) | (ins_addr & 0xF0000000)
                self.regs[31] = ins_addr + 8
                do_delay()
                self.exec_func(target)
                pc += 2
            elif op == 0x04:                           # beq/beqz
                take = self.regs[rs] == self.regs[rt]   # at issue
                do_delay()
                if take:
                    pc = ((ins_addr + 4 + (simm << 2)) - addr) // 4
                else:
                    pc += 2
            elif op == 0x05:                           # bne/bnez
                take = self.regs[rs] != self.regs[rt]
                do_delay()
                if take:
                    pc = ((ins_addr + 4 + (simm << 2)) - addr) // 4
                else:
                    pc += 2
            elif op == 0x01:                           # REGIMM bltz/bgez
                if rt == 0x00:
                    take = s32(self.regs[rs]) < 0
                elif rt == 0x01:
                    take = s32(self.regs[rs]) >= 0
                else:
                    raise SystemExit(f"unimpl REGIMM rt={rt:02X}")
                do_delay()
                if take:
                    pc = ((ins_addr + 4 + (simm << 2)) - addr) // 4
                else:
                    pc += 2
            else:
                self.run_one(w, ins_addr)
                pc += 1
        raise SystemExit(f"fell off the end of {name} without jr")


# ── Interpreter self-tests (mandatory B26 audit set) ─────────────────────
SELF_BASE = 0xB2301000
SELF_CALLEE = 0xB2302000


def selftest():
    """Prove the corrected delay-slot semantics on synthetic programs."""
    o = Oracle.__new__(Oracle)          # no exe needed for self-tests
    o.ram = bytearray(0x200000)
    o.regs = {i: 0 for i in range(32)}
    o.regs[28] = GP
    o.regs[29] = SP_INIT
    o.regs[31] = RA_SENTINEL
    o.hi = 0
    o.lo = 0
    o.reads = []
    o.writes = []
    o.calls = []
    o.db44_args = []
    o.steps = 0

    def fresh():
        o.regs = {i: 0 for i in range(32)}
        o.regs[28] = GP
        o.regs[29] = SP_INIT
        o.regs[31] = RA_SENTINEL
        o.reads.clear()
        o.writes.clear()
        o.steps = 0

    def run_at(words):
        FUNCS[SELF_BASE] = (words, "selftest")
        o.exec_func(SELF_BASE)
        del FUNCS[SELF_BASE]

    NOP = 0x00000000
    JR = 0x03E00008

    # T1: taken branch — delay slot executes exactly once.
    fresh()
    run_at([
        0x24020000,          # addiu v0, zero, 0
        0x10000001,          # beqz zero, +1 -> [3]
        0x24420001,          #  addiu v0, v0, 1  (slot)
        JR, NOP,
    ])
    assert o.regs[2] == 1, f"T1 failed: v0={o.regs[2]}"

    # T2: non-taken branch — delay slot executes exactly once.
    fresh()
    run_at([
        0x24020000,          # addiu v0, zero, 0
        0x14000001,          # bne zero, zero, +1 (never taken)
        0x24420001,          #  addiu v0, v0, 1  (slot)
        JR, NOP,
    ])
    assert o.regs[2] == 1, f"T2 failed: v0={o.regs[2]}"

    # T3: non-idempotent register update in a non-taken slot occurs
    # exactly once (cumulative add — double execution would give 2).
    fresh()
    run_at([
        0x24020005,          # addiu v0, zero, 5
        0x14000001,          # bne zero, zero (not taken)
        0x24420003,          #  addiu v0, v0, 3  (slot)
        JR, NOP,
    ])
    assert o.regs[2] == 8, f"T3 failed: v0={o.regs[2]}"

    # T4: non-idempotent memory store in a non-taken slot occurs once.
    fresh()
    target = 0x800A5550
    run_at([
        0x3C04800A,                              # lui a0, 0x800A
        0x24845550,                              # addiu a0, 0x5550
        0x24030001,                              # addiu v1, 1
        0x14000001,                              # bne zero, zero (not taken)
        0xAC830000,                              #  sw v1, 0(a0) (slot)
        JR, NOP,
    ])
    n_store = sum(1 for (a, wd, v) in o.writes if a == target)
    val = struct.unpack_from("<I", o.ram, target & 0x1FFFFF)[0]
    assert n_store == 1 and val == 1, \
        f"T4 failed: stores={n_store} val={val}"

    # T5a: taken branch continues at the target (skips PC+8 instruction).
    fresh()
    run_at([
        0x10000002,          # beqz zero, +2 -> [3]
        NOP,                 #  slot
        0x24020022,          # addiu v0, 0x22  (must be skipped)
        0x24030011,          # addiu v1, 0x11  (target)
        JR, NOP,
    ])
    assert o.regs[2] == 0 and o.regs[3] == 0x11, \
        f"T5a failed: v0={o.regs[2]:#x} v1={o.regs[3]:#x}"

    # T5b: non-taken branch continues at PC+8.
    fresh()
    run_at([
        0x14000002,          # bne zero, zero, +2 (not taken)
        NOP,                 #  slot
        0x24020022,          # addiu v0, 0x22  (PC+8 path)
        0x24030011,          # addiu v1, 0x11
        JR, NOP,
    ])
    assert o.regs[2] == 0x22 and o.regs[3] == 0x11, \
        f"T5b failed: v0={o.regs[2]:#x} v1={o.regs[3]:#x}"

    # T6: jal and jr delay slots each execute exactly once, and the
    # callee runs on the SAME active register file (slot writes visible
    # to the callee; callee clobbers visible to the caller).
    fresh()
    FUNCS[SELF_CALLEE] = ([
        0x24420005,          # addiu v0, v0, 5   (sees caller slot +2)
        0x03E00008,          # jr ra
        0x24420001,          #  addiu v0, v0, 1  (jr slot)
    ], "selftest_callee")
    run_at([
        0x24020000,                              # addiu v0, zero, 0
        0x0C000000 | ((SELF_CALLEE >> 2) & 0x3FFFFFF),  # jal callee
        0x24420002,                              #  addiu v0, v0, 2 (slot)
        JR,
        0x24420004,                              #  addiu v0, v0, 4 (jr slot)
    ])
    del FUNCS[SELF_CALLEE]
    # 0 + 2 (jal slot) + 5 (callee) + 1 (callee jr slot) + 4 (jr slot)
    assert o.regs[2] == 12, f"T6 failed: v0={o.regs[2]}"

    print("interpreter self-tests: PASS (7/7)")


# ── Scenario seed and expected footprint ─────────────────────────────────

def seed(m):
    """Deterministic guest state (same seed values as the original B23
    scenario; the descriptive claims in the old file were wrong).

    db44 reads base/alt via sign-extended immediates:
      0x800B0000 + s32(0x8038) = 0x800A8038  (base word)
      0x800B0000 + s32(0x8034) = 0x800A8034  (alt word)
    db44(idx) = idx*32 + 0x800A8028 + alt_base when idx < count.
    With alt_base = 0xFFFF7FD8, base = 0x800B01A0:
      count = (base - alt) >> 5 = 0x04005C0D (huge)
      db44(0) = 0x800A0000 (NON-NULL, byte6 = exe-image BSS = 0)
      db44(k) = 0x800A0000 + k*32, so db44(1+k) = seeded record k.
    """
    struct.pack_into("<I", m.ram, 0x800A8034 & 0x1FFFFF, 0xFFFF7FD8)
    struct.pack_into("<I", m.ram, 0x800A8038 & 0x1FFFFF, 0x800B01A0)
    for k in range(9):
        r = 0x800A0020 + k * 0x20
        for i in range(32):
            m.ram[(r + i) & 0x1FFFFF] = 0xA5
        m.ram[(r + 6) & 0x1FFFFF] = 0x13
        m.ram[(r + 9) & 0x1FFFFF] = 0x5A
        struct.pack_into("<H", m.ram, (r + 18) & 0x1FFFFF, 0x1234)
    m.ram[0x800C0E0C & 0x1FFFFF] = 0
    struct.pack_into("<I", m.ram, (GP + 0x2A8) & 0x1FFFFF, 0)


def expected_writes():
    """ROM-order expected write footprint (hardware-faithful)."""
    w = []
    sp0 = u32(SP_INIT - 0x28)           # func_80052C6C frame
    sp1 = u32(sp0 - 0x18)               # func_80052E30 frame
    sp2 = u32(sp1 - 0x18)               # func_80052F70 frame
    # func_80052C6C frame stores: ra, s4, s3, s2, s1; s0 lands via the
    # jal delay slot.
    w.append((u32(sp0 + 0x24), 4, RA_SENTINEL))
    for i, off in enumerate((0x20, 0x1C, 0x18, 0x14)):
        w.append((u32(sp0 + off), 4, 0))
    w.append((u32(sp0 + 0x10), 4, 0))                  # s0 (jal slot)
    # func_80052E30: beqz a0 taken; its delay slot stores ra.
    w.append((u32(sp1 + 0x10), 4, 0x80052C90))
    # D_8009D048 = 0x800C0E48 (52E30 a0==0 path)
    w.append((GP + 0x2D8, 4, 0x800C0E48))
    # func_80052F70 frame: ra (jal slot), s0.
    w.append((u32(sp2 + 0x14), 4, 0x80052E88))
    w.append((u32(sp2 + 0x10), 4, 0))
    # D_8009D050 = 0 (52F70 return XOR result), D_8009D058, D_8009D064.
    w.append((GP + 0x2E0, 4, 0))
    w.append((GP + 0x2E8, 4, 0x8009D05C))
    w.append((GP + 0x2F4, 4, 2))
    # Clear loop 1: 50 descending halfwords.
    a = 0x800C0EAA
    for _ in range(50):
        w.append((a, 2, 0))
        a -= 2
    # Search result.
    w.append((GP + 0x2CC, 4, 2))
    # Main loop: 9 iterations; each copies the 32-byte source record
    # (lwl/lwr + swl/swr: every aligned word stored twice, swl then
    # swr), then byte9=0, hword18=999.
    src_words = (0xA5A5A5A5, 0xA513A5A5, 0xA5A55AA5, 0xA5A5A5A5,
                 0x1234A5A5, 0xA5A5A5A5, 0xA5A5A5A5, 0xA5A5A5A5)
    for i in range(9):
        dest = 0x800A1E64 + i * 32
        for k, val in enumerate(src_words):
            w.append((u32(dest + k * 4), 4, val))     # swl
            w.append((u32(dest + k * 4), 4, val))     # swr
        w.append((u32(dest + 9), 1, 0))
        w.append((u32(dest + 18), 2, 999))
    # Clear loop 2: 100 descending halfwords.
    a = 0x800C1F7E
    for _ in range(100):
        w.append((a, 2, 0))
        a -= 2
    # Clear loop 3: 82 descending halfwords.
    a = 0x800C2022
    for _ in range(82):
        w.append((a, 2, 0))
        a -= 2
    # Final gp store.
    w.append((GP + 0x2DC, 4, 0))
    return w


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: b23_oracle.py /path/to/disc1.candidate.exe")
    path = sys.argv[1]

    selftest()

    m = Oracle(path)
    seed(m)
    m.exec_func(0x80052C6C)

    ok = True

    # 1. Exact ROM-order write footprint.
    exp = expected_writes()
    if m.writes != exp:
        print(f"  FAIL write log: got {len(m.writes)}, want {len(exp)}")
        for i, (g, e) in enumerate(zip(m.writes, exp)):
            if g != e:
                print(f"    first divergence at [{i}]: got {g}, want {e}")
                break
        ok = False
    else:
        print(f"  write log: {len(m.writes)} writes, ROM order exact")

    # 2. Return value and gp-state results.
    if m.regs[2] != 0x800C1F7E:
        print(f"  FAIL return v0 = {m.regs[2]:#x}, want 0x800C1F7E")
        ok = False
    d3c = struct.unpack_from("<I", m.ram, (GP + 0x2CC) & 0x1FFFFF)[0]
    d4c = struct.unpack_from("<I", m.ram, (GP + 0x2DC) & 0x1FFFFF)[0]
    if d3c != 2:
        print(f"  FAIL D_8009D03C = {d3c}, want 2")
        ok = False
    if d4c != 0:
        print(f"  FAIL D_8009D04C = {d4c}, want 0")
        ok = False

    # 3. Clear ranges zeroed.
    for lo, hi in ((0x800C0E48, 0x800C0EAA),
                   (0x800C1EB8, 0x800C1F7E),
                   (0x800C1F80, 0x800C2022)):
        a = lo
        while a <= hi:
            if struct.unpack_from("<H", m.ram, a & 0x1FFFFF)[0] != 0:
                print(f"  FAIL clear-range halfword {a:08X} != 0")
                ok = False
            a += 2

    # 4. Output records: ALL NINE carry byte9 = 0 and hword18 = 999,
    # plus the copied full 32-byte source record from db44(1 + i%3) =
    # seeded records 0..2 (then byte9/hword18 overwrite the copy).
    exp_rec = bytes([
        0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0x13, 0xA5,
        0xA5, 0x00, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
        0xA5, 0xA5, 0xE7, 0x03, 0xA5, 0xA5, 0xA5, 0xA5,
        0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
    ])
    for i in range(9):
        base = 0x800A1E64 + i * 32
        got = bytes(m.ram[base & 0x1FFFFF:(base + 32) & 0x1FFFFF])
        if got != exp_rec:
            print(f"  FAIL rec{i} bytes {got.hex()}, "
                  f"want {exp_rec.hex()}")
            ok = False

    # 5. Call chain and db44 argument sequence.
    exp_calls = (["func_80052C6C", "func_80052E30", "func_80052F70",
                  "func_80051E58", "func_80051E58"]
                 + ["func_8005DB44"] * 11)
    got_calls = [name for _, name in m.calls]
    if got_calls != exp_calls:
        print(f"  FAIL call chain: {got_calls}")
        ok = False
    exp_db44_args = [0, 1, 1, 2, 3, 1, 2, 3, 1, 2, 3]
    if m.db44_args != exp_db44_args:
        print(f"  FAIL db44 args: {m.db44_args}, want {exp_db44_args}")
        ok = False

    if ok:
        print("B23 ORACLE (corrected): PASS")
        sys.exit(0)
    print("B23 ORACLE (corrected): FAIL")
    sys.exit(1)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
Phase 6E-B26 independent oracle for func_8005DC4C.

Loads the verified retail executable (PS-X EXE, SHA-1 checked, supplied
at runtime — never embedded or committed), cross-checks every modeled
instruction word against it, then EXECUTES those words with a
delay-slot-aware MIPS-I interpreter:

  * one shared active register file for main path and delay slots
  * branch conditions sampled AT ISSUE, before the delay slot runs
  * every delay slot executes EXACTLY ONCE, taken or not
  * next PC is the branch target when taken, PC+8 when not taken
  * exact little-endian guest memory, exact 32-bit wraparound
  * every guest read and write logged with address, width, value, order

Guest memory is the plain 2 MiB KSEG0 window [0x80000000, 0x80200000),
matching the port's checked-access model.  Returned ADDRESSES are never
dereferenced by this routine, so a return outside that window is
recorded faithfully rather than clamped (see S11).

This oracle does NOT call or depend on the production C port.

Usage:
  b26_oracle.py /path/to/disc1.candidate.exe
"""
import hashlib
import struct
import sys

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

FUNC_5DC4C = 0x8005DC4C
FUNC_5DC9C = 0x8005DC9C
N_WORDS = 20

RAM_BASE = 0x80000000
RAM_SIZE = 0x00200000
RAM_END = RAM_BASE + RAM_SIZE

GP = 0x8009CD70
SP_INIT = 0x801FFF00
RA_SENTINEL = 0xDEAD0000

HDR = 0x800A8028          # lui 0x800B + addiu sext(0x8028)
HDR_REL = 0x800A802C      # lui 0x800B + lw   sext(0x802C)

# Transcribed body of func_8005DC4C.  Every word is re-verified against
# the SHA-checked exe at load time before anything is executed.
W_5DC4C = [
    0x3C02800B,  # 8005DC4C lui   $v0,0x800B
    0x8C42802C,  # 8005DC50 lw    $v0,%lo(D_800A802C)($v0)
    0x3C03800B,  # 8005DC54 lui   $v1,0x800B
    0x24638028,  # 8005DC58 addiu $v1,$v1,%lo(D_800A8028)
    0x00431021,  # 8005DC5C addu  $v0,$v0,$v1
    0x8C430004,  # 8005DC60 lw    $v1,4($v0)
    0x00000000,  # 8005DC64 nop
    0x00431821,  # 8005DC68 addu  $v1,$v0,$v1
    0x94620000,  # 8005DC6C lhu   $v0,0($v1)
    0x00000000,  # 8005DC70 nop
    0x0082102B,  # 8005DC74 sltu  $v0,$a0,$v0
    0x10400005,  # 8005DC78 beqz  $v0,.L8005DC90
    0x00041040,  # 8005DC7C  sll  $v0,$a0,1        (delay slot)
    0x00431021,  # 8005DC80 addu  $v0,$v0,$v1
    0x84420002,  # 8005DC84 lh    $v0,2($v0)
    0x08017725,  # 8005DC88 j     .L8005DC94
    0x00621021,  # 8005DC8C  addu $v0,$v1,$v0      (delay slot)
    0x00001021,  # 8005DC90 addu  $v0,$zero,$zero
    0x03E00008,  # 8005DC94 jr    $ra
    0x00000000,  # 8005DC98  nop                   (delay slot)
]


def u32(x):
    return x & 0xFFFFFFFF


def s32(x):
    x &= 0xFFFFFFFF
    return x - 0x100000000 if x & 0x80000000 else x


class Machine:
    def __init__(self, path=None):
        if path is not None:
            with open(path, "rb") as f:
                data = f.read()
            sha1 = hashlib.sha1(data).hexdigest()
            if sha1 != RETAIL_SHA1:
                raise SystemExit(
                    f"FATAL: {path} SHA-1 {sha1} != retail {RETAIL_SHA1}")
            if data[:8] != b"PS-X EXE":
                raise SystemExit("FATAL: not a PS-X EXE")
            taddr = struct.unpack_from("<I", data, 0x18)[0]
            for i, exp in enumerate(W_5DC4C):
                a = FUNC_5DC4C + i * 4
                got = struct.unpack_from("<I", data, a - taddr + 0x800)[0]
                if got != exp:
                    raise SystemExit(
                        f"FATAL: word {i} @0x{a:08X}: "
                        f"exe {got:08X} != transcribed {exp:08X}")
            if FUNC_5DC4C + N_WORDS * 4 != FUNC_5DC9C:
                raise SystemExit("FATAL: body extent mismatch")
            print(f"exe SHA-1 {sha1} OK; {N_WORDS} words cross-checked "
                  f"@0x{FUNC_5DC4C:08X}..0x{FUNC_5DC9C - 1:08X}")
        self.reset()

    def reset(self, fill=0):
        self.ram = bytearray([fill]) * RAM_SIZE
        self.regs = {i: 0 for i in range(32)}
        self.regs[28] = GP
        self.regs[29] = SP_INIT
        self.regs[31] = RA_SENTINEL
        self.reads = []
        self.writes = []
        self.steps = 0

    # ── memory ────────────────────────────────────────────────────────
    def _off(self, a, size):
        if not (RAM_BASE <= a and a + size <= RAM_END):
            raise SystemExit(
                f"FATAL: guest access out of RAM: 0x{a:08X} size {size}")
        return a - RAM_BASE

    def load(self, a, size, signed=False):
        o = self._off(a, size)
        v = int.from_bytes(self.ram[o:o + size], "little")
        if signed and v & (1 << (size * 8 - 1)):
            v -= 1 << (size * 8)
        self.reads.append((a, size, u32(v)))
        return u32(v)

    def store(self, a, size, val):
        o = self._off(a, size)
        val = u32(val) & ((1 << (size * 8)) - 1)
        self.ram[o:o + size] = val.to_bytes(size, "little")
        self.writes.append((a, size, val))

    def poke(self, a, size, val):
        """Seed guest memory without logging an executed access."""
        o = self._off(a, size)
        self.ram[o:o + size] = (u32(val) & ((1 << (size * 8)) - 1)
                                ).to_bytes(size, "little")

    # ── one instruction (main path AND delay slots use this) ──────────
    def run_one(self, w, addr):
        op = w >> 26
        rs = (w >> 21) & 0x1F
        rt = (w >> 16) & 0x1F
        rd = (w >> 11) & 0x1F
        sa = (w >> 6) & 0x1F
        fn = w & 0x3F
        imm = w & 0xFFFF
        simm = imm if imm < 0x8000 else imm - 0x10000
        r = self.regs
        if w == 0:
            return
        if op == 0x00:
            if fn == 0x00:
                r[rd] = u32(r[rt] << sa)
            elif fn == 0x02:
                r[rd] = u32(r[rt]) >> sa
            elif fn == 0x03:
                r[rd] = u32(s32(r[rt]) >> sa)
            elif fn == 0x21:
                r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x23:
                r[rd] = u32(r[rs] - r[rt])
            elif fn == 0x24:
                r[rd] = r[rs] & r[rt]
            elif fn == 0x25:
                r[rd] = r[rs] | r[rt]
            elif fn == 0x2A:
                r[rd] = 1 if s32(r[rs]) < s32(r[rt]) else 0
            elif fn == 0x2B:
                r[rd] = 1 if u32(r[rs]) < u32(r[rt]) else 0
            else:
                raise SystemExit(f"unimpl SPECIAL fn {fn:02X} @{addr:08X}")
        elif op == 0x09:
            r[rt] = u32(r[rs] + simm)
        elif op == 0x0C:
            r[rt] = r[rs] & imm
        elif op == 0x0D:
            r[rt] = r[rs] | imm
        elif op == 0x0F:
            r[rt] = u32(imm << 16)
        elif op == 0x20:
            r[rt] = u32(self.load(u32(r[rs] + simm), 1, signed=True))
        elif op == 0x21:
            r[rt] = u32(self.load(u32(r[rs] + simm), 2, signed=True))
        elif op == 0x23:
            r[rt] = self.load(u32(r[rs] + simm), 4)
        elif op == 0x24:
            r[rt] = self.load(u32(r[rs] + simm), 1)
        elif op == 0x25:
            r[rt] = self.load(u32(r[rs] + simm), 2)
        elif op == 0x28:
            self.store(u32(r[rs] + simm), 1, r[rt])
        elif op == 0x29:
            self.store(u32(r[rs] + simm), 2, r[rt])
        elif op == 0x2B:
            self.store(u32(r[rs] + simm), 4, r[rt])
        else:
            raise SystemExit(f"unimpl op {op:02X} @{addr:08X}")
        r[0] = 0

    # ── execution ─────────────────────────────────────────────────────
    def exec_words(self, base, words):
        n = len(words)
        pc = 0
        while pc < n:
            self.steps += 1
            if self.steps > 1_000_000:
                raise SystemExit("execution did not terminate (step limit)")
            w = words[pc]
            ins_addr = base + pc * 4
            op = w >> 26
            rs = (w >> 21) & 0x1F
            rt = (w >> 16) & 0x1F
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            af = w & 0x3FFFFFF
            fn = w & 0x3F
            delay = words[pc + 1] if pc + 1 < n else None

            def do_delay():
                # Executes EXACTLY ONCE on the shared register file,
                # whether or not the branch is taken.
                if delay is not None:
                    self.run_one(delay, ins_addr + 4)

            if op == 0x00 and fn == 0x08:              # jr
                do_delay()
                return self.regs[2]
            elif op == 0x02:                           # j
                target = (af << 2) | (ins_addr & 0xF0000000)
                do_delay()
                pc = (target - base) // 4
            elif op in (0x04, 0x05):                   # beq / bne
                take = (self.regs[rs] == self.regs[rt]) if op == 0x04 \
                    else (self.regs[rs] != self.regs[rt])
                do_delay()
                if take:
                    pc = ((ins_addr + 4 + (simm << 2)) - base) // 4
                else:
                    pc += 2                            # PC+8: slot is done
            elif op == 0x01:                           # bltz / bgez
                if rt == 0x00:
                    take = s32(self.regs[rs]) < 0
                elif rt == 0x01:
                    take = s32(self.regs[rs]) >= 0
                else:
                    raise SystemExit(f"unimpl REGIMM rt={rt:02X}")
                do_delay()
                if take:
                    pc = ((ins_addr + 4 + (simm << 2)) - base) // 4
                else:
                    pc += 2
            else:
                self.run_one(w, ins_addr)
                pc += 1
        raise SystemExit("fell off the end without jr")

    def call_5dc4c(self, idx, a1=0):
        self.reads = []
        self.writes = []
        self.regs[4] = u32(idx)
        self.regs[5] = u32(a1)
        self.regs[31] = RA_SENTINEL
        return self.exec_words(FUNC_5DC4C, W_5DC4C)


# ── interpreter self-tests (mandatory B26 audit set) ──────────────────
SELF_BASE = 0xB2601000
NOP = 0x00000000
JR = 0x03E00008


def selftest():
    m = Machine(None)
    ok = 0

    # T1: taken branch — delay slot executes exactly once.
    m.reset()
    m.exec_words(SELF_BASE, [
        0x10000002,   # beq zero,zero -> +2 (TAKEN), target = index 3
        0x24630001,   # (delay) addiu v1,v1,1
        0x24630001,   # must NOT run: branched over
        JR, NOP,
    ])
    assert m.regs[3] == 1, f"T1 slot ran {m.regs[3]}x, want 1"
    ok += 1

    # T2: non-taken branch — delay slot executes exactly once.
    m.reset()
    m.regs[8] = 1
    m.exec_words(SELF_BASE, [
        0x11000002,   # beq t0,zero -> NOT taken (t0=1)
        0x24630001,   # (delay) addiu v1,v1,1
        JR, NOP,
    ])
    assert m.regs[3] == 1, f"T2 slot ran {m.regs[3]}x, want 1"
    ok += 1

    # T3: non-idempotent REGISTER update in a non-taken delay slot runs once.
    m.reset()
    m.regs[8] = 1
    m.regs[9] = 100
    m.exec_words(SELF_BASE, [
        0x11000002,   # beq t0,zero -> NOT taken
        0x25290005,   # (delay) addiu t1,t1,5   — non-idempotent
        JR, NOP,
    ])
    assert m.regs[9] == 105, f"T3 t1={m.regs[9]}, want 105"
    ok += 1

    # T4: non-idempotent MEMORY store in a non-taken delay slot runs once,
    #     proven by the write log rather than the resulting value.
    m.reset()
    m.regs[8] = 1
    m.regs[10] = 0x80100000
    m.regs[11] = 0xAB
    m.exec_words(SELF_BASE, [
        0x11000002,   # beq t0,zero -> NOT taken
        0xAD4B0000,   # (delay) sw t3,0(t2)
        JR, NOP,
    ])
    n = len([w for w in m.writes if w[0] == 0x80100000])
    assert n == 1, f"T4 store logged {n}x, want 1"
    ok += 1

    # T5: next PC is the target when taken, PC+8 when not taken.
    m.reset()
    m.regs[8] = 0
    m.exec_words(SELF_BASE, [
        0x11000002,   # beq t0,zero -> TAKEN -> index 3
        NOP,          # (delay)
        0x240C00AA,   # t4 = 0xAA   <- where PC+8 would land; must be skipped
        0x240D00BB,   # t5 = 0xBB   <- branch target
        JR, NOP,
    ])
    assert m.regs[12] == 0 and m.regs[13] == 0xBB, "T5 taken next-PC wrong"
    m.reset()
    m.regs[8] = 1
    m.exec_words(SELF_BASE, [
        0x11000002,   # beq t0,zero -> NOT taken
        NOP,          # (delay)
        0x240C00AA,   # t4 = 0xAA   <- PC+8 lands here
        JR, NOP,
    ])
    assert m.regs[12] == 0xAA, "T5 not-taken next-PC wrong"
    ok += 1

    # T6: j and jr delay slots each execute exactly once.
    m.reset()
    target = SELF_BASE + 4 * 4
    jw = 0x08000000 | ((target >> 2) & 0x03FFFFFF)
    m.exec_words(SELF_BASE, [
        jw,           # j -> index 4
        0x24630001,   # (delay) addiu v1,v1,1
        0x24630001,   # skipped
        0x24630001,   # skipped
        JR,
        0x24630001,   # (jr delay) addiu v1,v1,1
    ])
    assert m.regs[3] == 2, f"T6 v1={m.regs[3]}, want 2"
    ok += 1

    # T7: main path and delay slot share ONE active register file.
    m.reset()
    m.regs[8] = 1
    m.exec_words(SELF_BASE, [
        0x240E0007,   # t6 = 7            (main path)
        0x11000002,   # beq t0,zero -> NOT taken
        0x25CE0001,   # (delay) addiu t6,t6,1 — reads the main-path value
        JR, NOP,
    ])
    assert m.regs[14] == 8, f"T7 t6={m.regs[14]}, want 8"
    ok += 1

    print(f"interpreter self-tests: PASS ({ok}/7)")


# ── scenarios ─────────────────────────────────────────────────────────
def seed_archive(m, rel, sub_rel, count, entries):
    """Build the nested relative-offset archive the routine walks."""
    m.poke(HDR_REL, 4, rel)
    ptr = u32(rel + HDR)
    m.poke(ptr + 4, 4, sub_rel)
    tbl = u32(ptr + sub_rel)
    m.poke(tbl, 2, count)
    for i, e in enumerate(entries):
        m.poke(tbl + 2 + i * 2, 2, e & 0xFFFF)
    return ptr, tbl


RESULTS = []


def expect(name, got, want):
    good = got == want
    RESULTS.append(good)
    print(f"  {name:<42} {str(got):>14}  want {str(want):<14} "
          f"{'ok' if good else 'FAIL'}")
    return good


def main():
    if len(sys.argv) != 2:
        sys.exit("usage: b26_oracle.py /path/to/disc1.candidate.exe")
    selftest()
    m = Machine(sys.argv[1])

    # Measured Disc 1 USA archive shape (read out of guest RAM at the
    # func_8005D6F4 call site, not taken from port output).
    REL, SUB, COUNT = 0x30, 0x14, 120
    entries = [242 + 4 * i for i in range(COUNT)]
    entries[30] = 573

    print("\nS1  empty source state (archive region all zero)")
    m.reset()
    r = m.call_5dc4c(30)
    expect("return", r, 0)
    expect("guest writes", len(m.writes), 0)
    expect("read order/widths", [(hex(a), w) for a, w, _ in m.reads],
           [(hex(HDR_REL), 4), (hex(HDR + 4), 4), (hex(HDR), 2)])

    print("\nS2  populated archive (measured Disc 1 USA shape)")
    m.reset()
    ptr, tbl = seed_archive(m, REL, SUB, COUNT, entries)
    expect("ptr", hex(ptr), hex(0x800A8058))
    expect("tbl", hex(tbl), hex(0x800A806C))
    expect("min(entry) == 2 + 2*count", min(entries), 2 + 2 * COUNT)
    expect("return", hex(m.call_5dc4c(30)), hex(0x800A82A9))
    expect("guest writes", len(m.writes), 0)
    expect("read order/widths", [(hex(a), w) for a, w, _ in m.reads],
           [(hex(HDR_REL), 4), (hex(ptr + 4), 4), (hex(tbl), 2),
            (hex(tbl + 2 + 60), 2)])

    print("\nS3  immediate 0xFF terminator at the returned record")
    m.reset()
    ptr, tbl = seed_archive(m, REL, SUB, COUNT, entries)
    m.poke(u32(tbl + 573), 1, 0xFF)
    r = m.call_5dc4c(30)
    expect("return", hex(r), hex(0x800A82A9))
    expect("record byte at return", hex(m.ram[u32(r) - RAM_BASE]), hex(0xFF))
    expect("reads (record itself NOT read)", len(m.reads), 4)

    print("\nS4  out-of-range index -> retail's own 0")
    m.reset()
    seed_archive(m, REL, SUB, COUNT, entries)
    expect("idx == count (120)", m.call_5dc4c(COUNT), 0)
    expect("reads on failure path", len(m.reads), 3)
    expect("writes on failure path", len(m.writes), 0)
    expect("idx = 65535", m.call_5dc4c(65535), 0)
    expect("idx = 0x80000000 (unsigned compare)",
           m.call_5dc4c(0x80000000), 0)

    print("\nS5  first and last valid entry")
    m.reset()
    ptr, tbl = seed_archive(m, REL, SUB, COUNT, entries)
    expect("idx 0", hex(m.call_5dc4c(0)), hex(u32(tbl + entries[0])))
    expect("idx 119 (last)", hex(m.call_5dc4c(119)),
           hex(u32(tbl + entries[119])))
    expect("idx 119 non-zero", m.call_5dc4c(119) != 0, True)

    print("\nS6  repeated calls stable (never advancing)")
    m.reset()
    seed_archive(m, REL, SUB, COUNT, entries)
    seq = [m.call_5dc4c(30) for _ in range(5)]
    expect("five returns identical", len(set(seq)), 1)
    expect("value", hex(seq[0]), hex(0x800A82A9))
    expect("writes across five calls", len(m.writes), 0)

    print("\nS7  $a1 is never read (one-argument signature)")
    m.reset()
    seed_archive(m, REL, SUB, COUNT, entries)
    a = m.call_5dc4c(30, a1=0xFF)
    b = m.call_5dc4c(30, a1=0x800C0DF4)
    c = m.call_5dc4c(30, a1=0xFFFFFFFF)
    expect("return independent of $a1", a == b == c, True)
    expect("value", hex(a), hex(0x800A82A9))

    print("\nS8  signed entries and exact 32-bit wraparound")
    for label, off in (("negative (-0x40)", -0x40),
                       ("s16 min (-32768)", -0x8000),
                       ("s16 max (32767)", 0x7FFF)):
        m.reset()
        e = list(entries)
        e[30] = off
        ptr, tbl = seed_archive(m, REL, SUB, COUNT, e)
        expect(label, hex(m.call_5dc4c(30)), hex(u32(tbl + off)))

    print("\nS9  dirty guest state (0xA5 fill outside the archive)")
    m.reset(fill=0xA5)
    ptr, tbl = seed_archive(m, REL, SUB, COUNT, entries)
    expect("return", hex(m.call_5dc4c(30)), hex(0x800A82A9))
    expect("guest writes", len(m.writes), 0)

    print("\nS10 PE_RamReset equivalent (fresh zero RAM)")
    m.reset()
    expect("return", m.call_5dc4c(30), 0)
    expect("guest writes", len(m.writes), 0)

    print("\nS11 return-address boundary cases")
    m.reset()
    m.poke(HDR_REL, 4, u32(RAM_BASE - HDR))     # ptr = RAM_BASE
    m.poke(RAM_BASE + 4, 4, 0)                  # tbl = RAM_BASE
    m.poke(RAM_BASE, 2, 1)
    m.poke(RAM_BASE + 2, 2, 0)
    expect("tbl == RAM_BASE, entry 0", hex(m.call_5dc4c(0)), hex(RAM_BASE))
    m.reset()
    tbl2 = 0x801F8000                           # entry 0x7FFF -> last byte
    m.poke(HDR_REL, 4, u32(tbl2 - HDR))
    m.poke(tbl2 + 4, 4, 0)
    m.poke(tbl2, 2, 1)
    m.poke(tbl2 + 2, 2, 0x7FFF)
    expect("entry -> last RAM byte", hex(m.call_5dc4c(0)), hex(RAM_END - 1))
    m.reset()
    tbl3 = 0x801FF000                           # deliberately overshoots RAM
    m.poke(HDR_REL, 4, u32(tbl3 - HDR))
    m.poke(tbl3 + 4, 4, 0)
    m.poke(tbl3, 2, 1)
    m.poke(tbl3 + 2, 2, 0x7FFF)
    expect("return may leave RAM (not clamped)",
           hex(m.call_5dc4c(0)), hex(u32(tbl3 + 0x7FFF)))
    expect("...and that is above RAM_END", u32(tbl3 + 0x7FFF) >= RAM_END, True)

    ok = all(RESULTS)
    print(f"\nB26 ORACLE: {'PASS' if ok else 'FAIL'} "
          f"({sum(RESULTS)}/{len(RESULTS)} checks)")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())

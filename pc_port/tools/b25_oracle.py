#!/usr/bin/env python3
"""Independent B25 oracle for func_8005D6F4 (resource-buffer + display
state initializer).

Phase 6E-B25.  Does NOT call or depend on the production C port
implementation.

Loads the verified retail executable (PS-X EXE, SHA-1 checked, supplied
at runtime — never embedded or committed), cross-checks all 147
instruction words of func_8005D6F4 against the transcribed retail body,
then EXECUTES those words with a delay-slot-aware MIPS-I interpreter:

  * one shared active register file for branch and delay-slot execution
  * branch conditions sampled at issue, BEFORE the delay slot runs
  * exact little-endian load/store semantics (byte/halfword/word)
  * exact 32-bit arithmetic (sltu/addu/subu/andi/lui/ori)
  * every guest read and write logged with address, width, value, order
  * func_80071A24 modeled by the proven BIOS A(28h) bzero contract
    (ascending byte zeroes over [a0, a0+a1), returns a0) — the same
    contract as tools/b21_bzero_oracle.py
  * the nine unresolved callees are recorded (target, a0, a1) but NOT
    executed; each returns a scenario-controlled value (the
    "controlled dependency return" model), exposing the partial state
    at the boundary
  * final guest state and return value asserted per scenario

Usage:
  b25_oracle.py /path/to/disc1.candidate.exe

Exit 0 on PASS, non-zero on mismatch.
"""

import hashlib
import struct
import sys
from pe_exe_words import exe_words

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

MASK = 0xFFFFFFFF
GP = 0x8009CD70
FUNC_ADDR = 0x8005D6F4

# Transcribed retail body, asm/disc1/4CC98.s:1340-1502 (comment-column
# little-endian byte order decoded to uint32 words).
W_5D6F4 = exe_words(0x8005D6F4, 147)

BUF_BASE = 0x800C0DE0
BUF_SEL = 0x800C0DF0
BZERO_LEN = 0x12E4
GA_FLAG = 0x8009D218
GA_C8 = 0x8009D0C8
GA_C0 = 0x8009D0C0
GA_C4 = 0x8009D0C4

SP_INIT = 0x801F0000
SP_FRAME = SP_INIT - 0x18            # addiu $sp, $sp, -0x18
SP_S0 = SP_FRAME + 0x10
SP_RA = SP_FRAME + 0x14
RA_SENTINEL = 0xDEADBEEF

REAL_BZERO = 0x80071A24          # modeled by the A(28h) contract
UNRESOLVED = {
    0x8005DC4C, 0x8005DC9C, 0x80052594, 0x8005CCA4, 0x800614AC,
    0x8005E884, 0x8005E850, 0x800649D0, 0x80052790,
}


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
        for i, exp in enumerate(W_5D6F4):
            got = struct.unpack_from("<I", self.data, off + i * 4)[0]
            if got != exp:
                raise SystemExit(
                    f"FATAL: word {i} @ {FUNC_ADDR + i*4:08X}: "
                    f"exe {got:08X} != transcribed {exp:08X}")
        self.reads = []
        self.writes = []
        self.calls = []

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

    # ── execution ─────────────────────────────────────────────────────
    def call(self, controlled):
        """Execute func_8005D6F4.  `controlled` maps (addr, nth-call-of-
        that-addr) to the return value an unresolved callee produces."""
        words = W_5D6F4
        regs = {i: 0 for i in range(32)}
        regs[28] = GP
        regs[29] = SP_INIT
        regs[31] = RA_SENTINEL
        n = len(words)
        pc = 0
        call_counts = {}

        def callee(target):
            """Model a jal target.  Returns the callee's $v0."""
            if target == REAL_BZERO:
                dst = regs[4]
                ln = regs[5]
                self.calls.append((target, dst, ln))
                for i in range(ln):
                    self.st8(dst + i, 0)
                return dst
            if target in UNRESOLVED:
                k = call_counts.get(target, 0)
                call_counts[target] = k + 1
                self.calls.append((target, regs[4], regs[5]))
                return controlled.get((target, k), 0)
            raise SystemExit(f"FATAL: unmodeled jal target {target:08X}")

        while pc < n:
            w = words[pc]
            ins_addr = FUNC_ADDR + pc * 4
            op = w >> 26
            rs = (w >> 21) & 0x1F
            rt = (w >> 16) & 0x1F
            rd = (w >> 11) & 0x1F
            sa = (w >> 6) & 0x1F
            func = w & 0x3F
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            addr_field = w & 0x3FFFFFF
            delay = words[pc + 1] if pc + 1 < n else None

            def run(word):
                if word == 0:
                    return
                o = word >> 26
                r_s = (word >> 21) & 0x1F
                r_t = (word >> 16) & 0x1F
                r_d = (word >> 11) & 0x1F
                s_a = (word >> 6) & 0x1F
                f = word & 0x3F
                im = word & 0xFFFF
                si = im if im < 0x8000 else im - 0x10000
                af = word & 0x3FFFFFF
                if o == 0x00:
                    if f == 0x21:
                        regs[r_d] = u32(regs[r_s] + regs[r_t])
                    elif f == 0x23:
                        regs[r_d] = u32(regs[r_s] - regs[r_t])
                    elif f == 0x2B:
                        regs[r_d] = 1 if u32(regs[r_s]) < u32(regs[r_t]) else 0
                    elif f == 0x00:
                        regs[r_d] = u32(regs[r_t] << s_a)
                    else:
                        raise SystemExit(f"delay SPECIAL {f:02X}")
                elif o == 0x08 or o == 0x09:
                    regs[r_t] = u32(regs[r_s] + si)
                elif o == 0x0C:
                    regs[r_t] = u32(regs[r_s] & im)
                elif o == 0x0D:
                    regs[r_t] = u32(regs[r_s] | im)
                elif o == 0x0F:
                    regs[r_t] = im << 16
                elif o == 0x23:
                    regs[r_t] = u32(self.ld32(u32(regs[r_s] + si)))
                elif o == 0x24:
                    regs[r_t] = self.ld8(u32(regs[r_s] + si))
                elif o == 0x28:
                    self.st8(u32(regs[r_s] + si), regs[r_t])
                elif o == 0x29:
                    self.st16(u32(regs[r_s] + si), regs[r_t])
                elif o == 0x2B:
                    self.st32(u32(regs[r_s] + si), regs[r_t])
                elif o == 0x03:
                    target = (af << 2) | (ins_addr & 0xF0000000)
                    regs[2] = callee(target)
                elif o == 0x02:
                    pass  # j in delay slot does not occur in this body
                else:
                    raise SystemExit(f"delay op {o:02X} word={word:08X}")

            def do_delay():
                if delay is not None:
                    run(delay)

            if op == 0x00 and func == 0x08:          # jr
                do_delay()
                return regs[2]
            elif op == 0x00 and w == 0:              # nop
                pc += 1
                continue
            elif op == 0x00:                         # SPECIAL
                if func == 0x21:
                    regs[rd] = u32(regs[rs] + regs[rt])
                elif func == 0x23:
                    regs[rd] = u32(regs[rs] - regs[rt])
                elif func == 0x2B:
                    regs[rd] = 1 if u32(regs[rs]) < u32(regs[rt]) else 0
                elif func == 0x00:
                    regs[rd] = u32(regs[rt] << sa)
                else:
                    raise SystemExit(f"unimpl SPECIAL {func:02X} @{ins_addr:08X}")
            elif op == 0x02:                         # j
                target = (addr_field << 2) | (ins_addr & 0xF0000000)
                do_delay()
                pc = (target - FUNC_ADDR) // 4
                continue
            elif op == 0x03:                         # jal
                target = (addr_field << 2) | (ins_addr & 0xF0000000)
                do_delay()
                regs[2] = callee(target)
                pc += 2                              # slot already executed
                continue
            elif op == 0x04:                         # beq / beqz
                take = regs[rs] == regs[rt]          # sampled at issue
                do_delay()
                if take:
                    pc = ((ins_addr + 4 + (simm << 2)) - FUNC_ADDR) // 4
                    continue
                pc += 2                              # slot already executed
                continue
            elif op == 0x05:                         # bne / bnez
                take = regs[rs] != regs[rt]
                do_delay()
                if take:
                    pc = ((ins_addr + 4 + (simm << 2)) - FUNC_ADDR) // 4
                    continue
                pc += 2                              # slot already executed
                continue
            elif op == 0x08 or op == 0x09:           # addi/addiu
                regs[rt] = u32(regs[rs] + simm)
            elif op == 0x0C:                         # andi
                regs[rt] = u32(regs[rs] & imm)
            elif op == 0x0D:                         # ori
                regs[rt] = u32(regs[rs] | imm)
            elif op == 0x0F:                         # lui
                regs[rt] = imm << 16
            elif op == 0x23:
                regs[rt] = u32(self.ld32(u32(regs[rs] + simm)))
            elif op == 0x24:
                regs[rt] = self.ld8(u32(regs[rs] + simm))
            elif op == 0x25:
                regs[rt] = self.ld16(u32(regs[rs] + simm))
            elif op == 0x28:
                self.st8(u32(regs[rs] + simm), regs[rt])
            elif op == 0x29:
                self.st16(u32(regs[rs] + simm), regs[rt])
            elif op == 0x2B:
                self.st32(u32(regs[rs] + simm), regs[rt])
            else:
                raise SystemExit(f"unimpl op {op:02X} @{ins_addr:08X}")
            pc += 1
        raise SystemExit("fell off the end without jr $ra")


def expected_writes(copy1_bytes):
    """ROM-order expected write list, parameterized by the bytes the
    first string copy transfers (controlled func_8005DC4C return)."""
    w = []
    # prologue frame stores (guest stack): sw $s0, sw $ra
    w.append((SP_S0, 4, 0))
    w.append((SP_RA, 4, RA_SENTINEL))
    # bzero(0x800C0DE0, 0x12E4), ascending
    for i in range(BZERO_LEN):
        w.append((BUF_BASE + i, 1, 0))
    # block 1 stores: flag, C8, C0, C4
    w.append((GA_FLAG, 4, 1))
    w.append((GA_C8, 4, 0))
    w.append((GA_C0, 4, BUF_SEL))
    w.append((GA_C4, 4, 8))
    # fill #1: 8 x 0xFF
    for i in range(8):
        w.append((BUF_SEL + i, 1, 0xFF))
    # string copy #1 (controlled content)
    for i, b in enumerate(copy1_bytes):
        w.append((BUF_SEL + i, 1, b))
    # block 2 stores: flag, C4, C8, C0 (retail order differs from block 1)
    w.append((GA_FLAG, 4, 1))
    w.append((GA_C4, 4, 8))
    w.append((GA_C8, 4, 0))
    w.append((GA_C0, 4, BUF_SEL))
    # fill #2: re-fill overwrites copy #1
    for i in range(8):
        w.append((BUF_SEL + i, 1, 0xFF))
    # string copy #2: degenerate source -> only the 0xFF terminator
    w.append((BUF_SEL, 1, 0xFF))
    # direct stores
    w.append((0x800C1F80, 2, 0x0203))
    w.append((0x800C0E44, 4, 0x00404040))
    w.append((0x800A76A4, 4, 0))
    w.append((0x800A76B0, 4, 0))
    w.append((0x800A76BC, 4, 0))
    w.append((0x800A76C8, 4, 0))
    w.append((0x800C20A4, 1, 0xFF))
    w.append((0x800C20B4, 1, 0xFF))
    return w


def expected_reads(copy1_src, copy1_len):
    """ROM-order expected read list."""
    r = []
    for _block in range(2):
        for _i in range(8):            # fill reloads C0/C4 each iteration
            r.append((GA_C0, 4, BUF_SEL))
            r.append((GA_C4, 4, 8))
        r.append((GA_C8, 4, 0))        # selection reloads
        r.append((GA_C0, 4, BUF_SEL))
        for k in range(copy1_len if _block == 0 else 1):
            # copy source byte reads
            if _block == 0:
                r.append((copy1_src + k, 1, None))
            else:
                r.append((BUF_SEL + k, 1, 0xFF))
    # epilogue frame restores: lw $ra, lw $s0
    r.append((SP_RA, 4, RA_SENTINEL))
    r.append((SP_S0, 4, 0))
    return r


def check(name, path, controlled, seeds, copy1_bytes, copy1_src,
          exp_calls, ret):
    m = Machine(path)
    for addr, width, val in seeds:
        if width == 4:
            struct.pack_into("<I", m.ram, addr & 0x1FFFFF, val)
        elif width == 2:
            struct.pack_into("<H", m.ram, addr & 0x1FFFFF, val)
        else:
            m.ram[addr & 0x1FFFFF] = val
    m.reads.clear()
    m.writes.clear()
    m.calls.clear()
    got_ret = m.call(controlled)
    ok = True

    exp_w = expected_writes(copy1_bytes)
    if m.writes != exp_w:
        print(f"  FAIL[{name}] write log length got {len(m.writes)} "
              f"want {len(exp_w)}")
        for i, (g, e) in enumerate(zip(m.writes, exp_w)):
            if g != e:
                print(f"    first divergence at index {i}: "
                      f"got {g}, want {e}")
                break
        ok = False

    exp_r = expected_reads(copy1_src, len(copy1_bytes))
    if len(m.reads) != len(exp_r):
        print(f"  FAIL[{name}] read log length got {len(m.reads)} "
              f"want {len(exp_r)}")
        ok = False
    else:
        for i, (g, e) in enumerate(zip(m.reads, exp_r)):
            ga, gw, gv = g
            ea, ew, ev = e
            if ga != ea or gw != ew or (ev is not None and gv != ev):
                print(f"  FAIL[{name}] read divergence at index {i}: "
                      f"got {g}, want {e}")
                ok = False
                break

    if m.calls != exp_calls:
        print(f"  FAIL[{name}] call log:")
        print(f"    got  {[(hex(a), hex(x), hex(y)) for a, x, y in m.calls]}")
        print(f"    want {[(hex(a), hex(x), hex(y)) for a, x, y in exp_calls]}")
        ok = False

    if got_ret != ret:
        print(f"  FAIL[{name}] return {got_ret:#x}, want {ret:#x}")
        ok = False

    # Final guest state (both scenarios converge here).
    checks = [
        (GA_FLAG, 4, 1), (GA_C8, 4, 0), (GA_C0, 4, BUF_SEL), (GA_C4, 4, 8),
        (0x800C1F80, 2, 0x0203), (0x800C0E44, 4, 0x00404040),
        (0x800A76A4, 4, 0), (0x800A76B0, 4, 0),
        (0x800A76BC, 4, 0), (0x800A76C8, 4, 0),
    ]
    for addr, width, want in checks:
        got = (struct.unpack_from("<H", m.ram, addr & 0x1FFFFF)[0]
               if width == 2 else struct.unpack_from("<I", m.ram, addr & 0x1FFFFF)[0])
        if got != want:
            print(f"  FAIL[{name}] final guest {addr:08X} = {got:X}, "
                  f"want {want:X}")
            ok = False
    for i in range(8):
        if m.ram[(BUF_SEL + i) & 0x1FFFFF] != 0xFF:
            print(f"  FAIL[{name}] buffer byte {i} = "
                  f"{m.ram[(BUF_SEL + i) & 0x1FFFFF]:02X}")
            ok = False
    if (m.ram[BUF_BASE & 0x1FFFFF] != 0
            or m.ram[(BUF_BASE + BZERO_LEN - 1) & 0x1FFFFF] != 0):
        print(f"  FAIL[{name}] bzero endpoints not zero")
        ok = False
    if (m.ram[0x800C20A4 & 0x1FFFFF] != 0xFF
            or m.ram[0x800C20B4 & 0x1FFFFF] != 0xFF):
        print(f"  FAIL[{name}] terminator bytes missing")
        ok = False

    if ok:
        print(f"  PASS[{name}] ret=0x{got_ret:X} writes={len(m.writes)} "
              f"reads={len(m.reads)} calls={len(m.calls)} (ROM order exact)")
    return ok


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: b25_oracle.py /path/to/disc1.candidate.exe")
    path = sys.argv[1]
    ok = True

    # S1 — boot path: every func_8005DC4C return is the degenerate
    # self-copy source (0x800C0DF0), so each string copy transfers only
    # the 0xFF terminator the fill loop just wrote.  All other boundary
    # callees return 0 (their returns are unconsumed, or consumed only
    # as 8-0 arithmetic for the next argument).
    #
    # Retail argument registers at the calls (proven by execution):
    # the three func_8005DC4C calls carry a1 = 0xFF — the residual
    # fill-value register; retail sets $a1 = D_8009D0C0 only AFTER each
    # call returns (addu $a1,$s0 at 0x8005D794/0x8005D854, not in a
    # delay slot).  The third call's a1 is the post-copy-#2 cursor.
    # func_80052594's a0 comes from the jal func_80052594 delay slot
    # (addu $a0,$v0 — the third DC4C return), and func_8005CCA4
    # inherits that same a0 (the 52594 return lands in $v0 only).
    dc4c = 0x8005DC4C
    s1_ctrl = {(dc4c, 0): BUF_SEL, (dc4c, 1): BUF_SEL, (dc4c, 2): BUF_SEL}
    s1_calls = [
        (0x80071A24, BUF_BASE, BZERO_LEN),
        (dc4c, 0x1E, 0xFF),
        (dc4c, 0x1E, 0xFF),
        (dc4c, 0x1E, BUF_SEL + 1),
        (0x80052594, BUF_SEL, BUF_SEL + 1),
        (0x8005CCA4, BUF_SEL, BUF_SEL + 1),
        (0x800614AC, 0x00404040, BUF_SEL + 1),
        (0x8005E884, 0x00404040, BUF_SEL + 1),
        (0x8005E850, 0, 8),
        (0x800649D0, 0, 8),
        (0x80052790, 1, 8),
    ]
    ok &= check("boot path, degenerate returns", path, s1_ctrl, [],
                [0xFF], BUF_SEL, s1_calls, 0xFF)

    # S2 — controlled dependency returns: the first func_8005DC4C return
    # points at a seeded 4-byte string ('A','B','C',0xFF).  Copy #1 must
    # transfer exactly those 4 bytes; block 2 then re-fills the buffer
    # (partial-state behavior: the copied content is visible in the
    # write log before the re-fill).  Copy #2 is degenerate again, so
    # the third call's cursor is BUF_SEL + 1 (copy #2's length), while
    # the 4-byte copy #1 remains visible in the write log.  The seed
    # address must lie OUTSIDE the leading bzero range (ends at
    # 0x800C20C4) — retail zeroes the seed region first, same as on
    # hardware.
    src2 = 0x800C2100
    s2_ctrl = {(dc4c, 0): src2, (dc4c, 1): BUF_SEL, (dc4c, 2): BUF_SEL}
    s2_seeds = [
        (src2 + 0, 1, 0x41), (src2 + 1, 1, 0x42),
        (src2 + 2, 1, 0x43), (src2 + 3, 1, 0xFF),
        (src2 + 4, 1, 0xFF),
    ]
    s2_calls = [
        (0x80071A24, BUF_BASE, BZERO_LEN),
        (dc4c, 0x1E, 0xFF),
        (dc4c, 0x1E, 0xFF),
        (dc4c, 0x1E, BUF_SEL + 1),
        (0x80052594, BUF_SEL, BUF_SEL + 1),
        (0x8005CCA4, BUF_SEL, BUF_SEL + 1),
        (0x800614AC, 0x00404040, BUF_SEL + 1),
        (0x8005E884, 0x00404040, BUF_SEL + 1),
        (0x8005E850, 0, 8),
        (0x800649D0, 0, 8),
        (0x80052790, 1, 8),
    ]
    ok &= check("controlled 4-byte string return", path, s2_ctrl, s2_seeds,
                [0x41, 0x42, 0x43, 0xFF], src2, s2_calls, 0xFF)

    if ok:
        print("B25 ORACLE: PASS")
        sys.exit(0)
    print("B25 ORACLE: FAIL")
    sys.exit(1)


if __name__ == "__main__":
    main()

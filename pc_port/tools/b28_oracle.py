#!/usr/bin/env python3
"""
Phase 6E-B28 independent oracle for func_8005DB8C, func_8005DBAC,
func_800438C0, and func_8005CCA4.

Loads the verified retail executable (PS-X EXE, SHA-1 checked, supplied
at runtime), cross-checks every modeled instruction word against it, then
EXECUTES those words with a delay-slot-aware MIPS-I interpreter.

This oracle does NOT call or depend on the production C port.

Usage:
  b28_oracle.py /path/to/disc1.candidate.exe
"""
import hashlib
import struct
import sys

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

RAM_BASE = 0x80000000
RAM_SIZE = 0x00200000
RAM_END = RAM_BASE + RAM_SIZE

GP = 0x8009CD70
SP_INIT = 0x801FFF00
RA_SENTINEL = 0xDEAD0000


# ── Transcribed instruction words ──────────────────────────────────────
# Every word is re-verified against the SHA-checked exe at load time.

# func_8005DB8C: 8 words (table base computation)
# pe_addr_t func_8005DB8C(int idx)
# return *D_800A8038 + (idx << 9) + (D_800A8038 - 0x10)
FUNC_5DB8C = 0x8005DB8C
N_5DB8C = 8
W_5DB8C = [
    0x3C02800B,  # 8005DB8C lui   $v0, 0x800B
    0x24428038,  # 8005DB90 addiu $v0, $v0, 0x8038   → v0 = 0x800A8038
    0x00042240,  # 8005DB94 sll   $a0, $a0, 9         idx << 9
    0x2443FFF0,  # 8005DB98 addiu $v1, $v0, -16       v1 = 0x800A8028
    0x8C420000,  # 8005DB9C lw    $v0, 0($v0)         *D_800A8038
    0x00832021,  # 8005DBA0 addu  $a0, $a0, $v1        (idx<<9) + 0x800A8028
    0x03E00008,  # 8005DBA4 jr    $ra
    0x00441021,  # 8005DBA8 addu  $v0, $v0, $a0        (delay: *ptr + const)
]

# func_8005DBAC: 20 words (clamped table base computation)
# pe_addr_t func_8005DBAC(int arg)
# clamped = arg (negative: no clamp!), clamp(arg, 0, 98) for non-negative
# return *D_800A803C + (clamped * 24) + (D_800A803C - 0x14)
FUNC_5DBAC = 0x8005DBAC
N_5DBAC = 20
W_5DBAC = [
    0x00801821,  # 8005DBAC addu  $v1, $a0, $zero    v1 = arg
    0x04610003,  # 8005DBB0 bgez  $v1, +3 → 0x8005DBC0
    0x00000000,  # 8005DBB4 nop (delay)
    0x080176F4,  # 8005DBB8 j     0x8005DBD0         (neg path: skip clamp)
    0x00001821,  # 8005DBBC addu  $v1, $zero, $zero  (delay: v1 = 0)
    0x28620063,  # 8005DBC0 slti  $v0, $v1, 99
    0x14400002,  # 8005DBC4 bne   $v0, $zero, +2 → 0x8005DBD0
    0x00000000,  # 8005DBC8 nop (delay)
    0x24030062,  # 8005DBCC addiu $v1, $zero, 98     clamp to 98
    0x3C04800B,  # 8005DBD0 lui   $a0, 0x800B
    0x2484803C,  # 8005DBD4 addiu $a0, $a0, 0x803C   → 0x800A803C
    0x00031040,  # 8005DBD8 sll   $v0, $v1, 1        clamped * 2
    0x00431021,  # 8005DBDC addu  $v0, $v0, $v1      clamped * 3
    0x000210C0,  # 8005DBE0 sll   $v0, $v0, 3        clamped * 24
    0x2483FFEC,  # 8005DBE4 addiu $v1, $a0, -20      → 0x800A8028
    0x8C840000,  # 8005DBE8 lw    $a0, 0($a0)        *D_800A803C
    0x00431021,  # 8005DBEC addu  $v0, $v0, $v1      offset + base
    0x03E00008,  # 8005DBF0 jr    $ra
    0x00821021,  # 8005DBF4 addu  $v0, $a0, $v0      (delay: *ptr + offset)
    0x3C02800B,  # 8005DBF8 lui   $v0, 0x800B        (past jr — func_8005DBAC ends)
]

# func_800438C0: 8 words (masked-state setter via $gp-relative store)
# int func_800438C0(int arg0)
# m = arg0 & 0x1FF; D_8009CEF0 = m; if m != 0: return 0; D_8009CEF0 = 0x1FF; return 0
FUNC_438C0 = 0x800438C0
N_438C0 = 8
W_438C0 = [
    0x308401FF,  # 800438C0 andi  $a0, $a0, 0x1FF
    0xAF840180,  # 800438C4 sw    $a0, 0x180($gp)    D_8009CEF0 = m
    0x14800003,  # 800438C8 bne   $a0, $zero, +3 → 0x800438D8
    0x00000000,  # 800438CC nop (delay)
    0x240201FF,  # 800438D0 addiu $v0, $zero, 0x1FF
    0xAF820180,  # 800438D4 sw    $v0, 0x180($gp)    D_8009CEF0 = 0x1FF
    0x03E00008,  # 800438D8 jr    $ra
    0x00001021,  # 800438DC addu  $v0, $zero, $zero   (delay: return 0)
]


def u32(x):
    return x & 0xFFFFFFFF

def s32(x):
    x &= 0xFFFFFFFF
    return x - 0x100000000 if x & 0x80000000 else x


class Machine:
    """Delay-slot-aware MIPS-I interpreter with guest memory logging."""

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
            # Cross-check all modeled words
            for label, func_addr, n_words, words in [
                ("func_8005DB8C", FUNC_5DB8C, N_5DB8C, W_5DB8C),
                ("func_8005DBAC", FUNC_5DBAC, N_5DBAC, W_5DBAC),
                ("func_800438C0", FUNC_438C0, N_438C0, W_438C0),
                ("func_8005CCA4", FUNC_5CCA4, N_5CCA4, W_5CCA4),
            ]:
                for i, exp in enumerate(words):
                    a = func_addr + i * 4
                    got = struct.unpack_from("<I", data, a - taddr + 0x800)[0]
                    if got != exp:
                        raise SystemExit(
                            f"FATAL: {label} word {i} @0x{a:08X}: "
                            f"exe {got:08X} != transcribed {exp:08X}")
                print(f"  {label}: {n_words} words cross-checked "
                      f"@0x{func_addr:08X}")
            print(f"exe SHA-1 {sha1} OK; all words verified")
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

    # ── one instruction ───────────────────────────────────────────────
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
        elif op == 0x0A:
            r[rt] = 1 if s32(r[rs]) < s32(simm) else 0
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
                    pc += 2
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
        return self.regs[2]


def test_5DB8c(m):
    """Test func_8005DB8C: *D_800A8038 + (idx << 9) + (D_800A8038 - 0x10)"""
    print("\n=== func_8005DB8C ===")
    # Test 1: null pointer (BSS state)
    m.reset()
    assert m.load(0x800A8038, 4) == 0, "precondition: D_800A8038 is 0"
    m.regs[4] = 0  # $a0 = idx = 0
    ret = m.exec_words(FUNC_5DB8C, W_5DB8C)
    expected = 0 + (0 << 9) + (0x800A8038 - 0x10)
    assert ret == expected, f"idx=0: got 0x{ret:08X}, want 0x{expected:08X}"
    print(f"  idx=0: return 0x{ret:08X} OK")

    m.regs[4] = 3
    ret = m.exec_words(FUNC_5DB8C, W_5DB8C)
    expected = 0 + (3 << 9) + (0x800A8038 - 0x10)
    assert ret == expected, f"idx=3: got 0x{ret:08X}, want 0x{expected:08X}"
    print(f"  idx=3: return 0x{ret:08X} OK")

    # Test 2: with pointer
    m.reset()
    fake_base = 0x800C2000
    m.poke(0x800A8038, 4, fake_base)
    m.regs[4] = 0
    ret = m.exec_words(FUNC_5DB8C, W_5DB8C)
    expected = u32(fake_base + (0 << 9) + (0x800A8038 - 0x10))
    assert ret == expected, f"ptr idx=0: got 0x{ret:08X}, want 0x{expected:08X}"
    print(f"  ptr idx=0: return 0x{ret:08X} OK")

    m.regs[4] = 5
    ret = m.exec_words(FUNC_5DB8C, W_5DB8C)
    expected = u32(fake_base + (5 << 9) + (0x800A8038 - 0x10))
    assert ret == expected, f"ptr idx=5: got 0x{ret:08X}, want 0x{expected:08X}"
    print(f"  ptr idx=5: return 0x{ret:08X} OK")

    # Test 3: no guest writes
    m.reset()
    for a in range(RAM_BASE, RAM_BASE + 0x100, 4):
        m.poke(a, 4, 0xA5A5A5A5)
    m.regs[4] = 3
    m.exec_words(FUNC_5DB8C, W_5DB8C)
    assert len(m.writes) == 0, f"func_8005DB8C made {len(m.writes)} writes"
    print(f"  no guest writes: OK")

    print("func_8005DB8C: ALL PASS")


def test_5DBAC(m):
    """Test func_8005DBAC: clamped table base computation.

    Retail behavior (verified from raw MIPS):
      negative arg → clamped to 0 via j delay slot (addu $v1,$zero,$zero)
      0..98 → kept as-is
      ≥99 → clamped to 98

    Note: the C port clamps negative to 0 explicitly (bgez path).
    The retail MIPS uses the j delay slot to zero $v1 for negatives.
    Both produce the same result.
    """
    print("\n=== func_8005DBAC ===")
    # Test 1: null pointer, arg=0
    m.reset()
    assert m.load(0x800A803C, 4) == 0, "precondition: D_800A803C is 0"
    m.regs[4] = 0
    ret = m.exec_words(FUNC_5DBAC, W_5DBAC)
    expected = 0 + (0 * 24) + (0x800A803C - 0x14)
    assert ret == expected, f"arg=0: got 0x{ret:08X}, want 0x{expected:08X}"
    print(f"  arg=0: return 0x{ret:08X} OK")

    # Test 2: arg=5
    m.regs[4] = 5
    ret = m.exec_words(FUNC_5DBAC, W_5DBAC)
    expected = 0 + (5 * 24) + (0x800A803C - 0x14)
    assert ret == expected, f"arg=5: got 0x{ret:08X}, want 0x{expected:08X}"
    print(f"  arg=5: return 0x{ret:08X} OK")

    # Test 3: clamp negative to 0 (retail: j delay slot zeroes $v1)
    m.reset()
    m.regs[4] = u32(-1)
    ret_neg = m.exec_words(FUNC_5DBAC, W_5DBAC)
    m.regs[4] = 0
    ret_zero = m.exec_words(FUNC_5DBAC, W_5DBAC)
    assert ret_neg == ret_zero, \
        f"negative not clamped: neg=0x{ret_neg:08X} zero=0x{ret_zero:08X}"
    print(f"  negative clamp: OK (0x{ret_neg:08X} == 0x{ret_zero:08X})")

    # Test 4: clamp >=99 to 98
    m.reset()
    m.regs[4] = 99
    ret_99 = m.exec_words(FUNC_5DBAC, W_5DBAC)
    m.regs[4] = 98
    ret_98 = m.exec_words(FUNC_5DBAC, W_5DBAC)
    assert ret_99 == ret_98, "99 not clamped to 98"
    print(f"  high clamp: OK")

    # Test 5: 97 != 98
    m.reset()
    m.regs[4] = 97
    ret_97 = m.exec_words(FUNC_5DBAC, W_5DBAC)
    m.regs[4] = 98
    ret_98 = m.exec_words(FUNC_5DBAC, W_5DBAC)
    assert ret_97 != ret_98, "97 and 98 should differ"
    print(f"  boundary 97/98: OK")

    # Test 6: malformed header (0xA49D968F at D_800A803C)
    m.reset()
    m.poke(0x800A803C, 4, 0xA49D968F)
    m.regs[4] = 0
    ret = m.exec_words(FUNC_5DBAC, W_5DBAC)
    # 0xA49D968F + 0 + (0x800A803C - 0x14) = 0xA49D968F + 0x800A8028
    # = 0x124A816B7 → mod 2^32 = 0x24A816B7
    assert ret == 0x24A816B7, \
        f"malformed: got 0x{ret:08X}, want 0x24A816B7"
    print(f"  malformed header: return 0x{ret:08X} OK (NOT dereferenced)")

    # Test 7: no guest writes
    m.reset()
    m.regs[4] = 7
    m.exec_words(FUNC_5DBAC, W_5DBAC)
    assert len(m.writes) == 0, f"func_8005DBAC made {len(m.writes)} writes"
    print(f"  no guest writes: OK")

    print("func_8005DBAC: ALL PASS")


def test_438C0(m):
    """Test func_800438C0: masked-state setter."""
    print("\n=== func_800438C0 ===")
    STATE = 0x8009CEF0

    # Test 1: basic store
    m.reset()
    m.regs[4] = 0x3D
    ret = m.exec_words(FUNC_438C0, W_438C0)
    assert ret == 0, f"return not 0: {ret}"
    assert m.load(STATE, 4) == 0x3D, f"state not 0x3D: {m.load(STATE, 4):08X}"
    print(f"  0x3D: state=0x{m.load(STATE, 4):08X} OK")

    # Test 2: mask
    m.regs[4] = 0x1234
    ret = m.exec_words(FUNC_438C0, W_438C0)
    assert m.load(STATE, 4) == 0x34, f"mask wrong: {m.load(STATE, 4):08X}"
    print(f"  0x1234 mask: state=0x{m.load(STATE, 4):08X} OK")

    # Test 3: zero → 0x1FF
    m.regs[4] = 0
    ret = m.exec_words(FUNC_438C0, W_438C0)
    assert m.load(STATE, 4) == 0x1FF, f"zero→0x1FF wrong: {m.load(STATE, 4):08X}"
    print(f"  0→0x1FF: state=0x{m.load(STATE, 4):08X} OK")

    # Test 4: 0x200 → 0 → 0x1FF
    m.regs[4] = 0x200
    ret = m.exec_words(FUNC_438C0, W_438C0)
    assert m.load(STATE, 4) == 0x1FF, f"0x200→0x1FF wrong: {m.load(STATE, 4):08X}"
    print(f"  0x200→0x1FF: state=0x{m.load(STATE, 4):08X} OK")

    # Test 5: full RAM canary
    m.reset()
    for a in range(RAM_BASE, RAM_END, 4):
        m.poke(a, 4, 0xA5A5A5A5)
    m.regs[4] = 0x42
    m.exec_words(FUNC_438C0, W_438C0)
    for a, sz, v in m.writes:
        if a != STATE:
            raise SystemExit(f"unexpected write to 0x{a:08X}")
    assert m.load(STATE, 4) == 0x42
    print(f"  footprint: only STATE written, value 0x42 OK")

    print("func_800438C0: ALL PASS")


# ── func_8005CCA4 full-function verification ───────────────────────────
# func_8005CCA4 is 223 words (0x8005CCA4..0x8005D01F), independently
# transcribed from the SHA-verified retail executable.  Every word is
# cross-checked against the exe at load time (no runtime generation).
# The transcribed words are then EXECUTED by a delay-slot-aware MIPS-I
# interpreter.  Leaf callees (func_8005DB8C/DBAC/438C0/52F70/51E58)
# execute from the exe too; only the genuine unresolved boundary funcs
# are stubbed (no-op → 0), matching the C port's Bootstrap_ReturnVoid
# boundary.

FUNC_5CCA4 = 0x8005CCA4
N_5CCA4 = 223
W_5CCA4 = [
    0x27BDFFD8,  # 0x8005CCA4  addiu $sp, $sp, -0x28
    0xAFB10014,  # 0x8005CCA8  sw    $s1, 0x14($sp)
    0x3C11800C,  # 0x8005CCAC  lui   $s1, 0x800C
    0x26310E28,  # 0x8005CCB0  addiu $s1, $s1, 0x0E28
    0xAFB00010,  # 0x8005CCB4  sw    $s0, 0x10($sp)
    0x00008021,  # 0x8005CCB8  addu  $s0, $zero, $zero
    0xAFBF0020,  # 0x8005CCBC  sw    $ra, 0x20($sp)
    0xAFB3001C,  # 0x8005CCC0  sw    $s3, 0x1C($sp)
    0xAFB20018,  # 0x8005CCC4  sw    $s2, 0x18($sp)
    0x0C0176E3,  # 0x8005CCC8  jal   func_8005DB8C
    0x02002021,  # 0x8005CCCC  addu  $a0, $s0, $zero
    0x94420000,  # 0x8005CCD0  lhu   $v0, 0($v0)
    0x26100001,  # 0x8005CCD4  addiu $s0, $s0, 1
    0xA6220000,  # 0x8005CCD8  sh    $v0, 0($s1)
    0x2A020007,  # 0x8005CCDC  slti  $v0, $s0, 7
    0x1440FFF9,  # 0x8005CCE0  bne   $v0, $zero, loop
    0x26310002,  # 0x8005CCE4  addiu $s1, $s1, 2
    0x0C0176EB,  # 0x8005CCE8  jal   func_8005DBAC
    0x00002021,  # 0x8005CCEC  addu  $a0, $zero, $zero
    0x00408021,  # 0x8005CCF0  addu  $s0, $v0, $zero
    0x3C03800C,  # 0x8005CCF4  lui   $v1, 0x800C
    0x24630E06,  # 0x8005CCF8  addiu $v1, $v1, 0x0E06
    0x96020000,  # 0x8005CCFC  lhu   $v0, 0($s0)
    0x24710042,  # 0x8005CD00  addiu $s1, $v1, 0x42
    0x3C01800C,  # 0x8005CD04  lui   $at, 0x800C
    0xA4220E08,  # 0x8005CD08  sh    $v0, 0x0E08($at)
    0xA4620000,  # 0x8005CD0C  sh    $v0, 0($v1)
    0x92040007,  # 0x8005CD10  lbu   $a0, 7($s0)
    0x24020001,  # 0x8005CD14  addiu $v0, $zero, 1
    0x3C01800C,  # 0x8005CD18  lui   $at, 0x800C
    0xAC220E24,  # 0x8005CD1C  sw    $v0, 0x0E24($at)
    0x3C01800A,  # 0x8005CD20  lui   $at, 0x800A
    0xA4201EAE,  # 0x8005CD24  sh    $zero, 0x1EAE($at)
    0x3C01800A,  # 0x8005CD28  lui   $at, 0x800A
    0xA4201E8E,  # 0x8005CD2C  sh    $zero, 0x1E8E($at)
    0x3C01800A,  # 0x8005CD30  lui   $at, 0x800A
    0xA4201E6E,  # 0x8005CD34  sh    $zero, 0x1E6E($at)
    0xAF9102D8,  # 0x8005CD38  sw    $s1, 0x02D8($gp)
    0x3C01800C,  # 0x8005CD3C  lui   $at, 0x800C
    0xA0240E0C,  # 0x8005CD40  sb    $a0, 0x0E0C($at)
    0x0C014BDC,  # 0x8005CD44  jal   func_80052F70
    0x24120002,  # 0x8005CD48  addiu $s2, $zero, 2
    0x3C13800A,  # 0x8005CD4C  lui   $s3, 0x800A
    0x2673D05C,  # 0x8005CD50  addiu $s3, $s3, 0xD05C
    0xAF8202E0,  # 0x8005CD54  sw    $v0, 0x02E0($gp)
    0xAF9302E8,  # 0x8005CD58  sw    $s3, 0x02E8($gp)
    0xAF9202F4,  # 0x8005CD5C  sw    $s2, 0x02F4($gp)
    0x92030007,  # 0x8005CD60  lbu   $v1, 7($s0)
    0x00000000,  # 0x8005CD64  nop
    0x28620033,  # 0x8005CD68  slti  $v0, $v1, 0x33
    0x14400002,  # 0x8005CD6C  bne   $v0, $zero, +8
    0x00000000,  # 0x8005CD70  nop
    0x24030032,  # 0x8005CD74  addiu $v1, $zero, 0x32
    0x8F8202D8,  # 0x8005CD78  lw    $v0, 0x02D8($gp)
    0x3C01800C,  # 0x8005CD7C  lui   $at, 0x800C
    0xA0230E0C,  # 0x8005CD80  sb    $v1, 0x0E0C($at)
    0x14510004,  # 0x8005CD84  bne   $v0, $s1, +16
    0x00000000,  # 0x8005CD88  nop
    0x0C014BDC,  # 0x8005CD8C  jal   func_80052F70
    0x00000000,  # 0x8005CD90  nop
    0xAF8202E0,  # 0x8005CD94  sw    $v0, 0x02E0($gp)
    0xAF9102D8,  # 0x8005CD98  sw    $s1, 0x02D8($gp)
    0x0C014BDC,  # 0x8005CD9C  jal   func_80052F70
    0x24100031,  # 0x8005CDA0  addiu $s0, $zero, 0x31
    0x8F8302D8,  # 0x8005CDA4  lw    $v1, 0x02D8($gp)
    0xAF8202E0,  # 0x8005CDA8  sw    $v0, 0x02E0($gp)
    0xAF9302E8,  # 0x8005CDAC  sw    $s3, 0x02E8($gp)
    0xAF9202F4,  # 0x8005CDB0  sw    $s2, 0x02F4($gp)
    0x24630062,  # 0x8005CDB4  addiu $v1, $v1, 0x62
    0xA4600000,  # 0x8005CDB8  sh    $zero, 0($v1)
    0x2610FFFF,  # 0x8005CDBC  addiu $s0, $s0, -1
    0x0601FFFD,  # 0x8005CDC0  bgez  $s0, loop
    0x2463FFFE,  # 0x8005CDC4  addiu $v1, $v1, -2
    0x0C014F4B,  # 0x8005CDC8  jal   func_80053D2C
    0x24040044,  # 0x8005CDCC  addiu $a0, $zero, 0x44
    0x0C014F4B,  # 0x8005CDD0  jal   func_80053D2C
    0x24040096,  # 0x8005CDD4  addiu $a0, $zero, 0x96
    0x0C014F4B,  # 0x8005CDD8  jal   func_80053D2C
    0x2404003F,  # 0x8005CDDC  addiu $a0, $zero, 0x3F
    0x0C014F4B,  # 0x8005CDE0  jal   func_80053D2C
    0x24040001,  # 0x8005CDE4  addiu $a0, $zero, 1
    0x0C014F4B,  # 0x8005CDE8  jal   func_80053D2C
    0x24040006,  # 0x8005CDEC  addiu $a0, $zero, 6
    0x3C05800C,  # 0x8005CDF0  lui   $a1, 0x800C
    0x24A50EAC,  # 0x8005CDF4  addiu $a1, $a1, 0x0EAC
    0x24A31000,  # 0x8005CDF8  addiu $v1, $a1, 0x1000
    0x00A3102B,  # 0x8005CDFC  sltu  $v0, $a1, $v1
    0x10400030,  # 0x8005CE00  beq   $v0, $zero, exit1
    0x00002021,  # 0x8005CE04  addu  $a0, $zero, $zero
    0x24070009,  # 0x8005CE08  addiu $a3, $zero, 9
    0x00603021,  # 0x8005CE0C  addu  $a2, $v1, $zero
    0x24A30005,  # 0x8005CE10  addiu $v1, $a1, 5
    0x90A20000,  # 0x8005CE14  lbu   $v0, 0($a1)
    0x00000000,  # 0x8005CE18  nop
    0x1040000A,  # 0x8005CE1C  beq   $v0, $zero, skip1
    0x00000000,  # 0x8005CE20  nop
    0x90620001,  # 0x8005CE24  lbu   $v0, 1($v1)
    0x00000000,  # 0x8005CE28  nop
    0x10470006,  # 0x8005CE2C  beq   $v0, $a3, found1
    0x00000000,  # 0x8005CE30  nop
    0x90620000,  # 0x8005CE34  lbu   $v0, 0($v1)
    0x00000000,  # 0x8005CE38  nop
    0x30420010,  # 0x8005CE3C  andi  $v0, $v0, 0x10
    0x14400005,  # 0x8005CE40  bne   $v0, $zero, break1
    0x00000000,  # 0x8005CE44  nop
    0x24A50020,  # 0x8005CE48  addiu $a1, $a1, 0x20
    0x00A6102B,  # 0x8005CE4C  sltu  $v0, $a1, $a2
    0x1440FFF0,  # 0x8005CE50  bne   $v0, $zero, loop1
    0x24630020,  # 0x8005CE54  addiu $v1, $v1, 0x20
    0x3C06800C,  # 0x8005CE58  lui   $a2, 0x800C
    0x24C61EAC,  # 0x8005CE5C  addiu $a2, $a2, 0x1EAC
    0x00A6102B,  # 0x8005CE60  sltu  $v0, $a1, $a2
    0x10400017,  # 0x8005CE64  beq   $v0, $zero, exit1
    0x24C2F000,  # 0x8005CE68  addiu $v0, $a2, -0x1000
    0x00A21023,  # 0x8005CE6C  subu  $v0, $a1, $v0
    0x00021143,  # 0x8005CE70  sra   $v0, $v0, 5
    0x24440100,  # 0x8005CE74  addiu $a0, $v0, 0x100
    0x24C300D4,  # 0x8005CE78  addiu $v1, $a2, 0xD4
    0x24C50178,  # 0x8005CE7C  addiu $a1, $a2, 0x178
    0x0065102B,  # 0x8005CE80  sltu  $v0, $v1, $a1
    0x1040000F,  # 0x8005CE84  beq   $v0, $zero, found1
    0x00000000,  # 0x8005CE88  nop
    0x84620000,  # 0x8005CE8C  lh    $v0, 0($v1)
    0x00000000,  # 0x8005CE90  nop
    0x10440005,  # 0x8005CE94  beq   $v0, $a0, clear1
    0x00000000,  # 0x8005CE98  nop
    0x24630002,  # 0x8005CE9C  addiu $v1, $v1, 2
    0x0065102B,  # 0x8005CEA0  sltu  $v0, $v1, $a1
    0x1440FFF9,  # 0x8005CEA4  bne   $v0, $zero, inner1
    0x00000000,  # 0x8005CEA8  nop
    0x3C02800C,  # 0x8005CEAC  lui   $v0, 0x800C
    0x24422024,  # 0x8005CEB0  addiu $v0, $v0, 0x2024
    0x0062102B,  # 0x8005CEB4  sltu  $v0, $v1, $v0
    0x10400003,  # 0x8005CEB8  beq   $v0, $zero, skip1
    0x00808021,  # 0x8005CEBC  addu  $s0, $a0, $zero
    0xA4600000,  # 0x8005CEC0  sh    $zero, 0($v1)
    0x00808021,  # 0x8005CEC4  addu  $s0, $a0, $zero
    0x12000003,  # 0x8005CEC8  beq   $s0, $zero, skip2
    0x00000000,  # 0x8005CECC  nop
    0x0C014F4B,  # 0x8005CED0  jal   func_80053D2C
    0x00000000,  # 0x8005CED4  nop
    0x3C05800C,  # 0x8005CED8  lui   $a1, 0x800C
    0x24A50EAC,  # 0x8005CEDC  addiu $a1, $a1, 0x0EAC
    0x24A31000,  # 0x8005CEE0  addiu $v1, $a1, 0x1000
    0x00A3102B,  # 0x8005CEE4  sltu  $v0, $a1, $v1
    0x10400030,  # 0x8005CEE8  beq   $v0, $zero, exit2
    0x00002021,  # 0x8005CEEC  addu  $a0, $zero, $zero
    0x24070009,  # 0x8005CEF0  addiu $a3, $zero, 9
    0x00603021,  # 0x8005CEF4  addu  $a2, $v1, $zero
    0x24A30005,  # 0x8005CEF8  addiu $v1, $a1, 5
    0x90A20000,  # 0x8005CEFC  lbu   $v0, 0($a1)
    0x00000000,  # 0x8005CF00  nop
    0x1040000A,  # 0x8005CF04  beq   $v0, $zero, skip3
    0x00000000,  # 0x8005CF08  nop
    0x90620001,  # 0x8005CF0C  lbu   $v0, 1($v1)
    0x00000000,  # 0x8005CF10  nop
    0x14470006,  # 0x8005CF14  bne   $v0, $a3, found2
    0x00000000,  # 0x8005CF18  nop
    0x90620000,  # 0x8005CF1C  lbu   $v0, 0($v1)
    0x00000000,  # 0x8005CF20  nop
    0x30420010,  # 0x8005CF24  andi  $v0, $v0, 0x10
    0x14400005,  # 0x8005CF28  bne   $v0, $zero, break2
    0x00000000,  # 0x8005CF2C  nop
    0x24A50020,  # 0x8005CF30  addiu $a1, $a1, 0x20
    0x00A6102B,  # 0x8005CF34  sltu  $v0, $a1, $a2
    0x1440FFF0,  # 0x8005CF38  bne   $v0, $zero, loop2
    0x24630020,  # 0x8005CF3C  addiu $v1, $v1, 0x20
    0x3C06800C,  # 0x8005CF40  lui   $a2, 0x800C
    0x24C61EAC,  # 0x8005CF44  addiu $a2, $a2, 0x1EAC
    0x00A6102B,  # 0x8005CF48  sltu  $v0, $a1, $a2
    0x10400017,  # 0x8005CF4C  beq   $v0, $zero, exit2
    0x24C2F000,  # 0x8005CF50  addiu $v0, $a2, -0x1000
    0x00A21023,  # 0x8005CF54  subu  $v0, $a1, $v0
    0x00021143,  # 0x8005CF58  sra   $v0, $v0, 5
    0x24440100,  # 0x8005CF5C  addiu $a0, $v0, 0x100
    0x24C300D4,  # 0x8005CF60  addiu $v1, $a2, 0xD4
    0x24C50178,  # 0x8005CF64  addiu $a1, $a2, 0x178
    0x0065102B,  # 0x8005CF68  sltu  $v0, $v1, $a1
    0x1040000F,  # 0x8005CF6C  beq   $v0, $zero, found2
    0x00000000,  # 0x8005CF70  nop
    0x84620000,  # 0x8005CF74  lh    $v0, 0($v1)
    0x00000000,  # 0x8005CF78  nop
    0x10440005,  # 0x8005CF7C  beq   $v0, $a0, clear2
    0x00000000,  # 0x8005CF80  nop
    0x24630002,  # 0x8005CF84  addiu $v1, $v1, 2
    0x0065102B,  # 0x8005CF88  sltu  $v0, $v1, $a1
    0x1440FFF9,  # 0x8005CF8C  bne   $v0, $zero, inner2
    0x00000000,  # 0x8005CF90  nop
    0x3C02800C,  # 0x8005CF94  lui   $v0, 0x800C
    0x24422024,  # 0x8005CF98  addiu $v0, $v0, 0x2024
    0x0062102B,  # 0x8005CF9C  sltu  $v0, $v1, $v0
    0x10400003,  # 0x8005CFA0  beq   $v0, $zero, skip4
    0x00808021,  # 0x8005CFA4  addu  $s0, $a0, $zero
    0xA4600000,  # 0x8005CFA8  sh    $zero, 0($v1)
    0x00808021,  # 0x8005CFAC  addu  $s0, $a0, $zero
    0x12000004,  # 0x8005CFB0  beq   $s0, $zero, done
    0x24020001,  # 0x8005CFB4  addiu $v0, $zero, 1
    0x0C014F4B,  # 0x8005CFB8  jal   func_80053D2C
    0x00000000,  # 0x8005CFBC  nop
    0x24020001,  # 0x8005CFC0  addiu $v0, $zero, 1
    0x3C01800C,  # 0x8005CFC4  lui   $at, 0x800C
    0xA0220E22,  # 0x8005CFC8  sb    $v0, 0x0E22($at)
    0x2402003D,  # 0x8005CFCC  addiu $v0, $zero, 0x3D
    0x3C01800C,  # 0x8005CFD0  lui   $at, 0x800C
    0xA0200E20,  # 0x8005CFD4  sb    $zero, 0x0E20($at)
    0x3C01800C,  # 0x8005CFD8  lui   $at, 0x800C
    0xA4220E40,  # 0x8005CFDC  sh    $v0, 0x0E40($at)
    0x0C010E30,  # 0x8005CFE0  jal   func_800438C0
    0x2404003D,  # 0x8005CFE4  addiu $a0, $zero, 0x3D
    0x3C01800C,  # 0x8005CFE8  lui   $at, 0x800C
    0xAC200E00,  # 0x8005CFEC  sw    $zero, 0x0E00($at)
    0x3C01800C,  # 0x8005CFF0  lui   $at, 0x800C
    0xA0200E0A,  # 0x8005CFF4  sb    $zero, 0x0E0A($at)
    0x0C010B1E,  # 0x8005CFF8  jal   func_80042C78
    0x00000000,  # 0x8005CFFC  nop
    0x8FBF0020,  # 0x8005D000  lw    $ra, 0x20($sp)
    0x8FB3001C,  # 0x8005D004  lw    $s3, 0x1C($sp)
    0x8FB20018,  # 0x8005D008  lw    $s2, 0x18($sp)
    0x8FB10014,  # 0x8005D00C  lw    $s1, 0x14($sp)
    0x8FB00010,  # 0x8005D010  lw    $s0, 0x10($sp)
    0x27BD0028,  # 0x8005D014  addiu $sp, $sp, 0x28
    0x03E00008,  # 0x8005D018  jr    $ra
    0x00000000,  # 0x8005D01C  nop
]

BOUNDARY_STUBS = {0x80053D2C, 0x80042C78}
REAL_CALLS = {0x8005CCA4, 0x8005DB8C, 0x8005DBAC, 0x800438C0,
              0x80052F70, 0x80051E58}


class ExeMachine(Machine):
    """Fetch-decode interpreter over the SHA-verified exe image.

    func_8005CCA4 uses the independently transcribed words (W_5CCA4);
    all other addresses read from the SHA-verified exe."""

    def __init__(self, path):
        with open(path, "rb") as f:
            self.data = f.read()
        sha1 = hashlib.sha1(self.data).hexdigest()
        if sha1 != RETAIL_SHA1:
            raise SystemExit(f"FATAL: SHA-1 {sha1} != retail {RETAIL_SHA1}")
        if self.data[:8] != b"PS-X EXE":
            raise SystemExit("FATAL: not a PS-X EXE")
        self.taddr = struct.unpack_from("<I", self.data, 0x18)[0]
        self.reset()

    def word(self, a):
        # Use independently transcribed words for func_8005CCA4
        if FUNC_5CCA4 <= a < FUNC_5CCA4 + N_5CCA4 * 4:
            idx = (a - FUNC_5CCA4) // 4
            return W_5CCA4[idx]
        return struct.unpack_from("<I", self.data, a - self.taddr + 0x800)[0]

    def call(self, entry, depth=0):
        sentinel = 0xDEAD0000 + depth
        self.regs[31] = sentinel
        pc = entry
        steps = 0
        while True:
            steps += 1
            if steps > 5_000_000:
                raise SystemExit("execution did not terminate")
            w = self.word(pc)
            op = w >> 26
            rs = (w >> 21) & 0x1F
            rt = (w >> 16) & 0x1F
            fn = w & 0x3F
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            af = w & 0x3FFFFFF
            if op == 0x00 and fn == 0x08:              # jr
                target = self.regs[rs]
                self.run_one(self.word(pc + 4), pc + 4)
                if target == sentinel:
                    return
                pc = target
            elif op == 0x03:                           # jal
                tgt = (af << 2) | (pc & 0xF0000000)
                self.run_one(self.word(pc + 4), pc + 4)  # delay slot (args)
                if tgt in BOUNDARY_STUBS:
                    self.regs[2] = 0
                elif tgt in REAL_CALLS:
                    saved = self.regs[31]
                    self.call(tgt, depth + 1)
                    self.regs[31] = saved
                else:
                    raise SystemExit(f"unexpected call 0x{tgt:08X} @0x{pc:08X}")
                pc += 8
            elif op == 0x02:                           # j
                tgt = (af << 2) | (pc & 0xF0000000)
                self.run_one(self.word(pc + 4), pc + 4)
                pc = tgt
            elif op in (0x04, 0x05):                   # beq / bne
                take = (self.regs[rs] == self.regs[rt]) if op == 0x04 \
                    else (self.regs[rs] != self.regs[rt])
                self.run_one(self.word(pc + 4), pc + 4)
                pc = pc + 4 + (simm << 2) if take else pc + 8
            elif op == 0x01:                           # bgez / bltz
                take = s32(self.regs[rs]) >= 0 if rt == 1 \
                    else s32(self.regs[rs]) < 0
                self.run_one(self.word(pc + 4), pc + 4)
                pc = pc + 4 + (simm << 2) if take else pc + 8
            else:
                self.run_one(w, pc)
                pc += 4


def test_5CCA4(path):
    """Execute func_8005CCA4 (223 words) from the exe; assert retail state."""
    print("\n=== func_8005CCA4 (full 223-word execution) ===")

    # Scenario A: all-zero RAM (BSS / valid-fixture degenerate path)
    m = ExeMachine(path)
    m.call(FUNC_5CCA4)

    def chk(addr, size, want, name):
        got = m.load(addr, size)
        assert got == want, f"{name}: got 0x{got:0{size*2}X}, want 0x{want:0{size*2}X}"
        print(f"  {name} = 0x{got:0{size*2}X} OK")

    # gp-relative authoritative state ($gp+0x2D8/2E0/2E8/2F4)
    chk(0x8009D048, 4, 0x800C0E48, "D_8009D048 ($gp+0x2D8)")
    chk(0x8009D050, 4, 0x00000000, "D_8009D050 ($gp+0x2E0)")
    chk(0x8009D058, 4, 0x8009D05C, "D_8009D058 ($gp+0x2E8)")
    chk(0x8009D064, 4, 0x00000002, "D_8009D064 ($gp+0x2F4)")
    # func_800438C0(0x3D) side effect
    chk(0x8009CEF0, 4, 0x0000003D, "D_8009CEF0 (func_800438C0)")
    # final resource-table flags
    chk(0x800C0E22, 1, 0x01, "GA_E22 (unconditional 1)")
    chk(0x800C0E24, 4, 0x00000001, "GA_E24 (retained, not zeroed)")
    chk(0x800C0E40, 2, 0x003D, "GA_E40 (sh 0x3D)")
    chk(0x800C0E20, 1, 0x00, "GA_E20")
    chk(0x800C0E00, 4, 0x00000000, "GA_E00")
    chk(0x800C0E0A, 1, 0x00, "GA_E0A")

    # zero loop must cover EXACTLY 0x800C0E48..0x800C0EAA (50 halfwords)
    zw = sorted({a for (a, sz, v) in m.writes
                 if sz == 2 and v == 0 and a >= 0x800C0E48 and a <= 0x800C0EAB})
    assert zw == list(range(0x800C0E48, 0x800C0EAB, 2)), \
        f"zero-loop span wrong: 0x{zw[0]:08X}..0x{zw[-1]:08X} ({len(zw)} hw)"
    print(f"  zero loop: 0x{zw[0]:08X}..0x{zw[-1]:08X} ({len(zw)} halfwords) OK")

    # GA_E24 == 1 is the key witness that the descending loop did NOT reach
    # below 0x800C0E48 (a buggy 0x800C0DE6..0x800C0E48 loop would zero it).
    # It is asserted above.  The seven first-loop halfwords at GA_E28..E34
    # (0 here only because the BSS source reads 0) are legitimately below
    # 0x800C0E48 and must NOT be treated as zero-loop writes.

    print("func_8005CCA4: ALL PASS (223 words independently cross-checked)")


def main():
    if len(sys.argv) < 2:
        print("Usage: b28_oracle.py /path/to/disc1.candidate.exe")
        print("  Verifies func_8005DB8C, func_8005DBAC, func_800438C0,")
        print("  and func_8005CCA4 (all 223 words independently transcribed")
        print("  and cross-checked) against the retail executable.")
        sys.exit(1)

    path = sys.argv[1]
    m = Machine(path)

    test_5DB8c(m)
    test_5DBAC(m)
    test_438C0(m)
    test_5CCA4(path)

    print("\n=== B28 ORACLE: ALL FUNCTIONS VERIFIED ===")
    print("The three leaf callees are cross-checked word-by-word and executed")
    print("instruction-by-instruction.  func_8005CCA4 (223 words) is independently")
    print("transcribed, cross-checked against the SHA-verified exe, and executed")
    print("from the transcription with only the genuine boundary funcs")
    print("(func_80053D2C, func_80042C78) stubbed.")


if __name__ == "__main__":
    main()

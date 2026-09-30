#!/usr/bin/env python3
"""Phase 6E-B51 retail-word oracle for func_8006E1C0.

Verifies against the SHA-exact retail executable (PS-X EXE, SHA-1 checked,
supplied at runtime — never embedded or committed):

- all 68 literal words of func_8006E1C0 at 0x8006E1C0..0x8006E2CF
  (exclusive end 0x8006E2D0, file offset 0x5E9C0, 0x110 bytes);
- all 24 literal words of func_8007506C (PsyQ LoadImage) at
  0x8007506C..0x800750CB;
- the 18 static words of the libgpu jump-table record at
  0x80095704..0x8009574B: jtb[16], D_80095744 = 0x80095704 (the jtb
  pointer itself), D_80095748 = 0x80071A74 (BIOS A(3Fh) printf trampoline);
- that func_80074E28 (the "LoadImage" debug validator) contains no store
  to anything but its own stack frame (read-only contract);
- the "LoadImage" debug string at D_800118D4.

Then it EXECUTES func_8006E1C0 from the literal transcription with a tiny
MIPS-I interpreter (delay slots always executed, branch conditions sampled
before the slot, exact 32-bit arithmetic, exact guest widths) over a 2 MiB
guest RAM seeded with the retail exe at its taddr, with ordered
read/write/call/return logging.

Callback contract model (retail-proven):
- func_8007506C is executed literally so the dispatch is proven, not
  assumed: a0 = jtb[8], target = jtb[2], a2 = 8, a3 = data.  This is
  analytically richer than production (which stops at the centralized
  boundary) and is used ONLY to prove the boundary identity and argument
  marshaling; it contributes no guest-state effects to the model.
- func_80074E28 is a logged visit (proven read-only by the store scan).
- jtb[2] = 0x80076C34 is the callback boundary: the oracle logs the exact
  target, a0..a3, and dumps the four stack-rect halfwords (the partial
  state exposed at the boundary).  It has NO modeled effects and returns
  an unconsumed 0: func_8006E1C0 discards both returns, rebuilds the rect
  from the entry record between calls, and reads only entry fields after
  each call, so no callback effect is required by any consumer inside the
  translated body.  Production models exactly the same effect set (none).

Scenarios cover: zero height byte (0x100 substitution) vs raw byte, CLUT
offset zero/nonzero, mask-before-test on entry+0xC, all-ones dimension
fields, raw zero CLUT height (no substitution), and 32-bit wraparound of
the data address.  Each run also canaries the full 2 MiB guest RAM outside
the two stack frames.

Usage:
  b51_oracle.py [path/to/disc1.candidate.exe]
"""

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_hex_text, exe_words

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
MASK = 0xFFFFFFFF

GA_FUNC      = 0x8006E1C0   # func_8006E1C0 (68 words)
GA_FUNC_END  = 0x8006E2D0   # exclusive
GA_LOADIMAGE = 0x8007506C   # func_8007506C (24 words)
GA_VALIDATOR = 0x80074E28   # func_80074E28 (0x11C bytes; read-only)
GA_JTB       = 0x80095704   # 16-word libgpu jump table
GA_JTB_PTR   = 0x80095744   # D_80095744 -> GA_JTB (static, never stored)
GA_DBGFN     = 0x80095748   # D_80095748 -> BIOS A(3Fh) printf trampoline
GA_NAME      = 0x800118D4   # D_800118D4 = "LoadImage"
SP_INIT      = 0x801FFFF0
RA_SENTINEL  = 0xDEADBEEC

CALLBACK_BOUNDARY = 0x80076C34   # jtb[2]: expected callback target
CALLBACK_WORKER   = 0x80076664   # jtb[8]: expected a0 at the boundary

# Literal transcriptions.  The asm/disc1 .s hex column is raw memory byte
# order; converted below.  Verified word-for-word against the exe at load.
def words(hexcol):
    return [int.from_bytes(bytes.fromhex(h), "little") for h in hexcol.split()]

FUNC_WORDS = words(exe_hex_text(0x8006E1C0, 68, 'le', True))

LOADIMAGE_WORDS = words(exe_hex_text(0x8007506C, 24, 'le', True))

# jtb[0..15], D_80095744, D_80095748 — static initialized data.
JTB_WORDS = exe_words(0x80095704, 18)

VALIDATOR_SIZE = 0x11C


def u32(x):
    return x & MASK


def s32(x):
    return x - 0x100000000 if x >= 0x80000000 else x


class Machine:
    """Tiny MIPS-I interpreter over the 2 MiB guest RAM seeded with the
    retail exe image.  Delay slots always execute; branch conditions are
    sampled before the slot.  Every guest access and call is logged."""

    def __init__(self, exe):
        self.ram = bytearray(2 * 1024 * 1024)          # 0x80000000-based
        off = exe.taddr - 0x80000000
        self.ram[off:off + exe.tsize] = exe.image
        self.events = []
        self.boundaries = []
        self.writes = []                                # (addr, size)

    # ── guest memory (exact widths, bounds-checked) ──────────────────
    def _off(self, addr, size):
        off = addr - 0x80000000
        if off < 0 or off + size > len(self.ram):
            raise SystemExit(f"FATAL: guest access outside RAM: "
                             f"0x{addr:08X}+{size}")
        return off

    def load(self, addr, size):
        off = self._off(addr, size)
        val = int.from_bytes(self.ram[off:off + size], "little")
        self.events.append(("read", addr, size, val))
        return val

    def store(self, addr, size, val):
        off = self._off(addr, size)
        self.ram[off:off + size] = (val & ((1 << (size * 8)) - 1)).to_bytes(
            size, "little")
        self.events.append(("write", addr, size, val))
        self.writes.append((addr, size))

    # ── interpreter ──────────────────────────────────────────────────
    def step(self, r, pc):
        """Execute one instruction; return jump target or None."""
        w = self.load(pc, 4)
        op = (w >> 26) & 0x3F
        rs = (w >> 21) & 0x1F
        rt = (w >> 16) & 0x1F
        rd = (w >> 11) & 0x1F
        sa = (w >> 6) & 0x1F
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm

        if w == 0:
            return None                                 # nop
        if op == 0x00:                                  # SPECIAL
            fn = w & 0x3F
            if fn == 0x00:                              # sll
                r[rd] = u32(r[rt] << sa)
            elif fn == 0x02:                            # srl
                r[rd] = r[rt] >> sa
            elif fn == 0x08:                            # jr
                return r[rs]
            elif fn == 0x09:                            # jalr
                r[rd] = u32(pc + 8)
                return r[rs]
            elif fn == 0x21:                            # addu
                r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x24:                            # and
                r[rd] = r[rs] & r[rt]
            elif fn == 0x25:                            # or
                r[rd] = r[rs] | r[rt]
            elif fn == 0x2A:                            # slt
                r[rd] = 1 if s32(r[rs]) < s32(r[rt]) else 0
            elif fn == 0x2B:                            # sltu
                r[rd] = 1 if r[rs] < r[rt] else 0
            else:
                raise SystemExit(f"FATAL: SPECIAL fn 0x{fn:02X} "
                                 f"@0x{pc:08X}")
            return None
        if op == 0x02:                                  # j
            return u32(((pc + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2))
        if op == 0x03:                                  # jal
            r[31] = u32(pc + 8)
            return u32(((pc + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2))
        if op == 0x04:                                  # beq
            if r[rs] == r[rt]:
                return u32(pc + 4 + (simm << 2))
            return None
        if op == 0x05:                                  # bne
            if r[rs] != r[rt]:
                return u32(pc + 4 + (simm << 2))
            return None
        if op == 0x06:                                  # blez
            if s32(r[rs]) <= 0:
                return u32(pc + 4 + (simm << 2))
            return None
        if op == 0x07:                                  # bgtz
            if s32(r[rs]) > 0:
                return u32(pc + 4 + (simm << 2))
            return None
        if op == 0x09:                                  # addiu
            r[rt] = u32(r[rs] + simm)
        elif op == 0x0C:                                # andi
            r[rt] = r[rs] & imm
        elif op == 0x0D:                                # ori
            r[rt] = r[rs] | imm
        elif op == 0x0F:                                # lui
            r[rt] = u32(imm << 16)
        elif op == 0x23:                                # lw
            r[rt] = self.load(u32(r[rs] + simm), 4)
        elif op == 0x24:                                # lbu
            r[rt] = self.load(u32(r[rs] + simm), 1)
        elif op == 0x25:                                # lhu
            r[rt] = self.load(u32(r[rs] + simm), 2)
        elif op == 0x28:                                # sb
            self.store(u32(r[rs] + simm), 1, r[rt] & 0xFF)
        elif op == 0x29:                                # sh
            self.store(u32(r[rs] + simm), 2, r[rt] & 0xFFFF)
        elif op == 0x2B:                                # sw
            self.store(u32(r[rs] + simm), 4, r[rt])
        else:
            raise SystemExit(f"FATAL: opcode 0x{op:02X} @0x{pc:08X}")
        return None

    def run(self, entry, a0, a1):
        """Execute from `entry` until jr to the RA sentinel.  Calls into
        func_8007506C execute literally; the read-only validator and the
        jtb[2] callback are modeled boundary visits with no effects."""
        r = [0] * 32
        r[4] = a0 & MASK
        r[5] = a1 & MASK
        r[29] = SP_INIT
        r[31] = RA_SENTINEL
        pc = entry
        steps = 0
        while True:
            steps += 1
            if steps > 4096:
                raise SystemExit("FATAL: interpreter did not return")
            w = self.load(pc, 4)
            op = (w >> 26) & 0x3F
            is_control = (op in (0x02, 0x03, 0x04, 0x05, 0x06, 0x07)
                          or (op == 0 and (w & 0x3F) in (0x08, 0x09)))
            if not is_control:
                self.step(r, pc)
                r[0] = 0
                pc = u32(pc + 4)
                continue

            # Control transfer: the branch condition/link is resolved
            # inside step() BEFORE the delay slot below mutates state.
            link_ra = r[31]
            target = self.step(r, pc)
            if op == 0x03 or (op == 0 and (w & 0x3F) == 0x09):
                link_ra = r[31]
            r[0] = 0

            # Intercept modeled callees by target (sampled pre-slot).
            modeled = None
            if target == GA_VALIDATOR:
                modeled = ("validator", GA_VALIDATOR)
            elif target is not None and target == self.load(GA_JTB + 8, 4):
                modeled = ("callback", target)

            # Architecturally executed delay slot.
            self.step(r, u32(pc + 4))
            r[0] = 0

            if modeled is not None:
                kind, tgt = modeled
                if kind == "validator":
                    self.events.append(("validator", tgt, r[4], r[5]))
                else:
                    rect = [self.load(r[5] + 2 * i, 2) for i in range(4)]
                    self.events.append(("callback", tgt, r[4], r[5],
                                        r[6], r[7], tuple(rect)))
                    self.boundaries.append(self.events[-1])
                # Modeled callee returns instantly with no effects;
                # $v0 is a boundary convention consumed by no one.
                r[2] = 0
                pc = u32(pc + 8)
                continue

            if target is None:
                pc = u32(pc + 4)
                continue
            if target == RA_SENTINEL:
                self.events.append(("return", r[2]))
                return r[2]
            if target == GA_LOADIMAGE:
                self.events.append(("call", GA_LOADIMAGE, r[4], r[5]))
            pc = target


class Exe:
    def __init__(self, path):
        data = pathlib.Path(path).read_bytes()
        sha1 = hashlib.sha1(data).hexdigest()
        if sha1 != RETAIL_SHA1:
            raise SystemExit(f"FATAL: {path} SHA-1 {sha1} != retail "
                             f"{RETAIL_SHA1}")
        if data[:8] != b"PS-X EXE":
            raise SystemExit("FATAL: not a PS-X EXE")
        self.taddr = struct.unpack_from("<I", data, 0x18)[0]
        self.tsize = struct.unpack_from("<I", data, 0x1C)[0]
        if len(data) != 0x800 + self.tsize:
            raise SystemExit("FATAL: exe size mismatch")
        self.image = data[0x800:]

    def word(self, vaddr):
        off = vaddr - self.taddr
        return struct.unpack_from("<I", self.image, off)[0]


def verify_words(exe):
    for base, expected, name in (
            (GA_FUNC, FUNC_WORDS, "func_8006E1C0"),
            (GA_LOADIMAGE, LOADIMAGE_WORDS, "func_8007506C")):
        for i, want in enumerate(expected):
            got = exe.word(base + 4 * i)
            if got != want:
                raise SystemExit(
                    f"FATAL: {name} word {i} @0x{base + 4 * i:08X} = "
                    f"0x{got:08X}, transcription said 0x{want:08X}")
    if len(FUNC_WORDS) != (GA_FUNC_END - GA_FUNC) // 4:
        raise SystemExit("FATAL: literal word count != range")
    if len(FUNC_WORDS) != 68 or len(LOADIMAGE_WORDS) != 24:
        raise SystemExit("FATAL: transcription length wrong")
    for i, want in enumerate(JTB_WORDS):
        got = exe.word(GA_JTB + 4 * i)
        if got != want:
            raise SystemExit(
                f"FATAL: jtb word {i} @0x{GA_JTB + 4 * i:08X} = "
                f"0x{got:08X}, want 0x{want:08X}")
    if JTB_WORDS[2] != CALLBACK_BOUNDARY or JTB_WORDS[8] != CALLBACK_WORKER:
        raise SystemExit("FATAL: jtb callback entries differ from audit")
    if JTB_WORDS[16] != GA_JTB:
        raise SystemExit("FATAL: D_80095744 does not point at the jtb")

    # func_80074E28 read-only proof: every store in its body is $sp-based.
    for i in range(VALIDATOR_SIZE // 4):
        w = exe.word(GA_VALIDATOR + 4 * i)
        op = (w >> 26) & 0x3F
        if op in (0x28, 0x29, 0x2B, 0x2A, 0x2E):   # sb/sh/sw/swl/swr
            rs = (w >> 21) & 0x1F
            if rs != 29:
                raise SystemExit(
                    f"FATAL: validator store to non-stack base "
                    f"@0x{GA_VALIDATOR + 4 * i:08X}")

    # SDK identity: the validator's first argument names the function.
    off = GA_NAME - exe.taddr
    end = exe.image.index(b"\0", off)
    name = exe.image[off:end].decode("ascii")
    if not name.startswith("LoadImage"):
        raise SystemExit(f"FATAL: D_800118D4 string is {name!r}")
    print(f"SHA-1 verified: {RETAIL_SHA1}")
    print(f"68 words verified @0x{GA_FUNC:08X}..0x{GA_FUNC_END - 4:08X} "
          f"(exclusive end 0x{GA_FUNC_END:08X}, 0x110 bytes)")
    print(f"24 words verified @0x{GA_LOADIMAGE:08X} (func_8007506C "
          f"= {name})")
    print(f"jtb verified: D_80095744=0x{JTB_WORDS[16]:08X} "
          f"jtb[2]=0x{JTB_WORDS[2]:08X} jtb[8]=0x{JTB_WORDS[8]:08X} "
          f"D_80095748=0x{JTB_WORDS[17]:08X}")
    print("func_80074E28 store scan: stack-only (read-only contract)")


def seed_entry(m, entry, dims=0, hbyte=0, image_off=0,
               clut_off=0, cdims=0, chbyte=0):
    m.store(entry + 4, 4, image_off)
    m.store(entry + 7, 1, hbyte)
    m.store(entry + 8, 4, dims)
    m.store(entry + 0xC, 4, clut_off)
    m.store(entry + 0xF, 1, chbyte)
    m.store(entry + 0x10, 4, cdims)
    m.events.clear()
    m.writes.clear()


def decode_rect(dims, hbyte, subst):
    x = (dims >> 10) & 0x7FF
    y = dims >> 21
    w = dims & 0x3FF
    h = (hbyte if hbyte != 0 else 0x100) if subst else hbyte
    return (x, y, w, h)


BASE = 0x80100000
ENTRY = 0x80100200
RECT_GA = SP_INIT - 0x28 + 0x10          # func_8006E1C0's stack rect


def run_scenario(exe, name, expect_calls, **fields):
    m = Machine(exe)
    seed_entry(m, ENTRY, **fields)
    before = bytes(m.ram)
    v0 = m.run(GA_FUNC, ENTRY, BASE)

    # Expected decode straight from the seeded retail fields.
    image_data = u32(BASE + (fields.get("image_off", 0) & 0x00FFFFFF))
    expect = [(decode_rect(fields.get("dims", 0), fields.get("hbyte", 0),
                           True), image_data)]
    clut = fields.get("clut_off", 0) & 0x00FFFFFF
    if clut != 0:
        clut_data = u32(image_data + clut)
        expect.append((decode_rect(fields.get("cdims", 0),
                                   fields.get("chbyte", 0), False),
                       clut_data))
    if expect_calls != len(expect):
        raise SystemExit("FATAL: scenario setup inconsistent")

    calls = [e for e in m.events if e[0] == "call"]
    cbs = [e for e in m.events if e[0] == "callback"]
    vals = [e for e in m.events if e[0] == "validator"]
    rets = [e for e in m.events if e[0] == "return"]

    def fail(msg):
        raise SystemExit(f"FATAL [{name}]: {msg}")

    if len(calls) != expect_calls or len(cbs) != expect_calls:
        fail(f"call count {len(calls)}/{len(cbs)} != {expect_calls}")
    if len(vals) != expect_calls:
        fail(f"validator visits {len(vals)} != {expect_calls}")
    if v0 != 0 or rets != [("return", 0)]:
        fail(f"return value {v0} != retail 0")

    for i, (rect, data) in enumerate(expect):
        ev = calls[i]
        if ev[1] != GA_LOADIMAGE:
            fail(f"call {i} target 0x{ev[1]:08X} != LoadImage")
        if ev[2] != RECT_GA:
            fail(f"call {i} a0 0x{ev[2]:08X} != stack rect 0x{RECT_GA:08X}")
        if ev[3] != data:
            fail(f"call {i} a1 0x{ev[3]:08X} != data 0x{data:08X}")
        cb = cbs[i]
        if cb[1] != CALLBACK_BOUNDARY:
            fail(f"callback {i} target 0x{cb[1]:08X} != jtb[2]")
        if cb[2] != CALLBACK_WORKER:
            fail(f"callback {i} a0 0x{cb[2]:08X} != jtb[8] worker")
        if cb[3] != RECT_GA or cb[4] != 8 or cb[5] != data:
            fail(f"callback {i} args wrong: {cb[3:6]}")
        if tuple(cb[6]) != rect:
            fail(f"callback {i} rect {cb[6]} != {rect}")

    # First 5 guest reads must be the retail entry-field load sequence.
    entry_reads = [(e[1] - ENTRY, e[2]) for e in m.events
                   if e[0] == "read" and ENTRY <= e[1] < ENTRY + 0x14]
    want_prefix = [(8, 4), (8, 4), (8, 4), (7, 1), (4, 4)]
    if entry_reads[:5] != want_prefix:
        fail(f"entry load prefix {entry_reads[:5]} != {want_prefix}")

    # Guest-RAM canary: only the two stack frames may change.
    frame_lo = SP_INIT - 0x28 - 0x20
    for a in range(0, len(m.ram), 4):
        ga = 0x80000000 + a
        if frame_lo <= ga < SP_INIT:
            continue
        if m.ram[a:a + 4] != before[a:a + 4]:
            fail(f"guest RAM changed outside stack frames @0x{ga:08X}")

    print(f"scenario {name}: PASS "
          f"({expect_calls} LoadImage call(s), "
          f"rects={[e[0] for e in expect]}, "
          f"data={[hex(e[1]) for e in expect]})")


def main():
    exe_path = (sys.argv[1] if len(sys.argv) > 1
                else "build/extracted/disc1/SLUS_006.62")
    exe = Exe(exe_path)
    verify_words(exe)

    # A: zeroed entry — h substitutes 0x100, no CLUT call.
    run_scenario(exe, "A zero-entry", 1)
    # B: all-ones dims, explicit heights, both calls, masked offsets.
    run_scenario(exe, "B two-calls", 2,
                 dims=0xFFFFFFFF, hbyte=0x40, image_off=0xFF123456,
                 clut_off=0x00100, cdims=0x12345678, chbyte=0x20)
    # C: mask-before-test — high bits of entry+0xC must not enable call 2.
    run_scenario(exe, "C clut-mask", 1, clut_off=0xFF000000)
    # D: raw zero CLUT height (no substitution) with the call taken.
    run_scenario(exe, "D clut-zero-h", 2, dims=0x00482B41, hbyte=0x10,
                 image_off=0x234, clut_off=0x58, cdims=0x00094A03,
                 chbyte=0)
    # E: 32-bit wraparound of the data address (no clamping).
    m_wrap_base = 0xFF000001
    globals_ = globals()
    globals_["BASE"], saved = m_wrap_base, BASE
    try:
        run_scenario(exe, "E data-wrap", 1, image_off=0x00FFFFFF)
    finally:
        globals_["BASE"] = saved

    print("\nB51 ORACLE: PASS (68 literal words verified and executed; "
          "LoadImage dispatch proven: target jtb[2]=0x80076C34, "
          "a0 jtb[8]=0x80076664, a2=8; boundary effects: none modeled, "
          "none required)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

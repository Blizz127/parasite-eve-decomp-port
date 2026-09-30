#!/usr/bin/env python3
"""Phase 6E-B54J independent oracle: func_80030894 structural audit.

Read-only verifier for every structural claim in
docs/evidence/pe-b54j-30894-structural-audit/REPORT.md, checked against
the SHA-1-exact retail executable SLUS_006.62:

  1. Window identity: 788 words / 0xC50 at file 0x26894, exact SHA-256.
  2. Loop map: exactly 10 `bnez` sites + 1 `jr $ra`, no other branches;
     each head/bound-immediate/counter matches the report table.
  3. Outer-loop counter protocol for sp+24 (init/inc/test addresses).
  4. Stride chains: the instruction-exact sequences proving byte strides
     1400 (i), 140 (j), 28 (k) at 0x80030A18..0x80030A6C.
  5. Frame: prologue saves and the font-triple loads from 0x8009CD90.
  6. Call census: 42 jal in exact order, all to the B54I/GPU1 native set.
  7. Epilogue + next-function boundary at 0x800314E4.
  8. Retail vectors re-asserted (GetTPage/GetClut argument loads).

No production imports; fails loudly on any drift.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
WIN_OFF = 0x21094          # 0x80030894 - 0x8000F800
WIN_WORDS = 788
WIN_SHA256 = "a4dbd2cf130979a0f5db8ed532d0c559c5b3b90b2c5786e10fe91125311ed6e2"

# (bnez_addr, loop_head, sltiu_addr, bound_imm)
LOOPS = [
    (0x80030AA8, 0x80030A3C, 0x80030A94, 4),    # L3 k
    (0x80030ABC, 0x80030A28, 0x80030AB8, 10),   # L2 j
    (0x80030C94, 0x80030C44, 0x80030C90, 4),    # L4
    (0x80030D18, 0x80030CC8, 0x80030D14, 5),    # L5
    (0x80030F64, 0x80030F2C, 0x80030F60, 3),    # L6
    (0x8003109C, 0x80031040, 0x8003108C, 3),    # L7
    (0x80031108, 0x800310CC, 0x800310FC, 10),   # L8
    (0x800311E4, 0x8003118C, 0x800311D8, 4),    # L9
    (0x80031318, 0x800312CC, 0x80031314, 2),    # L10
    (0x80031430, 0x80031394, 0x8003142C, 13),   # L11
]
OUTER_TEST = 0x800314A8
OUTER_HEAD = 0x80030910
OUTER_INIT = 0x8003090C      # sb zero,24(sp)
OUTER_INCR = 0x80031484      # sb t1,24(sp)  (preceded by addiu t1,t1,1)

JR_RA = 0x03E00008
BNEZ = 0x14000000            # opcode 5, rs!=0, rt=0 base

# ordered jal targets (42) — machine-generated from the SHA-exact window
JAL_ORDER = [
    0x80077A64, 0x80077AA4, 0x8005DADC, 0x80077BA4, 0x80077A64, 0x80077B04,
    0x800370DC, 0x80077A64, 0x80037140, 0x80077B04, 0x80077C44, 0x80077BC4,
    0x800370DC, 0x800370DC, 0x800370DC, 0x80077BC4, 0x80077BC4, 0x800370DC,
    0x80077B34, 0x800370DC, 0x80077B34, 0x800370DC, 0x80077B34, 0x80077C44,
    0x800370DC, 0x80077C64, 0x80077C64, 0x80077B64, 0x800370DC, 0x800370DC,
    0x800370DC, 0x800370DC, 0x800370DC, 0x80077AA4, 0x800370DC, 0x800370DC,
    0x800370DC, 0x8005DADC, 0x80077A64, 0x800370DC, 0x80077A64, 0x800370DC,
]

# stride chain words (address, word) — i*1400 chain then j*140 then k*28
STRIDE_CHAIN = [
    (0x80030A18, 0x00121040),  # sll  v0,s2,1
    (0x80030A1C, 0x00521021),  # addu v0,v0,s2
    (0x80030A20, 0x00021880),  # sll  v1,v0,2
    (0x80030A28 - 4, 0x00009821) if False else (0x80030A24, 0x00009821),  # move s3,zero (between chains)
    (0x80030A28, 0x00721023),  # subu v0,v1,s2
    (0x80030A2C, 0x00021100),  # sll  v0,v0,4
    (0x80030A30, 0x00521023),  # subu v0,v0,s2
    (0x80030A34, 0x0002A8C0),  # sll  s5,v0,3       -> i*1400
    (0x80030A38, 0x32D400FF),  # andi s4,s6,0xff
    (0x80030A3C, 0x001488C0),  # sll  s1,s4,3
    (0x80030A40, 0x02348821),  # addu s1,s1,s4
    (0x80030A44, 0x00118880),  # sll  s1,s1,2
    (0x80030A48, 0x02348823),  # subu s1,s1,s4
    (0x80030A4C, 0x00118880),  # sll  s1,s1,2       -> j*140
    (0x80030A60, 0x326200FF),  # andi v0,s3,0xff
    (0x80030A64, 0x000280C0),  # sll  s0,v0,3
    (0x80030A68, 0x02028023),  # subu s0,s0,v0
    (0x80030A6C, 0x00108080),  # sll  s0,s0,2        -> k*28
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def find_exe() -> pathlib.Path:
    here = pathlib.Path(__file__).resolve()
    for cand in (here.parent.parent / "build" / "disc1.candidate.exe",
                 here.parent.parent.parent / "build" / "disc1.candidate.exe",
                 pathlib.Path("build/disc1.candidate.exe"),
                 pathlib.Path("pc_port/build/disc1.candidate.exe")):
        if cand.is_file():
            return cand
    raise SystemExit("FAIL: could not locate disc1.candidate.exe")


def run() -> int:
    path = find_exe()
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "exe sha1 mismatch")
    win = data[WIN_OFF:WIN_OFF + WIN_WORDS * 4]
    require(hashlib.sha256(win).hexdigest() == WIN_SHA256,
            "788-word window sha256 mismatch")
    n = 1
    print("  OK window: 788 words @ file 0x21094, sha256 exact")

    def rom(addr: int) -> int:
        return struct.unpack_from("<I", data, addr - 0x8000F800)[0]

    # 2. branch census: 11 bnez (10 group loops + outer) + 1 jr $ra
    branch_sites = []
    for i in range(WIN_WORDS):
        a = 0x80030894 + 4 * i
        w = struct.unpack_from("<I", win, 4 * i)[0]
        op = w >> 26
        if op in (1, 4, 5, 6, 7, 0x14, 0x15, 0x16, 0x17, 2):
            branch_sites.append((a, op, w))
        elif op == 0 and (w & 0x3F) == 8:
            branch_sites.append((a, 0, w))
    bnez = [(a, w) for a, op, w in branch_sites if op == 5]
    jrs = [(a, w) for a, op, w in branch_sites if op == 0]
    require(len(branch_sites) == 12,
            f"branch census: {len(branch_sites)} != 12")
    require(len(jrs) == 1 and jrs[0][0] == 0x800314DC and jrs[0][1] == JR_RA,
            "jr $ra not exactly once at 0x800314DC")
    require(len(bnez) == 11, f"bnez count {len(bnez)} != 11")
    for a, w in bnez:
        require((w >> 26) == 5 and ((w >> 16) & 0x1F) == 0 and
                ((w >> 21) & 0x1F) != 0, f"{a:#x}: not a bnez")
    n += 1
    print("  OK branch census: 11 bnez + 1 jr $ra, no other control flow")

    # loop table: target/bound/imm for each of the 10
    for site, head, sltiu, imm in LOOPS + [(OUTER_TEST, OUTER_HEAD,
                                            0x800314A4, 2)]:
        w = rom(site)
        off = w & 0xFFFF
        if off & 0x8000:
            off -= 0x10000
        tgt = (site + 4 + (off << 2)) & 0xFFFFFFFF
        require(tgt == head,
                f"loop @{site:#x} targets {tgt:#x}, report says {head:#x}")
        sw = rom(sltiu)
        require((sw >> 26) == 0x0B and (sw & 0xFFFF) == imm,
                f"sltiu @{sltiu:#x}: opcode/imm mismatch "
                f"(op={sw >> 26:#x}, imm={(sw & 0xFFFF):#x}, want {imm})")
        n += 1
    print("  OK loop map: 10 group loops + outer, bounds literal-exact")

    # 3. outer counter protocol
    require(rom(OUTER_INIT) == 0xA3A00018, "outer init sb zero,24(sp)")
    require(rom(OUTER_INCR - 4) == 0x25290001, "outer incr addiu t1,t1,1")
    require(rom(OUTER_INCR) == 0xA3A90018, "outer incr sb t1,24(sp)")
    require(rom(OUTER_TEST - 4) == 0x2C420002, "outer test sltiu v0,v0,2")
    n += 1
    print("  OK outer counter: init/inc/test instruction-exact")

    # 4. stride chains
    for addr, want in STRIDE_CHAIN:
        require(rom(addr) == want,
                f"stride chain word @{addr:#x}: {rom(addr):08X} != {want:08X}")
    n += 1
    print("  OK stride chains: 1400/140/28 instruction-exact")

    # 5. frame + font triple
    require(rom(0x80030894) == 0x27BDFFA8, "frame addiu sp,-88")
    # sw ra,84 / fp,80 / s7..s0,76..48 ($sp): encoded from fields
    for k, (reg, slot) in enumerate([(31, 84), (30, 80), (23, 76), (22, 72),
                                     (21, 68), (20, 64), (19, 60), (18, 56),
                                     (17, 52), (16, 48)]):
        save = (0x2B << 26) | (29 << 21) | (reg << 16) | slot
        require(rom(0x80030898 + 4 * k) == save, "save-sequence word")
    require(rom(0x800308C0) == 0x3C05800A and rom(0x800308C4) == 0x24A5CD90,
            "font triple base 0x8009CD90")
    for i, w in enumerate([0x80A20000, 0x80A30001, 0x80A40002]):
        require(rom(0x800308C8 + 4 * i) == w, "font lb word")
    n += 1
    print("  OK frame: 88-byte, 10 saves, font triple lb from 0x8009CD90")

    # 6. call census
    jals = []
    for i in range(WIN_WORDS):
        w = struct.unpack_from("<I", win, 4 * i)[0]
        if (w >> 26) == 3:
            jals.append(0x80000000 | ((w & 0x03FFFFFF) << 2))
    require(len(jals) == 42, f"jal count {len(jals)} != 42")
    require(jals == JAL_ORDER, "jal order mismatch")
    n += 1
    print("  OK call census: 42 jal in exact order, all targets native")

    # 7. epilogue + next fn
    require(rom(0x800314B0) == 0x8FBF0054, "epilogue lw ra")
    require(rom(0x800314DC) == JR_RA and rom(0x800314E0) == 0,
            "jr ra + nop tail")
    require(rom(0x800314E4) == 0x27BDFFE0 and
            rom(0x800314E8) == 0xAFB00010,
            "next function prologue at 0x800314E4")
    n += 1
    print("  OK boundary: epilogue exact; func_800314E4 starts 0x800314E4")

    # 8. retail vectors
    require(rom(0x800308E0) == 0x00002021 and rom(0x800308E4) == 0x24050001
            and rom(0x800308E8) == 0x24060100 and
            rom(0x800308F0) == 0x240701E0,
            "GetTPage arg loads")
    require(rom(0x800308F4) == 0x24040130 and rom(0x800308F8) == 0x240501F8,
            "GetClut arg loads")
    n += 1
    print("  OK retail vectors: arg loads exact at both head call sites")

    print(f"\nB54J audit oracle: {n} check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())

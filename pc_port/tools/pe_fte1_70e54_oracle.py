#!/usr/bin/env python3
"""Phase 6E-FTE1 — func_80070E54 frame tail + func_80042FE8 LoadImage wrapper."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
CANDIDATE = ROOT / "build" / "disc1.candidate.exe"
EXTRACTED = ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62"
TADDR = 0x80010000
HDR = 0x800


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def imm16(blob: bytes, va: int) -> int:
    return load_u32(blob, va) & 0xFFFF


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    options = ([pathlib.Path(sys.argv[1])] if len(sys.argv) > 1
               else [CANDIDATE, EXTRACTED])
    blob = None
    used = None
    for exe in options:
        if exe.is_file() and hashlib.sha1(exe.read_bytes()).hexdigest() == SHA1:
            blob = exe.read_bytes()
            used = exe
            break
    require(blob is not None, f"no SHA-1-exact EXE among {options}")
    print(f"authority: {used}")

    # ── func_80070E54: 86 words, 0x80070E54..0x80070FAC ──
    require((0x80070FAC - 0x80070E54) // 4 == 86, "70E54 86w")
    require(
        window_sha(blob, 0x80070E54, 0x80070FAC)
        == "64c882e893f33f306cbd7ea90d1c2b6d43933bd6d5312f6079090ccd35310537",
        "70E54 sha",
    )
    calls = {
        0x80070E5C: 0x80074DC0,  # DrawSync
        0x80070E64: 0x80042FE8,
        0x80070E84: 0x80073A44,  # VSync(4)
        0x80070E8C: 0x8006EBE4,
        0x80070EA8: 0x80074D28,  # SetDispMask(1)
        0x80070EB8: 0x80073A44,  # VSync(2)
        0x80070EC0: 0x80074A44,  # (1)
        0x80070EE4: 0x800755F0,  # PutDispEnv
        0x80070EEC: 0x8006EC08,
        0x80070F40: 0x80075424,  # PutDrawEnv
        0x80070F84: 0x800754E4,  # DrawOTagEnv
    }
    for site, target in calls.items():
        require(jal_target(load_u32(blob, site)) == target,
                f"jal @{site:08X} -> {target:08X}")
    require(imm16(blob, 0x80070E60) == 0x2021 and load_u32(blob, 0x80070E60) == 0x00002021,
            "DrawSync arg 0")
    require(imm16(blob, 0x80070E78) == 0x200, "andi 0x200 (first)")
    require(imm16(blob, 0x80070F10) == 0x200, "andi 0x200 (second)")
    require(imm16(blob, 0x80070E88) == 4, "VSync(4)")
    require(imm16(blob, 0x80070EBC) == 2, "VSync(2)")
    require(imm16(blob, 0x80070E9C) == 3, "slti 3 on 6EBE4 result")
    require(load_u32(blob, 0x80070E94) == 0x00021400, "sll 16 (short)")
    require(load_u32(blob, 0x80070E98) == 0x00021403, "sra 16 (short)")
    require(imm16(blob, 0x80070EAC) == 1, "SetDispMask(1)")
    require(imm16(blob, 0x80070EC4) == 1, "74A44(1)")
    require(imm16(blob, 0x80070EC8) == 0x6C, "lw CDDC gp+0x6C")
    # stride 20 = (x<<2 + x)<<2
    require(load_u32(blob, 0x80070ED0) == 0x00022080, "sll 2")
    require(load_u32(blob, 0x80070ED4) == 0x00822021, "addu")
    require(load_u32(blob, 0x80070ED8) == 0x00042080, "sll 2 -> *20")
    require(imm16(blob, 0x80070EE0) == 0xCE80, "%lo(D_800BCE80)")
    require(load_u32(blob, 0x80070EF4) == 0x00021600, "sll 24 low-byte test")
    # stride 92 = (((x<<1 + x)<<3) - x)<<2
    require(load_u32(blob, 0x80070F24) == 0x00022040, "sll 1")
    require(load_u32(blob, 0x80070F2C) == 0x000420C0, "sll 3")
    require(load_u32(blob, 0x80070F30) == 0x00822023, "subu")
    require(load_u32(blob, 0x80070F34) == 0x00042080, "sll 2 -> *92")
    require(imm16(blob, 0x80070F3C) == 0xCDC8, "%lo(D_800BCDC8)")
    require(imm16(blob, 0x80070F74) == 0x160, "lw 0x160(B0CD8+4*CDDC) = B0E38[CDDC]")
    require(imm16(blob, 0x80070F88) == 0x3FFC, "OT tail +0x3FFC")
    require(load_u32(blob, 0x80070F94) == 0x2C420001, "sltiu 1 (CDDC flip)")
    require(imm16(blob, 0x80070F98) == 0x6C, "sw CDDC gp+0x6C")
    require(imm16(blob, 0x80070E70) == 0x0CD8 and imm16(blob, 0x80070F04) == 0x0CD8,
            "%lo(D_800B0CD8)")

    # ── func_80042FE8: 20 words, 0x80042FE8..0x80043038 ──
    require((0x80043038 - 0x80042FE8) // 4 == 20, "42FE8 20w")
    require(
        window_sha(blob, 0x80042FE8, 0x80043038)
        == "b9d03c0f89d7481fcccccc4406053dcd4fe27443287cfbe677a5da86d838c303",
        "42FE8 sha",
    )
    require(imm16(blob, 0x80042FEC) == 0x168, "lw gp+0x168 (D_8009CED8)")
    require(imm16(blob, 0x80042FF0) == 6, "== 6 gate")
    require(imm16(blob, 0x80042FFC) == 0x1E0, "rect.y 0x1E0")
    require(imm16(blob, 0x80043004) == 0x100, "rect.w 0x100")
    require(imm16(blob, 0x80043008) == 0x16C, "lw gp+0x16C (D_8009CEDC) -> rect.h")
    require(imm16(blob, 0x80043010) == 0x0E54, "%lo(D_800B0E54) data")
    require(load_u32(blob, 0x80043018) == 0xA7A00010, "sh zero rect.x")
    require(jal_target(load_u32(blob, 0x80043020)) == 0x8007506C, "LoadImage")

    # ── 3F3C4 routing: retail jal 70E54 at 0x8003F590 ──
    require(jal_target(load_u32(blob, 0x8003F590)) == 0x80070E54, "3F3C4 jal 70E54")
    require(jal_target(load_u32(blob, 0x8006EB54)) == 0x80070E54, "6E9A0 jal 70E54")
    print("PASS: 70E54 window, 11 jal sites, immediates/strides; 42FE8 window; 3F590 routing")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

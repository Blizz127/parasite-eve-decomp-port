#!/usr/bin/env python3
"""Phase 6E-OTC1 — ClearOTagR (752AC) + jtb[11] OTC worker (76354)."""
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
    require((0x80075358 - 0x800752AC) // 4 == 43, "752AC 43w")
    require((0x80076434 - 0x80076354) // 4 == 56, "76354 56w")
    require(
        window_sha(blob, 0x800752AC, 0x80075358)
        == "23468418497f4d936880590efa0f807543a7a7aed8744ee5c90453e04d16e1dc",
        "752AC sha",
    )
    require(
        window_sha(blob, 0x80076354, 0x80076434)
        == "b7a681b46e8bcbbb20ac3156a83d9adb1718abeb7b68ff7523324a1e0e0dcacf",
        "76354 sha",
    )
    require(
        window_sha(blob, 0x800773D0, 0x80077404)
        == "3b0637e8b6472855bf28115f4712ea1f43c3d3ad130c388ab71c8101d3d00cb5",
        "773D0 sha",
    )
    require(jal_target(load_u32(blob, 0x800763C4)) == 0x800773D0, "773D0 call")
    require(jal_target(load_u32(blob, 0x800763F0)) == 0x80077404, "77404 call")
    require(load_u32(blob, 0x80095704 + 0x2C) == 0x80076354, "jtb[11]")
    require(load_u32(blob, 0x80095864) == 0x1F8010E0, "D6_MADR ptr")
    require(load_u32(blob, 0x80095868) == 0x1F8010E4, "D6_BCR ptr")
    require(load_u32(blob, 0x8009586C) == 0x1F8010E8, "D6_CHCR ptr")
    require(load_u32(blob, 0x80095870) == 0x1F8010F0, "DPCR ptr")
    require(
        blob[exe_off(0x80011910) : exe_off(0x80011910) + 24]
        == b"ClearOTagR(%08x,%d)...\n\x00",
        "debug name",
    )
    # Load-bearing immediates, decoded from the retail words themselves.
    require(load_u32(blob, 0x800752C8) & 0xFFFF == 2, "gate sltiu 2")
    require(load_u32(blob, 0x80075310) & 0xFFFF == 0xFF, "tail lui 0xFF")
    require(load_u32(blob, 0x80075314) & 0xFFFF == 0xFFFF, "tail ori")
    require(load_u32(blob, 0x8007531C) & 0xFFFF == 0x8009, "580C hi")
    require(load_u32(blob, 0x80075320) & 0xFFFF == 0x580C, "580C lo")
    require(load_u32(blob, 0x80075324) & 0xFFFF == 0x8009, "57F8 hi")
    require(load_u32(blob, 0x80075328) & 0xFFFF == 0x57F8, "57F8 lo")
    require(load_u32(blob, 0x80075330) & 0xFFFF == 0x0400, "flag 0x400000")
    require(load_u32(blob, 0x80075300) & 0xFFFF == 0x2C, "jtb[11] offset")
    require(load_u32(blob, 0x80076374) & 0xFFFF == 0x0800, "DPCR bit27")
    require(load_u32(blob, 0x80076390) >> 6 & 0x1F == 2, "n<<2 shift")
    require(load_u32(blob, 0x80076394) & 0xFFFF == 0xFFFC, "minus 4")
    require(load_u32(blob, 0x800763B0) & 0xFFFF == 0x1100, "CHCR hi")
    require(load_u32(blob, 0x800763C0) & 0xFFFF == 0x0002, "CHCR lo")
    require(load_u32(blob, 0x800763DC) & 0xFFFF == 0x0100, "busy mask")
    print("PASS: ClearOTagR + OTC worker windows, calls, jtb[11], D6 ptrs")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

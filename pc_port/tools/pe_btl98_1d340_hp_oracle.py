#!/usr/bin/env python3
"""PE-BTL98 — 0x95 mode 0, 2A4FC jal 1D340, 1F704 HP subtract."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SHA_192B8 = "cf7731ecd995a5cf453d63fc4c58ac003d9cad9143d039f24beb00085d286897"
SHA_71A54 = "db6da1122438031261cce8eaabe41b8050249f6a6fe2133f8447ea15eba6cffa"
SHA_1F4D4 = "4eb5c818c8f5e76c75968ea5014e4b4d138b3b6be1182e06d688beb5d0accb02"
ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
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
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def jal_sites(blob: bytes, target: int) -> list[int]:
    want = 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)
    text = blob[HDR : HDR + 0x1EE000]
    return [TADDR + i for i in range(0, len(text), 4)
            if struct.unpack_from("<I", text, i)[0] == want]


def window_sha(blob: bytes, lo: int, hi: int) -> str:
    return hashlib.sha256(blob[exe_off(lo) : exe_off(hi)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x800910A0 + 0x95 * 4) == 0x800192B8, "table[0x95]")
    require(load_u32(blob, 0x800192BC) == 0xAC20D28C, "192BC sw zero D28C")
    require(window_sha(blob, 0x800192B8, 0x800192C8) == SHA_192B8, "192B8 sha")
    require(jal_sites(blob, 0x8001D340) == [0x8002A4FC], "1D340 sole jal")
    require(jal_target(load_u32(blob, 0x8002A4FC)) == 0x8001D340, "2A4FC jal")
    require(load_u32(blob, 0x8002A500) == 0x02202021, "a0=s1")
    require(load_u32(blob, 0x800299F0) == 0x24110001, "s1=1")
    require(jal_target(load_u32(blob, 0x8001E7F4)) == 0x8001F4D4, "1D340 jal 1F4D4")
    require(load_u32(blob, 0x8001F700) == 0x00501023, "1F704 subu")
    require(load_u32(blob, 0x8001F704) == 0xA482000C, "1F704 sh +0x0C")
    require(window_sha(blob, 0x8001F4D4, 0x8001F814) == SHA_1F4D4, "1F4D4 sha")
    require(window_sha(blob, 0x80071A54, 0x80071A60) == SHA_71A54, "71A54 sha")
    require(load_u32(blob, 0x80071A54) == 0x240A00A0, "A-table")
    require(load_u32(blob, 0x80071A5C) == 0x2409002F, "A(2Fh)")
    print("PASS: 0x95 mode 0; 2A4FC jal 1D340(s1=1); 1F704 sh HP-s0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

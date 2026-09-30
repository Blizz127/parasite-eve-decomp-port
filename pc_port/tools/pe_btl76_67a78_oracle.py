#!/usr/bin/env python3
"""PE-BTL76 — 67A78 / 67294 / 67B74 / 67D18 68CE0 tails."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
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
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require((0x80067B40 - 0x80067A78) // 4 == 50, "67A78 50w")
    require(jal_target(load_u32(blob, 0x80068D00)) == 0x80067A78, "68D00")
    require(jal_target(load_u32(blob, 0x80067B08)) == 0x80067294, "67B08")
    require((0x80067678 - 0x80067294) // 4 == 249, "67294 249w")
    require((0x80067CBC - 0x80067B74) // 4 == 82, "67B74 82w")
    require((0x80067E1C - 0x80067D18) // 4 == 65, "67D18 65w")
    require(load_u32(blob, 0x80067B80) == 0x30420400, "67B74 andi 0x400")
    require(load_u32(blob, 0x80067D24) == 0x30621000, "67D18 andi 0x1000")
    require(
        window_sha(blob, 0x80067A78, 0x80067B40)
        == "cc38093d88eb8c3b67b7687484a4036d783d3cccb9cb61e8f2568671451f2759",
        "67A78 sha",
    )
    require(
        window_sha(blob, 0x80067294, 0x80067678)
        == "bb61228af0e1f6a63299f2442d05b5fc705499fb86e48d67d4e0133ebcacbdc6",
        "67294 sha",
    )
    print("PASS: 67A78 50w jal 67294; 67B74/67D18 flag outs")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

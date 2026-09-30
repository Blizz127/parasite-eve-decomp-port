#!/usr/bin/env python3
"""PE-BTL61 — 0xC7/0xAD/0x94/0x8B ROM contract."""
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

    cases = (
        (0xC7, 0x80019BE4, 0x80019C04, "ad1774026537b3de2fff5adfa0b0815f0460a0d1b5de64e3bf860c7519fa043a"),
        (0xAD, 0x80019748, 0x80019768, "6776e7ebd6583acc0c697c7b4f19ddcffcbbd726fe217f2a582e216d611e4327"),
        (0x94, 0x80019154, 0x80019170, "d126b80d56e74bf89eb7c9e12b71f6f67b60136788b640a2a811f7bec72e8c97"),
        (0x8B, 0x80018080, 0x80018164, "a896f3b9839e239002929b4239ef244ab7ecc2836c606f2e28ec7712a57bd1c7"),
    )
    for op, va, end, sha in cases:
        require(load_u32(blob, 0x800910A0 + op * 4) == va, f"table[{op:#x}]")
        require((end - va) % 4 == 0, f"{va:#x} align")
        require(window_sha(blob, va, end) == sha, f"{va:#x} sha")

    require(load_u32(blob, 0x80019BE4) == 0x8F830590, "lw D300 gp+0x590")
    require(load_u32(blob, 0x80019BF4) == 0x34420080, "ori 0x80")
    require(load_u32(blob, 0x80019750) == 0x2403FFFB, "addiu ~4")
    require(load_u32(blob, 0x80019754) == 0x00431024, "and D2E8")
    require(jal_target(load_u32(blob, 0x800180B4)) == 0x8002FE78, "jal 2FE78")
    require(jal_target(load_u32(blob, 0x80018138)) == 0x8003010C, "jal 3010C")
    require(load_u32(blob, 0x800180C8) == 0x8C84D20C, "lw D20C")
    require(load_u32(blob, 0x80018108) == 0x30420010, "andi +0x98 0x10")
    require(load_u32(blob, 0x80018124) == 0x1080000A, "miss skips dest store")
    print("PASS: 0xC7/0xAD/0x94/0x8B type-6 scratch-clear path")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

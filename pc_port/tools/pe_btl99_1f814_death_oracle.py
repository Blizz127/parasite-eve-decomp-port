#!/usr/bin/env python3
"""PE-BTL99 — 1F814 after 1F704; 1D340 death when HP<=0."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SHA_1F814 = "f4060ee9542ccf77fe0fb2afacb6eb23fda64a10b6baea1e6a829e365e586f2c"
SHA_305C8 = "09592b85470e3c551ec3b0bdbb5a40591464f39dca59bbf1ecb27560d1fa38a8"
SHA_JTBL = "00f72fb3f50dce919f50576da2d01a766ddf4d6aa28c75bb501815d70aebe715"
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
    require(jal_target(load_u32(blob, 0x8001F724)) == 0x8001F814, "1F4D4 jal 1F814")
    require(load_u32(blob, 0x8001F714) == 0x8442000C, "1F708 lh +0x0C")
    require(load_u32(blob, 0x8001F71C) == 0x1040002E, "beqz skip 1F814")
    require(jal_sites(blob, 0x8001F814) == [0x8001EA20, 0x8001F724], "1F814 jals")
    require(window_sha(blob, 0x8001F814, 0x8001F9C4) == SHA_1F814, "1F814 sha")
    require(window_sha(blob, 0x800305C8, 0x80030640) == SHA_305C8, "305C8 sha")
    require(window_sha(blob, 0x800106E4, 0x8001070C) == SHA_JTBL, "jtbl sha")
    require(jal_target(load_u32(blob, 0x8001F900)) == 0x800305C8, "1F814 jal 305C8")
    require(jal_target(load_u32(blob, 0x8001F94C)) == 0x8001A680, "1F814 jal 1A680")
    require(jal_target(load_u32(blob, 0x800305EC)) == 0x80079FB4, "305C8 jal 79FB4")
    require(load_u32(blob, 0x8001F078) == 0x8442000C, "1F078 lh HP")
    require(load_u32(blob, 0x8001F080) == 0x1C40010C, "1F080 bgtz skip death")
    require(load_u32(blob, 0x8001F41C) == 0xAC22D28C, "1F41C sw mode 3")
    require(load_u32(blob, 0x8001F43C) == 0xA020D244, "1F43C sb 4D4=0")
    require(jal_target(load_u32(blob, 0x8001F49C)) == 0x8001A680, "death jal 1A680")
    require(load_u32(blob, 0x8001F4A0) == 0x24050013, "1A680 a1=19")
    require(load_u32(blob, 0x8001F4B0) == 0xA440000C, "1F4B0 sh zero")
    require(load_u32(blob, 0x800106E4) == 0x8001F860, "jtbl[0]")
    require(load_u32(blob, 0x800106E8) == 0x8001F898, "jtbl[1]")
    require(jal_target(load_u32(blob, 0x8001F970)) == 0x8006DE80, "1F814 jal 6DE80")
    print("PASS: 1F814→305C8/1A680/6DE80; 1F078 death arm is ROM-located, not 1F4D4")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

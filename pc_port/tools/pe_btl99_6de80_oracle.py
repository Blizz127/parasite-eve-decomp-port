#!/usr/bin/env python3
"""PE-BTL99 — 1F814 jal 6DE80 after 1A680; death 6DE80 is 0x46B."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SHA_6DE80 = "860d94bcbf926fb977614f7d917d69f919c20aee54126f58fbba90dc426529d9"
SHA_6DED4 = "5e8106d2411ef157268cd1e5dece35876914dde86064a02381c8eb58c022d7d2"
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


def window_sha(blob: bytes, lo: int, hi: int) -> str:
    return hashlib.sha256(blob[exe_off(lo) : exe_off(hi)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(jal_target(load_u32(blob, 0x8001F94C)) == 0x8001A680, "1F814 jal 1A680")
    require(load_u32(blob, 0x8001F95C) == 0x2404046A, "1F814 a0=0x46A")
    require(jal_target(load_u32(blob, 0x8001F970)) == 0x8006DE80, "1F814 jal 6DE80")
    require(load_u32(blob, 0x8001F960) == 0x8446002A, "lh Aya+0x2A")
    require(load_u32(blob, 0x8001F964) == 0x8447002E, "lh Aya+0x2E")
    require(load_u32(blob, 0x8001F968) == 0x84420032, "lh Aya+0x32")
    require(window_sha(blob, 0x8006DE80, 0x8006DED4) == SHA_6DE80, "6DE80 sha")
    require(jal_target(load_u32(blob, 0x8006DEBC)) == 0x8006DED4, "6DE80 jal 6DED4")
    require(load_u32(blob, 0x8006DEA8) == 0x8C840E08, "lw D_800B0E08")
    require(window_sha(blob, 0x8006DED4, 0x8006DF50) == SHA_6DED4, "6DED4 sha")
    require(jal_target(load_u32(blob, 0x8006DF10)) == 0x8006DFA8, "6DED4 jal 6DFA8")
    require(jal_target(load_u32(blob, 0x8006DF2C)) == 0x8006DF50, "6DED4 jal 6DF50")
    require(load_u32(blob, 0x8001F3E8) == 0x2404046B, "death a0=0x46B")
    require(jal_target(load_u32(blob, 0x8001F430)) == 0x8006DE80, "1F078 arm jal 6DE80")
    require(load_u32(blob, 0x8001F078) == 0x8442000C, "1F078 lh HP")
    require(load_u32(blob, 0x8001F080) == 0x1C40010C, "1F080 bgtz skip death")
    print("PASS: 1F814 jal 6DE80(0x46A); 6DE80→6DED4; death 6DE80 is 0x46B")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

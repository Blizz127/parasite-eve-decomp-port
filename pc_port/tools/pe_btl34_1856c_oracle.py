#!/usr/bin/env python3
"""PE-BTL34 independent oracle: 0x65 / 1856C pose-group clear."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((0x80018598 - 0x8001856C) // 4 == 11, "1856C 11w")
    require(
        window_sha(data, 0x8001856C, 0x80018598)
        == "309d956fb8cbcd2b225ff0cfa0a704dfc9c6262703803161610b2d743706c25f",
        "1856C sha",
    )
    require(load_u32(data, 0x800910A0 + 0x65 * 4) == 0x8001856C, "table[0x65]")
    require(load_u32(data, 0x80018570) == 0x8C42D2F0, "lw D2F0")
    require(load_u32(data, 0x80018578) == 0xAC400068, "sw +0x68")
    require(load_u32(data, 0x8001857C) == 0xAC40006C, "sw +0x6C")
    require(load_u32(data, 0x80018580) == 0xAC400070, "sw +0x70")
    require(load_u32(data, 0x80018584) == 0xAC400078, "sw +0x78")
    require(load_u32(data, 0x80018588) == 0xAC40007C, "sw +0x7C")
    require(load_u32(data, 0x8001858C) == 0xAC400080, "sw +0x80")
    require(load_u32(data, 0x80018594) == 0x24020001, "v0=1")

    print("PASS: 0x65 1856C zeros D2F0 +0x68/6C/70 and +0x78/7C/80; v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())

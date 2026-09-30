#!/usr/bin/env python3
"""PE-BTL57 independent oracle: 0x43 / 17DE4 and 37864."""
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

    require((0x80017E20 - 0x80017DE4) // 4 == 15, "17DE4 15w")
    require(
        window_sha(data, 0x80017DE4, 0x80017E20)
        == "71be0cb334c6b16f99d7e4e13fd8b3411fd488653645cdec22f87e14c76ac405",
        "17DE4 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x43 * 4) == 0x80017DE4, "table[0x43]")
    require(load_u32(data, 0x80017DF0) == 0x0C00DE19, "jal 37864")
    require((0x80037870 - 0x80037864) // 4 == 3, "37864 3w")
    require(load_u32(data, 0x80037864) == 0x83820134, "lb gp+0x134")
    require(load_u32(data, 0x800382C4) == 0xA3820134, "FB09 sb 134")
    require(load_u32(data, 0x80038238) == 0x30420020, "FB09 down 0x20")
    require(load_u32(data, 0x80038278) == 0x30420008, "FB09 up 0x08")

    print("PASS: 0x43/17DE4 + 37864 lb CEA4 + FB09 cursor")
    return 0


if __name__ == "__main__":
    sys.exit(main())

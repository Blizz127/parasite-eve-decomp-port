#!/usr/bin/env python3
"""PE-BTL28 independent oracle: 0x0C / 12E7C and 0xD9 / 1A15C / 79FB4."""
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


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((0x800130B4 - 0x80012E7C) // 4 == 142, "12E7C 142w")
    require(
        window_sha(data, 0x80012E7C, 0x800130B4)
        == "76d826404dee608ee0573c67d78ae203f889454ca1b923cfe22ae2f11ad93dc8",
        "12E7C sha",
    )
    require(load_u32(data, 0x800910A0 + 0x0C * 4) == 0x80012E7C, "table[0x0C]")
    require(load_u32(data, 0x80012E8C) == 0x2C620007, "sltiu 7")
    require(load_u32(data, 0x80010080) == 0x80012EB4, "jtbl[0]")
    require(load_u32(data, 0x80010094) == 0x8001301C, "jtbl[5]")
    require(load_u32(data, 0x80012EC0) == 0x8C420028, "code0 +0x28")
    require(load_u32(data, 0x80013028) == 0x84420038, "code5 lh +0x38")
    require(load_u32(data, 0x800130B0) == 0x24020001, "v0=1")

    require((0x8001A1A8 - 0x8001A15C) // 4 == 19, "1A15C 19w")
    require(
        window_sha(data, 0x8001A15C, 0x8001A1A8)
        == "f53fb5e6422ab77a4c98a3a103e01567f716d34db8439b84fac08af8eba49760",
        "1A15C sha",
    )
    require(load_u32(data, 0x800910A0 + 0xD9 * 4) == 0x8001A15C, "table[0xD9]")
    require(jal_target(load_u32(data, 0x8001A17C)) == 0x80079FB4, "jal 79FB4")

    require((0x8007A128 - 0x80079FB4) // 4 == 93, "79FB4 93w")
    require(
        window_sha(data, 0x80079FB4, 0x8007A128)
        == "e5b0edc7308d715c3fd821bd7f75478555d6937ec97c9a8278f6c7746ad0f820",
        "79FB4 sha",
    )
    require(load_u32(data, 0x80079FE0) == 0x1080004F, "both-zero return")
    require(load_u32(data, 0x8007A0F8) == 0x8463A6EC, "lh D_8009A6EC")

    print("PASS: 0x0C 12E7C read-twin; 0xD9 1A15C jal 79FB4 ratan2")
    return 0


if __name__ == "__main__":
    sys.exit(main())

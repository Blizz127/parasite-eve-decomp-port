#!/usr/bin/env python3
"""PE-BTL17 independent oracle: 17588 label-pointer opcode 0x14.

Pins SHA-1-exact EXE. 76 words. Cases 1/2/3 write actor+0x1A0,
actor+0x19C, or task+4. Does not import production C.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x80017588
END = 0x800176B8
SHA = "f4bc3706ce9c2c2c6e913981cbfc512c86adf2b05d4deee411265dc42c8a5e69"


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
    require((END - FN) // 4 == 76, "17588 76 words")
    require(window_sha(data, FN, END) == SHA, "17588 sha")
    require(load_u32(data, 0x800910A0 + 0x14 * 4) == FN, "table[0x14]")
    require(load_u32(data, 0x80017598) == 0x04A1001D, "bgez *arg1")
    require(load_u32(data, 0x80017600) == 0xAC40019C, "neg code2 sw +0x19C")
    require(load_u32(data, 0x80017690) == 0xAC43019C, "pos code2 sw +0x19C")
    require(load_u32(data, 0x800176B4) == 0x24020001, "v0=1")
    print("PASS: 17588 76w op 0x14; code 2 → actor+0x19C; v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())

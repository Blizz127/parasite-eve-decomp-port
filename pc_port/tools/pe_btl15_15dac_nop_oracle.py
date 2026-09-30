#!/usr/bin/env python3
"""PE-BTL15 independent oracle: 15DAC nop keys and ops 0xA / 0x1D.

Pins SHA-1-exact EXE. 15DAC is 727 words. Keys 0x193/0x194 jump to
the v0=1 epilogue. 173F4 is 7 words; 17E20 is 18 words. Does not
import production C. Does not invent bytecode. 0x190 is BTL18.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN_EA = 0x80015DAC
END_EA = 0x80016908
SHA_EA = "bd05f480e8eb9967d3b2b21283e8ad48d2dc52eafe35c36f36744a9bcbf5a978"
TABLE = 0x800101B0
NOP = 0x800168F4
FN_A = 0x800173F4
END_A = 0x80017410
SHA_A = "daa92cf0962ef4b106d29b0ed9d51cb0eecaacd00c752ff08ccda4a317c4cb00"
FN_1D = 0x80017E20
END_1D = 0x80017E68
SHA_1D = "7362ce6819e9b481f54d12e283ed211035c326770006e13e96e508878010290f"


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
    require((END_EA - FN_EA) // 4 == 727, "15DAC 727 words")
    require(window_sha(data, FN_EA, END_EA) == SHA_EA, "15DAC sha")
    require(load_u32(data, 0x800910A0 + 0xEA * 4) == FN_EA, "table[0xEA]")
    require(load_u32(data, 0x80015DD4) == 0x2C620137, "sltiu 311")
    require(load_u32(data, TABLE + (0x194 - 100) * 4) == NOP, "0x194 nop")
    require(load_u32(data, TABLE + (0x193 - 100) * 4) == NOP, "0x193 nop")
    require(load_u32(data, TABLE + (0x190 - 100) * 4) == 0x80016658, "0x190 real")
    require(load_u32(data, NOP) == 0x24020001, "nop addiu v0,1")
    require((END_A - FN_A) // 4 == 7, "173F4 7 words")
    require(window_sha(data, FN_A, END_A) == SHA_A, "173F4 sha")
    require(load_u32(data, 0x800910A0 + 0xA * 4) == FN_A, "table[0xA]")
    require(load_u32(data, 0x80017404) == 0xAC620000, "173F4 sw")
    require(load_u32(data, 0x8001740C) == 0x24020001, "173F4 v0=1")
    require((END_1D - FN_1D) // 4 == 18, "17E20 18 words")
    require(window_sha(data, FN_1D, END_1D) == SHA_1D, "17E20 sha")
    require(load_u32(data, 0x800910A0 + 0x1D * 4) == FN_1D, "table[0x1D]")
    require(load_u32(data, 0x80017E58) == 0x00621821, "17E20 addu base")
    require(load_u32(data, 0x80017E5C) == 0xAF830090, "17E20 sw gp+0x90")
    print(
        "PASS: 15DAC 727w; 0x193/0x194→168F4 v0=1; 0x190→16658; "
        "173F4 7w store v0=1; 17E20 18w cond CE00"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

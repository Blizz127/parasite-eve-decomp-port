#!/usr/bin/env python3
"""PE-BTL7 independent oracle: 3F074 6C5BC poll vs 144FC 0x3B.

Pins SHA-1-exact EXE. 3F074 jals 6C4C4(CE4) with s0=1, then
jal 6C5BC / beq v0,s0 back — tight poll until 6C5BC returns 0.
EE=0 + bit1: sb EE=11, j dispatcher (same call). EE=11: sb 12,
ori +0xE bit0, return 1. EE=12: sb 13, return 1. EE=13 returns 1
after 6CC2C. EE=0 with bits clear: jal 6CC68, return 0 — that is
the poll exit. EE=13 returning 1 is not 0x55 complete. 144FC 0x3B
has no jal of 6C5BC. Does not import production C.
"""
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


def j_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require(load_u32(data, 0x8003F220) == 0x80840CE4, "3F074 lb CE4")
    require(jal_target(load_u32(data, 0x8003F224)) == 0x8006C4C4, "jal 6C4C4")
    require(load_u32(data, 0x8003F228) == 0x24100001, "s0=1")
    require(jal_target(load_u32(data, 0x8003F22C)) == 0x8006C5BC, "jal 6C5BC")
    require(load_u32(data, 0x8003F234) == 0x1050FFFD, "beq v0,s0 poll")

    require(load_u32(data, 0x8006C638) == 0x9283000E, "EE0 lbu +0xE")
    require(load_u32(data, 0x8006C640) == 0x30620001, "EE0 andi bit0")
    require(load_u32(data, 0x8006C654) == 0x30620002, "EE0 andi bit1")
    require(load_u32(data, 0x8006C65C) == 0x2402000B, "EE0 delay v0=11")
    require(j_target(load_u32(data, 0x8006C9C4)) == 0x8006C60C, "bit1 j dispatch")
    require(load_u32(data, 0x8006C9C8) == 0xA28200EE, "bit1 sb EE=11")
    require(jal_target(load_u32(data, 0x8006C660)) == 0x8006CC68, "EE0 idle jal 6CC68")
    require(load_u32(data, 0x8006C66C) == 0x00001021, "EE0 idle v0=0")

    require(load_u32(data, 0x8006C9D8) == 0xA28400EE, "EE11 sb 12")
    require(load_u32(data, 0x8006C9DC) == 0x34630001, "EE11 ori bit0")
    require(j_target(load_u32(data, 0x8006C9E0)) == 0x8006CC3C, "EE11 return")
    require(load_u32(data, 0x8006C9E8) == 0x2402000D, "EE12 v0=13")
    require(load_u32(data, 0x8006C9F4) == 0x24020001, "EE12 return 1")
    require(load_u32(data, 0x8006CC38) == 0x00001021, "join v0=0")

    jal_6c5bc = 0x0C01B16F
    for va in range(0x800144FC, 0x80014694, 4):
        require(load_u32(data, va) != jal_6c5bc, f"144FC jal 6C5BC @{va:#x}")

    require(jal_target(load_u32(data, 0x8003F23C)) == 0x8001A918, "poll-exit jal 1A918")
    require(jal_target(load_u32(data, 0x8003F284)) == 0x800E0060, "poll-exit jal E0060")
    require((0x8003F2FC - 0x8003F074) // 4 == 162, "3F074 162 words")

    print(
        "PASS: 3F074 s0=1 poll 6C5BC; EE0 bit1→11 redisp; "
        "EE11/12 return 1; EE0 idle jal 6CC68 v0=0; after-poll "
        "1A918 then E0060 (loaded); 144FC no jal"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

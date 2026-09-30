#!/usr/bin/env python3
"""PE-BTL6 independent oracle: func_8006CDA4 +0xF0 state machine.

Pins SHA-1-exact EXE. 181 words, no +0xE store. Live 6D078 0x28 is
a0=1 a1=1: state 0 skips 87198/87414, sets state7 then continues. State7 jals
6E6D4. Checks instruction encodings only, not the complete internal loop. See
pe_disc_loader_loop_oracle.py for full control-flow execution. Does not import production C. Does not claim 6CDA4/6E6D4
success, 6914C success, mode 7, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x8006CDA4
FN_END = 0x8006D078
JT = 0x80011428
TABLE = 0x8009317C
WIN_SHA256 = (
    "c94c08adeed015909c745201d6c1f142f9966204b52470fc0fc0778fce3c8f79"
)


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def load_u16(data: bytes, addr: int) -> int:
    return struct.unpack_from("<H", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start):exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((FN_END - FN) // 4 == 181, "6CDA4 181 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA256, "6CDA4 sha256")
    require(load_u32(data, FN + 8) == 0x0080A021, "s4=a0")
    require(load_u32(data, FN + 0x20) == 0x3C12800B, "s2 overlay")
    require(load_u32(data, FN + 0x24) == 0x26520CD8, "s2 +0xCD8")
    require(load_u32(data, FN + 0x3C) == 0x00052840, "sll a1,1")
    require(load_u32(data, FN + 0x6C) == 0x924300F0, "lbu +0xF0")
    require(load_u32(data, FN + 0x74) == 0x2C62000B, "sltiu 11")
    require(load_u32(data, FN + 0x88) == 0x8C221428, "lw JT 0x80011428")

    require(load_u32(data, JT + 0) == 0x8006CE3C, "JT[0]")
    for i in range(1, 7):
        require(load_u32(data, JT + i * 4) == 0x8006D03C, f"JT[{i}] epilogue")
    require(load_u32(data, JT + 7 * 4) == 0x8006CEC4, "JT[7]")
    require(load_u32(data, JT + 8 * 4) == 0x8006CF28, "JT[8]")
    require(load_u32(data, JT + 9 * 4) == 0x8006CF54, "JT[9]")
    require(load_u32(data, JT + 10 * 4) == 0x8006CFE8, "JT[0xA]")

    require(jal_target(load_u32(data, 0x8006CE84)) == 0x80087198, "a0==0 jal 87198")
    require(jal_target(load_u32(data, 0x8006CE9C)) == 0x80087414, "a0==3 jal 87414")
    require(load_u32(data, 0x8006CEC0) == 0xA24200F0, "state0 sb +0xF0")
    require(load_u32(data, 0x8006CEAC) == 0x24020007, "state0 li 7")
    require(jal_target(load_u32(data, 0x8006CEFC)) == 0x8006E6D4, "state7 jal 6E6D4")
    require(jal_target(load_u32(data, 0x8006CF28)) == 0x8006E7E8, "state8 jal 6E7E8")

    require(load_u32(data, TABLE) == 0x000008B0, "table word0")
    require(load_u16(data, TABLE + 6) == 0x0016, "table +6 a1=1 half0")
    require(load_u16(data, TABLE + 8) == 0x0025, "table +8 a1=1 half6")

    for va in range(FN, FN_END, 4):
        word = load_u32(data, va)
        if (word >> 26) in (0x28, 0x29, 0x2B) and (word & 0xFFFF) == 0x000E:
            raise SystemExit(f"FAIL: 6CDA4 stores +0xE @{va:#x}")

    require(jal_target(load_u32(data, 0x8006D10C)) == FN, "6D078 0x28 jal 6CDA4")
    require(load_u32(data, 0x8006D0F4) == 0x24040001, "0x28 a0=1")
    require(load_u32(data, 0x8006D0F8) == 0x24050001, "0x28 a1=1")
    require(load_u32(data, 0x8006D100) == 0x8E070194, "0x28 lw dest +0x194")
    require(load_u32(data, 0x8006CE04) == 0x2416FFFF, "s6=-1")
    require(load_u32(data, 0x8006CEEC) == 0x8F840400, "state7 a0 gp+0x400")
    require(load_u32(data, 0x8006CEF0) == 0x8F850404, "state7 a1 gp+0x404")
    require(load_u32(data, 0x8006CF00) == 0x00A32823, "state7 a1 -= remain")
    require(load_u32(data, 0x8006CF18) == 0xA24000F0, "state7 zero remaining sb F0=0")
    require((load_u32(data, 0x8006CF08) & 0xFFFF) * 4 + 0x8006CF0C == 0x8006D00C,
            "read failure branches to retry/yield, not archive completion")
    require(load_u32(data, 0x8006CF14) == 0xA24200F0, "state7 ok sb F0")

    print(
        "PASS: 6CDA4 181w sha256; +0xF0 JT 0/7/8/9/A; live a0=1 sb 7 "
        "skips 87198/87414; state7 6E6D4(LBA,0,lw+0x194,0x0F); no +0xE"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

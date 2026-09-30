#!/usr/bin/env python3
"""Independent retail oracle for the BIOS A0(44h) FlushCache veneer."""

from __future__ import annotations

import hashlib
import pathlib
import struct

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8000F800
TARGET = 0x800726C4
CALLERS = (
    0x80014EE0, 0x80014F50, 0x8006B78C, 0x8006E938,
    0x8006F010, 0x8006F148, 0x8006F1F8, 0x8006F460,
    0x8007A1BC, 0x8007E280, 0x8007E2E8, 0x8007E454,
    0x8007E4C4, 0x8007E560,
)


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x03FFFFFF) << 2)


def branch_target(pc: int, word: int) -> int:
    imm = word & 0xFFFF
    if imm & 0x8000:
        imm -= 0x10000
    return pc + 4 + imm * 4


def main() -> None:
    root = pathlib.Path(__file__).resolve().parents[2]
    path = root / "build" / "disc1.candidate.exe"
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == EXE_SHA1, "executable SHA-1")

    def word(address: int) -> int:
        return struct.unpack_from("<I", data, address - BASE)[0]

    require(tuple(word(TARGET + i * 4) for i in range(3)) ==
            (0x240A00A0, 0x01400008, 0x24090044),
            "A0(44h) veneer words")
    require(word(TARGET - 4) == 0 and word(TARGET + 12) == 0,
            "veneer boundary padding")
    print("  OK veneer: addiu t2,0xA0 / jr t2 / addiu t1,0x44")

    jal = 0x0C000000 | ((TARGET >> 2) & 0x03FFFFFF)
    found = tuple(BASE + off for off in range(0, len(data) - 3, 4)
                  if struct.unpack_from("<I", data, off)[0] == jal)
    require(found == CALLERS, "exact executable caller census")
    print(f"  OK caller census: {len(found)} exact jal sites")

    require(jal_target(word(0x800122C4)) == 0x8006AD40,
            "main-loop func_8006AD40 call")
    require(word(0x800122CC) == 0x3C03800A and
            word(0x800122D0) == 0x8C63D280 and
            word(0x800122D8) == 0xAC23D1C4,
            "main-loop destination load/publish")
    require(branch_target(0x800122DC, word(0x800122DC)) == 0x800123AC,
            "canonical A9400048 dispatch branch")
    require(jal_target(word(0x800123AC)) == 0x8006E834 and
            jal_target(word(0x800123B4)) == 0x801909B4,
            "canonical loader then overlay boundary")
    print("  OK caller path: 6AD40 -> D_8009D280 publish -> 6E834 -> 801909B4")

    require(jal_target(word(0x8006E930)) == 0x80072714 and
            jal_target(word(0x8006E938)) == TARGET and
            jal_target(word(0x8006E940)) == 0x80072724,
            "6E834 critical/FlushCache sequence")
    print("  OK 6E834 sequence: EnterCriticalSection / FlushCache / ExitCriticalSection")
    print("\nB54K-N FlushCache oracle: PASS (retail identity and caller path).")


if __name__ == "__main__":
    main()

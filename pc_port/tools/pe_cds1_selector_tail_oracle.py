#!/usr/bin/env python3
"""Phase 6E-CDS1 — func_8007F0C8 completion-selector tail + func_8007E8F4 /
func_8007FB44 dispatch predicates."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
CANDIDATE = ROOT / "build" / "disc1.candidate.exe"
EXTRACTED = ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62"
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
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    options = ([pathlib.Path(sys.argv[1])] if len(sys.argv) > 1
               else [CANDIDATE, EXTRACTED])
    blob = None
    used = None
    for exe in options:
        if exe.is_file() and hashlib.sha1(exe.read_bytes()).hexdigest() == SHA1:
            blob = exe.read_bytes()
            used = exe
            break
    require(blob is not None, f"no SHA-1-exact EXE among {options}")
    print(f"authority: {used}")
    # Window sizes from the retail labels.
    require((0x8007F3EC - 0x8007F394) // 4 == 22, "F394 tail 22w")
    require((0x8007E964 - 0x8007E8F4) // 4 == 28, "7E8F4 28w")
    require((0x8007FBC0 - 0x8007FB44) // 4 == 31, "7FB44 31w")
    require(
        window_sha(blob, 0x8007F394, 0x8007F3EC)
        == "63fc18b28fe7167efb7afdd138857076a0f5808c0d327bc50393b53986c374f8",
        "F394 tail sha",
    )
    require(
        window_sha(blob, 0x8007E8F4, 0x8007E964)
        == "2fb3278284e1555eba10344509d0949c1d95cb2cd5375c24f47d59f2d112b19b",
        "7E8F4 sha",
    )
    require(
        window_sha(blob, 0x8007FB44, 0x8007FBC0)
        == "5ef33d987f04b129c9221d40950b28c74065a38f0bbee7e999902fb6c0892cc5",
        "7FB44 sha",
    )
    # jal targets: FBF0 gate, 7E8F4 issue, 7FB44 predicate, 7FCFC worker.
    require(jal_target(load_u32(blob, 0x8007F394)) == 0x8007FBF0,
            "F394 FBF0 call")
    require(jal_target(load_u32(blob, 0x8007F3D8)) == 0x8007E8F4,
            "7E8F4 call")
    require(jal_target(load_u32(blob, 0x8007E948)) == 0x8007FB44,
            "7FB44 call")
    require(jal_target(load_u32(blob, 0x8007FB9C)) == 0x8007FCFC,
            "7FCFC call")
    # The two fall-through arms return seq via the delay slot
    # (asm comments print bytes MSB-first; these are the LE words).
    require(load_u32(blob, 0x8007F3A4) == 0x02A01021, "3A4 addu seq")
    require(load_u32(blob, 0x8007F3D4) == 0x02A01021, "3D4 addu seq")
    require(load_u32(blob, 0x8007F3E8) == 0x00001021, "3E8 addu zero")
    require(load_u32(blob, 0x8007E950) == 0x0002102B, "7E8F4 sltu tail")
    # Stride (*24 = sll1/addu/sll3) in both 7F0C8 and 7E8F4.
    require(load_u32(blob, 0x8007F3B4) == 0x00021840, "head sll 1")
    require(load_u32(blob, 0x8007F3B8) == 0x00621821, "head addu")
    require(load_u32(blob, 0x8007F3BC) == 0x000318C0, "head sll 3")
    require(load_u32(blob, 0x8007E920) == 0x00041040, "7E8F4 sll 1")
    require(load_u32(blob, 0x8007E924) == 0x00441021, "7E8F4 addu")
    require(load_u32(blob, 0x8007E928) == 0x000210C0, "7E8F4 sll 3")
    require(load_u32(blob, 0x8007E940) & 0xFFFF == 4, "rec byte offset")
    require(load_u32(blob, 0x8007E944) & 0xFFFF == 0x000C, "rec word offset")
    require(load_u32(blob, 0x8007FB84) & 0xFFFF == 0x001F, "7FB44 0x1F")
    require(load_u32(blob, 0x8007FB8C) & 0xFFFF == 2, "7FB44 lane 2")
    require(load_u32(blob, 0x8007FB94) & 0xFFFF == 0x000B, "7FB44 0xB")
    require(load_u32(blob, 0x8007FB98) & 0xFFFF == 0x00FF, "7FB44 cmd mask")
    print("PASS: F394/7E8F4/7FB44 windows, jal chain, seq-delay slots, "
          "stride/offsets")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

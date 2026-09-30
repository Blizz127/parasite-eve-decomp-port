#!/usr/bin/env python3
"""PE-BTL100 — 2A7F8 mode-3 is 2AA98 xor 2B29C; 2B0E8 is mode 2."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SHA_2AA98 = "41a7819e6525ce1bfb44eab5c13953844f73dcff107f7b6370f38170e061bb39"
SHA_2B29C = "6588e4c2fecc6265e4f4c4233642f565f7f0475fc3a20a935be567d8a54eacba"
SHA_2B0E8 = "9eabc01299179e888714b69f69487bac38ecd50f1f1e40f6ddcb8e632bfefd67"
SHA_53E6C = "44acf6631550e49ec52f8d905003e2ba123c250b1b24d8b6c10c52f9620e14db"
SHA_2A85C = "f78ab794c6f0c94d376d49caa294cd5df2be02b031b3d5087e670c7ec38b55c9"
SHA_JTBL_A = "f6f3019f1ab54ab3f6b9115e76e95012b8d324c195cf7e070fab6785464e3ee1"
SHA_JTBL_B = "a7462ca7efa81d5d1867c11982fcc24ae1fb7594124848709c5a93b9349dd26e"
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


def jal_sites(blob: bytes, target: int) -> list[int]:
    want = 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)
    text = blob[HDR : HDR + 0x1EE000]
    return [TADDR + i for i in range(0, len(text), 4)
            if struct.unpack_from("<I", text, i)[0] == want]


def window_sha(blob: bytes, lo: int, hi: int) -> str:
    return hashlib.sha256(blob[exe_off(lo) : exe_off(hi)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x8002A7F8) == 0x8F83051C, "2A7F8 lw mode")
    require(load_u32(blob, 0x8002A844) == 0x14620005, "mode!=2 → 2A85C")
    require(jal_target(load_u32(blob, 0x8002A84C)) == 0x8002B0E8, "mode 2 jal 2B0E8")
    require(jal_sites(blob, 0x8002B0E8) == [0x8002A84C], "2B0E8 sole jal")
    require(load_u32(blob, 0x8002A85C) == 0x14620055, "mode!=3 → 2A9B4")
    require(load_u32(blob, 0x8002A874) == 0x30420800, "andi bit 0x800")
    require(jal_target(load_u32(blob, 0x8002A880)) == 0x8002AA98, "bit set jal 2AA98")
    require(jal_target(load_u32(blob, 0x8002A8B0)) == 0x80053E6C, "53E6C(18)")
    require(load_u32(blob, 0x8002A8B4) == 0x24040012, "a0=18")
    require(jal_target(load_u32(blob, 0x8002A8C0)) == 0x8002AA98, "53E6C jal 2AA98")
    require(jal_target(load_u32(blob, 0x8002A8E4)) == 0x8002B29C, "else jal 2B29C")
    require(window_sha(blob, 0x8002A85C, 0x8002A9B4) == SHA_2A85C, "mode3 sha")
    require(window_sha(blob, 0x8002AA98, 0x8002B0E8) == SHA_2AA98, "2AA98 sha")
    require(window_sha(blob, 0x8002B29C, 0x8002B94C) == SHA_2B29C, "2B29C sha")
    require(window_sha(blob, 0x8002B0E8, 0x8002B29C) == SHA_2B0E8, "2B0E8 sha")
    require(window_sha(blob, 0x80053E6C, 0x80053F20) == SHA_53E6C, "53E6C sha")
    require(window_sha(blob, 0x800108F0, 0x80010910) == SHA_JTBL_A, "2AA98 jtbl")
    require(window_sha(blob, 0x80010910, 0x80010928) == SHA_JTBL_B, "2B29C jtbl")
    require(load_u32(blob, 0x800108F0) == 0x8002AAD8, "2AA98 jtbl[0]")
    require(load_u32(blob, 0x80010910) == 0x8002B2D4, "2B29C jtbl[0]")
    require(load_u32(blob, 0x8002AAE8) == 0x24020013, "2AA98 li 19")
    require(load_u32(blob, 0x8002B2E4) == 0x24020013, "2B29C li 19")
    require(jal_target(load_u32(blob, 0x8002B8E8)) == 0x800295E4, "2B29C[5] 295E4")
    require(load_u32(blob, 0x8002B8F0) == 0x2402FFFF, "2B29C[5] mode=-1")
    require(jal_target(load_u32(blob, 0x8002B8F8)) == 0x8006A25C, "2B29C[5] 6A25C")
    require(load_u32(blob, 0x8001F41C) == 0xAC22D28C, "1F41C mode 3")
    require(load_u32(blob, 0x8001F3E8) == 0x2404046B, "death 6DE80 a0=0x46B")
    print("PASS: mode3=2AA98 xor 2B29C; 2B0E8 is mode 2; 2B29C[5] mode=-1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

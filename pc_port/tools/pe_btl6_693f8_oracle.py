#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 6914C EF=0x34 issue at 0x800693F8.

Pins SHA-1-exact EXE. 0x34 jals 6E6A8(LBA=+0x100+lhu 930E2,
dest=+0x194, sectors=lhu 930E4-lhu 930E2). 6A8D4 dest is
0x801ED800. Table halves 0x7E/0x83 → 5 sectors. 6E6A8 is 11 words
→ 6E6D4(a0,0,a1,a2). Hashes PE.IMG [0x7E,0x83) when Disc 1 is
present. Does not import production C. Does not claim 6914C
success, overlay jalr, mode 7, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN_6E6A8 = 0x8006E6A8
END_6E6A8 = 0x8006E6D4
SHA_6E6A8 = (
    "94b56abec1c999413ca59e6b061419d153680c93db3aff6cbd0f63f0b58a4aba"
)
FN_6E6D4 = 0x8006E6D4
FN_6914C_34 = 0x800693F8
PE_IMG_LBA = 1013
REL = 0x7E
NSEC = 5
PAYLOAD_SHA256 = (
    "3b2ff0b8db2fefed21003ea3d87c559db728db9f316fec483a4715fdf0a1d3c9"
)
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048


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
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def find_disc(root: pathlib.Path) -> pathlib.Path | None:
    pointer = root / "local" / "pe_disc1.path"
    if pointer.is_file():
        line = pointer.read_text().strip().splitlines()[0].strip()
        path = pathlib.Path(line)
        if path.is_file():
            return path
    env = os.environ.get("PE_DISC1_BIN", "").strip()
    if env:
        path = pathlib.Path(env)
        if path.is_file():
            return path
    return None


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for sector in range(lba, lba + nsec):
            fh.seek(sector * SECTOR_RAW + FORM1_OFF)
            chunk = fh.read(FORM1_USER)
            require(len(chunk) == FORM1_USER, f"disc read {sector}")
            out.extend(chunk)
    return bytes(out)


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((END_6E6A8 - FN_6E6A8) // 4 == 11, "6E6A8 11 words")
    sha = window_sha(data, FN_6E6A8, END_6E6A8)
    require(sha == SHA_6E6A8, "6E6A8 sha256")
    require(jal_target(load_u32(data, FN_6E6A8 + 0x14)) == FN_6E6D4, "6E6A8 jal 6E6D4")
    require(load_u32(data, FN_6E6A8 + 0x10) == 0x00002821, "6E6A8 a1=0")
    require(load_u32(data, FN_6E6A8 + 0x18) == 0x00403021, "6E6A8 delay a2=dest")

    require(load_u32(data, FN_6914C_34) == 0x8E850194, "0x34 lw +0x194")
    require(load_u32(data, 0x80069400) == 0x944230E2, "0x34 lhu 930E2")
    require(load_u32(data, 0x80069404) == 0x8E840100, "0x34 lw +0x100")
    require(load_u32(data, 0x8006940C) == 0x94C630E4, "0x34 lhu 930E4")
    require(jal_target(load_u32(data, 0x80069414)) == FN_6E6A8, "0x34 jal 6E6A8")
    require(load_u16(data, 0x800930E2) == REL, "930E2=0x7E")
    require(load_u16(data, 0x800930E4) == REL + NSEC, "930E4=0x83")
    require(load_u32(data, 0x8006A9C4) == 0xAC220E6C, "6A8D4 sw +0x194")
    require(load_u32(data, 0x8006A9B8) == 0x3C02801F, "6A8D4 lui 0x801F")
    require(load_u32(data, 0x8006A9BC) == 0x2442D800, "6A8D4 0x801ED800")
    require(jal_target(load_u32(data, 0x80069430)) == 0x8006E7E8, "0x35 jal 6E7E8")
    require(load_u32(data, 0x80069450) == 0x24020001, "0x34 return 1")
    require(jal_target(load_u32(data, 0x8006949C)) == 0x8006E1C0, "0x36 jal 6E1C0")

    disc = find_disc(root)
    require(disc is not None, "missing Disc 1")
    payload = read_form1(disc, PE_IMG_LBA + REL, NSEC)
    require(len(payload) == NSEC * FORM1_USER, "5 sectors")
    require(struct.unpack_from("<I", payload, 0)[0] == 0x2050, "payload w0")
    require(struct.unpack_from("<I", payload, 4)[0] == 0x2008, "payload w1")
    require(hashlib.sha256(payload).hexdigest() == PAYLOAD_SHA256, "PE.IMG 7E sha")

    print(
        f"PASS: 693F8 jal 6E6A8 dest=0x801ED800 LBA=PE.IMG+0x7E n=5; "
        f"6E6A8 11w jal 6E6D4; payload sha256; 6E6A8-win {sha}; "
        f"no 6914C-success/overlay-jalr/mode7/0x55 claim"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

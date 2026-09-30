#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 3B97C lighting at 0x8003BA24.

Pins SHA-1-exact EXE COP2 words (RTIR 0x049E012, NCCT 0x118043F — not
NCLIP) and the CE2=11+CE4=1 PE.IMG [428,434) 2-bone object.
Does not import production C. Does not claim andi 0xFC.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x8003B97C
FN_END = 0x8003BCE0
LIGHT = 0x8003BA24
CMD_RTIR = 0x4A49E012
CMD_NCCT = 0x4B18043F
BANK_LBA = 428
BANK_END = 434
BANK_SHA = "56b2db6d2b4a2b76e083851f29b311be68dd129fad5a578d359a33f04e7674e0"
OBJ_SHA = "fcf33e91064ee158da51cd51558ac094120a19755897680696a9d45a4e95fba4"
PE_IMG_LBA = 1013
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

    require((FN_END - FN) // 4 == 217, "3B97C 217 words")
    require(load_u32(data, LIGHT) == 0x8D620004, "lw dest+4")
    require(load_u32(data, 0x8003BA30) == 0x91E30004, "lbu rec+4")
    require(load_u32(data, 0x8003BA38) == 0x14620097, "bne byte4")
    require(load_u32(data, 0x8003BA88) == CMD_RTIR, "RTIR col0")
    require(load_u32(data, 0x8003BAC8) == CMD_RTIR, "RTIR col1")
    require(load_u32(data, 0x8003BB0C) == CMD_RTIR, "RTIR col2")
    require(load_u32(data, 0x8003BBDC) == CMD_NCCT, "NCCT B1638")
    require(load_u32(data, 0x8003BC64) == CMD_NCCT, "NCCT A6360")
    require(load_u32(data, 0x8003BB60) == 0x24421638, "table B1638")
    require(load_u32(data, 0x8003BB70) == 0x24426360, "table A6360")
    require(load_u32(data, 0x8003BA18) == 0x3C108009, "lui D_80091A58")
    require(load_u32(data, 0x8003BA1C) == 0x26101A58, "addiu D_80091A58")

    nclip = 0
    for va in range(FN, FN_END, 4):
        w = load_u32(data, va)
        if (w >> 26) == 0x12 and (w & 0x3F) == 0x06:
            nclip += 1
    require(nclip == 0, "no NCLIP")

    ncct = 0x118043F
    require(((ncct >> 19) & 1) == 1 and ((ncct >> 10) & 1) == 1, "NCCT sf/lm")
    require((ncct & 0x3F) == 0x3F, "NCCT op")

    disc = find_disc(root)
    require(disc is not None, "missing Disc 1")
    bank = read_form1(disc, PE_IMG_LBA + BANK_LBA, BANK_END - BANK_LBA)
    require(hashlib.sha256(bank).hexdigest() == BANK_SHA, "bank sha256")
    section = struct.unpack_from("<I", bank, 4)[0]
    rec = struct.unpack_from("<I", bank, section + 0x0C)[0] & 0x3FFFFF
    size, ptrw, _w8 = struct.unpack_from("<III", bank, rec)
    ptr = ptrw & 0xFFFFFF
    obj = bank[ptr : ptr + size]
    require(hashlib.sha256(obj).hexdigest() == OBJ_SHA, "object sha256")
    require(obj[2] == 2, "obj+2")
    require(obj[0x1C + 4] == 0, "rec0 byte4")
    require(obj[0x1C + 12 + 4] == 1, "rec1 byte4")
    require(struct.unpack_from("<H", obj, 0x1C + 12)[0] == 1, "rec1 lhu0")
    require(struct.unpack_from("<H", obj, 0x1C + 14)[0] == 38, "rec1 lhu2")
    dest_c = 0x1C + 24 + (struct.unpack_from("<H", obj, 6)[0] << 3)
    require(obj[dest_c + 4 + 3] == 0, "color+3 live 0")
    require(obj[dest_c + 4 + 7] == 0, "color+7 live 0")

    print(
        "PASS: 3B97C lighting 0x8003BA24 RTIR/NCCT; "
        "no NCLIP; PE.IMG [428,434) rec1 byte4=1 lhu2=38"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

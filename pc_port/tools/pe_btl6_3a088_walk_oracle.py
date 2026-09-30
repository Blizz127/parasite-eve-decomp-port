#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 3A088 GTE walk at 0x8003A3B4.

Pins SHA-1-exact EXE COP2 words and the CE2=11+CE4=1 PE.IMG [428,434)
2-bone object. Does not import production C. Does not claim andi 0xFC.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x8003A088
FN_END = 0x8003A6A8
WALK = 0x8003A3B4
CMD_RTIR = 0x4A49E012
CMD_RTV0 = 0x4A480012
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

    require((FN_END - FN) // 4 == 392, "3A088 392 words")
    require(load_u32(data, 0x8003A3A4) == 0x94420018, "lhu obj+0x18")
    require(load_u32(data, 0x8003A3AC) == 0x184000B5, "blez empty")
    require(load_u32(data, WALK) == 0x80E40000, "walk lb parent")
    require(load_u32(data, 0x8003A460) == 0x8E030058, "lw dest+0x58")
    require(load_u32(data, 0x8003A48C) == CMD_RTIR, "RTIR column0")
    require(load_u32(data, 0x8003A4EC) == CMD_RTIR, "RTIR column1")
    require(load_u32(data, 0x8003A530) == CMD_RTIR, "RTIR column2")
    require(load_u32(data, 0x8003A5A0) == CMD_RTV0, "RTV0 translation")
    require(load_u32(data, 0x8003A648) == CMD_RTV0, "RTV0 dest+0x18")
    require(load_u32(data, 0x8003A5A8) == 0xE8590000, "swc2 MAC1")
    require(load_u32(data, 0x8003A658) == 0xA52C0000, "sh IR dest+0x80")

    rtir = 0x049E012
    rtv0 = 0x0480012
    require(((rtir >> 19) & 1) == 1 and ((rtir >> 17) & 3) == 0, "RTIR sf/mx")
    require(((rtir >> 15) & 3) == 3 and ((rtir >> 13) & 3) == 3, "RTIR v/cv")
    require(((rtir >> 10) & 1) == 0, "RTIR lm=0")
    require(((rtv0 >> 19) & 1) == 1 and ((rtv0 >> 15) & 3) == 0, "RTV0 sf/v")
    require(((rtv0 >> 13) & 3) == 0 and ((rtv0 >> 10) & 1) == 0, "RTV0 cv/lm")

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
    require(struct.unpack_from("<H", obj, 0x18)[0] == 2, "obj+0x18")
    require(list(obj[0x3D8:0x3DA]) == [0, 1], "parents")
    require(obj[0x1C + 4] == 0, "rec0 byte4")
    require(obj[0x1C + 12 + 4] == 1, "rec1 byte4")
    require(struct.unpack_from("<h", obj, 0x3B0 + 16 + 14)[0] == 1, "slot1 +0xE")

    print(
        "PASS: 3A088 walk 0x8003A3B4 RTIR/RTV0; "
        "PE.IMG [428,434) 2-bone parents 0,1 obj+0x18=2"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

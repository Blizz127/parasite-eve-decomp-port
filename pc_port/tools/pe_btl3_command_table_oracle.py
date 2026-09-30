#!/usr/bin/env python3
"""PE-BTL3 independent oracle: command-table producer and 1A680 index math.

Pins SHA-1-exact EXE windows for Writer B (0x8006C140), CE2=14 selection,
D_800930D8[CE2+8..+9] PE.IMG mapping, and func_8001A680's
D_800B0E98[type*192 + command*4] load. Optionally hash-gates PE.IMG
[396,428) clip idB=4. Does not import production C.
"""

from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
WRITER = (0x8006C140, 0x8006C174)
WRITER_WORDS = exe_words(0x8006C140, 13)
INDEX = (0x8001A6A0, 0x8001A6CC)
INDEX_WORDS = exe_words(0x8001A6A0, 11)
CE2_STORE = (0x8006C23C, 0x8006C250)
CE2_WORDS = [0x2402000E, 0x3C01800B, 0xA0220CE2, 0x3C01800B, 0xA0230CEB]
CD_INDEX = (0x8006C068, 0x8006C090)
CD_WORDS = exe_words(0x8006C068, 10)
TABLE = 0x800930D8
BANK_SHA = "db785a5eea1f78f945284e57955605326f5856adda1ebc83d0a95d7a0142b1b2"
CLIP4_SHA = "6207fbca2fe44a3549bf0b7fbcf1ce3e979a0a12e8b130e606ea4a985b1885e4"
CLIP4_PTR = 0x6C14
CLIP4_SIZE = 1700
CLIP4_FRAMES = 0x12
CLIP4_BONES_M1 = 0x1E
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


def load_u16(data: bytes, addr: int) -> int:
    return struct.unpack_from("<H", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def op(word: int) -> int:
    return word >> 26


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


def read_form1(disc: pathlib.Path, lba: int, size: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        remaining = size
        sector = lba
        while remaining > 0:
            fh.seek(sector * SECTOR_RAW + FORM1_OFF)
            chunk = fh.read(min(FORM1_USER, remaining))
            require(len(chunk) == min(FORM1_USER, remaining), f"disc read {sector}")
            out.extend(chunk)
            remaining -= len(chunk)
            sector += 1
    return bytes(out)


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require(len(WRITER_WORDS) == (WRITER[1] - WRITER[0]) // 4, "writer size")
    for i, want in enumerate(WRITER_WORDS):
        got = load_u32(data, WRITER[0] + i * 4)
        require(got == want, f"writer {WRITER[0] + i * 4:#x}")
    require(load_u32(data, 0x8006C140) == 0x90830007, "lbu idB rec+7")
    require(load_u32(data, 0x8006C144) == 0x8C820004, "lw ptr rec+4")
    require(load_u32(data, 0x8006C158) == 0xAC6201C0, "sw overlay+0x1C0")
    require(load_u32(data, 0x8006C170) == 0x2484000C, "record stride 12")

    require(len(INDEX_WORDS) == (INDEX[1] - INDEX[0]) // 4, "index size")
    for i, want in enumerate(INDEX_WORDS):
        got = load_u32(data, INDEX[0] + i * 4)
        require(got == want, f"index {INDEX[0] + i * 4:#x}")
    require(load_u32(data, 0x8001A6A0) == 0x9223000C, "lbu type actor+0x0C")
    require(load_u32(data, 0x8001A6A8) == 0x24840E98, "addiu D_800B0E98")
    require(load_u32(data, 0x8001A6B4) == 0x00021180, "sll 6 => type*192")
    require(load_u32(data, 0x8001A6C0) == 0x00031880, "sll 2 => command*4")
    require(load_u32(data, 0x8001A6C8) == 0x8C620000, "lw table slot")
    require(load_u32(data, 0x8001A6DC) == 0xAE2201B0, "sw actor+0x1B0")
    require(load_u32(data, 0x8001A6F0) == 0x90820002, "lbu resource+2")
    require(load_u32(data, 0x8001A6FC) == 0xA222000F, "sb actor+0x0F")

    for i, want in enumerate(CE2_WORDS):
        got = load_u32(data, CE2_STORE[0] + i * 4)
        require(got == want, f"CE2 {CE2_STORE[0] + i * 4:#x}")
    require(load_u32(data, 0x8006C23C) == 0x2402000E, "addiu 14")
    require(load_u32(data, 0x8006C244) == 0xA0220CE2, "sb D_800B0CE2")

    for i, want in enumerate(CD_WORDS):
        got = load_u32(data, CD_INDEX[0] + i * 4)
        require(got == want, f"CD {CD_INDEX[0] + i * 4:#x}")
    require(load_u16(data, TABLE + 22 * 2) == 396, "D_800930D8[CE2=14+8]")
    require(load_u16(data, TABLE + 23 * 2) == 428, "D_800930D8[CE2=14+9]")
    require(load_u16(data, TABLE + 18 * 2) == 288, "D_800930D8[CE2=10+8]")
    require(load_u16(data, TABLE + 19 * 2) == 316, "D_800930D8[CE2=10+9]")

    require(jal_target(load_u32(data, 0x80024A78)) == 0x8006C1CC,
            "24A3C state0 jal 6C1CC")
    require(load_u32(data, 0x80024A7C) == 0x24040001, "6C1CC a0=1")
    require(jal_target(load_u32(data, 0x800299B0)) == 0x8001A680,
            "29810 jal 1A680")
    for addr in range(0x80029810, 0x80029C00, 4):
        word = load_u32(data, addr)
        if op(word) == 3:
            require(jal_target(word) != 0x8006C1CC, f"29810 jal 6C1CC at {addr:#x}")
            require(jal_target(word) != 0x8006BECC, f"29810 jal 6BECC at {addr:#x}")
    for addr in range(0x800144FC, 0x80014680, 4):
        word = load_u32(data, addr)
        if op(word) == 3:
            require(jal_target(word) != 0x8006C1CC, f"144FC jal 6C1CC at {addr:#x}")
            require(jal_target(word) != 0x8006BECC, f"144FC jal 6BECC at {addr:#x}")

    require(load_u32(data, 0x8006B87C) == 0xAC830000, "Writer A sw")
    require((load_u32(data, 0x8006B868) & 0xFFFF) == 0x01C0, "Writer A +0x1C0")

    disc = find_disc(root)
    require(disc is not None, "missing Disc 1 (local/pe_disc1.path or PE_DISC1_BIN)")
    bank = read_form1(disc, PE_IMG_LBA + 396, 32 * FORM1_USER)
    require(hashlib.sha256(bank).hexdigest() == BANK_SHA, "CE2=14 bank sha256")
    section = struct.unpack_from("<I", bank, 4)[0]
    packed = struct.unpack_from("<I", bank, section + 0x10)[0]
    count = packed >> 22
    table = packed & 0x3FFFFF
    found = None
    type0_ids = []
    for i in range(count):
        rec = table + i * 12
        size, ptrw, w8 = struct.unpack_from("<III", bank, rec)
        idb = (ptrw >> 24) & 0xFF
        ida = (w8 >> 24) & 0xFF
        ptr = ptrw & 0xFFFFFF
        if ida == 0:
            type0_ids.append(idb)
        if ida == 0 and idb == 4:
            found = (ptr, size)
    require(found is not None, "CE2=14 missing idA=0 idB=4")
    require(found[0] == CLIP4_PTR, f"clip4 ptr {found[0]:#x}")
    require(found[1] == CLIP4_SIZE, f"clip4 size {found[1]}")
    payload = bank[found[0]:found[0] + found[1]]
    require(hashlib.sha256(payload).hexdigest() == CLIP4_SHA, "clip4 sha256")
    require(payload[1] == CLIP4_BONES_M1, "clip4 bones-1")
    require(payload[2] == CLIP4_FRAMES, "clip4 frames lbu+2")
    require(4 in type0_ids, "idB=4 present")
    bank10 = read_form1(disc, PE_IMG_LBA + 288, 28 * FORM1_USER)
    section10 = struct.unpack_from("<I", bank10, 4)[0]
    packed10 = struct.unpack_from("<I", bank10, section10 + 0x10)[0]
    count10 = packed10 >> 22
    table10 = packed10 & 0x3FFFFF
    has4 = False
    for i in range(count10):
        rec = table10 + i * 12
        ptrw, w8 = struct.unpack_from("<II", bank10, rec + 4)
        if ((w8 >> 24) & 0xFF) == 0 and ((ptrw >> 24) & 0xFF) == 4:
            has4 = True
    require(not has4, "CE2=10 must not contain type0 idB=4")

    print(
        "PASS: Writer B 13 + 1A680 index 11 + CE2=14 map + PE.IMG clip4 "
        f"idB=4 size={CLIP4_SIZE} frames={CLIP4_FRAMES}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

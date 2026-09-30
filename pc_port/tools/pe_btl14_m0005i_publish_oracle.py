#!/usr/bin/env python3
"""PE-BTL14 independent oracle: m0005i 6B4F8 publish and opcode 0xCE.

Pins SHA-1-exact EXE. Token 0xA80002C8 decodes to M0005I / table[4].
Chunk 2 12574 list desc is type 1 + type 6 (no type 0). First type-6
word is 0x000080CE → 181CC. Does not import production C. Does not
invent a bytecode stream. Does not mark M2 or M5.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TOKEN = 0xA80002C8
CHARSET = 0x800930B4
TABLE = 0x80093378
FN_6E2D0 = 0x8006E2D0
END_6E2D0 = 0x8006E338
SHA_6E2D0 = "c0d67bd4d0690ff599980cc5a1c9935405d4a440c428aae339e2d8419b6531d9"
FN_6E454 = 0x8006E454
END_6E454 = 0x8006E498
SHA_6E454 = "7c98a25d8b4a0c10d4c10d8936f500a265c7dfa75a56dbd1398faa09383ef4f4"
FN_181CC = 0x800181CC
END_181CC = 0x800182A0
SHA_181CC = "79f58896def734cffcb1755380750d9e8244eb09438f29dbb7d24fb09cc90b07"
FN_3999C = 0x8003999C
JAL_12574 = 0x8006B8BC
JAL_6B4F8 = 0x8003F088
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
LIST_OFF = 0x202A4
DESC_OFF = 0x25014
TYPE6_OFF = 0x2341C
TYPE6_FIRST = 0x000080CE
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


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def jal_sites(data: bytes, target: int) -> list[int]:
    jal_word = 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)
    sites = []
    for offset in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, offset)[0] == jal_word:
            sites.append(0x80010000 + offset - 0x800)
    return sites


def decode_token(data: bytes, token: int) -> bytes:
    table = data[exe_off(CHARSET) : exe_off(CHARSET) + 32]
    out = bytearray(7)
    for i in range(6):
        idx = (token >> ((5 - i) * 5 + 2)) & 0x1F
        ch = table[idx]
        if 97 <= ch <= 122:
            ch -= 32
        out[i] = ch
    return bytes(out)


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
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            out += fh.read(FORM1_USER)
    return bytes(out)


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")
    tsize = struct.unpack_from("<I", data, 0x1C)[0]
    require(tsize == 0x1EE000, "tsize")

    require((END_6E2D0 - FN_6E2D0) // 4 == 26, "6E2D0 26 words")
    require(window_sha(data, FN_6E2D0, END_6E2D0) == SHA_6E2D0, "6E2D0 sha")
    require((END_6E454 - FN_6E454) // 4 == 17, "6E454 17 words")
    require(window_sha(data, FN_6E454, END_6E454) == SHA_6E454, "6E454 sha")
    require((END_181CC - FN_181CC) // 4 == 53, "181CC 53 words")
    require(window_sha(data, FN_181CC, END_181CC) == SHA_181CC, "181CC sha")
    require(load_u32(data, 0x800910A0 + 0xCE * 4) == FN_181CC, "table[0xCE]")
    require(jal_target(load_u32(data, 0x80018200)) == 0x8002FF78, "181CC jal 2FF78")
    require(jal_target(load_u32(data, 0x80018288)) == 0x80030220, "181CC jal 30220")
    require(jal_sites(data, FN_181CC) == [], "181CC jalr-only from 17018")
    require(jal_sites(data, 0x8006B4F8) == [JAL_6B4F8], "6B4F8 sole caller")
    require(jal_target(load_u32(data, JAL_6B4F8)) == 0x8006B4F8, "3F074 jal 6B4F8")
    require(jal_target(load_u32(data, JAL_12574)) == 0x80012574, "6B4F8 jal 12574")
    require(load_u32(data, 0x8006B8C4) == 0xAEC20944, "sw v0 overlay+0x944")
    require(load_u32(data, 0x80017248) == 0x1440FFA9, "17018 bne v0 re-fetch")
    require(jal_sites(data, FN_3999C) == [0x80035D14], "3999C still only 35C84")

    name = decode_token(data, TOKEN)
    require(name == b"M0005I\x00", f"token name {name!r}")
    require(name[2:5] == b"005", "atoi digits")
    rel = load_u32(data, TABLE + 4 * 8)
    packed = load_u32(data, TABLE + 4 * 8 + 4)
    require(rel == REL, "table[4] rel")
    require(packed == PACKED, "table[4] packed")
    sec0 = packed & 0xFF
    sec1 = (packed >> 8) & 0xFFF
    sec2 = packed >> 20
    require((sec0, sec1, sec2) == (33, 169, 96), "chunk sectors")

    disc = find_disc(root)
    require(disc is not None, "Disc 1 BIN required for chunk2 hash")
    chunk2 = read_form1(disc, PE_IMG_LBA + REL + sec0 + sec1, sec2)
    require(hashlib.sha256(chunk2).hexdigest() == CHUNK2_SHA, "chunk2 sha")
    require(struct.unpack_from("<I", chunk2, 4)[0] == 0x0002FAF0, "chunk2 +4")
    require(struct.unpack_from("<I", chunk2, LIST_OFF + 4)[0] == 7, "list count 7")
    require(chunk2[DESC_OFF] == 2, "desc count 2")
    require(chunk2[DESC_OFF + 1] == 1, "actor0 type 1")
    require(chunk2[DESC_OFF + 3] == 6, "actor1 type 6")
    require(chunk2[DESC_OFF + 2] == 0 and chunk2[DESC_OFF + 4] == 0, "idB 0")
    require(struct.unpack_from("<I", chunk2, TYPE6_OFF)[0] == TYPE6_FIRST, "type6 0xCE")
    require((TYPE6_FIRST & 0x1FFF) == 0xCE, "op 0xCE")
    require(((TYPE6_FIRST >> 13) & 0xF) == 4, "argc 4")

    print(
        "PASS: 0xA80002C8→M0005I table[4] 0x266A; chunk2 sha 01a64ba3…; "
        "125E0 desc type1+type6; type6 first 0xCE=181CC; "
        "17018 re-fetch bne; 3999C not this spawn"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

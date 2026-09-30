#!/usr/bin/env python3
"""PE-BTL18 independent oracle: 15DAC key 0x190 overlay table walk.

Pins SHA-1-exact EXE and the live m0005i chunk2 12-byte row whose
+0xA key is 0x28 and +3 bit 0x10 is set. Does not import production
C. Does not invent a table.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
CASE = 0x80016658
CASE_END = 0x800166E0
SHA_CASE = "ca0abef217c95528aa87befe644f75f68af9a3fc72d7c185f4e0f4a933ebde20"
MATCH = 0x800168A4
MATCH_END = 0x800168E0
SHA_MATCH = "05338c24303718f2bc254aabf9b3780bbff4f70ad04709fc6989f68f8609128d"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
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
    require(load_u32(data, 0x800101B0 + (0x190 - 100) * 4) == CASE, "0x190")
    require((CASE_END - CASE) // 4 == 34, "16658 search 34 words")
    require(window_sha(data, CASE, CASE_END) == SHA_CASE, "16658 sha")
    require((MATCH_END - MATCH) // 4 == 15, "match 15 words")
    require(window_sha(data, MATCH, MATCH_END) == SHA_MATCH, "match sha")
    require(load_u32(data, CASE) == 0x3C03003F, "lui 0x3f")
    require(load_u32(data, 0x80016660) == 0x8C420E64, "lw B0E64")
    require(load_u32(data, 0x80016688) == 0x1040009A, "beqz count")
    require(load_u32(data, 0x800166A4) == 0x30420010, "andi 0x10")
    require(load_u32(data, 0x800166C0) == 0x10A20078, "beq key")
    require(load_u32(data, MATCH) == 0x3C01800B, "match lui")
    require(load_u32(data, 0x800168A8) == 0xA0250DB8, "sb B0DB8")
    require(load_u32(data, 0x800168B8) == 0xA0220DB9, "sb B0DB9")
    require(load_u32(data, 0x800168D4) == 0xAC230DFC, "sw B0DFC")
    require(load_u32(data, 0x800168DC) == 0x24020001, "match v0=1")

    disc = find_disc(root)
    require(disc is not None, "Disc 1 BIN required")
    sec0 = PACKED & 0xFF
    sec1 = (PACKED >> 8) & 0xFFF
    sec2 = PACKED >> 20
    chunk2 = read_form1(disc, PE_IMG_LBA + REL + sec0 + sec1, sec2)
    require(hashlib.sha256(chunk2).hexdigest() == CHUNK2_SHA, "chunk2 sha")
    hdr = struct.unpack_from("<I", chunk2, 4)[0]
    word = struct.unpack_from("<I", chunk2, hdr + 0x30)[0]
    count = word >> 22
    off = word & 0x3FFFFF
    require(count == 3, "count 3")
    require(off == 0x2FD78, "table off")
    require(chunk2[off + 24 + 3] & 0x10, "row2 bit4")
    require(struct.unpack_from("<H", chunk2, off + 24 + 10)[0] == 0x28, "row2 key")
    require(chunk2[off + 24 + 8] == 0x17, "row2 +8")
    require(struct.unpack_from("<I", chunk2, off + 24 + 4)[0] == 0x2E838, "row2 +4")
    print(
        "PASS: 0x190→16658 search 34w; match sb B0DB8/B0DB9 sw B0DFC; "
        "m0005i row2 key 0x28 bit4 +4=0x2E838"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

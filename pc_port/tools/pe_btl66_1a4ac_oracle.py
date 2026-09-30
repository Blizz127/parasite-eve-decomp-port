#!/usr/bin/env python3
"""PE-BTL66 — Writer A / 1A4AC ROM contract."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800
WRITER_A = tuple(exe_words(0x8006B84C, 13))
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921


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


def load_m0005i_chunk2() -> bytes | None:
    pointer = ROOT / "local" / "pe_disc1.path"
    if not pointer.is_file():
        return None
    path = pointer.read_text().strip().splitlines()[0].strip()
    sec0, sec1, sec2 = PACKED & 0xFF, (PACKED >> 8) & 0xFFF, PACKED >> 20
    out = bytearray()
    with open(path, "rb") as fh:
        for i in range(sec2):
            fh.seek((PE_IMG_LBA + REL + sec0 + sec1 + i) * 2352 + 24)
            out += fh.read(2048)
    data = bytes(out)
    require(hashlib.sha256(data).hexdigest() == CHUNK2_SHA, "m0005i chunk2")
    return data


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require((0x8001A680 - 0x8001A4AC) // 4 == 117, "1A4AC 117w")
    require(
        window_sha(blob, 0x8001A4AC, 0x8001A680)
        == "53d93b571f022b1ca40fb8c8244845cd75baadc6c00559cd79d36cd969b42174",
        "1A4AC sha",
    )
    require(jal_target(load_u32(blob, 0x8001A4E4)) == 0x8001A4AC, "recurse")
    require(jal_target(load_u32(blob, 0x8001A4EC)) == 0x8006A318, "jal 6A318")
    require(jal_target(load_u32(blob, 0x8001A55C)) == 0x8001A784, "jal 1A784")
    require(load_u32(blob, 0x8001A524) == 0x30880200, "andi 0x200")
    require(load_u32(blob, 0x8001A530) == 0x96030012, "lhu +0x12")
    require(load_u32(blob, 0x8001A578) == 0x8E02001C, "lw +0x1C")
    require(jal_target(load_u32(blob, 0x80035B84)) == 0x8001A4AC, "35558 D254")
    require(jal_target(load_u32(blob, 0x80035BEC)) == 0x8001A4AC, "35558 D20C")
    require(load_u32(blob, 0x8006B828) == 0x8EA20010, "hdr+0x10")
    for i, word in enumerate(WRITER_A):
        require(load_u32(blob, 0x8006B84C + i * 4) == word, f"Writer A[{i}]")
    chunk2 = load_m0005i_chunk2()
    if chunk2 is not None:
        hdr = struct.unpack_from("<I", chunk2, 4)[0] & 0x3FFFFF
        packed = struct.unpack_from("<I", chunk2, hdr + 0x10)[0]
        rec = packed & 0x3FFFFF
        hit = False
        for i in range(packed >> 22):
            off = rec + i * 12
            ptr = struct.unpack_from("<I", chunk2, off + 4)[0] & 0xFFFFFF
            if chunk2[off + 0xB] == 2 and chunk2[off + 7] == 0x17:
                require(chunk2[ptr + 2] == 51, "type2 cmd17 byte2")
                hit = True
        require(hit, "type2 cmd17 present")
    print("PASS: Writer A hdr+0x10; 1A4AC 117w; 35558 jals; cmd17=51")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

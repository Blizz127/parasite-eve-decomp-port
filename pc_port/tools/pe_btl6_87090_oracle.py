#!/usr/bin/env python3
"""PE-BTL6 independent oracle: func_80087090 + 6CDA4 state-9 a0=1.

Pins SHA-1-exact EXE. 87090 is 20 words, jal 851A8 until v0!=1.
Live state 9 is 87090(dest, 0). AKAO + 0xB0BEB4BF == 0. Does not
import production C. Does not claim 851A8/870E0/6914C success,
mode 7, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x80087090
FN_END = 0x800870E0
WIN_SHA256 = (
    "3cb0ae9829fa97505104d41f8517ef9a96a8339bc7ee76c7ea5d63901b522e3f"
)
MAGIC = 0xB0BEB4BF
AKAO = 0x4F414B41
PE_IMG_LBA = 1013
REL = 0x8C6
NSEC = 0x0F
PAYLOAD_SHA256 = (
    "cf9670eb8c7b0b75223aac794e66d1c007757a40cf801317611a445af4f1c28b"
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


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start):exe_off(end)]).hexdigest()


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

    require((FN_END - FN) // 4 == 20, "87090 20 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA256, "87090 sha256")
    require(load_u32(data, FN + 0x18) == 0x24120001, "s2=1")
    require(jal_target(load_u32(data, FN + 0x24)) == 0x800851A8, "jal 851A8")
    require(load_u32(data, FN + 0x2C) == 0x1052FFFD, "beq v0,1 retry")
    require(load_u32(data, FN + 0x48) == 0x03E00008, "jr")

    require(jal_target(load_u32(data, 0x8006CF98)) == FN, "state9 a0=1 jal 87090")
    require(load_u32(data, 0x8006CF9C) == 0x00002821, "a1=0")
    require(load_u32(data, 0x8006CFD4) == 0x2402000A, "ok li 0xA")
    require(load_u32(data, 0x8006CFDC) == 0xA24000F0, "-1 sb F0=0")
    require(load_u32(data, 0x8006CFE4) == 0xA24200F0, "ok sb F0")
    require(jal_target(load_u32(data, 0x8006CFE8)) == 0x800870E0, "stateA jal 870E0")
    require((0x800870F0 - 0x800870E0) // 4 == 4, "870E0 4 words")
    require(load_u32(data, 0x800870E4) == 0x8C42D24C, "870E0 lw D_8009D24C")

    disc = find_disc(root)
    require(disc is not None, "missing Disc 1")
    payload = read_form1(disc, PE_IMG_LBA + REL, NSEC)
    require(hashlib.sha256(payload).hexdigest() == PAYLOAD_SHA256, "AKAO sha")
    word0 = struct.unpack_from("<I", payload, 0)[0]
    require(word0 == AKAO, "AKAO magic")
    require(((word0 + MAGIC) & 0xFFFFFFFF) == 0, "851A8 magic check")
    require(struct.unpack_from("<I", payload, 0x10)[0] == 0x00048000, "+0x10 SPU")
    require(struct.unpack_from("<I", payload, 0x14)[0] == 0x00006CD0, "+0x14 size")
    require(struct.unpack_from("<I", payload, 0x18)[0] == 0x00000080, "+0x18 off")
    require(struct.unpack_from("<I", payload, 0x1C)[0] == 0x00000090, "+0x1C end")

    print(
        "PASS: 87090 20w sha256; jal 851A8 retry; state9 a1=0; "
        "AKAO magic-check 0; params 48000/6CD0/80/90; next 870E0"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

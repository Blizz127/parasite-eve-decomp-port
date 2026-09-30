#!/usr/bin/env python3
"""PE-BTL6 independent oracle: func_8006E7E8 + 6CDA4 state-8 poll.

Pins SHA-1-exact EXE. 6E7E8 is 19 words. State 8 jals it; -1 sb 7,
pending stays 8, 0 sb 9. Live a0=1 state 9 jals 87090. gp+0x40C is
sectors (a0==0 path sll 11). Optional PE.IMG [0x8C6, 0x8D5] hash.
Does not import production C. Does not claim 87090/6914C success,
mode 7, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x8006E7E8
FN_END = 0x8006E834
WIN_SHA256 = (
    "968f5fb0f55b63c6b38d3a2cc155e06bd89095b6e621c665590d280501e54c89"
)
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

    require((FN_END - FN) // 4 == 19, "6E7E8 19 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA256, "6E7E8 sha256")
    require(jal_target(load_u32(data, FN + 8)) == 0x800811E4, "jal 811E4")
    require(load_u32(data, FN + 0xC) == 0x27A40010, "a0=sp+0x10")
    require(load_u32(data, FN + 0x18) == 0x2C420002, "sltiu 2")
    require(load_u32(data, FN + 0x24) == 0x3C03800B, "lui overlay")
    require(load_u32(data, FN + 0x28) == 0x24630CD8, "addiu overlay")
    require(load_u32(data, FN + 0x34) == 0x00441024, "and RMW")
    require(load_u32(data, FN + 0x44) == 0x03E00008, "jr")

    require(jal_target(load_u32(data, 0x8006CF28)) == FN, "state8 jal 6E7E8")
    require(load_u32(data, 0x8006CF38) == 0x24020007, "timeout li 7")
    require(load_u32(data, 0x8006CF40) == 0xA24200F0, "timeout sb F0")
    require(load_u32(data, 0x8006CF44) == 0x14C00031, "pending bne")
    require(load_u32(data, 0x8006CF48) == 0x24020009, "done li 9")
    require(load_u32(data, 0x8006CF50) == 0xA24200F0, "done sb F0")
    require(jal_target(load_u32(data, 0x8006CF98)) == 0x80087090, "a0==1 jal 87090")
    require(load_u32(data, 0x8006CF8C) == 0x00052AC0, "a0==0 sll 11 sectors")
    require((0x800870E0 - 0x80087090) // 4 == 20, "87090 20 words")

    disc = find_disc(root)
    require(disc is not None, "missing Disc 1")
    payload = read_form1(disc, PE_IMG_LBA + REL, NSEC)
    require(len(payload) == NSEC * FORM1_USER, "15 sectors")
    require(payload[:4] == b"AKAO", "AKAO magic")
    require(hashlib.sha256(payload).hexdigest() == PAYLOAD_SHA256, "PE.IMG 8C6 sha")

    print(
        "PASS: 6E7E8 19w sha256; state8 -1→7 pending→8 0→9; "
        "live a0=1 jal 87090; PE.IMG 8C6/0x0F AKAO sha256"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

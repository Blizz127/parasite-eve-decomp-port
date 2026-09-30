#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 6914C EF=0x36 at 0x80069468.

Pins SHA-1-exact EXE. 0x36 walks dest+lw(dest+4)+0x28: count=hi16,
entries at dest+(word&0xFFFF) stride 0x14, jal 6E1C0(entry, dest).
Then jal 6E498(+0x18C, key 0x73DECD80) until miss; hit is image-only
LoadImage 7506C, key+=4. Miss/done: sb EF=0, overlay&=~8, v0=0.
PE.IMG [0x7E,0x83) header: w0=0x2050 w1=0x2008 s3+0x28=0x0040203C
count=64 entry0=+0x203C rect 832,448 64x64 off=8. 6E1C0 reads the
buffer (LoadImage); it does not write it. 6E498 does not touch it.
Does not import production C. Does not claim overlay jalr, mode 7,
or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN_36 = 0x80069468
FN_6E1C0 = 0x8006E1C0
END_6E1C0 = 0x8006E2D0
SHA_6E1C0 = (
    "1f07ebe9b903ffeb765d5588459be27e66dfb4a22c330bd6063946d705eb47fe"
)
FN_6E498 = 0x8006E498
END_6E498 = 0x8006E514
SHA_6E498 = (
    "b9c428dc909f08b3abc61fdcd98a6e4fbf48317a688204fa4bd4abb23b854be7"
)
FN_7506C = 0x8007506C
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


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def j_target(word: int) -> int:
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

    require(load_u32(data, FN_36) == 0x8E920194, "0x36 lw +0x194")
    require(load_u32(data, FN_36 + 8) == 0x8E430004, "0x36 lw dest+4")
    require(load_u32(data, FN_36 + 0x14) == 0x8E630028, "0x36 lw s3+0x28")
    require(jal_target(load_u32(data, 0x8006949C)) == FN_6E1C0, "0x36 jal 6E1C0")
    require(load_u32(data, 0x800694A0) == 0x02402821, "6E1C0 a1=dest")
    require(load_u32(data, 0x800694B8) == 0x26310014, "stride 0x14")
    require(load_u32(data, 0x800694BC) == 0x8E92018C, "0x36 lw +0x18C")
    require(load_u32(data, 0x800694C0) == 0x3C1073DE, "key lui 0x73DE")
    require(load_u32(data, 0x800694C4) == 0x3610CD80, "key ori 0xCD80")
    require(jal_target(load_u32(data, 0x800694CC)) == FN_6E498, "0x36 jal 6E498")
    require(jal_target(load_u32(data, 0x80069544)) == FN_7506C, "hit jal 7506C")
    require(load_u32(data, 0x8006953C) == 0x26100004, "key += 4")
    require(j_target(load_u32(data, 0x8006954C)) == 0x800694CC, "hit re-lookup")
    require(load_u32(data, 0x8006955C) == 0xA28000EF, "done sb EF=0")
    require(load_u32(data, 0x80069558) == 0x2404FFF7, "andi ~8")
    require(load_u32(data, 0x8006956C) == 0x00001021, "v0=0")

    require((END_6E1C0 - FN_6E1C0) // 4 == 68, "6E1C0 68 words")
    require(window_sha(data, FN_6E1C0, END_6E1C0) == SHA_6E1C0, "6E1C0 sha")
    require(jal_target(load_u32(data, FN_6E1C0 + 0x78)) == FN_7506C, "6E1C0 jal 7506C")
    require((END_6E498 - FN_6E498) // 4 == 31, "6E498 31 words")
    require(window_sha(data, FN_6E498, END_6E498) == SHA_6E498, "6E498 sha")

    disc = find_disc(root)
    require(disc is not None, "missing Disc 1")
    payload = read_form1(disc, PE_IMG_LBA + REL, NSEC)
    require(hashlib.sha256(payload).hexdigest() == PAYLOAD_SHA256, "payload sha")
    require(struct.unpack_from("<I", payload, 0)[0] == 0x2050, "w0")
    require(struct.unpack_from("<I", payload, 4)[0] == 0x2008, "w1")
    word28 = struct.unpack_from("<I", payload, 0x2008 + 0x28)[0]
    require(word28 == 0x0040203C, "s3+0x28")
    require(word28 >> 16 == 64, "count 64")
    require((word28 & 0xFFFF) == 0x203C, "entry0 +0x203C")
    dims = struct.unpack_from("<I", payload, 0x203C + 8)[0]
    require(dims == 0x380D0040, "ent0 +8")
    require(((dims >> 10) & 0x7FF) == 832, "ent0 x")
    require((dims >> 21) == 448, "ent0 y")
    require((dims & 0x3FF) == 64, "ent0 w")
    require(payload[0x203C + 7] == 64, "ent0 h")
    require((struct.unpack_from("<I", payload, 0x203C + 4)[0] & 0xFFFFFF) == 8, "ent0 off")
    require(struct.unpack_from("<I", payload, 0x2050)[0] == 0, "ent1 zero")

    print(
        "PASS: 69468 6E1C0 x64 stride 0x14 over dest 0x801ED800; "
        "6E498 +0x18C key 0x73DECD80; done EF=0 overlay&=~8 v0=0; "
        "payload TIM-like not overlay; no mode7/0x55 claim"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

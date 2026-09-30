#!/usr/bin/env python3
"""Independent retail oracle for the 6E834 overlay-load authority."""

import hashlib
import pathlib
import struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8000F800


def main() -> None:
    root = pathlib.Path(__file__).resolve().parents[2]
    data = (root / "build/disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(data).hexdigest() == SHA1

    def u32(address: int) -> int:
        return struct.unpack_from("<I", data, address - BASE)[0]

    def u16(address: int) -> int:
        return struct.unpack_from("<H", data, address - BASE)[0]

    dest = u32(0x80011614)
    values = tuple(u16(0x80093164 + i * 2) for i in range(4))
    assert dest == 0x8018EFF0
    assert values == (0x03D2, 0x0457, 0x04FC, 0x0516)
    size = (values[1] - values[0]) * 0x800
    assert size == 0x42800 and dest + size == 0x801D17F0
    assert u32(0x8006E898) == 0x3C108009
    assert u32(0x8006E89C) == 0x26103164
    assert u32(0x8006E8A8) == 0x3C068001
    assert u32(0x8006E8AC) == 0x8CC61614
    print("  OK retail destination: D_80011614=0x8018EFF0")
    print("  OK range table: 03D2 0457 04FC 0516")
    print("  OK first load extent: 0x42800 bytes through 0x801D17F0")
    print("  OK func_8006E834 reads both image-backed authorities")
    print("\nB54K-P overlay authority oracle: PASS.")


if __name__ == "__main__":
    main()

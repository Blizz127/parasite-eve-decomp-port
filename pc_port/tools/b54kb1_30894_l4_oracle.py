#!/usr/bin/env python3
"""Independent B54K-B1 oracle for func_80030894's complete L4 group.

Reads only the SHA-1-exact retail executable.  It verifies the full
0x80030AC4..0x80030C9C window, boundaries, control-flow/call census, and
critical instruction sequences.  A separate zero-backed byte model applies
the decoded bank-0 wrapper/header/store semantics without importing native
production code and emits a stable final write-map digest.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
EXE_BASE = 0x8000F800
START = 0x80030AC4
END = 0x80030C9C
WORDS = 118
WINDOW_SHA256 = (
    "79fa2086406d087e2781025634ec533d50e262f3543f5916eedcdd383da8f0f6"
)
MODEL_UNIQUE_BYTES = 121
MODEL_SHA256 = "812440af2f0f6c0778421e177ce747d616420bd91678e2a4d6ede3b49b2a1874"

JALS = [
    (0x80030AD0, 0x80077A64),
    (0x80030AF4, 0x80037140),
    (0x80030B18, 0x80077B04),
    (0x80030B30, 0x80077C44),
    (0x80030B78, 0x80077BC4),
    (0x80030B94, 0x800370DC),
    (0x80030C58, 0x800370DC),
]

# Words that prove the cut, bank-index scales, materialized bases, loop stride,
# bound, and the first excluded L5 instruction.  The full hash covers every
# other word; these make the semantic claims independently auditable.
CRITICAL = {
    0x80030AC0: 0x00009821,  # preceding L2 delay slot: move s3,zero
    0x80030AC4: 0x00002021,  # move a0,zero
    0x80030AC8: 0x00002821,  # move a1,zero
    0x80030ACC: 0x00003021,  # move a2,zero
    0x80030AD4: 0x00003821,  # call delay: move a3,zero
    0x80030AD8: 0x3C11800A,  # D_8009E068 high
    0x80030ADC: 0x2631E068,  # D_8009E068 low
    0x80030AE0: 0x93B20018,  # bank = lbu 24(sp)
    0x80030AE8: 0x00128040,  # bank * 2
    0x80030AEC: 0x02128021,  # bank * 3
    0x80030AF0: 0x001080C0,  # bank * 24
    0x80030B20: 0x00128900,  # bank * 16
    0x80030B38: 0x001298C0,  # bank * 8
    0x80030B40: 0x00108080,  # bank * 36 final shift
    0x80030B80: 0x02729823,  # bank * 7
    0x80030B84: 0x00139080,  # bank * 28
    0x80030BA0: 0x00139900,  # bank * 112
    0x80030C44: 0x000280C0,  # j * 8
    0x80030C48: 0x02028023,  # j * 7
    0x80030C4C: 0x00108080,  # j * 28
    0x80030C60: 0x26D60001,  # ++j
    0x80030C90: 0x2C420004,  # j < 4
    0x80030C94: 0x1440FFEB,  # bnez -> 0x80030C44
    0x80030C98: 0x32C200FF,  # delay: L4 j & 0xff; last included
    0x80030C9C: 0x0000B021,  # first excluded: L5 j = 0
    0x80030CA0: 0x93A30018,  # following L5 bank load
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FAIL: {message}")


def find_exe() -> pathlib.Path:
    here = pathlib.Path(__file__).resolve()
    for candidate in (
        here.parent.parent.parent / "build" / "disc1.candidate.exe",
        pathlib.Path("build/disc1.candidate.exe"),
        pathlib.Path("pc_port/build/disc1.candidate.exe"),
    ):
        if candidate.is_file():
            return candidate
    raise SystemExit("FAIL: could not locate build/disc1.candidate.exe")


class ByteModel:
    def __init__(self) -> None:
        self.bytes: dict[int, int] = {}

    def u8(self, address: int) -> int:
        return self.bytes.get(address, 0)

    def put8(self, address: int, value: int) -> None:
        self.bytes[address] = value & 0xFF

    def put16(self, address: int, value: int) -> None:
        self.put8(address, value)
        self.put8(address + 1, value >> 8)

    def put32(self, address: int, value: int) -> None:
        for index in range(4):
            self.put8(address + index, value >> (index * 8))

    def set_header(self, address: int, length: int, code: int) -> None:
        self.put8(address + 3, length)
        self.put8(address + 7, code)

    def append(self, head: int, tail: int) -> None:
        length = self.u8(head + 3) + self.u8(tail + 3) + 1
        require(length < 17, "decoded wrapper exceeded packet budget")
        self.put8(head + 3, length)
        self.put32(tail, 0)

    def wrapper(self, head: int, mode: int, tail_code: int,
                tail_length: int) -> None:
        self.put8(head + 3, 1)
        self.put32(head + 4, 0xE1000200 | (mode & 0x9FF))
        tail = head + 8
        self.set_header(tail, tail_length, tail_code)
        self.append(head, tail)


def expected_l4() -> ByteModel:
    model = ByteModel()
    clut = 0x7E13
    mode = 0x34

    tile_head = 0x8009E068
    tile = tile_head + 8
    model.wrapper(tile_head, 0, 0x60, 3)
    model.put8(tile + 4, 0x30)
    model.put8(tile + 5, 0x30)
    model.put8(tile + 6, 0x30)
    model.put8(tile + 7, model.u8(tile + 7) | 2)

    standalone_tile = 0x8009E098
    model.set_header(standalone_tile, 3, 0x60)
    for offset, value in ((4, 0x1D), (5, 0x3E), (6, 0x32)):
        model.put8(standalone_tile + offset, value)
    model.put16(standalone_tile + 0x0C, 0x38)
    model.put16(standalone_tile + 0x0E, 3)

    poly = 0x800B00E8
    model.set_header(poly, 8, 0x38)
    for offset, value in (
        (6, 0x82), (0x0D, 0xFF), (0x16, 0x82),
        (4, 0), (5, 0x46), (0x0C, 0x9F), (0x0E, 0xF9),
        (0x14, 0), (0x15, 0x46), (0x1C, 0x9F),
        (0x1D, 0xFF), (0x1E, 0xF9),
    ):
        model.put8(poly + offset, value)

    sprite_head = 0x800B6920
    sprite = sprite_head + 8
    model.wrapper(sprite_head, mode, 0x64, 4)
    model.put8(sprite + 0x0C, 0xC8)
    model.put8(sprite + 0x0D, 0xE0)
    model.put16(sprite_head + 0x16, clut)
    model.put16(sprite + 0x10, 4)
    model.put16(sprite + 0x12, 8)
    model.put8(sprite + 4, 0x9F)
    model.put8(sprite + 5, 0xFF)
    model.put8(sprite + 6, 0xF9)

    for index in range(4):
        head = 0x8009E0F0 + index * 28
        sprite = head + 8
        model.wrapper(head, mode, 0x64, 4)
        model.put16(head + 0x16, clut)
        model.put16(sprite + 0x10, 6)
        model.put16(sprite + 0x12, 10)
    return model


def run() -> int:
    path = find_exe()
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == EXE_SHA1,
            "retail executable SHA-1 mismatch")

    def word(address: int) -> int:
        return struct.unpack_from("<I", data, address - EXE_BASE)[0]

    window = data[START - EXE_BASE:END - EXE_BASE]
    require(len(window) == WORDS * 4, "L4 window size mismatch")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "L4 window SHA-256 mismatch")
    print(f"OK window: {WORDS} words / {len(window):#x} bytes, SHA-256 exact")

    for address, expected in CRITICAL.items():
        require(word(address) == expected,
                f"word {address:#010x}: {word(address):08X} != {expected:08X}")
    print("OK boundaries/scales/constants: 27 instruction-exact words")

    calls: list[tuple[int, int]] = []
    branches: list[tuple[int, int]] = []
    for address in range(START, END, 4):
        instruction = word(address)
        opcode = instruction >> 26
        if opcode == 3:
            calls.append((address,
                          0x80000000 | ((instruction & 0x03FFFFFF) << 2)))
        if opcode in (1, 2, 4, 5, 6, 7, 20, 21, 22, 23):
            displacement = instruction & 0xFFFF
            if displacement & 0x8000:
                displacement -= 0x10000
            branches.append((address, address + 4 + displacement * 4))
    require(calls == JALS, f"jal census/order changed: {calls!r}")
    require(branches == [(0x80030C94, 0x80030C44)],
            f"branch census changed: {branches!r}")
    print("OK control flow: seven native jal sites; one four-item back-edge")

    model = expected_l4()
    packed = b"".join(
        struct.pack("<IB", address, value)
        for address, value in sorted(model.bytes.items())
    )
    digest = hashlib.sha256(packed).hexdigest()
    print(f"model_unique_written_bytes={len(model.bytes)}")
    print(f"model_write_map_sha256={digest}")
    require(len(model.bytes) == MODEL_UNIQUE_BYTES,
            "independent model write-byte count changed")
    require(digest == MODEL_SHA256,
            "independent model write-map digest changed")
    require(model.u8(0x8009E0F0 + 112) == 0,
            "model crossed into bank-1 L4 storage")
    require(model.u8(0x8009E1D0) == 0,
            "model crossed into L5 storage")
    print("OK independent bank-0 model: tile/PolyG4/sprites; L5 untouched")
    return 0


if __name__ == "__main__":
    sys.exit(run())

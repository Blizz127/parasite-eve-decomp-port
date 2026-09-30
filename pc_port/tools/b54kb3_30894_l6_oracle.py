#!/usr/bin/env python3
"""Independent B54K-B3 oracle for func_80030894's mixed L6 group."""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
EXE_BASE = 0x8000F800
START = 0x80030D20
END = 0x80030F6C
WORDS = 147
WINDOW_SHA256 = (
    "51fe2651b8b7df743b8fc277f79101757a0b0904c4fbee00d28dc4a30bf19125"
)
MODEL_UNIQUE_BYTES = 106
MODEL_SHA256 = "0b043d826381f73ef82f05aa09c0eb6e7b274d546b5ba0c3b484afcadbef75d6"

JALS = [
    (0x80030D3C, 0x80077BC4),
    (0x80030D4C, 0x80077BC4),
    (0x80030DF8, 0x800370DC),
    (0x80030E0C, 0x80077B34),
    (0x80030E5C, 0x800370DC),
    (0x80030E70, 0x80077B34),
    (0x80030EBC, 0x800370DC),
    (0x80030ED0, 0x80077B34),
    (0x80030F34, 0x80077C44),
]

CRITICAL = {
    0x80030D1C: 0x32C200FF,  # preceding L5 delay slot
    0x80030D20: 0x93B50018,  # first included: bank load
    0x80030D24: 0x3C10800B,  # PolyG4 base high
    0x80030D28: 0x26100130,  # PolyG4 base low
    0x80030D2C: 0x0015A0C0,  # bank * 8
    0x80030D30: 0x02959021,  # bank * 9
    0x80030D34: 0x001290C0,  # bank * 72
    0x80030D3C: 0x0C01DEF1,  # first SetPolyG4
    0x80030D4C: 0x0C01DEF1,  # second SetPolyG4
    0x80030D54: 0x0295A023,  # bank * 7
    0x80030D58: 0x0014A080,  # bank * 28
    0x80030DF8: 0x0C00DC37,  # first wrap_sprt
    0x80030DFC: 0xA242001E,  # call delay completes second PolyG4
    0x80030E0C: 0x0C01DECD,  # first SetShadeTex
    0x80030E5C: 0x0C00DC37,  # second wrap_sprt
    0x80030E70: 0x0C01DECD,  # second SetShadeTex
    0x80030EBC: 0x0C00DC37,  # third wrap_sprt
    0x80030ED0: 0x0C01DECD,  # third SetShadeTex
    0x80030ED8: 0x0000B021,  # L6 tile counter reset
    0x80030EDC: 0x00151040,  # bank * 2
    0x80030EE0: 0x00551021,  # bank * 3
    0x80030EE4: 0x00029900,  # bank * 48
    0x80030F04: 0x3C15800A,  # tile base high
    0x80030F08: 0x26B5E358,  # tile base low
    0x80030F2C: 0x00118100,  # item * 16
    0x80030F34: 0x0C01DF11,  # SetTile
    0x80030F44: 0x03B11021,  # sp + item
    0x80030F48: 0x90420010,  # font byte snapshot
    0x80030F60: 0x2C420003,  # item < 3
    0x80030F64: 0x1440FFF1,  # bnez -> 0x80030F2C
    0x80030F68: 0x32D100FF,  # last included / delay slot
    0x80030F6C: 0x3C10800A,  # first excluded next-group base
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

    def poly_g4(self, head: int, colors: tuple[tuple[int, int, int], ...]) -> None:
        self.put8(head + 3, 8)
        self.put8(head + 7, 0x38)
        for offset, rgb in zip((4, 12, 20, 28), colors):
            for component, value in enumerate(rgb):
                self.put8(head + offset + component, value)

    def shaded_sprite(self, head: int, u: int) -> None:
        self.put8(head + 3, 6)
        self.put32(head + 4, 0xE1000234)
        self.put32(head + 8, 0)
        self.put8(head + 15, 0x65)
        for offset in (12, 13, 14):
            self.put8(head + offset, 0x80)
        self.put8(head + 20, u)
        self.put8(head + 21, 0xF4)
        self.put16(head + 22, 0x7E13)
        self.put16(head + 24, 8)
        self.put16(head + 26, 4)


def expected_l6() -> ByteModel:
    model = ByteModel()
    model.poly_g4(0x800B0130, (
        (0x00, 0x82, 0x36), (0x4A, 0xFF, 0x3B),
        (0x00, 0x82, 0x36), (0x4A, 0xFF, 0x3B),
    ))
    model.poly_g4(0x800B0154, (
        (0xFF, 0x3D, 0x81), (0x83, 0x13, 0x01),
        (0xFF, 0x3D, 0x81), (0x83, 0x13, 0x01),
    ))
    for head, u in zip((0x8009E0B8, 0x8009E2E8, 0x8009E320),
                       (0x50, 0x58, 0x60)):
        model.shaded_sprite(head, u)
    for index, value in enumerate((0x11, 0xA5, 0xFE)):
        tile = 0x8009E358 + index * 16
        model.put8(tile + 3, 3)
        model.put8(tile + 7, 0x60)
        for offset in (4, 5, 6):
            model.put8(tile + offset, value)
    return model


def run() -> int:
    path = find_exe()
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == EXE_SHA1,
            "retail executable SHA-1 mismatch")

    def word(address: int) -> int:
        return struct.unpack_from("<I", data, address - EXE_BASE)[0]

    window = data[START - EXE_BASE:END - EXE_BASE]
    require(len(window) == WORDS * 4, "L6 window size mismatch")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "L6 window SHA-256 mismatch")
    print(f"OK window: {WORDS} words / {len(window):#x} bytes, SHA-256 exact")
    for address, expected in CRITICAL.items():
        require(word(address) == expected,
                f"word {address:#010x}: {word(address):08X} != {expected:08X}")
    print(f"OK boundaries/scales/calls: {len(CRITICAL)} instruction-exact words")

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
    require(branches == [(0x80030F64, 0x80030F2C)],
            f"branch census changed: {branches!r}")
    print("OK control flow: nine native jal sites; one three-item back-edge")

    model = expected_l6()
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
    require(model.u8(0x8009E460 + 3) == 0,
            "model crossed into the next packet group")
    print("OK independent model: two PolyG4, three sprites, three tiles")
    return 0


if __name__ == "__main__":
    sys.exit(run())

#!/usr/bin/env python3
"""Independent B54K-B2 oracle for func_80030894's L5 sprite loop.

Reads only the SHA-1-exact retail executable. It verifies the complete
0x80030CA0..0x80030D20 window, both boundaries, the sole call/back-edge,
and the decoded bank/item stride chains. A separate zero-backed byte model
constructs all five bank-0 packets without importing native production code.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
EXE_BASE = 0x8000F800
START = 0x80030CA0
END = 0x80030D20
WORDS = 32
WINDOW_SHA256 = (
    "b45f5a6c9a6d1565f3fcc6affce6edc91a679ebe2bd538734d75fd2c933575d0"
)
MODEL_UNIQUE_BYTES = 80
MODEL_SHA256 = "e50f04b1034c31a1fc080584a39ab93843f624ad8adab1a33772571f8085c3f1"

JALS = [(0x80030CDC, 0x800370DC)]

# The full-window hash covers all 32 words. These selected words expose the
# semantic decode: boundaries, 140-byte bank stride, 28-byte item stride,
# packet arguments, CLUT/dimension stores, and the literal five-item bound.
CRITICAL = {
    0x80030C9C: 0x0000B021,  # predecessor: move s6,zero
    0x80030CA0: 0x93A30018,  # bank = lbu 24(sp)
    0x80030CA4: 0x3C12800A,  # D_8009E1D0 high
    0x80030CA8: 0x2652E1D0,  # D_8009E1D0 low
    0x80030CAC: 0x26530008,  # tail base = head base + 8
    0x80030CB0: 0x000310C0,  # bank * 8
    0x80030CB4: 0x00431021,  # bank * 9
    0x80030CB8: 0x00021080,  # bank * 36
    0x80030CBC: 0x00431023,  # bank * 35
    0x80030CC0: 0x00028880,  # bank * 140
    0x80030CC8: 0x000280C0,  # item * 8
    0x80030CCC: 0x02028023,  # item * 7
    0x80030CD0: 0x00108080,  # item * 28
    0x80030CDC: 0x0C00DC37,  # jal func_800370DC
    0x80030CE0: 0x03C02821,  # delay: a1 = fp tpage mode
    0x80030CE4: 0x26D60001,  # ++item
    0x80030CEC: 0x97A90020,  # clut = lhu 32(sp)
    0x80030CFC: 0xA429E1E6,  # sh clut at head + 0x16
    0x80030D04: 0xA6020010,  # tail width = 6
    0x80030D0C: 0xA6020012,  # tail height = 10
    0x80030D14: 0x2C420005,  # item < 5
    0x80030D18: 0x1440FFEB,  # bnez -> 0x80030CC8
    0x80030D1C: 0x32C200FF,  # delay: item & 0xff; last included
    0x80030D20: 0x93B50018,  # first excluded next-group bank load
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

    def wrap_sprt(self, head: int, mode: int) -> None:
        # Independent expansion of func_800370DC's three packet helpers.
        self.put8(head + 3, 1)
        self.put32(head + 4, 0xE1000200 | (mode & 0x9FF))
        tail = head + 8
        self.put8(tail + 3, 4)
        self.put8(tail + 7, 0x64)
        length = self.u8(head + 3) + self.u8(tail + 3) + 1
        require(length == 6, "sprite wrapper length changed")
        self.put8(head + 3, length)
        self.put32(tail, 0)


def expected_l5() -> ByteModel:
    model = ByteModel()
    for index in range(5):
        head = 0x8009E1D0 + index * 28
        tail = head + 8
        model.wrap_sprt(head, 0x34)
        model.put16(head + 0x16, 0x7E13)
        model.put16(tail + 0x10, 6)
        model.put16(tail + 0x12, 10)
    return model


def run() -> int:
    path = find_exe()
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == EXE_SHA1,
            "retail executable SHA-1 mismatch")

    def word(address: int) -> int:
        return struct.unpack_from("<I", data, address - EXE_BASE)[0]

    window = data[START - EXE_BASE:END - EXE_BASE]
    require(len(window) == WORDS * 4, "L5 window size mismatch")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "L5 window SHA-256 mismatch")
    print(f"OK window: {WORDS} words / {len(window):#x} bytes, SHA-256 exact")

    for address, expected in CRITICAL.items():
        require(word(address) == expected,
                f"word {address:#010x}: {word(address):08X} != {expected:08X}")
    print(f"OK boundaries/strides/stores: {len(CRITICAL)} instruction-exact words")

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
    require(branches == [(0x80030D18, 0x80030CC8)],
            f"branch census changed: {branches!r}")
    print("OK control flow: one native jal site; one five-item back-edge")

    model = expected_l5()
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
    require(model.u8(0x8009E1D0 + 140) == 0,
            "model crossed into bank-1 L5 storage")
    require(model.u8(0x800B0130) == 0,
            "model crossed into the next packet group")
    print("OK independent bank-0 model: five compound sprites; next group untouched")
    return 0


if __name__ == "__main__":
    sys.exit(run())

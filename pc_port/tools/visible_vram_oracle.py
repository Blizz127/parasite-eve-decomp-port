#!/usr/bin/env python3
"""Independent VIS1 oracle for a raw PSX-VRAM/PPM artifact pair.

The production exporter is not imported.  This script decodes every
little-endian BGR555/STP word independently, compares every RGB byte, and
reports the occupied word bounds and deterministic digests.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

WIDTH = 1024
HEIGHT = 512
PIXELS = WIDTH * HEIGHT
RAW_BYTES = PIXELS * 2
HEADER = b"P6\n1024 512\n255\n"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FAIL: {message}")


def expand5(value: int) -> int:
    value &= 0x1F
    return (value << 3) | (value >> 2)


def run(raw_path: pathlib.Path, ppm_path: pathlib.Path) -> int:
    raw = raw_path.read_bytes()
    ppm = ppm_path.read_bytes()
    require(len(raw) == RAW_BYTES,
            f"raw size {len(raw)} != {RAW_BYTES}")
    require(ppm.startswith(HEADER), "PPM header mismatch")
    payload = ppm[len(HEADER):]
    require(len(payload) == PIXELS * 3,
            f"PPM payload size {len(payload)} != {PIXELS * 3}")

    expected = bytearray(PIXELS * 3)
    nonzero = 0
    min_x, min_y = WIDTH, HEIGHT
    max_x = max_y = -1
    for ordinal, (pixel,) in enumerate(struct.iter_unpack("<H", raw)):
        out = ordinal * 3
        expected[out] = expand5(pixel)
        expected[out + 1] = expand5(pixel >> 5)
        expected[out + 2] = expand5(pixel >> 10)
        if pixel != 0:
            x = ordinal % WIDTH
            y = ordinal // WIDTH
            nonzero += 1
            min_x, min_y = min(min_x, x), min(min_y, y)
            max_x, max_y = max(max_x, x), max(max_y, y)

    require(payload == expected, "PPM pixels differ from independent decode")
    require(nonzero != 0, "VRAM snapshot is entirely zero")
    print(f"raw_sha256={hashlib.sha256(raw).hexdigest()}")
    print(f"ppm_sha256={hashlib.sha256(ppm).hexdigest()}")
    print(f"nonzero_words={nonzero}")
    print(f"nonzero_bounds={min_x},{min_y}..{max_x},{max_y}")
    print("decode=PASS all 524288 words / 1572864 RGB bytes")
    return 0


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        raise SystemExit(f"usage: {argv[0]} RAW_VRAM PPM")
    return run(pathlib.Path(argv[1]), pathlib.Path(argv[2]))


if __name__ == "__main__":
    sys.exit(main(sys.argv))

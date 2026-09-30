#!/usr/bin/env python3
"""Authenticate the PE.IMG overlay containing func_801909B4."""

from __future__ import annotations

import argparse
import hashlib
import pathlib
import struct

PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
LBA_START = 0x03D2
LBA_END = 0x0457
LOAD_VA = 0x8018EFF0
FUNC_VA = 0x801909B4
FUNC_END = 0x801918F8
OVERLAY_SHA1 = "a0b604ead810592dd9a8d74f7c74fe94151f4d32"
FUNC_SHA256 = "9072713338b26c335c1a31964282105dcd5f14951950c2d24585fa5948554d30"


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("peimg", type=pathlib.Path)
    args = parser.parse_args()
    data = args.peimg.read_bytes()
    require(hashlib.sha1(data).hexdigest() == PEIMG_SHA1, "PE.IMG SHA-1")

    overlay = data[LBA_START * 0x800:LBA_END * 0x800]
    require(len(overlay) == 0x42800, "overlay extent")
    require(hashlib.sha1(overlay).hexdigest() == OVERLAY_SHA1, "overlay SHA-1")

    offset = FUNC_VA - LOAD_VA
    body = overlay[offset:offset + (FUNC_END - FUNC_VA)]
    require(len(body) == 0xF44, "function extent")
    require(hashlib.sha256(body).hexdigest() == FUNC_SHA256, "function SHA-256")
    words = [struct.unpack_from("<I", body, i)[0]
             for i in range(0, len(body), 4)]
    require(words[:2] == [0x27BDFFB8, 0x3C05800C], "function prologue")
    require(words[-3:] == [0x27BD0048, 0x03E00008, 0x00000000],
            "function epilogue")
    require(struct.unpack_from("<I", overlay, offset + len(body))[0] ==
            0x27BDFFC8, "next function boundary")

    calls = [(FUNC_VA + i, 0x80000000 | ((word & 0x03FFFFFF) << 2))
             for i, word in enumerate(words)
             if word >> 26 == 3]
    require(len(calls) == 70 and len({target for _, target in calls}) == 32,
            "direct-call census")
    print("  OK PE.IMG and [03D2,0457) overlay identities")
    print("  OK func_801909B4: 0xF44 bytes / 977 words / normal return")
    print("  OK call census: 70 sites / 32 unique targets")
    print("  OK next function starts at 0x801918F8")
    print("\nB54K-O overlay oracle: PASS.")


if __name__ == "__main__":
    main()

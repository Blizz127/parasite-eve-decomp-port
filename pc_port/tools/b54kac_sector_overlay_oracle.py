#!/usr/bin/env python3
"""Independent B54K-AC oracle for the retail CD sector contract."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import re
import struct
import subprocess

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
OVERLAY_SHA1 = "a0b604ead810592dd9a8d74f7c74fe94151f4d32"
OVERLAY_START = 0x03D2
OVERLAY_END = 0x0457
OVERLAY_LOAD = 0x8018EFF0
OVERLAY_FNV1A64 = 0x55EC1574DF7D6A3D


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def exe_u32(exe: bytes, va: int) -> int:
    off = 0x800 + va - 0x80010000
    return struct.unpack_from("<I", exe, off)[0]


def jal_target(word: int, pc: int) -> int:
    return ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def fnv1a64(data: bytes) -> int:
    value = 1469598103934665603
    for byte in data:
        value ^= byte
        value = (value * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return value


def run(argv: list[str], expected: int,
        env: dict[str, str] | None = None) -> str:
    result = subprocess.run(argv, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True,
                            timeout=180, check=False, env=env)
    require(result.returncode == expected,
            f"exit {result.returncode}, expected {expected}\n"
            f"{result.stdout[-3000:]}")
    return result.stdout


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("peimg", type=pathlib.Path)
    parser.add_argument("exe", type=pathlib.Path)
    parser.add_argument("--port", type=pathlib.Path)
    parser.add_argument("--tests", type=pathlib.Path)
    parser.add_argument("--disc", type=pathlib.Path)
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[2]
    exe = args.exe.read_bytes()
    peimg = args.peimg.read_bytes()
    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1, "retail EXE identity")
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1,
            "PE.IMG identity")

    # func_8006E6A8 forwards its third argument as func_8006E6D4's a3.
    require(exe_u32(exe, 0x8006E6B4) == 0x00C03821 and
            exe_u32(exe, 0x8006E6B8) == 0x00002821 and
            jal_target(exe_u32(exe, 0x8006E6BC), 0x8006E6BC) ==
                0x8006E6D4 and
            exe_u32(exe, 0x8006E6C0) == 0x00403021,
            "6E6A8 verbatim argument forwarding")

    # func_8006E6D4 retains incoming a3 in s3, then passes it as a1 to
    # func_80080E34. That callee retains a1 in s4 and publishes it to
    # D_8009B6B4, the transfer's remaining-sector word.
    require(exe_u32(exe, 0x8006E714) == 0x00E09821 and
            exe_u32(exe, 0x8006E784) == 0x02602821 and
            jal_target(exe_u32(exe, 0x8006E78C), 0x8006E78C) ==
                0x80080E34 and
            exe_u32(exe, 0x80080E44) == 0x00A0A021 and
            exe_u32(exe, 0x80080E98) == 0xAE14FFE8,
            "6E6D4 -> 80E34 sector-count path")

    # func_8006E834 supplies end-start directly as a3. Its authenticated
    # table range is 0x85 sectors, not 0x85 bytes.
    require(jal_target(exe_u32(exe, 0x8006E8C4), 0x8006E8C4) ==
                0x8006E6D4 and
            exe_u32(exe, 0x8006E8C8) == 0x00473823,
            "6E834 end-start sector count")
    overlay = peimg[OVERLAY_START * 0x800:OVERLAY_END * 0x800]
    require(len(overlay) == 0x42800 and
            hashlib.sha1(overlay).hexdigest() == OVERLAY_SHA1 and
            fnv1a64(overlay) == OVERLAY_FNV1A64,
            "133-sector overlay extent")
    print("  OK retail ABI: a3 -> 80E34 a1 -> D_8009B6B4 sector count")
    print("  OK overlay: [03D2,0457) = 133 sectors = 0x42800 bytes")

    wrapper = (root / "pc_port/game/boot/func_8006E6A8_port.c").read_text()
    provider = (root / "pc_port/platform/pe_libcd.c").read_text()
    xa = (root / "pc_port/game/boot/func_80029810_port.c").read_text()
    require("func_8006E6D4(lba, 0, dest, sectors)" in wrapper and
            "<< 11" not in wrapper and
            "sectors * PE_DISC_USER_SECTOR" in provider and
            "(int)(chunk << 11)" not in xa,
            "native sector contract")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    if args.disc:
        disc = args.disc
    else:
        disc = pathlib.Path(os.environ.get(
            "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    for test_filter in ("read_", "6E6A8", "B54KY"):
        env = os.environ.copy()
        env["PE_TEST_FILTER"] = test_filter
        output = run([str(tests)], 0, env)
        require("0 failed" in output, f"focused {test_filter} tests")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    reached_movie_frontier = (
        re.search(r"func_801924F8_80192[0-9A-Fa-f]+_cut", strict) is not None or
        ("func_8010C0FC" in strict and "called from: func_8010BE3C" in strict)
    )
    require(reached_movie_frontier,
            "production reached func_801924F8 internal frontier")
    load = run([str(port), "--headless", "--disc-image", str(disc),
                "--disc-load-test"], 0)
    require("disc_load_issue=1_dest=0x8018EFF0_len=32768" in load and
            "disc_load_test_ok" in load,
            "sector-based production load smoke")
    print("  OK native: focused guards/forwarding/full-overlay contracts")
    print("  OK production: strict reaches func_801924F8; load smoke passes")
    print("\nB54K-AC sector/overlay oracle: PASS.")


if __name__ == "__main__":
    main()

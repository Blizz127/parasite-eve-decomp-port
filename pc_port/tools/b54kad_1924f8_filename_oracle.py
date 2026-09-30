#!/usr/bin/env python3
"""Independent B54K-AD oracle for func_801924F8 filename/search phase."""

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
BASE = 0x03D2 * 0x800
LOAD = 0x8018EFF0
START = 0x801924F8
CUT = 0x80192614
END = 0x80192934
PREFIX_SHA256 = "c5ac60f55bb880129bf6cb3363a84d1a3febec10fea7d81a78ea809658eb6c8c"
FULL_SHA256 = "ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a"


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def u32(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def exe_u32(exe: bytes, va: int) -> int:
    return u32(exe, 0x800 + va - 0x80010000)


def jal_target(word: int, pc: int) -> int:
    return ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


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
    peimg = args.peimg.read_bytes()
    exe = args.exe.read_bytes()
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1, "PE.IMG identity")
    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1, "retail EXE identity")

    off = BASE + START - LOAD
    body = peimg[off:off + END - START]
    prefix = body[:CUT - START]
    require(len(body) == 271 * 4 and
            hashlib.sha256(body).hexdigest() == FULL_SHA256,
            "complete func_801924F8 identity")
    require(len(prefix) == 71 * 4 and
            hashlib.sha256(prefix).hexdigest() == PREFIX_SHA256 and
            u32(body, CUT - START) == 0x3C06801D,
            "71-word prefix and exact cut")
    require(u32(body, 0x8019292C - START) == 0x03E00008 and
            u32(body, 0x80192930 - START) == 0,
            "complete function return")

    calls = {
        pc: jal_target(u32(body, pc - START), pc)
        for pc in (0x801925AC, 0x801925C4, 0x801925CC,
                   0x801925E0, 0x801925F8)
    }
    require(calls == {
        0x801925AC: 0x800719F4,
        0x801925C4: 0x800719F4,
        0x801925CC: 0x8007F72C,
        0x801925E0: 0x8007F778,
        0x801925F8: 0x80081414,
    }, "filename/search callee census")
    require(exe_u32(exe, 0x800719F4) == 0x240A00A0 and
            exe_u32(exe, 0x800719F8) == 0x01400008 and
            exe_u32(exe, 0x800719FC) == 0x24090015,
            "BIOS A(15h) strcat trampoline")
    for va, expected in ((0x8018F2E4, b"\\FMV1\0"),
                         (0x8018F2EC, b"\\FMV2\0")):
        pos = BASE + va - LOAD
        require(peimg[pos:pos + len(expected)] == expected,
                f"prefix string {va:08X}")
    require(peimg[BASE + 0x8018F2C4 - LOAD:
                  BASE + 0x8018F2C4 - LOAD + 14] == b"\\FMV001.STR;1\0",
            "Disc 1 record-1 suffix")
    print("  OK retail: 71-word prefix, A(15h) strcat x2, CD search loop")
    print("  OK names: index <21 uses \\FMV1; index >=21 uses \\FMV2")

    source = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    provider = (root / "pc_port/platform/func_800719E4_port.c").read_text()
    source_cut = re.search(r'func_801924F8_(80192[0-9A-Fa-f]+)_cut', source)
    require("record_index < 21u" in source and
            "func_800719F4" in source and
            "func_80081414(0x801D0DC4u, filename)" in source and
            source_cut is not None and
            int(source_cut.group(1), 16) >= CUT and
            "return strcat(destination, source)" in provider and
            "m0360i" not in source and "0xA8066048" not in source,
            "native scope")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    if args.disc:
        disc = args.disc
    else:
        disc = pathlib.Path(os.environ.get(
            "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    for test_filter in ("B54KY", "B54KAD"):
        env = os.environ.copy()
        env["PE_TEST_FILTER"] = test_filter
        output = run([str(tests)], 0, env)
        require("0 failed" in output, f"focused {test_filter} tests")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    strict_cut = re.search(r'func_801924F8_(80192[0-9A-Fa-f]+)_cut', strict)
    reached_later_mdec = ("func_8010C0FC" in strict and
                          "called from: func_8010BE3C" in strict)
    require((strict_cut is not None and
             int(strict_cut.group(1), 16) >= CUT and
             "called from: func_801924F8" in strict) or reached_later_mdec,
            "strict frontier did not reach the authenticated filename phase")
    print("  OK runtime: Disc 1 FMV001 and synthetic FMV2 threshold paths")
    print("  OK production: strict frontier reaches or passes 0x80192614")
    print("\nB54K-AD func_801924F8 filename oracle: PASS.")


if __name__ == "__main__":
    main()

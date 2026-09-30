#!/usr/bin/env python3
"""Independent B54K-AE oracle for func_801924F8 movie-state setup."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import struct
import subprocess

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
BASE = 0x03D2 * 0x800
LOAD = 0x8018EFF0
START = 0x801924F8
SETUP_START = 0x80192614
CUT = 0x80192728
END = 0x80192934
SETUP_SHA256 = "dc9862aea7cf86ce063394d2c9ddae42774de12940250c8c219de3ab42ef1eca"
PREFIX_SHA256 = "526fb8ca0e5558585d3fc06b8d6977d9999d648c57a5c259d69b214f3fd2fc1e"
FULL_SHA256 = "ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a"


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def u32(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


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
    setup = body[SETUP_START - START:CUT - START]
    prefix = body[:CUT - START]
    require(len(body) == 271 * 4 and
            hashlib.sha256(body).hexdigest() == FULL_SHA256,
            "complete func_801924F8 identity")
    require(len(setup) == 69 * 4 and
            hashlib.sha256(setup).hexdigest() == SETUP_SHA256,
            "69-word movie-state block identity")
    require(len(prefix) == 140 * 4 and
            hashlib.sha256(prefix).hexdigest() == PREFIX_SHA256,
            "140-word combined prefix identity")
    require(not any((u32(setup, off) >> 26) == 3
                    for off in range(0, len(setup), 4)),
            "unexpected call inside state-only block")
    next_word = u32(body, CUT - START)
    require(jal_target(next_word, CUT) == 0x8010BE3C and
            u32(body, CUT + 4 - START) == 0x00002021,
            "exact pre-call cut")
    print("  OK retail: 69-word call-free state block, exact 0x80192728 cut")
    print("  OK prefix: 140/271 authenticated words of func_801924F8")

    source = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    tests_source = (root / "pc_port/tests/test_native.c").read_text()
    required_source = (
        "PE_StoreU32(0x801D0DDCu, PE_LoadU32(0x801D0DC4u))",
        "display_buffer & 0xFFu",
        "active_pair + 22u",
        "active_pair + 24u",
        "PE_LoadU8(0x800B0DBBu) != 0u ? 24u : 16u",
        "func_8010BE3C(0)",
    )
    require(all(token in source for token in required_source) and
            "m0360i" not in source and "0xA8066048" not in source,
            "native scope")
    require("TEST(\"B54KAE_movie_state_setup\")" in tests_source and
            "PE_StoreU32(0x800ACDDCu, 1u)" in tests_source and
            "PE_LoadU16(0x801D148Eu) == 33u" in tests_source and
            "PE_LoadU16(0x801D1490u) == 16u" in tests_source,
            "independent active-pair/kind control")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    if args.disc:
        disc = args.disc
    else:
        disc = pathlib.Path(os.environ.get(
            "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    for test_filter, pass_name in (
            ("B54KY", "B54KY_192CE8_real_disc_issue_poll_and_boundary... PASS"),
            ("B54KAE", "B54KAE_movie_state_setup... PASS")):
        env = os.environ.copy()
        env["PE_TEST_FILTER"] = test_filter
        output = run([str(tests)], 0, env)
        require(pass_name in output and "0 failed" in output,
                f"focused {test_filter} contract")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    reached_state_or_later = (
        ("func_801924F8_80192728_cut" in strict and
         "called from: func_801924F8" in strict) or
        ("func_801924F8_80192730_cut" in strict and
         "called from: func_801924F8" in strict) or
        ("func_801924F8_80192740_cut" in strict and
         "called from: func_801924F8" in strict) or
        ("func_801924F8_80192750_cut" in strict and
         "called from: func_801924F8" in strict) or
        ("func_801924F8_80192770_cut" in strict and
         "called from: func_801924F8" in strict) or
        ("func_8007F0C8_completion_selector" in strict and
         "called from: func_8007F0C8" in strict) or
        ("func_8010C0FC" in strict and
         "called from: func_8010BE3C" in strict)
    )
    require(reached_state_or_later,
            "strict frontier did not reach the authenticated state block")
    print("  OK runtime: real Disc 1 and synthetic active-pair controls")
    print("  OK production: strict frontier reaches or passes 0x80192728")
    print("\nB54K-AE func_801924F8 state oracle: PASS.")


if __name__ == "__main__":
    main()

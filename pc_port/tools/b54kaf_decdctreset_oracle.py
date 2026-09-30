#!/usr/bin/env python3
"""Independent B54K-AF oracle for the retail DecDCTReset wrapper."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import struct
import subprocess

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
MOVIE_START = 0x039F * 0x800
MOVIE_END = 0x03C5 * 0x800
MOVIE_LOAD = 0x8010BCF8
MOVIE_SHA256 = "d0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5ba8d0b40"
WRAPPER = 0x8010BE3C
WRAPPER_END = 0x8010BE70
WRAPPER_SHA256 = "d29b6f4ff5d6c611a9193862caffbce26eae005f5cca46747f4fc780ffdc4f9d"
INTERNAL = 0x8010C0FC
INTERNAL_END = 0x8010C1EC
INTERNAL_SHA256 = "53670ac94ba335b065bac7d63504e80a75a3bffe01cc3dff07c1582da260808b"
CALLER_MODULE = 0x03D2 * 0x800
CALLER_LOAD = 0x8018EFF0
CALLER = 0x80192728
CALLER_PREFIX_START = 0x801924F8
CALLER_PREFIX_END = 0x80192730
CALLER_PREFIX_SHA256 = "d633247323e92b405eedfcea7d79d2078bc4217f77c263740d4dbeccc7dab180"


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

    movie = peimg[MOVIE_START:MOVIE_END]
    require(len(movie) == 38 * 0x800 and
            hashlib.sha256(movie).hexdigest() == MOVIE_SHA256,
            "38-sector movie module identity")
    require(movie[4:4 + 25] == b"MDEC_rest:bad option(%d)\n",
            "MDEC reset diagnostic")
    wrapper_off = WRAPPER - MOVIE_LOAD
    wrapper = movie[wrapper_off:wrapper_off + WRAPPER_END - WRAPPER]
    internal_off = INTERNAL - MOVIE_LOAD
    internal = movie[internal_off:internal_off + INTERNAL_END - INTERNAL]
    require(len(wrapper) == 13 * 4 and
            hashlib.sha256(wrapper).hexdigest() == WRAPPER_SHA256,
            "13-word wrapper identity")
    require(len(internal) == 60 * 4 and
            hashlib.sha256(internal).hexdigest() == INTERNAL_SHA256,
            "60-word internal reset boundary identity")
    require(u32(wrapper, 0x0C) == 0x16000003 and
            jal_target(u32(wrapper, 0x14), WRAPPER + 0x14) == 0x80073C94 and
            jal_target(u32(wrapper, 0x1C), WRAPPER + 0x1C) == INTERNAL and
            u32(wrapper, 0x2C) == 0x03E00008 and
            u32(wrapper, 0x30) == 0x27BD0018,
            "mode guard, callees, or return boundary")

    caller_off = CALLER_MODULE + CALLER - CALLER_LOAD
    require(jal_target(u32(peimg, caller_off), CALLER) == WRAPPER and
            u32(peimg, caller_off + 4) == 0x00002021,
            "production mode-zero caller")
    prefix_off = CALLER_MODULE + CALLER_PREFIX_START - CALLER_LOAD
    prefix = peimg[prefix_off:prefix_off + CALLER_PREFIX_END - CALLER_PREFIX_START]
    require(len(prefix) == 142 * 4 and
            hashlib.sha256(prefix).hexdigest() == CALLER_PREFIX_SHA256,
            "142-word func_801924F8 prefix")
    print("  OK retail: MDEC module, 13-word DecDCTReset wrapper, 60-word next boundary")
    print("  OK caller: func_801924F8 invokes mode 0 at 0x80192728")

    wrapper_source = (root / "pc_port/game/boot/func_8010BE3C_port.c").read_text()
    caller_source = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    require("if (mode == 0)" in wrapper_source and
            "func_80073C94()" in wrapper_source and
            "func_8010C0FC(mode)" in wrapper_source and
            "func_8010BE3C(0)" in caller_source and
            "m0360i" not in wrapper_source + caller_source and
            "0xA8066048" not in wrapper_source + caller_source,
            "native wrapper scope")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    if args.disc:
        disc = args.disc
    else:
        disc = pathlib.Path(os.environ.get(
            "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KAF"
    focused = run([str(tests)], 0, env)
    require("B54KAF_dec_dct_reset_wrapper... PASS" in focused and
            "0 failed" in focused,
            "mode-zero/mode-one wrapper controls")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    reached_reset_or_later = (
        ("func_8010C0FC" in strict and
         "called from: func_8010BE3C" in strict) or
        ("func_8007F0C8_completion_selector" in strict and
         "called from: func_8007F0C8" in strict)
    )
    require(reached_reset_or_later,
            "strict path did not reach the authenticated MDEC reset")
    print("  OK runtime: mode 0 resets callbacks; mode 1 does not")
    print("  OK production: strict frontier reaches or passes func_8010C0FC")
    print("\nB54K-AF DecDCTReset wrapper oracle: PASS.")


if __name__ == "__main__":
    main()

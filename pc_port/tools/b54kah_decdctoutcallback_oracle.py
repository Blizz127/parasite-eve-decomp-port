#!/usr/bin/env python3
"""Independent B54K-AH oracle for retail DecDCToutCallback registration."""

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
WRAPPER = 0x8010C0D8
WRAPPER_END = 0x8010C0FC
WRAPPER_SHA256 = "e74b3ac9231b36b93ae760b3c2cd8c018905679c0da656f07b8cbd164933d59b"
CALLER_MODULE = 0x03D2 * 0x800
CALLER_LOAD = 0x8018EFF0
CALLER_START = 0x801924F8
CALLER_END = 0x80192740
CALLER_SHA256 = "34b3cf3eec558cde8a6979762bd762d50873bbc1d2c76f489e5e38d1762fad7e"
CALL_SITE = 0x80192738
CALLBACK = 0x80191DC8


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
    wrapper_off = WRAPPER - MOVIE_LOAD
    wrapper = movie[wrapper_off:wrapper_off + WRAPPER_END - WRAPPER]
    words = struct.unpack("<9I", wrapper)
    require(len(wrapper) == 9 * 4 and
            hashlib.sha256(wrapper).hexdigest() == WRAPPER_SHA256,
            "9-word DecDCToutCallback identity")
    require(words == (
        0x27BDFFE8, 0xAFBF0010, 0x00802821,
        0x0C01CF3D, 0x24040001, 0x8FBF0010,
        0x27BD0018, 0x03E00008, 0x00000000),
        "wrapper is not func_80073CF4(1, callback) plus canonical return")
    require(jal_target(words[3], WRAPPER + 0x0C) == 0x80073CF4,
            "wrapper callee")

    caller_off = CALLER_MODULE + CALLER_START - CALLER_LOAD
    caller = peimg[caller_off:caller_off + CALLER_END - CALLER_START]
    call_off = CALL_SITE - CALLER_START
    require(len(caller) == 146 * 4 and
            hashlib.sha256(caller).hexdigest() == CALLER_SHA256,
            "146-word func_801924F8 prefix")
    require(u32(caller, call_off - 8) == 0x3C048019 and
            u32(caller, call_off - 4) == 0x24841DC8 and
            jal_target(u32(caller, call_off), CALL_SITE) == WRAPPER and
            u32(caller, call_off + 4) == 0x24100001 and
            u32(peimg, caller_off + call_off + 8) == 0x3C04801D,
            "exact callback argument, call, delay slot, or next boundary")
    print("  OK retail: 9-word DecDCToutCallback wrapper and canonical return")
    print("  OK caller: exact 0x80191DC8 registration; next instruction 0x80192740")

    wrapper_source = (root / "pc_port/game/boot/func_8010BE3C_port.c").read_text()
    caller_source = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    require("func_80073CF4(1u, callback)" in wrapper_source and
            "func_8010C0D8(0x80191DC8u)" in caller_source and
            "func_801924F8_80192790_cut" in caller_source and
            "m0360i" not in wrapper_source + caller_source and
            "0xA8066048" not in wrapper_source + caller_source,
            "native callback-registration scope")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    disc = args.disc or pathlib.Path(os.environ.get(
        "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KAH"
    focused = run([str(tests)], 0, env)
    require("B54KAH_dec_dct_out_callback_registration... PASS" in focused and
            "0 failed" in focused,
            "registration and no-delivery controls")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    require("func_8007F0C8_completion_selector" in strict and
            "called from: func_8007F0C8" in strict,
            "strict path did not reach the post-registration cut")
    print("  OK runtime: DMA slot 1 only; DICR enabled; no callback delivery")
    print("  OK production: strict path continues through the later record-pool initializer")
    print("\nB54K-AH DecDCToutCallback oracle: PASS.")


if __name__ == "__main__":
    main()

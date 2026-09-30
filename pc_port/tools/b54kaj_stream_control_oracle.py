#!/usr/bin/env python3
"""Independent B54K-AJ oracle for the stream-control initializer."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import struct
import subprocess

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
SETUP_FILE = 0x6CB04
SETUP = 0x8007C304
SETUP_END = 0x8007C388
SETUP_SHA256 = "b36aa720216a101d55e1f1fc01fdb4de5d469616649d35efb6e3637cc71992f2"
SETTER_FILE = 0x6CD44
SETTER = 0x8007C544
SETTER_END = 0x8007C560
SETTER_SHA256 = "0b85dee972b3ff9e4c01682f71ae2e5c63ed9c55d0934f20911104b6fb66f3db"
CALLER_MODULE = 0x03D2 * 0x800
CALLER_LOAD = 0x8018EFF0
CALLER_START = 0x801924F8
CALLER_END = 0x80192770
CALLER_SHA256 = "fbbb2ffe13e98fa176ce686b491f28fc0e44db98c594365a802f97ebbefd5196"
CALL_SITE = 0x80192768


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

    setup = exe[SETUP_FILE:SETUP_FILE + SETUP_END - SETUP]
    setter = exe[SETTER_FILE:SETTER_FILE + SETTER_END - SETTER]
    require(len(setup) == 33 * 4 and
            hashlib.sha256(setup).hexdigest() == SETUP_SHA256,
            "33-word setup identity")
    require(len(setter) == 7 * 4 and
            hashlib.sha256(setter).hexdigest() == SETTER_SHA256,
            "7-word setter identity")
    setup_words = struct.unpack("<33I", setup)
    setter_words = struct.unpack("<7I", setter)
    require(jal_target(setup_words[8], SETUP + 0x20) == SETTER and
            setup_words[9] == 0x24040001 and
            setup_words[10] == 0x32100001 and
            setup_words[-2:] == (0x03E00008, 0x27BD0020),
            "setter call, constant mode, bit mask, or return")
    require(setter_words == (
        0x3C01800C, 0xAC240DC0, 0x3C01800B, 0xAC256918,
        0x3C01800C, 0x03E00008, 0xAC260DBC),
        "three-global setter body")
    print("  OK retail: 33-word setup and 7-word three-global setter")
    print("  OK semantics: constant setter mode, signed bounds, callback/aux state")

    caller_off = CALLER_MODULE + CALLER_START - CALLER_LOAD
    caller = peimg[caller_off:caller_off + CALLER_END - CALLER_START]
    call_off = CALL_SITE - CALLER_START
    require(len(caller) == 158 * 4 and
            hashlib.sha256(caller).hexdigest() == CALLER_SHA256,
            "158-word func_801924F8 prefix")
    require(u32(caller, call_off - 24) == 0x24040001 and
            u32(caller, call_off - 20) == 0x3C02801D and
            u32(caller, call_off - 16) == 0x8C4211AC and
            u32(caller, call_off - 12) == 0x2406FFFF and
            u32(caller, call_off - 8) == 0x84450006 and
            u32(caller, call_off - 4) == 0x00003821 and
            jal_target(u32(caller, call_off), CALL_SITE) == SETUP and
            u32(caller, call_off + 4) == 0xAFA00010 and
            u32(peimg, caller_off + call_off + 8) == 0x0C01FDCB,
            "production arguments, call, stack argument, or next boundary")
    print("  OK caller: (1, signed record+6, -1, 0, 0); next call at 0x80192770")

    source = (root / "pc_port/game/boot/func_8007A214_port.c").read_text()
    caller_source = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    require("func_8007C544(1u, start, end)" in source and
            "PE_StoreU32(GA_STREAM_OPTION, mode & 1u)" in source and
            "func_8007C304(1u, (int32_t)(int16_t)PE_LoadU16(record + 6u)" in caller_source and
            "func_801924F8_80192790_cut" in caller_source and
            "m0360i" not in source + caller_source and
            "0xA8066048" not in source + caller_source,
            "native stream-control scope")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    disc = args.disc or pathlib.Path(os.environ.get(
        "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KAJ"
    focused = run([str(tests)], 0, env)
    require("B54KAJ_stream_control_initializer... PASS" in focused and
            "0 failed" in focused,
            "stream state, widths, and bit-mask controls")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    require("func_8007F0C8_completion_selector" in strict and
            "called from: func_8007F0C8" in strict,
            "strict path did not reach the post-setup cut")
    print("  OK runtime: focused state/width controls")
    print("  OK production: strict path continues through the CD idle wait")
    print("\nB54K-AJ stream-control oracle: PASS.")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Independent B54K-AK oracle for the movie CD idle wait."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import struct
import subprocess

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
CALLER_MODULE = 0x03D2 * 0x800
CALLER_LOAD = 0x8018EFF0
CALLER_START = 0x801924F8
CALLER_END = 0x80192790
CALLER_SHA256 = "f82288e0a5e751bb7399cd1c34d9d60904de59150b9f98ad0f539d090dbac8eb"
WAIT_START = 0x80192770


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

    caller_off = CALLER_MODULE + CALLER_START - CALLER_LOAD
    caller = peimg[caller_off:caller_off + CALLER_END - CALLER_START]
    wait_off = WAIT_START - CALLER_START
    wait_words = struct.unpack_from("<8I", caller, wait_off)
    require(len(caller) == 166 * 4 and
            hashlib.sha256(caller).hexdigest() == CALLER_SHA256,
            "166-word func_801924F8 prefix")
    require(jal_target(wait_words[0], WAIT_START) == 0x8007F72C and
            wait_words[1] == 0x00000000 and
            wait_words[2:4] == (0x1450FFFD, 0x00000000) and
            jal_target(wait_words[4], WAIT_START + 0x10) == 0x8007F778 and
            wait_words[5:] == (0x00000000, 0x1440FFF9, 0x24040002),
            "CdReady/queue call order, back edges, or final delay slot")
    require(u32(peimg, caller_off + wait_off + 0x20) == 0x3C05801D,
            "next boundary at 0x80192790")
    print("  OK retail: exact 8-word CdReady/queue drain loop")
    print("  OK boundary: next unresolved command setup begins at 0x80192790")

    source = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    require("while (func_8007F72C() != 1)" in source and
            "while (func_8007F778() != 0)" in source and
            "func_801924F8_80192790_cut" in source and
            "PE_StoreU32(0x8009B574u" not in source and
            "PE_StoreU32(0x800A3608u" not in source,
            "native wait changed CD authority or cut")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    disc = args.disc or pathlib.Path(os.environ.get(
        "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    for test_filter, pass_name in (
            ("B54KY", "B54KY_192CE8_real_disc_issue_poll_and_boundary... PASS"),
            ("B54KAE", "B54KAE_movie_state_setup... PASS")):
        env = os.environ.copy()
        env["PE_TEST_FILTER"] = test_filter
        output = run([str(tests)], 0, env)
        require(pass_name in output and "0 failed" in output,
                f"focused {test_filter} idle-state contract")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    require("func_8007F0C8_completion_selector" in strict and
            "called from: func_8007F0C8" in strict,
            "strict path did not reach the post-wait cut")
    print("  OK runtime: real and synthetic idle states pass without mutation")
    print("  OK production: strict frontier is 0x80192790")
    print("\nB54K-AK CD idle-wait oracle: PASS.")


if __name__ == "__main__":
    main()

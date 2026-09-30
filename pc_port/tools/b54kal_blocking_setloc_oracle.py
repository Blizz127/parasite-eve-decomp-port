#!/usr/bin/env python3
"""Independent B54K-AL oracle for the movie's blocking CdlSetloc arm."""

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
CALLER_END = 0x801927A0
CALLER_COMPLETE_END = 0x80192934
CALLER_SHA256 = "fb44b0aeb6ab2d3e6c52cf5242603e038dc32cd096062ade16a0b6cf713c9f39"
WRAPPER_OFF = 0x7155C
WRAPPER_SIZE = 0x68
WRAPPER_SHA256 = "78dab759fceb6569cc6d7b903db2e16e026f46b39c75a32f351289c1af6db824"


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

    wrapper = exe[WRAPPER_OFF:WRAPPER_OFF + WRAPPER_SIZE]
    require(len(wrapper) == 26 * 4 and
            hashlib.sha256(wrapper).hexdigest() == WRAPPER_SHA256,
            "complete 26-word func_80080D5C identity")
    require(u32(exe, WRAPPER_OFF - 4) == 0x00000000 and
            u32(exe, WRAPPER_OFF + WRAPPER_SIZE) == 0x27BDFFE0 and
            u32(wrapper, 0x60) == 0x03E00008 and
            u32(wrapper, 0x64) == 0x27BD0020,
            "wrapper boundaries or canonical return")
    require(peimg[CALLER_MODULE:CALLER_MODULE + 0x4000].count(
                (0x0C020357).to_bytes(4, "little")) == 3,
            "overlay exact-start call census")
    require(exe[0x22BC:0x22C6] == b"CdlSetloc\0",
            "retail command-2 name-table witness")

    caller_off = CALLER_MODULE + CALLER_START - CALLER_LOAD
    caller = peimg[caller_off:caller_off + CALLER_END - CALLER_START]
    require(len(caller) == 170 * 4 and
            hashlib.sha256(caller).hexdigest() == CALLER_SHA256,
            "170-word func_801924F8 prefix")
    call_off = 0x80192790 - CALLER_START
    words = struct.unpack_from("<4I", caller, call_off)
    require(words[0] == 0x3C05801D and words[1] == 0x24A50DC4 and
            jal_target(words[2], 0x80192798) == 0x80080D5C and
            words[3] == 0x27A60040,
            "command 2 location/result setup or blocking-wrapper call")
    require(u32(peimg, caller_off + len(caller)) == 0x3C04801D,
            "next boundary at 0x801927A0")

    # The response buffer is stack +0x40..+0x47.  Prove that the complete
    # caller suffix performs no load from it after the blocking call.
    suffix_off = caller_off + (0x801927A0 - CALLER_START)
    suffix = peimg[suffix_off:suffix_off + CALLER_COMPLETE_END - 0x801927A0]
    load_ops = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26}
    dead_result = True
    for off in range(0, len(suffix), 4):
        word = u32(suffix, off)
        op = word >> 26
        base = (word >> 21) & 31
        disp = word & 0xFFFF
        if op in load_ops and base == 29 and 0x40 <= disp <= 0x47:
            dead_result = False
    require(dead_result, "complete caller reads the Setloc response buffer")
    print("  OK retail: command 2 is CdlSetloc; complete blocking wrapper")
    print("  OK caller: exact CdlLOC; eight-byte response is dead; boundary 0x801927A0")

    libcd = (root / "pc_port/platform/pe_libcd.c").read_text()
    caller_source = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    require("func_80080D5C(2, 0x801D0DC4u, 0u)" in caller_source and
            "func_801924F8_801927A0_cut" in caller_source and
            "((uint32_t)command & 0xFFu) != 2u" in libcd and
            "result != 0u" in libcd and
            "g_cd_setloc_raw = PE_LoadU32(param)" in libcd,
            "native Setloc arm, dead-result boundary, or strict cut")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    disc = args.disc or pathlib.Path(os.environ.get(
        "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KAL"
    focused = run([str(tests)], 0, env)
    require("B54KAL_blocking_setloc_arm... PASS" in focused and
            "0 failed" in focused,
            "focused positive/negative Setloc controls")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    require("func_8007F0C8_completion_selector" in strict and
            "called from: func_8007F0C8" in strict,
            "strict production did not cross the CdlSetloc call")
    print("  OK native: exact location retained; unsupported paths inert")
    print("  OK production: strict frontier is 0x801927A0")
    print("\nB54K-AL blocking CdlSetloc oracle: PASS.")


if __name__ == "__main__":
    main()

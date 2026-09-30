#!/usr/bin/env python3
"""Independent B54K-AI oracle for the record-pool initializer."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import struct
import subprocess

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
ENTRY_FILE = 0x6AA14
ENTRY = 0x8007A214
ENTRY_END = 0x8007A240
ENTRY_SHA256 = "71f85f940c5697918f4902822149f70490cfa4f66d388530e7fee3c3945ce986"
WORKER_FILE = 0x6AA44
WORKER = 0x8007A244
WORKER_END = 0x8007A2A4
WORKER_SHA256 = "be9001c4af959d678340c4d233f6f8d169420493ddde920f7e86e483c284f78d"
CLEAR_FILE = 0x6CC44
CLEAR = 0x8007C444
CLEAR_END = 0x8007C478
CLEAR_SHA256 = "04b4534bd4721b1dfdc3bf42335c54cd9d4b3342d5387c2f52da9ffb26e847c4"
CALLER_MODULE = 0x03D2 * 0x800
CALLER_LOAD = 0x8018EFF0
CALLER_START = 0x801924F8
CALLER_END = 0x80192750
CALLER_SHA256 = "cddb140d8d927057b46a04a2e22afeafce3e24bc1d1faf13c6599302a035313c"
CALL_SITE = 0x80192748


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

    entry = exe[ENTRY_FILE:ENTRY_FILE + ENTRY_END - ENTRY]
    worker = exe[WORKER_FILE:WORKER_FILE + WORKER_END - WORKER]
    clear = exe[CLEAR_FILE:CLEAR_FILE + CLEAR_END - CLEAR]
    require(len(entry) == 11 * 4 and
            hashlib.sha256(entry).hexdigest() == ENTRY_SHA256,
            "11-word entry identity")
    require(len(worker) == 24 * 4 and
            hashlib.sha256(worker).hexdigest() == WORKER_SHA256,
            "24-word worker identity")
    require(len(clear) == 13 * 4 and
            hashlib.sha256(clear).hexdigest() == CLEAR_SHA256,
            "13-word clear-helper identity")
    entry_words = struct.unpack("<11I", entry)
    worker_words = struct.unpack("<24I", worker)
    clear_words = struct.unpack("<13I", clear)
    require(entry_words[2:7] == (
        0x3C01800C, 0xAC240DC8, 0x3C01800C,
        0x0C01E891, 0xAC2520C4) and
        jal_target(entry_words[5], ENTRY + 0x14) == WORKER and
        entry_words[-2:] == (0x03E00008, 0x00000000),
        "entry base/count stores, worker call, or return")
    require(jal_target(worker_words[12], WORKER + 0x30) == CLEAR and
            worker_words[13] == 0x00002021 and
            worker_words[-2:] == (0x03E00008, 0x00000000),
            "worker clear call or return")
    require(clear_words[0:2] == (0x10A0000A, 0x00003021) and
            clear_words[6] == 0x00021140 and
            clear_words[9:11] == (0x1440FFF8, 0xAC600000) and
            clear_words[-2:] == (0x03E00008, 0x00000000),
            "unsigned count loop, 32-byte stride, store, or return")
    print("  OK retail: 11-word entry, 24-word worker, 13-word clear helper")
    print("  OK semantics: base/count publication and word-zero/32-byte record walk")

    caller_off = CALLER_MODULE + CALLER_START - CALLER_LOAD
    caller = peimg[caller_off:caller_off + CALLER_END - CALLER_START]
    call_off = CALL_SITE - CALLER_START
    require(len(caller) == 150 * 4 and
            hashlib.sha256(caller).hexdigest() == CALLER_SHA256,
            "150-word func_801924F8 prefix")
    require(u32(caller, call_off - 8) == 0x3C04801D and
            u32(caller, call_off - 4) == 0x8C840DFC and
            jal_target(u32(caller, call_off), CALL_SITE) == ENTRY and
            u32(caller, call_off + 4) == 0x24050040 and
            u32(peimg, caller_off + call_off + 8) == 0x24040001,
            "production base, count, call, or following boundary")
    print("  OK caller: D_801D0DFC, count 0x40, exact next instruction 0x80192750")

    source = (root / "pc_port/game/boot/func_8007A214_port.c").read_text()
    caller_source = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    require("RECORD_SHIFT         5u" in source and
            "PE_StoreU32(record, 0u)" in source and
            "func_8007A214(PE_LoadU32(0x801D0DFCu), 0x40u)" in caller_source and
            "func_801924F8_80192790_cut" in caller_source and
            "ClearOTagR" not in source and
            "m0360i" not in source + caller_source and
            "0xA8066048" not in source + caller_source,
            "native record-pool scope")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    disc = args.disc or pathlib.Path(os.environ.get(
        "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KAI"
    focused = run([str(tests)], 0, env)
    require("B54KAI_record_pool_initializer... PASS" in focused and
            "0 failed" in focused,
            "record footprint, widths, and zero-count controls")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    require("func_8007F0C8_completion_selector" in strict and
            "called from: func_8007F0C8" in strict,
            "strict path did not reach the post-initializer cut")
    print("  OK runtime: bounded footprint and zero-count negative control")
    print("  OK production: strict path continues through stream-control setup")
    print("\nB54K-AI record-pool oracle: PASS.")


if __name__ == "__main__":
    main()

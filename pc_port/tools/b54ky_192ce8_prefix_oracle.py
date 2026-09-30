#!/usr/bin/env python3
"""Independent B54K-Y oracle for the saved-bit arm and 80192CE8 prefix."""

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
PEIMG_OFFSET = 0x03D2 * 0x800
LOAD = 0x8018EFF0
START = 0x80192CE8
CUT = 0x80192DFC
END = 0x80192F98
PREFIX_SHA256 = "0670dc9a913589495f623812b226d1ac665ca7984a6cbbe15efc7c200c038855"
FULL_SHA256 = "ed89408bffde43742781c89f1d16b7a98c52dc165b67cafd62cfdcf9319e03e7"
READ_SHA256 = "d0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5ba8d0b40"


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def jal_target(pc: int, word: int) -> int:
    require(word >> 26 == 3, f"{pc:#x} is not jal")
    return (pc & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def run(argv: list[str], expected: int,
        env: dict[str, str] | None = None) -> str:
    result = subprocess.run(argv, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True,
                            timeout=120, check=False, env=env)
    require(result.returncode == expected,
            f"exit {result.returncode}, expected {expected}\n{result.stdout[-3000:]}")
    return result.stdout


def resolve_disc(root: pathlib.Path, explicit: str | None) -> pathlib.Path:
    if explicit:
        result = pathlib.Path(explicit)
    elif os.environ.get("PE_DISC1_BIN"):
        result = pathlib.Path(os.environ["PE_DISC1_BIN"])
    else:
        result = pathlib.Path((root / "local" / "pe_disc1.path").read_text(
            encoding="utf-8").strip())
    require(result.is_file(), f"Disc 1 image absent: {result}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("peimg")
    parser.add_argument("--port")
    parser.add_argument("--tests")
    parser.add_argument("--disc")
    args = parser.parse_args()
    root = pathlib.Path(__file__).resolve().parents[2]
    peimg = pathlib.Path(args.peimg).read_bytes()
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1,
            "retail executable identity")
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1,
            "PE.IMG identity")

    off = PEIMG_OFFSET + START - LOAD
    prefix = peimg[off:off + CUT - START]
    full = peimg[off:off + END - START]
    require(len(prefix) == 69 * 4 and
            hashlib.sha256(prefix).hexdigest() == PREFIX_SHA256,
            "69-word func_80192CE8 prefix identity")
    require(len(full) == 172 * 4 and
            hashlib.sha256(full).hexdigest() == FULL_SHA256 and
            u32(full, 0x80192F90 - START) == 0x03E00008 and
            u32(full, 0x80192F94 - START) == 0,
            "complete function identity and return")
    rel = lambda pc: pc - START
    require(u32(prefix, rel(0x80192D18)) == 0x34420200 and
            u32(prefix, rel(0x80192D38)) == 0xA0230E04 and
            jal_target(0x80192D7C, u32(prefix, rel(0x80192D7C))) == 0x8006E6A8 and
            u32(prefix, rel(0x80192D80)) == 0x00463023 and
            u32(prefix, rel(0x80192D84)) == 0x1052FFF6 and
            jal_target(0x80192D90, u32(prefix, rel(0x80192D90))) == 0x8006E7E8 and
            jal_target(0x80192DF4, u32(prefix, rel(0x80192DF4))) == 0x80191FB8 and
            u32(prefix, rel(0x80192DF8)) == 0xAFA20010,
            "prefix state/read/retry/poll/next-call anchors")
    print("  OK overlay: 172-word function; exact 69-word prefix and first callee")

    caller = PEIMG_OFFSET + 0x80190D7C - LOAD
    require(u32(peimg, caller) == 0x128000E8 and
            u32(peimg, caller + 4) == 0 and
            jal_target(0x80190D84, u32(peimg, caller + 8)) == START and
            u32(peimg, caller + 12) == 0x24040001,
            "saved-bit branch and canonical positive arm")
    print("  OK caller: zero -> 0x80191120; nonzero -> func_80192CE8(1)")

    exe_base = 0x8000F800
    table = 0x8009315E - exe_base
    start, end, following = (u16(exe, table + i) for i in (0, 2, 4))
    require((start, end, following) == (0x039F, 0x03C5, 0x03C9),
            "retail read/stream table")
    require(u32(exe, 0x8001160C - exe_base) == 0x8010BCF8 and
            u32(exe, 0x80011610 - exe_base) == 0x80120D00,
            "retail read destination and arena pointers")
    payload = peimg[start * 0x800:end * 0x800]
    require(len(payload) == 38 * 0x800 and
            hashlib.sha256(payload).hexdigest() == READ_SHA256 and
            u32(payload, 0) == 7 and u32(payload, 4) == 0x4345444D,
            "authenticated first read payload")
    require(0x80120D00 + ((following - end) << 11) == 0x80122D00,
            "stack-word stream pointer arithmetic")
    print("  OK data: PE.IMG+0x39F, 38 sectors; stack word 0x80122D00")

    source = (root / "pc_port/game/boot/func_80192CE8_port.c").read_text(
        encoding="utf-8")
    caller_source = (root / "pc_port/game/boot/func_801909B4_port.c").read_text(
        encoding="utf-8")
    require("retry_issue:" in source and "func_8006E7E8()" in source and
            "PE_func_80191FB8_Values" in source and
            "func_801924F8((int16_t)index)" in source and
            "func_80192CE8(1)" in caller_source and
            "func_8018F2F4" in caller_source,
            "native branch/prefix wiring")
    require("m0360i" not in source + caller_source and
            "0xA8066048" not in source + caller_source,
            "forbidden scheduler/scene special case")

    tests = pathlib.Path(args.tests) if args.tests else \
        root / "pc_port/build/pe-native-tests"
    port = pathlib.Path(args.port) if args.port else \
        root / "pc_port/build/parasite-eve-port"
    disc = resolve_disc(root, args.disc)
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KY"
    focused = run([str(tests)], 0, env)
    require(re.search(r"Results: 985 run, 2 passed, 0 failed, 983 skipped",
                      focused) is not None,
            "two focused B54K-Y contracts")
    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict,
            "strict frontier after completed func_80191FB8")
    normal = run(common + ["--dma-checkpoint-report"], 0)
    require("[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0" in normal and
            "[DMA_CHECKPOINT] calls=27 queries=26 services=26 "
            "captured=26 serviced=26" in normal,
            "production telemetry")
    print("  OK runtime: real-disc read; negative arm; strict frontier")
    print("\nB54K-Y 80192CE8-prefix oracle: PASS.")


if __name__ == "__main__":
    main()

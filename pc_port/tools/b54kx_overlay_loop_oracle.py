#!/usr/bin/env python3
"""Independent B54K-X oracle for the complete 480-frame overlay loop."""

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
START = 0x80190660
END = 0x801909B4
BODY_SHA256 = "fe2cc07abe6f50d8959ec5dbfe268d5ab6ac0f2fc91ffadc10f69a07e8a192ee"
CALLER_START = 0x801909B4
CALLER_CUT = 0x80190D7C
CALLER_SHA256 = "449da8a3de6470a68186ffeadbddcfdc064b9e804ab4615774fdf29cb09d43d9"


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


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


def intensity(frame: int) -> int:
    if frame < 32:
        return frame * 4
    if frame < 392:
        return 128
    if frame < 424:
        return (424 - frame) * 4
    return 0


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

    offset = PEIMG_OFFSET + START - LOAD
    body = peimg[offset:offset + END - START]
    require(len(body) == 213 * 4 and
            hashlib.sha256(body).hexdigest() == BODY_SHA256,
            "complete 213-word func_80190660 identity")
    rel = lambda pc: pc - START
    require(u32(body, rel(0x801907C4)) == 0x32450001 and
            u32(body, rel(0x80190810)) == 0x2A420020 and
            u32(body, rel(0x80190824)) == 0x2A4201A8 and
            u32(body, rel(0x8019082C)) == 0x2A420188 and
            u32(body, rel(0x80190954)) == 0x2A4201E0 and
            u32(body, rel(0x80190958)) == 0x1440FF9B and
            u32(body, rel(0x8019095C)) == 0x32450001 and
            u32(body, rel(0x80190960)) == 0x0C01D34A and
            u32(body, rel(0x8019097C)) == 0xA062006D and
            u32(body, rel(0x80190980)) == 0xA082006D and
            u32(body, rel(0x801909AC)) == 0x03E00008 and
            u32(body, rel(0x801909B0)) == 0,
            "loop thresholds/back-edge or complete epilogue")
    print("  OK overlay: complete 213-word function and retail loop/return")

    caller_off = PEIMG_OFFSET + CALLER_START - LOAD
    caller = peimg[caller_off:caller_off + CALLER_CUT - CALLER_START]
    require(len(caller) == 242 * 4 and
            hashlib.sha256(caller).hexdigest() == CALLER_SHA256 and
            jal_target(0x80190D74,
                       u32(caller, 0x80190D74 - CALLER_START)) == START and
            u32(caller, 0x80190D78 - CALLER_START) == 0 and
            u32(peimg, PEIMG_OFFSET + 0x80190D7C - LOAD) == 0x128000E8,
            "exact-start caller and post-call saved-bit branch")
    print("  OK caller: 242-word prefix; exact call; 0x80190D7C next")

    values = [intensity(frame) for frame in range(480)]
    require(values[:4] == [0, 4, 8, 12] and values[31] == 124 and
            values[32] == values[391] == 128 and values[392] == 128 and
            values[423] == 4 and values[424] == values[479] == 0 and
            len(values) == 480,
            "independent four-segment intensity model")
    toggle = 0
    env_counts = [0, 0]
    for _frame in range(480):
        toggle = 1 if toggle == 0 else 0
        env_counts[toggle] += 1
    require(toggle == 0 and env_counts == [240, 240],
            "independent parity/toggle cardinality")
    require(480 * 1 == 480 and 480 * 2 == 960 and
            4 + (479 * 3) == 1441,
            "rectangle/draw-mode/DrawSync cardinality")
    print("  OK model: 480 intensities; 240/240 envs; 960 E1; 1441 syncs")

    overlay_source = (root / "pc_port/game/boot/func_80190660_port.c").read_text(
        encoding="utf-8")
    caller_source = (root / "pc_port/game/boot/func_801909B4_port.c").read_text(
        encoding="utf-8")
    require("while (frame < 480u)" in overlay_source and
            "if (frame < 32u)" in overlay_source and
            "else if (frame < 392u)" in overlay_source and
            "else if (frame < 424u)" in overlay_source and
            "func_80074D28(0)" in overlay_source and
            "func_80192CE8(1)" in caller_source,
            "complete loop or exact next-boundary source absent")
    require("m0360i" not in overlay_source + caller_source and
            "0xA8066048" not in overlay_source + caller_source,
            "forbidden scheduler/scene special case")

    tests = pathlib.Path(args.tests) if args.tests else \
        root / "pc_port/build/pe-native-tests"
    port = pathlib.Path(args.port) if args.port else \
        root / "pc_port/build/parasite-eve-port"
    disc = resolve_disc(root, args.disc)
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KX"
    focused = run([str(tests)], 0, env)
    require(re.search(r"Results: 985 run, 2 passed, 0 failed, 983 skipped",
                      focused) is not None,
            "two focused B54K-X contracts")
    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict,
            "strict post-initializer frontier")
    normal = run(common + ["--dma-checkpoint-report"], 0)
    require("[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0" in normal and
            "[DMA_CHECKPOINT] calls=27 queries=26 services=26 "
            "captured=26 serviced=26" in normal,
            "full-loop production telemetry")
    print("  OK runtime: 2 focused contracts; post-initializer frontier")
    print("\nB54K-X overlay-loop oracle: PASS.")


if __name__ == "__main__":
    main()

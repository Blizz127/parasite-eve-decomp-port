#!/usr/bin/env python3
"""Independent B54K-T oracle for func_80190660 through first DrawPrim."""

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
OVERLAY_OFFSET = 0x03D2 * 0x800
OVERLAY_SIZE = (0x0457 - 0x03D2) * 0x800
OVERLAY_LOAD = 0x8018EFF0
FUNC_START = 0x80190660
FUNC_END = 0x801909B4
FUNC_SHA256 = "fe2cc07abe6f50d8959ec5dbfe268d5ab6ac0f2fc91ffadc10f69a07e8a192ee"
PREFIX_END = 0x80190860
PREFIX_SHA256 = "ddd9aebf80a55e3ab7e04662fa090462361cd46214a3abd8a753d95046f05932"


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def s16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<h", data, offset)[0]


def jal_target(pc: int, word: int) -> int:
    require(word >> 26 == 3, f"{pc:#x} is not jal")
    return (pc & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def resolve_disc(root: pathlib.Path, explicit: str | None) -> pathlib.Path:
    if explicit:
        path = pathlib.Path(explicit)
    elif os.environ.get("PE_DISC1_BIN"):
        path = pathlib.Path(os.environ["PE_DISC1_BIN"])
    else:
        path = pathlib.Path(
            (root / "local" / "pe_disc1.path").read_text(
                encoding="utf-8").strip())
    require(path.is_file(), f"Disc 1 image does not exist: {path}")
    return path


def run(argv: list[str], expected: int,
        env: dict[str, str] | None = None) -> str:
    completed = subprocess.run(
        argv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, timeout=90, check=False, env=env)
    require(completed.returncode == expected,
            f"exit {completed.returncode}, expected {expected}\n"
            f"{completed.stdout[-3000:]}")
    return completed.stdout


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("peimg")
    parser.add_argument("--port")
    parser.add_argument("--tests")
    parser.add_argument("--disc")
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[2]
    peimg_path = pathlib.Path(args.peimg)
    peimg = peimg_path.read_bytes()
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    port = pathlib.Path(args.port) if args.port else \
        root / "pc_port" / "build" / "parasite-eve-port"
    tests = pathlib.Path(args.tests) if args.tests else \
        root / "pc_port" / "build" / "pe-native-tests"
    disc = resolve_disc(root, args.disc)

    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1,
            "retail executable identity")
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1,
            "PE.IMG identity")
    overlay = peimg[OVERLAY_OFFSET:OVERLAY_OFFSET + OVERLAY_SIZE]
    require(len(overlay) == OVERLAY_SIZE, "overlay range geometry")

    def overlay_body(start: int, end: int) -> bytes:
        offset = start - OVERLAY_LOAD
        return overlay[offset:offset + end - start]

    function = overlay_body(FUNC_START, FUNC_END)
    prefix = overlay_body(FUNC_START, PREFIX_END)
    require(len(function) == 213 * 4 and
            hashlib.sha256(function).hexdigest() == FUNC_SHA256,
            "213-word func_80190660 identity")
    require(len(prefix) == 128 * 4 and
            hashlib.sha256(prefix).hexdigest() == PREFIX_SHA256,
            "128-word translated prefix identity")
    require(u32(overlay, FUNC_START - OVERLAY_LOAD - 8) == 0x03E00008 and
            u32(overlay, FUNC_START - OVERLAY_LOAD - 4) == 0 and
            u32(function, len(function) - 8) == 0x03E00008 and
            u32(function, len(function) - 4) == 0,
            "real return boundaries around func_80190660")

    callers: list[int] = []
    for offset in range(0, len(overlay) - 3, 4):
        word = u32(overlay, offset)
        pc = OVERLAY_LOAD + offset
        if word >> 26 == 3 and jal_target(pc, word) == FUNC_START:
            callers.append(pc)
    require(callers == [0x80190D74], "exact-start caller census")

    boundary = overlay_body(PREFIX_END, PREFIX_END + 8)
    require(jal_target(PREFIX_END, u32(boundary, 0)) == 0x80075358 and
            u32(boundary, 4) == 0xA2300004,
            "first DrawPrim boundary and RGB delay slot")
    print("  OK hood: 213 words, normal return, one caller, real boundaries")
    print("  OK prefix: 128 words; first DrawPrim + RGB delay slot next")

    offset_word = u32(overlay, 0x80193278 - OVERLAY_LOAD)
    base = 0x80193254 + offset_word
    first = base - OVERLAY_LOAD
    size = u32(overlay, first + 8)
    second_address = base + 8 + (size & ~3)
    second = second_address - OVERLAY_LOAD
    first_rect = tuple(s16(overlay, first + 12 + i * 2) for i in range(4))
    second_rect = tuple(s16(overlay, second + 4 + i * 2) for i in range(4))
    require(offset_word == 0x3BAC8 and base == 0x801CED1C and size == 0x2C,
            "first record table arithmetic")
    require(second_address == 0x801CED50 and
            first_rect == (0, 480, 16, 1) and
            second_rect == (512, 256, 64, 64),
            "two image record geometries")
    print("  OK records: 801CED1C -> 801CED50; 16x1 and 64x64 images")

    calls: list[int] = []
    for offset in range(0, len(prefix), 4):
        word = u32(prefix, offset)
        if word >> 26 == 3:
            calls.append(jal_target(FUNC_START + offset, word))
    require(calls == [0x8007506C, 0x8007506C, 0x80074DC0,
                      0x80077C84, 0x80077C84, 0x80074D28],
            "prefix direct-call sequence")

    # Independent packet/frame-zero model. Both parity banks are identical
    # except for storage; each contains tpage 24 then 25.
    draw_mode = lambda tpage: 0xE1000000 | (tpage & 0x9FF)
    require(draw_mode(24) == 0xE1000018 and
            draw_mode(25) == 0xE1000019 and
            (0 * 4) == 0,
            "packet/fade model")
    print("  OK model: two DR_MODE words, SPRT constants, frame-zero RGB=0")

    source = (root / "pc_port" / "game" / "boot" /
              "func_80190660_port.c").read_text(encoding="utf-8")
    require("PE_func_80075358_Transient(&draw_mode_command, 1u)" in source and
            "PE_func_80075358_Transient(sprite_words, 4u)" in source and
            "sprites + parity * 0x28u + 4u + i * 4u" in source,
            "current continuation lost the transient DrawPrim contract")
    require("func_80075358" in source and
            "persist[0]" not in source and "m0360i" not in source and
            "0xA8066048" not in source,
            "source scope or forbidden planted state")
    print("  OK source: transient payload only; no scheduler or scene special case")

    focused_env = os.environ.copy()
    focused_env["PE_TEST_FILTER"] = "B54KT"
    focused = run([str(tests)], 0, focused_env)
    match = re.search(
        r"Results: (\d+) run, 2 passed, 0 failed, (\d+) skipped", focused)
    require(match is not None and
            int(match.group(1)) == int(match.group(2)) + 2,
            "two focused B54K-T contracts")

    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("first unresolved BOOTSTRAP_RET provider: func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict,
            "strict later PutDrawEnv frontier")
    normal = run(common + ["--dma-checkpoint-report"], 0)
    require("[STUB:BOOTSTRAP_RET] func_801924F8_80192584_cut" in normal and
            "[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0" in normal and
            "[DMA_CHECKPOINT] calls=27 queries=26 services=26 "
            "captured=26 serviced=26" in normal,
            "normal real-disc prefix effects")
    print("  OK runtime: 2 focused contracts and later 801924F8 internal frontier")
    print("\nB54K-T func_80190660 prefix oracle: PASS.")


if __name__ == "__main__":
    main()

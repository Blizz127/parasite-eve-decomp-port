#!/usr/bin/env python3
"""Independent B54K-R oracle for MoveImage and the 801909B4 continuation."""

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
EXE_BASE = 0x8000F800
OVERLAY_OFFSET = 0x03D2 * 0x800
OVERLAY_LOAD = 0x8018EFF0
OVERLAY_START = 0x801909B4
OVERLAY_END = 0x80190D74
OVERLAY_SHA256 = "9c127e501de9cffeb5f0c3448cc1ca6f866d6819f3b90b4730ce6de4ccc81565"
MOVE_START = 0x8007512C
MOVE_END = 0x800751E4
MOVE_SHA256 = "60e0cff194c462c05f75b0a4dddf0a6260a93f41bca053bc06cf442364749b2e"
WORKER_START = 0x80076B98
WORKER_END = 0x80076BE0
WORKER_SHA256 = "6c3b057b1349a56c2ccf0928523cd44822f7f7c4bf2ea47aa2e04adb09e099ff"


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def jal_target(pc: int, instruction: int) -> int:
    require(instruction >> 26 == 3, f"{pc:#x} is not jal")
    return (pc & 0xF0000000) | ((instruction & 0x03FFFFFF) << 2)


def resolve_disc(root: pathlib.Path, explicit: str | None) -> pathlib.Path:
    if explicit:
        path = pathlib.Path(explicit)
    elif os.environ.get("PE_DISC1_BIN"):
        path = pathlib.Path(os.environ["PE_DISC1_BIN"])
    else:
        pointer = root / "local" / "pe_disc1.path"
        require(pointer.is_file(), "missing local/pe_disc1.path")
        path = pathlib.Path(pointer.read_text(encoding="utf-8").strip())
    require(path.is_file(), f"Disc 1 image does not exist: {path}")
    return path


def run(argv: list[str], expected: int, env: dict[str, str] | None = None) -> str:
    completed = subprocess.run(
        argv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, timeout=60, check=False, env=env)
    require(completed.returncode == expected,
            f"exit {completed.returncode}, expected {expected}\n"
            f"{completed.stdout[-3000:]}")
    return completed.stdout


def decode_size(raw: int) -> tuple[int, int]:
    width = (((raw & 0xFFFF) - 1) & 0x3FF) + 1
    height = ((((raw >> 16) & 0xFFFF) - 1) & 0x1FF) + 1
    return width, height


def overlap_model() -> list[int]:
    row = [0] * 32
    row[10:16] = [1, 2, 3, 4, 5, 6]
    # Hardware-tested GP0(80h) direction for src_x < dst_x.
    for column in range(3, -1, -1):
        row[12 + column] = row[10 + column]
    return row


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("peimg", type=pathlib.Path)
    parser.add_argument("--port")
    parser.add_argument("--tests")
    parser.add_argument("--disc")
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[2]
    exe_path = root / "build" / "disc1.candidate.exe"
    port = pathlib.Path(args.port) if args.port else \
        root / "pc_port" / "build" / "parasite-eve-port"
    tests = pathlib.Path(args.tests) if args.tests else \
        root / "pc_port" / "build" / "pe-native-tests"
    disc = resolve_disc(root, args.disc)
    exe = exe_path.read_bytes()
    peimg = args.peimg.read_bytes()
    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1,
            "retail executable identity")
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1,
            "PE.IMG identity")

    def exe_body(start: int, end: int) -> bytes:
        return exe[start - EXE_BASE:end - EXE_BASE]

    require(hashlib.sha256(exe_body(MOVE_START, MOVE_END)).hexdigest() ==
            MOVE_SHA256, "46-word MoveImage body identity")
    require(hashlib.sha256(exe_body(WORKER_START, WORKER_END)).hexdigest() ==
            WORKER_SHA256, "18-word linked-list worker identity")
    require(u32(exe, 0x80095744 - EXE_BASE) == 0x80095704,
            "D_80095744 jump-table pointer")
    require(u32(exe, 0x8009570C - EXE_BASE) == 0x80076C34 and
            u32(exe, 0x8009571C - EXE_BASE) == 0x80076B98,
            "MoveImage target/worker jump-table entries")
    require(exe[0x800957E4 - EXE_BASE:0x800957F8 - EXE_BASE] ==
            bytes.fromhex("ffffff0400000080000000000000000000000000"),
            "persistent MoveImage packet template")
    print("  OK retail MoveImage: 46 words, direct +8 dispatcher, +0x18 worker")

    overlay_offset = OVERLAY_OFFSET + OVERLAY_START - OVERLAY_LOAD
    prefix = peimg[overlay_offset:overlay_offset + OVERLAY_END - OVERLAY_START]
    require(len(prefix) == 0x3C0 and len(prefix) // 4 == 240,
            "overlay prefix geometry")
    require(hashlib.sha256(prefix).hexdigest() == OVERLAY_SHA256,
            "240-word overlay prefix identity")
    boundary = peimg[overlay_offset + len(prefix):overlay_offset + len(prefix) + 8]
    require(jal_target(OVERLAY_END, u32(boundary, 0)) == 0x80190660 and
            u32(boundary, 4) == 0,
            "overlay boundary is not jal func_80190660 + nop")
    require(jal_target(0x80190C08,
                       u32(peimg, overlay_offset + 0x254)) == MOVE_START,
            "overlay no longer calls MoveImage at 0x80190C08")
    print("  OK retail overlay: 240 words through display setup; 80190660 next")

    require(decode_size(0x010000A0) == (160, 256) and
            decode_size(0x00010000) == (1024, 1) and
            decode_size(0) == (1024, 512), "GP0 copy size decoding")
    row = overlap_model()
    require(row[10:16] == [1, 2, 1, 2, 3, 4],
            "rightward overlap model")
    source = (0 << 16) | 320
    destination = (0 << 16) | 704
    size = (256 << 16) | 160
    require((source, destination, size) ==
            (0x00000140, 0x000002C0, 0x010000A0),
            "canonical packet model")
    print("  OK independent model: masks, zero-size maxima, overlap, packet")

    wrapper_source = (root / "pc_port" / "game" / "boot" /
                      "func_8007512C_port.c").read_text(encoding="utf-8")
    gpu_source = (root / "pc_port" / "platform" /
                  "pe_gpu.c").read_text(encoding="utf-8")
    require("jtb + 8u" in wrapper_source and "jtb + 0x18u" in wrapper_source,
            "native wrapper lost retail jump-table arithmetic")
    require("func_80076C10" not in wrapper_source.split("#include", 1)[1],
            "native executable code incorrectly routes through 80076C10")
    require("func_801909B4" not in gpu_source and
            "func_801909B4" not in wrapper_source,
            "MoveImage provider contains an overlay special case")
    require("func_80076B98_packet_cut" in wrapper_source and
            "IsGp0EnvironmentWord" in wrapper_source and
            "(tag & 0x00FFFFFFu) != 0x00FFFFFFu" in wrapper_source,
            "terminal one-node/environment packet fences absent")
    print("  OK source scope: generic provider; no overlay special case")

    focused_env = os.environ.copy()
    focused_env["PE_TEST_FILTER"] = "B54KR"
    focused = run([str(tests)], 0, focused_env)
    focused_result = re.search(
        r"Results: (\d+) run, 8 passed, 0 failed, (\d+) skipped", focused)
    require(focused_result is not None and
            int(focused_result.group(1)) == int(focused_result.group(2)) + 8,
            "focused B54K-R native contract count")
    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("first unresolved BOOTSTRAP_RET provider: func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict and
            "func_8007512C" not in strict,
            "strict real-disc path no longer traverses MoveImage")
    normal = run(common, 0)
    require("[STUB:BOOTSTRAP_RET] func_801924F8_80192584_cut" in normal and
            "[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0" in normal and
            "[HOST] stop_reason=unresolved-boundary" in normal and
            "func_8007512C" not in normal,
            "normal real-disc path no longer traverses MoveImage")
    print("  OK runtime: 8 focused contracts; later loop frontier observed")
    print("\nB54K-R MoveImage oracle: PASS.")


if __name__ == "__main__":
    main()

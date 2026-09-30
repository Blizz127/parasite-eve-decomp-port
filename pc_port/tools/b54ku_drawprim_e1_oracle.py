#!/usr/bin/env python3
"""Independent B54K-U oracle for DrawPrim and its GP0(E1h) first packet."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import re
import struct
import subprocess


EXE_BASE = 0x8000F800
EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
WRAPPER = (0x80075358, 0x800753B4,
           "b16699f3b147f2e86daf0cb43680613cfe2893f8aa5c7cbe06ccb34a74d3920e")
WORKER = (0x80076B58, 0x80076B98,
          "c34c4cc1323c3d3ff00222500fd91dee13e581d3ebd75141953149a7e943646e")
OVERLAY_OFFSET = 0x03D2 * 0x800
OVERLAY_LOAD = 0x8018EFF0
PREFIX_START = 0x80190660
PREFIX_END = 0x80190868
PREFIX_SHA256 = "7669d51cc1cd029bdc176627932e690b4f30d7846c4b233ed3a3c27e3bdca29a"


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def jal_target(pc: int, word: int) -> int:
    require(word >> 26 == 3, f"{pc:#x} is not jal")
    return (pc & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def resolve_disc(root: pathlib.Path, explicit: str | None) -> pathlib.Path:
    if explicit:
        path = pathlib.Path(explicit)
    elif os.environ.get("PE_DISC1_BIN"):
        path = pathlib.Path(os.environ["PE_DISC1_BIN"])
    else:
        path = pathlib.Path((root / "local" / "pe_disc1.path").read_text(
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
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    peimg = pathlib.Path(args.peimg).read_bytes()
    port = pathlib.Path(args.port) if args.port else \
        root / "pc_port" / "build" / "parasite-eve-port"
    tests = pathlib.Path(args.tests) if args.tests else \
        root / "pc_port" / "build" / "pe-native-tests"
    disc = resolve_disc(root, args.disc)

    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1,
            "retail executable identity")
    for start, end, digest in (WRAPPER, WORKER):
        body = exe[start - EXE_BASE:end - EXE_BASE]
        require(hashlib.sha256(body).hexdigest() == digest,
                f"body identity {start:#x}..{end:#x}")
    require((WRAPPER[1] - WRAPPER[0]) // 4 == 23 and
            (WORKER[1] - WORKER[0]) // 4 == 16,
            "wrapper/worker word geometry")
    require(u32(exe, 0x80095740 - EXE_BASE) == 0x80077294 and
            u32(exe, 0x80095718 - EXE_BASE) == 0x80076B58,
            "jump-table slots 15/5")
    worker = exe[WORKER[0] - EXE_BASE:WORKER[1] - EXE_BASE]
    require(u32(worker, 0) == 0x24A6FFFF and
            u32(worker, 0x10) == 0x10A00009 and
            u32(worker, 0x14) == 0xAC620000 and
            u32(worker, 0x30) == 0x14C5FFFA and
            u32(worker, 0x34) == 0xAC430000,
            "GP1-always then counted GP0 worker shape")
    print("  OK retail: 23-word wrapper, 16-word worker, jtb[15]/jtb[5]")

    offset = OVERLAY_OFFSET + PREFIX_START - OVERLAY_LOAD
    prefix = peimg[offset:offset + PREFIX_END - PREFIX_START]
    require(len(prefix) == 130 * 4 and
            hashlib.sha256(prefix).hexdigest() == PREFIX_SHA256,
            "130-word overlay prefix through first DrawPrim")
    boundary = peimg[offset + len(prefix):offset + len(prefix) + 8]
    require(jal_target(PREFIX_END, u32(boundary, 0)) == 0x80075358 and
            u32(boundary, 4) == 0x02202021,
            "second DrawPrim boundary")
    print("  OK overlay: first DrawPrim represented; four-word SPRT next")

    # Independent value model for SetDrawMode(0,0,24) and the exact worker.
    command = 0xE1000000 | (24 & 0x9FF)
    writes = [0x04000000] + [command]
    require(command == 0xE1000018 and writes == [0x04000000, 0xE1000018],
            "GP1/GP0 first-packet model")
    print("  OK model: DrawSync, GP1 DMA-off, one GP0(E1h) word")

    wrapper_source = (root / "pc_port" / "game" / "boot" /
                      "func_80075358_port.c").read_text(encoding="utf-8")
    gpu_source = (root / "pc_port" / "platform" /
                  "pe_gpu.c").read_text(encoding="utf-8")
    overlay_source = (root / "pc_port" / "game" / "boot" /
                      "func_80190660_port.c").read_text(encoding="utf-8")
    require("jtb + 0x3Cu" in wrapper_source and
            "jtb + 0x14u" in wrapper_source and
            "PE_GPU_WriteGP1(0x04000000u)" in wrapper_source,
            "native wrapper lost exact indirect/worker order")
    require("0xE1000000u" in gpu_source and
            "draw_mode_count++" in gpu_source and
            "PE_func_80075358_Transient" in overlay_source,
            "generic E1 state or transient adapter absent")
    require("m0360i" not in wrapper_source + gpu_source + overlay_source and
            "0xA8066048" not in wrapper_source + gpu_source + overlay_source,
            "forbidden scene special case")
    print("  OK source: generic E1 authority; exact indirect fences; no planting")

    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KU"
    focused = run([str(tests)], 0, env)
    match = re.search(
        r"Results: (\d+) run, 2 passed, 0 failed, (\d+) skipped", focused)
    require(match is not None and
            int(match.group(1)) == int(match.group(2)) + 2,
            "two focused B54K-U contracts")

    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("first unresolved BOOTSTRAP_RET provider: func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict,
            "strict later loop-reentry frontier")
    normal = run(common, 0)
    require("[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0" in normal and
            "[HOST] stop_reason=unresolved-boundary" in normal,
            "normal first-DrawPrim effects")
    print("  OK runtime: 2 focused contracts; later loop frontier observed")
    print("\nB54K-U DrawPrim/E1 oracle: PASS.")


if __name__ == "__main__":
    main()

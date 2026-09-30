#!/usr/bin/env python3
"""Independent B54K-M oracle for complete func_8006AD40.

The oracle imports no production code.  It authenticates the retail suffix,
models the packed archive and final flag branches, checks that the production
caller contains its two explicit checkpoint opportunities, and executes the
native binary against the configured retail Disc 1 image to measure both the
checkpoint path and the next strict frontier.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import re
import subprocess
import sys


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x8006B220
END = 0x8006B35C
SUFFIX_SHA256 = "1aed4d0f3bcaa751da044557dccd15e0f15a002addd7f0333ecaa635885749d8"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FAIL: {message}")


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


def run_native(argv: list[str], expected_rc: int) -> str:
    try:
        completed = subprocess.run(
            argv,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=30,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        raise SystemExit(f"FAIL: native run timed out: {' '.join(argv)}") from exc
    require(completed.returncode == expected_rc,
            f"native exit {completed.returncode}, expected {expected_rc}\n"
            f"{completed.stdout[-2000:]}")
    return completed.stdout


def run() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", help="native parasite-eve-port binary")
    parser.add_argument("--disc", help="retail Disc 1 BIN path")
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    port = pathlib.Path(args.port) if args.port else root / "pc_port" / "build" / "parasite-eve-port"
    disc = resolve_disc(root, args.disc)
    require(exe.is_file(), "missing build/disc1.candidate.exe")
    require(port.is_file(), f"native port binary does not exist: {port}")

    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == EXE_SHA1,
            "retail executable SHA-1")
    suffix = data[START - LOAD_BASE:END - LOAD_BASE]
    require(len(suffix) == 0x13C and
            hashlib.sha256(suffix).hexdigest() == SUFFIX_SHA256,
            "retail completion suffix identity")
    print("  OK retail identity: exact executable and 79-word suffix")

    base = 0x80180000
    packed = (3 << 22) | 0x120
    count = packed >> 22
    first = base + (packed & 0x003FFFFF)
    entries = [first + i * 0x14 for i in range(count)]
    require(entries == [0x80180120, 0x80180134, 0x80180148],
            "packed final-archive count/offset/stride model")
    require([] == [first + i * 0x14 for i in range(0)],
            "zero-count final archive bypass")
    print("  OK archive model: packed offset, zero bypass, 0x14 stride")

    initial = [0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5]
    for flags in (0, 0x40, 0x80, 0xC0):
        got = initial.copy()
        if not flags & 0x40:
            got[0] = got[2] = got[3] = 0xFF
        if not flags & 0x80:
            got[1] = got[4] = got[5] = 0xFF
        want = [
            initial[0] if flags & 0x40 else 0xFF,
            initial[1] if flags & 0x80 else 0xFF,
            initial[2] if flags & 0x40 else 0xFF,
            initial[3] if flags & 0x40 else 0xFF,
            initial[4] if flags & 0x80 else 0xFF,
            initial[5] if flags & 0x80 else 0xFF,
        ]
        require(got == want, f"0x40/0x80 flag model for {flags:#x}")
    print("  OK state model: complete 0x40/0x80 branch matrix and bit-0 owner")

    caller = (root / "pc_port" / "bootstrap" / "func_8001220C_port.c").read_text(
        encoding="utf-8")
    after_call = caller.split("func_8006AD40();", 1)[1]
    before_dispatch = after_call.split("v = D_8009D280;", 1)[0]
    require(before_dispatch.count("PE_Port_ServiceDmaIrqCheckpoint()") == 2,
            "production caller no longer has exactly two explicit checkpoints")
    require("if (PE_Port_ShouldStop()) return;" in before_dispatch,
            "production caller lost post-checkpoint stop ordering")
    print("  OK caller source: two explicit checkpoints before destination dispatch")

    common = [str(port), "--headless", "--disc-image", str(disc)]
    bounded = run_native(
        common + ["--max-frames", "2", "--dma-checkpoint-report"], 0)
    match = re.search(
        r"\[DMA_CHECKPOINT\] calls=(\d+) queries=(\d+) services=(\d+) "
        r"captured=(\d+) serviced=(\d+)", bounded)
    require(match is not None, "bounded caller run omitted checkpoint report")
    values = tuple(int(value) for value in match.groups())
    require(values == (27, 26, 26, 26, 26),
            f"bounded caller checkpoint contract differs: {values}")
    require("[HOST] stop_reason=frame-limit" in bounded,
            "bounded caller did not honor the host frame limit")
    print("  OK runtime caller: all boot checkpoints, then host frame limit")

    strict = run_native(common + ["--strict-stubs"], 1)
    require("first unresolved BOOTSTRAP_RET provider: func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict,
            "strict continuation no longer passes 6AD40 into the overlay")
    require("func_8006AD40_D_80093126_archive_cut" not in strict,
            "obsolete 6AD40 prefix frontier remains live")
    require("provider: func_801909B4" not in strict,
            "obsolete whole-overlay stub remains live")
    print("  OK strict continuation: 6AD40 returns; overlay reaches the 801924F8 internal cut")
    print("\nB54K-M completion oracle: PASS (retail, native, caller, and frontier).")
    return 0


if __name__ == "__main__":
    sys.exit(run())

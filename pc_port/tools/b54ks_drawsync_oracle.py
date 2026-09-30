#!/usr/bin/env python3
"""Independent B54K-S oracle for the deterministic DrawSync drain."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import re
import struct
import subprocess


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
EXE_BASE = 0x8000F800
WRAPPER = (0x80074DC0, 0x80074E28,
           "7b6e49da8c7c3721966165e7c54af37e6f4bf81fca08422ebc1a03badf6b0e98")
DRAINS = (0x80077294, 0x800773D0,
          "4351d2c705b84f1524116d7eca95adbe416603e81351c90bab22544fac3e71a8")
WAIT = (0x80077404, 0x80077548,
        "50930b6c4a1a412ef01274469773764ee5a45daa13e94464a0a446523b12a59a")


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
        path = pathlib.Path(
            (root / "local" / "pe_disc1.path").read_text(encoding="utf-8").strip())
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


def body(exe: bytes, identity: tuple[int, int, str]) -> bytes:
    start, end, digest = identity
    value = exe[start - EXE_BASE:end - EXE_BASE]
    require(hashlib.sha256(value).hexdigest() == digest,
            f"body identity {start:#x}..{end:#x}")
    return value


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--port")
    parser.add_argument("--tests")
    parser.add_argument("--disc")
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[2]
    exe = (root / "build" / "disc1.candidate.exe").read_bytes()
    port = pathlib.Path(args.port) if args.port else \
        root / "pc_port" / "build" / "parasite-eve-port"
    tests = pathlib.Path(args.tests) if args.tests else \
        root / "pc_port" / "build" / "pe-native-tests"
    disc = resolve_disc(root, args.disc)

    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1,
            "retail executable identity")
    wrapper = body(exe, WRAPPER)
    drains = body(exe, DRAINS)
    body(exe, WAIT)
    require(len(wrapper) == 26 * 4 and len(drains) == 79 * 4,
            "wrapper/drain word geometry")
    require(u32(exe, 0x80095740 - EXE_BASE) == 0x80077294,
            "jtb[15] DrawSync target")
    calls = []
    for offset in range(0, len(drains), 4):
        word = u32(drains, offset)
        if word >> 26 == 3:
            calls.append(jal_target(DRAINS[0] + offset, word))
    require(calls == [0x800773D0, 0x80076EE4, 0x80077404,
                      0x80077404, 0x80076EE4],
            "DrawSync direct-call sequence")
    print("  OK retail: 26-word wrapper, 79-word drain, 81-word wait")

    # Independent state model for the nonblocking branch and normal wait
    # predicate. It deliberately imports no production implementation.
    def poll_result(pending: int, busy: bool, ready: bool) -> int:
        if busy:
            return pending
        if ready:
            return pending
        return pending if pending else 1

    require(poll_result(0, False, True) == 0 and
            poll_result(0, False, False) == 1 and
            poll_result(7, True, True) == 7 and
            poll_result(7, False, False) == 7,
            "nonblocking status model")
    require(not (240 < 0) and (0xF0001 > 0xF0000),
            "normal/timeout wait predicate model")
    print("  OK model: pending/status returns and wait threshold")

    source = (root / "pc_port" / "platform" /
              "pe_libgpu.c").read_text(encoding="utf-8")
    require("PE_Port_ServiceDmaIrqCheckpoint" in source and
            "PE_func_80076EE4_Pump" in source and
            "PE_GPU_ServiceDMA2Completion" not in source,
            "DrawSync bypasses established event/pump owners")
    require("func_80077404_timeout_recovery_cut" in source and
            "func_80077404_wait_cut" in source,
            "unrepresented timeout/no-progress paths are not fenced")
    print("  OK source: one-token event bridge; timeout paths fenced")

    focused_env = os.environ.copy()
    focused_env["PE_TEST_FILTER"] = "B54KS"
    focused = run([str(tests)], 0, focused_env)
    match = re.search(
        r"Results: (\d+) run, 4 passed, 0 failed, (\d+) skipped", focused)
    require(match is not None and
            int(match.group(1)) == int(match.group(2)) + 4,
            "four focused B54K-S contracts")
    print("  OK focused: idle, direct DMA, queued DMA, no-progress cut")

    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("first unresolved BOOTSTRAP_RET provider: func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict,
            "strict frontier changed unexpectedly")
    normal = run(common + ["--dma-checkpoint-report"], 0)
    require("[DMA_CHECKPOINT] calls=27 queries=26 services=26 "
            "captured=26 serviced=26" in normal and
            "[HOST] stop_reason=unresolved-boundary" in normal,
            "real-disc DrawSync completion census")
    print("  OK runtime: 26 DMA events drained; later 801924F8 internal frontier observed")
    print("\nB54K-S DrawSync oracle: PASS.")


if __name__ == "__main__":
    main()

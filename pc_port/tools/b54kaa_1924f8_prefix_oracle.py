#!/usr/bin/env python3
"""Independent B54K-AA oracle for the func_801924F8 prefix."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import re
import struct
import subprocess

PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
BASE = 0x03D2 * 0x800
LOAD = 0x8018EFF0
START, CUT, END = 0x801924F8, 0x80192584, 0x80192934
FULL_SHA = "ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a"
PREFIX_SHA = "9f6476d633f517cd6e17fee8a76167180a9f87d320ecf0e62ef4e4f3b45114b1"


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def u32(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def run(argv: list[str], expected: int, env: dict[str, str] | None = None) -> str:
    result = subprocess.run(argv, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, timeout=120,
                            check=False, env=env)
    require(result.returncode == expected,
            f"exit {result.returncode}, expected {expected}\n{result.stdout[-2000:]}")
    return result.stdout


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
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1, "PE.IMG identity")
    off = BASE + START - LOAD
    body = peimg[off:off + END - START]
    prefix = body[:CUT - START]
    require(len(body) == 271 * 4 and hashlib.sha256(body).hexdigest() == FULL_SHA,
            "complete function identity")
    require(hashlib.sha256(prefix).hexdigest() == PREFIX_SHA and
            u32(body, CUT - START) == 0x2E020015 and
            u32(body, 0x8019292C - START) == 0x03E00008 and
            u32(body, 0x80192930 - START) == 0,
            "prefix cut or retail return")
    jal_word = 0x0C000000 | ((START >> 2) & 0x03FFFFFF)
    overlay = peimg[BASE:BASE + (0x0457 - 0x03D2) * 0x800]
    callers = [LOAD + i for i in range(0, len(overlay) - 3, 4)
               if u32(overlay, i) == jal_word]
    require(callers == [0x80192E00], "exact-start caller census")
    print("  OK hood: 271 words, normal return, exact caller, real boundaries")
    print("  OK prefix: 35 words; indexed record and both display calls")

    source = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    require("record_index >= 47u" in source and "0x801D0E00u" in source and
            source.count("func_801918F8(") == 2 and
            "func_801924F8_80192584_cut" in source and
            "m0360i" not in source and "0xA8066048" not in source,
            "native prefix scope")
    tests = pathlib.Path(args.tests) if args.tests else root / "pc_port/build/pe-native-tests"
    port = pathlib.Path(args.port) if args.port else root / "pc_port/build/parasite-eve-port"
    disc = pathlib.Path(args.disc) if args.disc else pathlib.Path(
        os.environ.get("PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KY"
    focused = run([str(tests)], 0, env)
    require(re.search(r"Results: 985 run, 2 passed, 0 failed, 983 skipped", focused),
            "focused positive/negative contracts")
    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("func_801924F8_80192584_cut" in strict and "called from: func_801924F8" in strict,
            "strict frontier")
    normal = run(common + ["--dma-checkpoint-report"], 0)
    require("[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0" in normal and
            "[DMA_CHECKPOINT] calls=27 queries=26 services=26 captured=26 serviced=26" in normal,
            "production telemetry")
    print("  OK runtime: positive/negative contracts; func_801924F8 internal frontier")
    print("\nB54K-AA func_801924F8-prefix oracle: PASS.")


if __name__ == "__main__":
    main()

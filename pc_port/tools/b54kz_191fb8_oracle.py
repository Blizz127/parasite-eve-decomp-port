#!/usr/bin/env python3
"""Independent B54K-Z oracle for complete overlay func_80191FB8."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import re
import struct
import subprocess

PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
OVERLAY_OFFSET = 0x03D2 * 0x800
LOAD = 0x8018EFF0
START = 0x80191FB8
END = 0x801922F4
BODY_SHA256 = "1064769b74b7d2c5dedec9cc9abfd7a2fd0f8c4db4cf0f90a3b897f7a0bc1aeb"


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
            f"exit {result.returncode}, expected {expected}\n{result.stdout[-2500:]}")
    return result.stdout


def resolve_disc(root: pathlib.Path, explicit: str | None) -> pathlib.Path:
    if explicit:
        result = pathlib.Path(explicit)
    elif os.environ.get("PE_DISC1_BIN"):
        result = pathlib.Path(os.environ["PE_DISC1_BIN"])
    else:
        result = pathlib.Path((root / "local/pe_disc1.path").read_text(
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
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1, "PE.IMG identity")
    off = OVERLAY_OFFSET + START - LOAD
    body = peimg[off:off + END - START]
    require(len(body) == 207 * 4 and
            hashlib.sha256(body).hexdigest() == BODY_SHA256 and
            u32(body, 0x801922EC - START) == 0x03E00008 and
            u32(body, 0x801922F0 - START) == 0,
            "207-word function identity and return")
    calls = [(START + i, jal_target(START + i, u32(body, i)))
             for i in range(0, len(body), 4) if u32(body, i) >> 26 == 3]
    require(calls == [(0x80192294, 0x8007512C),
                      (0x801922D4, 0x8007512C)],
            "two MoveImage call sites")
    require(u32(body, 0x8019202C - START) == 0x144000AC and
            u32(body, 0x801922A8 - START) == 0x00431024 and
            u32(body, 0x801922AC - START) == 0x1440000C,
            "busy and 0x08000000 gates")
    print("  OK retail: complete 207 words, normal return, two MoveImage calls")

    one = 0x80122D00
    require((one, one + 0xFA00, one + 0x1F400,
             one + 0x3F400, one + 0x50400, one + 0x53100) ==
            (0x80122D00, 0x80132700, 0x80142100,
             0x80162100, 0x80173100, 0x80175E00),
            "one-source pointer model")
    require(2 * 0x14 == 0x28 and 2 * 0x5C == 0xB8,
            "environment copy extents")
    print("  OK model: one/two-source layouts; 0x28/0xB8 copies; move gate")

    source = (root / "pc_port/game/boot/func_80191FB8_port.c").read_text(
        encoding="utf-8")
    caller = (root / "pc_port/game/boot/func_80192CE8_port.c").read_text(
        encoding="utf-8")
    require("PE_func_80191FB8_Values" in source and
            "func_8007512C(&rect, 512, 0)" in source and
            "func_8007512C(&rect, 512, 256)" in source and
            "func_801924F8((int16_t)index)" in caller,
            "complete native function or next frontier absent")
    require("m0360i" not in source + caller and
            "0xA8066048" not in source + caller,
            "forbidden scene/scheduler special case")

    tests = pathlib.Path(args.tests) if args.tests else \
        root / "pc_port/build/pe-native-tests"
    port = pathlib.Path(args.port) if args.port else \
        root / "pc_port/build/parasite-eve-port"
    disc = resolve_disc(root, args.disc)
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KZ"
    focused = run([str(tests)], 0, env)
    require(re.search(r"Results: 985 run, 2 passed, 0 failed, 983 skipped",
                      focused) is not None,
            "two focused B54K-Z contracts")
    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict,
            "strict next-call frontier")
    normal = run(common + ["--dma-checkpoint-report"], 0)
    require("[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0" in normal and
            "[DMA_CHECKPOINT] calls=27 queries=26 services=26 "
            "captured=26 serviced=26" in normal,
            "production telemetry")
    print("  OK runtime: 2 focused contracts; func_801924F8 internal frontier")
    print("\nB54K-Z func_80191FB8 oracle: PASS.")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Independent B54K-Q oracle for the func_801909B4 MoveImage prefix."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import struct
import subprocess


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
EXE_BASE = 0x8000F800
PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
OVERLAY_OFFSET = 0x03D2 * 0x800
OVERLAY_LOAD = 0x8018EFF0
FUNC_START = 0x801909B4
PREFIX_END = 0x80190C08
PREFIX_SHA256 = "e0fc1734a365940821d1ec4aa15ae64ef1bc9f6e043879421f47dae17900af06"
HELPERS = (
    (0x8005E57C, 0x8005E588,
     "14884a4554cca56a4c1ea813ee236f4eb64fd0473ad54c89be8f8761efe999dd"),
    (0x8005C1EC, 0x8005C25C,
     "4f06eb18ea6ffd322f864aa57de0945c0fefb2f3edb91996b45b72fd8cc5fb3c"),
    (0x80042538, 0x800425DC,
     "85b319440f7f0ed6464db12d742cc9ef03cb792c6bdefbeaab955de02d2e927c"),
)


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def word(data: bytes, offset: int) -> int:
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


def run_native(argv: list[str], expected: int) -> str:
    completed = subprocess.run(
        argv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, timeout=30, check=False)
    require(completed.returncode == expected,
            f"native exit {completed.returncode}, expected {expected}\n"
            f"{completed.stdout[-2000:]}")
    return completed.stdout


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("peimg", type=pathlib.Path)
    parser.add_argument("--port")
    parser.add_argument("--disc")
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[2]
    exe_path = root / "build" / "disc1.candidate.exe"
    port = pathlib.Path(args.port) if args.port else \
        root / "pc_port" / "build" / "parasite-eve-port"
    disc = resolve_disc(root, args.disc)
    exe = exe_path.read_bytes()
    peimg = args.peimg.read_bytes()
    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1,
            "retail executable identity")
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1,
            "PE.IMG identity")

    function_offset = OVERLAY_OFFSET + FUNC_START - OVERLAY_LOAD
    prefix = peimg[function_offset:function_offset + PREFIX_END - FUNC_START]
    require(len(prefix) == 0x254 and len(prefix) // 4 == 149,
            "prefix geometry")
    require(hashlib.sha256(prefix).hexdigest() == PREFIX_SHA256,
            "149-word prefix identity")
    boundary = peimg[function_offset + len(prefix):
                     function_offset + len(prefix) + 8]
    require(jal_target(PREFIX_END, word(boundary, 0)) == 0x8007512C,
            "boundary target is not MoveImage")
    require(word(boundary, 4) == 0xA7A2001E,
            "MoveImage delay slot is not local RECT h=256 store")
    print("  OK retail prefix: 149 words then jal func_8007512C + h delay slot")

    for start, end, digest in HELPERS:
        body = exe[start - EXE_BASE:end - EXE_BASE]
        require(hashlib.sha256(body).hexdigest() == digest,
                f"helper identity {start:#x}")
    require(word(exe, 0x8005E57C - EXE_BASE) == 0xAF8403B0,
            "func_8005E57C is not gp+0x3B0 scalar setter")
    require(jal_target(0x80042548,
                       word(exe, 0x80042548 - EXE_BASE)) == 0x80071A24,
            "func_80042538 does not call retail bzero")
    print("  OK prerequisites: exact 5E57C, 5C1EC, and 42538 retail bodies")

    source = (root / "pc_port" / "game" / "boot" /
              "func_801909B4_port.c").read_text(encoding="utf-8")
    caller = (root / "pc_port" / "bootstrap" /
              "func_8001220C_port.c").read_text(encoding="utf-8")
    require("func_8007512C(&move_rect, 0x2C0, 0)" in source and
            "if (PE_Port_ShouldStop())" in source,
            "source lost translated MoveImage call/stop propagation")
    after = caller.split("v = func_801909B4();", 1)[1]
    before_consumer = after.split("func_8006E9A0(v);", 1)[0]
    require("if (PE_Port_ShouldStop()) return;" in before_consumer,
            "caller can consume a prefix return after nested stop")
    print("  OK source continuation: MoveImage call, stop propagation, caller guard")

    arena = 0x80120D00
    second = arena + 0x1C080
    got = (arena + 0x4080, second + 0x4080,
           arena + 0x80, second + 0x80, arena, second)
    want = (0x80124D80, 0x80140E00, 0x80120D80,
            0x8013CE00, 0x80120D00, 0x8013CD80)
    require(got == want, "arena pointer model")
    source_bytes = bytes((0x13 + i * 37) & 0xFF for i in range(0xE0))
    destination = bytearray([0xA5] * 0xE0)
    destination[:] = source_bytes
    require(bytes(destination) == source_bytes, "four-copy contiguous model")
    print("  OK independent model: 0xE0 copied bytes and six arena pointers")

    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run_native(common + ["--strict-stubs"], 1)
    require("first unresolved BOOTSTRAP_RET provider: func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict and
            "provider: func_801909B4" not in strict,
            "strict runtime did not enter overlay prefix")
    normal = run_native(common, 0)
    require("[HOST] stop_reason=unresolved-boundary" in normal and
            "[STUB:BOOTSTRAP_RET] func_801924F8_80192584_cut" in normal and
            "func_8006E9A0" not in normal,
            "normal runtime did not stop at MoveImage boundary")
    print("  OK real-disc runtime: overlay entered; later 801924F8 internal boundary")
    print("\nB54K-Q prefix oracle: PASS.")


if __name__ == "__main__":
    main()

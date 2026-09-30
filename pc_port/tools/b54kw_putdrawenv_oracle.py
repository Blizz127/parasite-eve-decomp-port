#!/usr/bin/env python3
"""Independent B54K-W oracle for PutDrawEnv and the first loop back-edge."""

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
PUT_START = 0x80075424
PUT_END = 0x800754E4
PUT_SHA256 = "31dd565c7af1c78d85eb4e50a6abaf8dde75857b6edfd4926f208bc858f7a852"
OVERLAY_OFFSET = 0x03D2 * 0x800
OVERLAY_LOAD = 0x8018EFF0
PREFIX_START = 0x80190660
PREFIX_END = 0x80190960
PREFIX_SHA256 = "6b548466e8e58277757897a9ab91c91b6e791a43a02c6e42c2a2c3adb92c6bc2"
EXE_LOAD_DELTA = 0x8000F800


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
        result = pathlib.Path(explicit)
    elif os.environ.get("PE_DISC1_BIN"):
        result = pathlib.Path(os.environ["PE_DISC1_BIN"])
    else:
        result = pathlib.Path((root / "local" / "pe_disc1.path").read_text(
            encoding="utf-8").strip())
    require(result.is_file(), f"Disc 1 image absent: {result}")
    return result


def run(argv: list[str], expected: int,
        env: dict[str, str] | None = None) -> str:
    result = subprocess.run(argv, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True,
                            timeout=90, check=False, env=env)
    require(result.returncode == expected,
            f"exit {result.returncode}, expected {expected}\n{result.stdout[-3000:]}")
    return result.stdout


def clip(command: int, x: int, y: int) -> int:
    return command | (max(0, min(1023, x)) & 0x3FF) | \
        ((max(0, min(511, y)) & 0x1FF) << 10)


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
    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1,
            "retail executable identity")
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1,
            "PE.IMG identity")

    put_off = PUT_START - EXE_LOAD_DELTA
    body = exe[put_off:put_off + PUT_END - PUT_START]
    require(len(body) == 48 * 4 and hashlib.sha256(body).hexdigest() == PUT_SHA256,
            "48-word PutDrawEnv identity")
    require(u32(exe, put_off - 4) == 0x27BD0018 and
            u32(exe, put_off + len(body)) == 0x27BDFFD8 and
            u32(body, len(body) - 8) == 0x03E00008 and
            u32(body, len(body) - 4) == 0x27BD0020,
            "PutDrawEnv return and real neighboring instructions")
    callers = []
    for off in range(0, len(exe) - 3, 4):
        word = u32(exe, off)
        pc = EXE_LOAD_DELTA + off
        if word >> 26 == 3 and jal_target(pc, word) == PUT_START:
            callers.append(pc)
    require(callers == [0x8005E7D8, 0x80069FB8, 0x8006A1AC, 0x80070F40],
            f"executable PutDrawEnv caller census: {callers!r}")
    print("  OK executable: 48 words, return/boundaries, four direct callers")

    overlay_off = OVERLAY_OFFSET + PREFIX_START - OVERLAY_LOAD
    prefix = peimg[overlay_off:overlay_off + PREFIX_END - PREFIX_START]
    require(len(prefix) == 192 * 4 and
            hashlib.sha256(prefix).hexdigest() == PREFIX_SHA256,
            "192-word overlay prefix identity")
    rel = lambda pc: pc - PREFIX_START
    require(jal_target(0x8019093C, u32(prefix, rel(0x8019093C))) == PUT_START and
            u32(prefix, rel(0x80190940)) == 0x26520001 and
            jal_target(0x8019094C, u32(prefix, rel(0x8019094C))) == 0x800755F0 and
            u32(prefix, rel(0x80190950)) == 0x2484005C and
            u32(prefix, rel(0x80190954)) == 0x2A4201E0 and
            u32(prefix, rel(0x80190958)) == 0x1440FF9B and
            u32(prefix, rel(0x8019095C)) == 0x32450001,
            "PutDrawEnv/PutDispEnv/taken-loop sequence")
    print("  OK overlay: 192 words through PutDispEnv and loop delay slot")

    env = dict(x=0, y=240, w=320, h=240, ofs_x=0, ofs_y=240,
               tpage=0xA, dtd=1, dfe=1)
    packet = [
        0x06FFFFFF,
        clip(0xE3000000, env["x"], env["y"]),
        clip(0xE4000000, env["x"] + env["w"] - 1,
             env["y"] + env["h"] - 1),
        0xE5000000 | ((env["ofs_y"] & 0x7FF) << 11) |
        (env["ofs_x"] & 0x7FF),
        0xE1000000 | env["tpage"] | (env["dtd"] << 9) |
        (env["dfe"] << 10),
        0xE2000000,
        0xE6000000,
    ]
    require(packet == [0x06FFFFFF, 0xE303C000, 0xE4077D3F,
                       0xE5078000, 0xE100060A, 0xE2000000, 0xE6000000],
            "independent canonical DR_ENV construction")
    print("  OK model: terminal tag + E3/E4/E5/E1/E2/E6 packet")

    sources = "".join((root / path).read_text(encoding="utf-8") for path in [
        "pc_port/platform/pe_gpu.c",
        "pc_port/platform/pe_libgpu.c",
        "pc_port/game/boot/func_8007512C_port.c",
        "pc_port/game/boot/func_80190660_port.c",
    ])
    require("func_80075424(pe_addr_t env)" in sources and
            "IsGp0EnvironmentWord" in sources and
            "ApplyTextureWindow" in sources and
            "while (frame < 480u)" in sources,
            "generic provider or exact continuation source absent")
    require("m0360i" not in sources and "0xA8066048" not in sources,
            "forbidden scheduler/scene special case")

    tests = pathlib.Path(args.tests) if args.tests else \
        root / "pc_port" / "build" / "pe-native-tests"
    port = pathlib.Path(args.port) if args.port else \
        root / "pc_port" / "build" / "parasite-eve-port"
    disc = resolve_disc(root, args.disc)
    test_env = os.environ.copy()
    test_env["PE_TEST_FILTER"] = "B54KW"
    focused = run([str(tests)], 0, test_env)
    require(re.search(r"Results: 985 run, 2 passed, 0 failed, 983 skipped",
                      focused) is not None,
            "two focused B54K-W contracts")
    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict,
            "strict post-initializer frontier")
    normal = run(common + ["--dma-checkpoint-report"], 0)
    require("[STUB:BOOTSTRAP_RET] func_801924F8_80192584_cut" in normal and
            "[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0" in normal and
            "[DMA_CHECKPOINT] calls=27 queries=26 services=26 "
            "captured=26 serviced=26" in normal,
            "normal production continuation effects")
    print("  OK runtime: 2 focused contracts; later post-initializer frontier")
    print("\nB54K-W PutDrawEnv oracle: PASS.")


if __name__ == "__main__":
    main()

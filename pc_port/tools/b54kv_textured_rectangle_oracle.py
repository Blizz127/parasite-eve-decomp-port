#!/usr/bin/env python3
"""Independent B54K-V oracle for GP0(64h) and the PutDrawEnv frontier."""

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
OVERLAY_LOAD = 0x8018EFF0
PREFIX_START = 0x80190660
PREFIX_END = 0x8019093C
PREFIX_SHA256 = "820ca0d865218c5954ff6b15ba2a14af0d35aa43962e8331d60faa0f6b43cde0"
PALETTE_SHA256 = "bafc82602eef6096effccc1e4f658ba859b78135459de426fd0aa0ace634c3df"
INDEX_SHA256 = "caa27d386763de7d6d33212e380bfe6505bbd1d4ae80dda90fcfca69c0d4196a"


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


def modulate(texture: int, command: int) -> int:
    channels = []
    for shift in (0, 5, 10):
        texel = (texture >> shift) & 31
        vertex = (command >> (shift // 5 * 8)) & 0xFF
        channels.append(min(31, (texel * vertex) >> 7))
    return (channels[0] | (channels[1] << 5) | (channels[2] << 10) |
            (texture & 0x8000))


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
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1,
            "PE.IMG identity")
    offset = OVERLAY_OFFSET + PREFIX_START - OVERLAY_LOAD
    prefix = peimg[offset:offset + PREFIX_END - PREFIX_START]
    require(len(prefix) == 183 * 4 and
            hashlib.sha256(prefix).hexdigest() == PREFIX_SHA256,
            "183-word overlay prefix identity")
    boundary = peimg[offset + len(prefix):offset + len(prefix) + 8]
    require(jal_target(PREFIX_END, u32(boundary, 0)) == 0x80075424 and
            u32(boundary, 4) == 0x26520001,
            "PutDrawEnv boundary and frame-increment delay slot")

    calls: list[tuple[int, int]] = []
    for rel in range(0, len(prefix), 4):
        word = u32(prefix, rel)
        pc = PREFIX_START + rel
        if word >> 26 == 3:
            calls.append((pc, jal_target(pc, word)))
    require(calls[-6:] == [
        (0x80190860, 0x80075358),
        (0x80190868, 0x80075358),
        (0x80190880, 0x80074DC0),
        (0x8019091C, 0x8007506C),
        (0x80190924, 0x80073A44),
        (0x8019092C, 0x80074A44),
    ], "DrawPrim/DrawSync/optional-LoadImage/VSync/ResetGraph call order")
    require(u32(prefix, 0x80190894 - PREFIX_START) == 0x84C20074 and
            u32(prefix, 0x8019089C - PREFIX_START) == 0x18400021,
            "environment +0x74 canonical bypass branch")
    print("  OK overlay: 183 words through ResetGraph; PutDrawEnv next")

    packet = [0x64000000, 0x00580020, 0x78000000, 0x00400100]
    draw_mode = 0xE1000018
    clut = packet[2] >> 16
    require((packet[0] >> 24) == 0x64 and
            (packet[1] & 0xFFFF, packet[1] >> 16) == (32, 88) and
            (packet[3] & 0x3FF, (packet[3] >> 16) & 0x1FF) == (256, 64) and
            ((draw_mode & 0xF) * 64,
             ((draw_mode >> 4) & 1) * 256) == (512, 256) and
            ((clut & 0x3F) * 16, (clut >> 6) & 0x1FF) == (0, 480),
            "SPRT/tpage/CLUT value model")

    def overlay_data(address: int, size: int) -> bytes:
        start = OVERLAY_OFFSET + address - OVERLAY_LOAD
        return peimg[start:start + size]

    palette = struct.unpack("<16H", overlay_data(0x801CED30, 32))
    packed = struct.unpack("<4096H", overlay_data(0x801CED5C, 8192))
    indices = bytes((word >> (nibble * 4)) & 0xF
                    for word in packed for nibble in range(4))
    require(hashlib.sha256(struct.pack("<16H", *palette)).hexdigest() ==
            PALETTE_SHA256 and palette[0] == 0x8000 and
            palette[15] == 0x7FFF and len(indices) == 256 * 64 and
            hashlib.sha256(indices).hexdigest() == INDEX_SHA256 and
            all(palette[index] != 0 for index in indices),
            "retail CLUT and expanded 4bpp texture identity")
    require(modulate(0x83E0, 0x64808080) == 0x83E0 and
            modulate(0x7FFF, 0x64404040) == 0x3DEF and
            modulate(0x1110, 0x64404040) == 0x0888,
            "independent modulation and bit-15 model")
    print("  OK model: 4bpp/CLUT decode, UV extent, transparency, modulation")

    gpu_source = (root / "pc_port" / "platform" /
                  "pe_gpu.c").read_text(encoding="utf-8")
    overlay_source = (root / "pc_port" / "game" / "boot" /
                      "func_80190660_port.c").read_text(encoding="utf-8")
    require("PE_GPU_GP0_RECT_EXPECT_POSITION" in gpu_source and
            "TexturedRectangle4ModeSupported" in gpu_source and
            "ModulateTextureColor" in gpu_source and
            "texture_color == 0u" in gpu_source,
            "generic GP0(64h) parser/raster source absent")
    require("PE_func_80075358_Transient(sprite_words, 4u)" in
            overlay_source and "func_80074DC0(0)" in overlay_source and
            "func_80073A44(0)" in overlay_source and
            "func_80074A44(1)" in overlay_source and
            "func_80075424(environment)" in overlay_source and
            "while (frame < 480u)" in overlay_source,
            "overlay continuation source absent")
    require("m0360i" not in gpu_source + overlay_source and
            "0xA8066048" not in gpu_source + overlay_source,
            "forbidden scene/scheduler special case")
    print("  OK source: generic GPU state; exact continuation; no planting")

    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KV"
    focused = run([str(tests)], 0, env)
    match = re.search(
        r"Results: (\d+) run, 2 passed, 0 failed, (\d+) skipped", focused)
    require(match is not None and
            int(match.group(1)) == int(match.group(2)) + 2,
            "two focused B54K-V contracts")

    common = [str(port), "--headless", "--disc-image", str(disc)]
    strict = run(common + ["--strict-stubs"], 1)
    require("first unresolved BOOTSTRAP_RET provider: func_801924F8_80192584_cut" in strict and
            "called from: func_801924F8" in strict,
            "strict loop-reentry frontier")
    normal = run(common + ["--dma-checkpoint-report"], 0)
    require("[STUB:BOOTSTRAP_RET] func_801924F8_80192584_cut" in normal and
            "[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0" in normal and
            "[DMA_CHECKPOINT] calls=27 queries=26 services=26 "
            "captured=26 serviced=26" in normal,
            "normal real-disc continuation effects")
    print("  OK runtime: 2 focused contracts; later loop frontier observed")
    print("\nB54K-V textured rectangle oracle: PASS.")


if __name__ == "__main__":
    main()

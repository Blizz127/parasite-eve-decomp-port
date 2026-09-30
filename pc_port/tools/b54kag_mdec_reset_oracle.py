#!/usr/bin/env python3
"""Independent B54K-AG oracle for MDEC reset and table submission."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import struct
import subprocess

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
MOVIE_START = 0x039F * 0x800
MOVIE_END = 0x03C5 * 0x800
MOVIE_LOAD = 0x8010BCF8
RESET = 0x8010C0FC
RESET_END = 0x8010C1EC
RESET_SHA256 = "53670ac94ba335b065bac7d63504e80a75a3bffe01cc3dff07c1582da260808b"
SUBMIT = 0x8010C1EC
SUBMIT_END = 0x8010C27C
SUBMIT_SHA256 = "31f4ab22c790632c4409a877ecd6f1d93bdaf391fc5069fabe543fb6554e7f0e"
QUANT_COMMAND = 0x8010DA0C
SCALE_COMMAND = 0x8010DA90
QUANT_SHA256 = "09cd1578bb59e1ed3968b2043278174f0eb74ecaab3b400ea048cbdab9a80328"
SCALE_SHA256 = "b128878a4faf48a10681b904435430150bc932b420b87a5b4cf42ae2da5a0e63"
CALLER_MODULE = 0x03D2 * 0x800
CALLER_LOAD = 0x8018EFF0
CALLER_CUT = 0x80192730


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {message}")


def u32(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def jal_target(word: int, pc: int) -> int:
    return ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def run(argv: list[str], expected: int,
        env: dict[str, str] | None = None) -> str:
    result = subprocess.run(argv, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True,
                            timeout=180, check=False, env=env)
    require(result.returncode == expected,
            f"exit {result.returncode}, expected {expected}\n"
            f"{result.stdout[-3000:]}")
    return result.stdout


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("peimg", type=pathlib.Path)
    parser.add_argument("exe", type=pathlib.Path)
    parser.add_argument("--port", type=pathlib.Path)
    parser.add_argument("--tests", type=pathlib.Path)
    parser.add_argument("--disc", type=pathlib.Path)
    args = parser.parse_args()
    root = pathlib.Path(__file__).resolve().parents[2]
    peimg = args.peimg.read_bytes()
    exe = args.exe.read_bytes()
    require(hashlib.sha1(peimg).hexdigest() == PEIMG_SHA1, "PE.IMG identity")
    require(hashlib.sha1(exe).hexdigest() == EXE_SHA1, "retail EXE identity")
    movie = peimg[MOVIE_START:MOVIE_END]

    reset_off = RESET - MOVIE_LOAD
    reset = movie[reset_off:reset_off + RESET_END - RESET]
    submit_off = SUBMIT - MOVIE_LOAD
    submit = movie[submit_off:submit_off + SUBMIT_END - SUBMIT]
    require(len(reset) == 60 * 4 and
            hashlib.sha256(reset).hexdigest() == RESET_SHA256,
            "60-word reset identity")
    require(len(submit) == 36 * 4 and
            hashlib.sha256(submit).hexdigest() == SUBMIT_SHA256,
            "36-word table-submit identity")
    require(jal_target(u32(reset, 0x64), RESET + 0x64) == SUBMIT and
            jal_target(u32(reset, 0x74), RESET + 0x74) == SUBMIT,
            "mode-zero table-submit calls")

    pointer_off = 0x8010DB1C - MOVIE_LOAD
    pointers = struct.unpack_from("<15I", movie, pointer_off)
    # DMA ch0..ch3 MADR/BCR/CHCR, then MDEC0/MDEC1 and DPCR (psx-spx map)
    require(pointers == tuple(
        0x1F801080 + 0x10 * ch + 4 * reg for ch in range(4) for reg in range(3)
    ) + (0x1F801820, 0x1F801824, 0x1F8010F0),
        "retail DMA/MDEC/DPCR pointer table")
    quant_off = QUANT_COMMAND - MOVIE_LOAD
    scale_off = SCALE_COMMAND - MOVIE_LOAD
    require(u32(movie, quant_off) == 0x40000001 and
            hashlib.sha256(movie[quant_off + 4:quant_off + 132]).hexdigest() ==
                QUANT_SHA256 and
            u32(movie, scale_off) == 0x60000000 and
            hashlib.sha256(movie[scale_off + 4:scale_off + 132]).hexdigest() ==
                SCALE_SHA256,
            "retail quantization/scale command blocks")
    print("  OK retail: 60-word reset, 36-word submit, exact MMIO table")
    print("  OK tables: quantization and scale command payloads authenticated")

    source = (root / "pc_port/game/boot/func_8010BE3C_port.c").read_text()
    model = (root / "pc_port/platform/pe_mdec.c").read_text()
    require("mode != 0 && mode != 1" in source and
            "func_8010C1EC(0x8010DA0Cu, 32u)" in source and
            "func_8010C1EC(0x8010DA90u, 32u)" in source and
            "PE_MDEC_SubmitInputTable" in model and
            "MDEC_DMA_CHCR_INPUT 0x01000201u" in model and
            "PE_GPU_ReadDPCR() | 0x88u" in model and
            "m0360i" not in source + model and
            "0xA8066048" not in source + model,
            "native MDEC-reset scope")

    tests = args.tests or root / "pc_port/build/pe-native-tests"
    port = args.port or root / "pc_port/build/parasite-eve-port"
    if args.disc:
        disc = args.disc
    else:
        disc = pathlib.Path(os.environ.get(
            "PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    for test_filter, pass_name in (
            ("B54KAG", "B54KAG_mdec_table_upload... PASS"),
            ("B54KY", "B54KY_192CE8_real_disc_issue_poll_and_boundary... PASS")):
        env = os.environ.copy()
        env["PE_TEST_FILTER"] = test_filter
        output = run([str(tests)], 0, env)
        require(pass_name in output and "0 failed" in output,
                f"focused {test_filter} contract")
    strict = run([str(port), "--headless", "--disc-image", str(disc),
                  "--strict-stubs"], 1)
    require("func_8007F0C8_completion_selector" in strict and
            "called from: func_8007F0C8" in strict,
            "strict post-reset caller frontier")
    print("  OK runtime: real tables plus synthetic and invalid-mode controls")
    print("  OK production: reset returns; strict path continues through callback registration")
    print("\nB54K-AG MDEC reset/table oracle: PASS.")


if __name__ == "__main__":
    main()

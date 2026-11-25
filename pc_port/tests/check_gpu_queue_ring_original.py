#!/usr/bin/env python3
"""Compare queue-field reads to a bounded original pump instruction span.

The executable is an already provisioned private input. Only the specified
608-byte span is read, verified, and held in memory. No retail bytes are output.
The MIPS runner is test-only; no emulator enters the production runtime.
"""
import argparse
import ctypes
import hashlib
import random
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "pc_port/tools"))
from pe_battle_hud_oracle import execute

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("library")
parser.add_argument("exe")
args = parser.parse_args()
with open(args.exe, "rb") as f:
    f.seek(0x676e4)
    subject = f.read(0x260)
expected_sha = "a124857ab6fd91a3b68ea3e5c2efa5337bf6ed5528ef94ca23bc4a781337a78c"
if len(subject) != 0x260 or hashlib.sha256(subject).hexdigest() != expected_sha:
    raise SystemExit("pump span missing or digest mismatch; no comparison run")

lib = ctypes.CDLL(args.library)
lib.PE_RamInit.argtypes = []
lib.PE_RamReset.argtypes = []
lib.PE_RamDestroy.argtypes = []
lib.PE_StoreU32.argtypes = [ctypes.c_uint32, ctypes.c_uint32]
lib.PE_GpuQueue_ReadWord.argtypes = [ctypes.c_uint32, ctypes.c_uint32,
                                      ctypes.POINTER(ctypes.c_uint32)]
lib.PE_GpuQueue_ReadWord.restype = ctypes.c_int
rng = random.Random(0x80076fcc)
cases = [0, 1, 63, 64, 0xffffffff, 0x08000000]
for _ in range(250):
    cases.append((rng.randrange(32) * 0x08000000 + rng.randrange(512)) & 0xffffffff)
lib.PE_RamInit()
for case, index in enumerate(cases):
    address = (0x800bd030 + index * 96) & 0xffffffff
    if not 0x80000000 <= address <= 0x801ffff4:
        raise AssertionError("fixture only uses valid, aligned RAM fields")
    ram = bytearray(0x200000)
    ram[0x76ee4:0x77144] = subject
    struct.pack_into("<I", ram, 0x95878, index)
    lib.PE_RamReset()
    for field in (0, 4, 8):
        value = rng.getrandbits(32)
        struct.pack_into("<I", ram, (address & 0x1fffff) + field, value)
        lib.PE_StoreU32(address + field, value)
    visited = set()
    regs = execute(ram, 0x80076fcc, stop_at=(0x80077034,),
                   visited_pcs=visited, instruction_budget=64)
    expected_pcs = set(range(0x80076fcc, 0x80077034, 4))
    if visited != expected_pcs:
        raise AssertionError(f"case {case}: execution left the bounded straight-line span")
    for field, register in ((0, 2), (4, 4), (8, 5)):
        value = ctypes.c_uint32()
        if not lib.PE_GpuQueue_ReadWord(index, field, ctypes.byref(value)):
            raise AssertionError(f"case {case}: unexpected reader rejection")
        if value.value != regs[register]:
            raise AssertionError(f"case {case}: mismatch in field {field}")
lib.PE_RamDestroy()
print(f"PASS {len(cases)} bounded-original queue-read cases, three fields each; pump span SHA256 {expected_sha}")

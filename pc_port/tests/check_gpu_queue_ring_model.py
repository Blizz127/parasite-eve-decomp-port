#!/usr/bin/env python3
"""Compare the standalone native reader to an independent host arithmetic model."""
import argparse
import ctypes
import random

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("library", help="freshly built test-only shared library")
args = p.parse_args()
lib = ctypes.CDLL(args.library)
lib.PE_RamInit.argtypes = []
lib.PE_RamDestroy.argtypes = []
lib.PE_Translate.argtypes = [ctypes.c_uint32, ctypes.c_size_t]
lib.PE_Translate.restype = ctypes.c_void_p
lib.PE_GpuQueue_ReadWord.argtypes = [ctypes.c_uint32, ctypes.c_uint32,
                                      ctypes.POINTER(ctypes.c_uint32)]
lib.PE_GpuQueue_ReadWord.restype = ctypes.c_int

size = 0x200000
memory = bytes(((i * 37 + (i >> 8) * 13 + 71) & 255) for i in range(size))
lib.PE_RamInit()
ctypes.memmove(lib.PE_Translate(0x80000000, size), memory, size)
rng = random.Random(0x76EE4)
cases = [(0, 0), (63, 4), (64, 8), (0x08000000, 0),
         (0xffffffff, 0), (0x10000, 0), (0, 12), (0, 0xffffffff)]
for _ in range(24000):
    kind = rng.randrange(3)
    index = (rng.getrandbits(32) if kind == 0 else
             rng.randrange(0x10000) if kind == 1 else
             (rng.randrange(32) * 0x08000000 + rng.randrange(0x10000)) & 0xffffffff)
    field = rng.choice((0, 4, 8, 12, 0xffffffff))
    cases.append((index, field))
accepted = rejected = 0
for index, field in cases:
    address = (0x800bd030 + index * 96 + field) & 0xffffffff
    valid = field in (0, 4, 8) and 0x80000000 <= address <= 0x801ffffc
    out = ctypes.c_uint32(0xdeadbeef)
    got = lib.PE_GpuQueue_ReadWord(index, field, ctypes.byref(out))
    if valid:
        offset = address - 0x80000000
        expected = int.from_bytes(memory[offset:offset + 4], "little")
        accepted += 1
    else:
        expected = 0xdeadbeef
        rejected += 1
    if got != int(valid) or out.value != expected:
        raise AssertionError(f"index={index:#x} field={field:#x} "
                             f"got=({got},{out.value:#x}) expected=({int(valid)},{expected:#x})")
if lib.PE_GpuQueue_ReadWord(0, 0, None) != 0:
    raise AssertionError("null output must fail")
lib.PE_RamDestroy()
print(f"PASS {len(cases)} independent arithmetic-model cases: {accepted} valid, {rejected} rejected; null output rejected")

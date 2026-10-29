#!/usr/bin/env python3
"""List every MIPS store (sb/sh/sw/swl/swr) whose 16-bit immediate is one of
the given values, in the retail EXE text and in each extracted PE.IMG blob.

A coarse but complete syntactic search for direct writers of a field or a
global (e.g. 0x944 = D_800B0CD8+0x944 through a base register, 0x161C =
%lo(D_800B161C)).  Stores through a computed address are not found; a blob
hit can be a data word that happens to decode as a store, so read the
neighbouring words before trusting one.

Read-only over the user's extracted disc; prints addresses and instruction
fields, never bytes.

usage: store_imm_scan.py IMM [IMM ...]   (run from the repo root)
"""
import struct
import sys
from pathlib import Path

STORES = {0x28: "sb", 0x29: "sh", 0x2A: "swl", 0x2B: "sw", 0x2E: "swr"}


def scan(data, base, imms):
    for off in range(0, len(data) - 3, 4):
        w = struct.unpack_from("<I", data, off)[0]
        op = w >> 26
        if op in STORES and (w & 0xFFFF) in imms:
            yield base + off, STORES[op], (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    imms = {int(a, 0) for a in sys.argv[1:]}
    disc = Path("build/extracted/disc1")
    exe = (disc / "SLUS_006.62").read_bytes()
    t_addr, t_size = struct.unpack_from("<II", exe, 0x18)
    hits = 0
    for addr, op, base, rt, imm in scan(exe[0x800:0x800 + t_size], t_addr, imms):
        print(f"EXE {addr:08X} {op} ${rt},0x{imm:X}(${base})")
        hits += 1
    blobs = sorted((disc / "peimg").glob("*.bin"))
    for blob in blobs:
        for off, op, base, rt, imm in scan(blob.read_bytes(), 0, imms):
            print(f"{blob.name} +{off:X} {op} ${rt},0x{imm:X}(${base})")
            hits += 1
    print(f"store_imm_scan: {hits} hit(s) in the EXE text and {len(blobs)} PE.IMG blobs")


if __name__ == "__main__":
    main()

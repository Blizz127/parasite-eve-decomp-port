#!/usr/bin/env python3
"""Trim trailing ELF section padding caused by GNU as section alignment.

Phase 4I: gas emits .text with sh_addralign=16 and .rodata with sh_addralign=8,
so section sizes grow with trailing zeros (+4 / +12 / +8 for PE1 units). The
original PS-X EXE file spans are not 16-byte aligned at segment boundaries
(e.g. text starts at file 0x2A0C ≡ 12 mod 16). Trailing zero pad plus high
align causes linker gaps and shifts all later content.

This tool:
  1. Verifies bytes beyond the target size are all zero (else refuses).
  2. Sets sh_size to the target size.
  3. Lowers sh_addralign to min(current, 4) so ld does not re-insert pad.

Does not invent content. In-place update of a relocatable ELF32 LE object.
"""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path


def die(msg: str) -> None:
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("object", type=Path, help="relocatable ELF32 object")
    ap.add_argument("section", help="section name, e.g. .text or .rodata")
    ap.add_argument("size", help="target size in bytes (int or 0x hex)")
    ap.add_argument(
        "--align",
        default="4",
        help="max sh_addralign after trim (default 4)",
    )
    args = ap.parse_args()
    target = int(args.size, 0)
    max_align = int(args.align, 0)

    data = bytearray(args.object.read_bytes())
    if data[:4] != b"\x7fELF":
        die(f"{args.object}: not ELF")
    ei_class, ei_data = data[4], data[5]
    if ei_class != 1 or ei_data != 1:
        die(f"{args.object}: need ELF32 little-endian (class={ei_class} data={ei_data})")

    e_shoff = struct.unpack_from("<I", data, 32)[0]
    e_shentsize = struct.unpack_from("<H", data, 46)[0]
    e_shnum = struct.unpack_from("<H", data, 48)[0]
    e_shstrndx = struct.unpack_from("<H", data, 50)[0]
    if e_shentsize != 40:
        die(f"unexpected e_shentsize {e_shentsize}")

    def sh_at(i: int) -> int:
        return e_shoff + i * e_shentsize

    # Section header: name, type, flags, addr, offset, size, link, info, addralign, entsize
    shstr_off = struct.unpack_from("<I", data, sh_at(e_shstrndx) + 16)[0]

    def sec_name(i: int) -> str:
        name_off = struct.unpack_from("<I", data, sh_at(i) + 0)[0]
        start = shstr_off + name_off
        end = data.index(b"\x00", start)
        return data[start:end].decode("ascii", errors="replace")

    idx = None
    for i in range(e_shnum):
        if sec_name(i) == args.section:
            idx = i
            break
    if idx is None:
        die(f"section {args.section!r} not found in {args.object}")

    base = sh_at(idx)
    sh_offset = struct.unpack_from("<I", data, base + 16)[0]
    sh_size = struct.unpack_from("<I", data, base + 20)[0]
    sh_addralign = struct.unpack_from("<I", data, base + 32)[0]

    if target > sh_size:
        die(f"target size 0x{target:X} > current 0x{sh_size:X}")
    if target == sh_size and sh_addralign <= max_align:
        print(f"OK {args.object} {args.section}: already size 0x{sh_size:X} align {sh_addralign}")
        return

    tail = bytes(data[sh_offset + target : sh_offset + sh_size])
    if any(tail):
        die(
            f"{args.object} {args.section}: bytes beyond 0x{target:X} are not all zero "
            f"(first nonzero evidence — refusing silent truncate)"
        )

    new_align = min(sh_addralign, max_align) if sh_addralign else max_align
    if new_align < 1:
        new_align = 1

    struct.pack_into("<I", data, base + 20, target)  # sh_size
    struct.pack_into("<I", data, base + 32, new_align)  # sh_addralign
    args.object.write_bytes(data)
    print(
        f"OK {args.object} {args.section}: "
        f"size 0x{sh_size:X}→0x{target:X} (-{sh_size - target}), "
        f"align {sh_addralign}→{new_align} (tail was zero pad)"
    )


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Print the PS-X EXE header of a PlayStation executable.

Usage: psxexe_info.py <SLUS_006.XX>

Header layout (2048-byte header, code follows):
    0x00  magic "PS-X EXE"
    0x10  pc0     initial program counter
    0x14  gp0     initial GP
    0x18  t_addr  load address of text+data
    0x1C  t_size  size of text+data (should equal filesize - 2048)
    0x30  s_addr  initial stack pointer base
    0x34  s_size  initial stack size
    0x4C  ASCII region marker string
"""

import os
import struct
import sys


def main(argv):
    if len(argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2
    path = argv[1]
    size = os.path.getsize(path)
    with open(path, "rb") as f:
        hdr = f.read(2048)
    if hdr[:8] != b"PS-X EXE":
        print(f"ERROR: {path}: missing 'PS-X EXE' magic", file=sys.stderr)
        return 1
    pc0, gp0, t_addr, t_size = struct.unpack_from("<4I", hdr, 0x10)
    s_addr, s_size = struct.unpack_from("<2I", hdr, 0x30)
    region = hdr[0x4C:0x100].split(b"\x00")[0].decode("ascii", "replace")
    print(f"file:      {path}")
    print(f"file_size: {size}")
    print(f"pc0:       0x{pc0:08X}")
    print(f"gp0:       0x{gp0:08X}")
    print(f"t_addr:    0x{t_addr:08X}")
    print(f"t_size:    0x{t_size:X} ({t_size})")
    print(f"s_addr:    0x{s_addr:08X}")
    print(f"s_size:    0x{s_size:X}")
    print(f"region:    {region}")
    if t_size != size - 2048:
        print(
            f"WARNING: t_size {t_size} != file_size-2048 {size - 2048}",
            file=sys.stderr,
        )
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

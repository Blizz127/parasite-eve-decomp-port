#!/usr/bin/env python3
"""Check that a packaged port binary carries every overlay fingerprint.

A build made without build/extracted present silently writes fingerprint 0
for every overlay entry (the r3/r4 defect), so the binary can never select
generated room/overlay code. This counts, in the given binary, how many of the
distinct non-zero overlay fingerprints from the generated registry
(pc_port/game/decomp_ovl/_dispatch/zz_dispatch.c) are present as little-endian
32-bit constants. A correct build shows N/N (221/221 at r5).

Usage: check_release_fingerprints.py <binary> [zz_dispatch.c]
Exit 0 only when every fingerprint is present.
"""
import re
import struct
import sys

def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    binary = sys.argv[1]
    registry = sys.argv[2] if len(sys.argv) > 2 else \
        "pc_port/game/decomp_ovl/_dispatch/zz_dispatch.c"
    text = open(registry, encoding="utf-8").read()
    fps = sorted({int(m, 16) for m in re.findall(r"(0x[0-9A-Fa-f]{8})u, pe_gct", text)} - {0})
    data = open(binary, "rb").read()
    have = sum(struct.pack("<I", f) in data for f in fps)
    print(f"{have}/{len(fps)} distinct overlay fingerprints present in {binary}")
    return 0 if fps and have == len(fps) else 1

if __name__ == "__main__":
    sys.exit(main())

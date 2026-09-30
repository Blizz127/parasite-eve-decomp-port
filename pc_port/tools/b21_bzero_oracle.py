#!/usr/bin/env python3
"""Independent oracle for BIOS A(28h) bzero.

This models the contract from the BIOS table, not the native provider and not
post-execution RAM: every expected byte write is generated from dst/len.
"""
from __future__ import annotations

import json
import sys

RAM_BASE = 0x80000000
RAM_END = RAM_BASE + 0x200000


def expected(dst: int, length: int) -> dict:
    dst &= 0xFFFFFFFF
    length &= 0xFFFFFFFF
    # The project policy accepts only a complete in-RAM range; zero length
    # still requires an in-RAM address, matching PE_RangeIsRam.
    valid = (RAM_BASE <= dst < RAM_END and
             (length == 0 or dst + length <= RAM_END))
    writes = list(range(dst, dst + length)) if valid else []
    return {"destination": dst, "length": length, "value": 0,
            "writes": writes, "valid": valid,
            "first": writes[0] if writes else None,
            "last": writes[-1] if writes else None}


def main() -> int:
    if len(sys.argv) != 3:
        raise SystemExit(f"usage: {sys.argv[0]} DST LEN")
    dst = int(sys.argv[1], 0)
    length = int(sys.argv[2], 0)
    print(json.dumps(expected(dst, length), separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

"""PE-BTL150: search retail name and index forms for m0360i.

The retail alphabet and six 5-bit shifts are taken directly from
func_8006E2D0/func_8006E3D4 and D_800930B4.  This scanner deliberately reports
both byte orders for packed words and the raw ASCII spellings.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path


ALPHABET = "0123456789abcdefghiklmnopqrstuvwxy"
SHIFTS = (27, 22, 17, 12, 7, 2)
TARGET = "m0360i"
TARGET_INDEX = 359


def pack(name: str) -> int:
    if len(name) != 6 or any(c not in ALPHABET for c in name):
        raise ValueError(name)
    return sum(ALPHABET.index(c) << shift for c, shift in zip(name, SHIFTS))


def unpack(token: int) -> str:
    return "".join(ALPHABET[(token >> shift) & 0x1F] for shift in SHIFTS)


def hits(data: bytes, needle: bytes) -> list[int]:
    out: list[int] = []
    start = 0
    while True:
        pos = data.find(needle, start)
        if pos < 0:
            return out
        out.append(pos)
        start = pos + 1


def scan(path: Path) -> dict:
    data = path.read_bytes()
    token = pack(TARGET)
    result = {
        "path": str(path),
        "size": len(data),
        "ascii_lower": [hex(x) for x in hits(data, TARGET.encode("ascii"))],
        "ascii_upper": [hex(x) for x in hits(data, TARGET.upper().encode("ascii"))],
        "packed_le": [hex(x) for x in hits(data, token.to_bytes(4, "little"))],
        "packed_be": [hex(x) for x in hits(data, token.to_bytes(4, "big"))],
        "index_359_u16_le": [hex(x) for x in hits(data, TARGET_INDEX.to_bytes(2, "little"))],
        "index_359_u16_be": [hex(x) for x in hits(data, TARGET_INDEX.to_bytes(2, "big"))],
        "index_359_u32_le": [hex(x) for x in hits(data, TARGET_INDEX.to_bytes(4, "little"))],
        "index_359_u32_be": [hex(x) for x in hits(data, TARGET_INDEX.to_bytes(4, "big"))],
    }
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--peimg", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    token = pack(TARGET)
    payload = {
        "target_name": TARGET,
        "target_index": TARGET_INDEX,
        "alphabet": ALPHABET,
        "shifts": list(SHIFTS),
        "packed_token": f"0x{token:08X}",
        "unpacked_token": unpack(token),
        "known_round_trips": {
            name: f"0x{pack(name):08X}"
            for name in ("m0005i", "m0295i", "m0291i", "m0424i", "m0367i")
        },
        "scans": [scan(args.exe), scan(args.peimg)],
    }
    args.output.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Insert a matching `c` leaf row into configs/USA/disc1.yaml, idempotently.

A leaf lives inside an existing `asm` span. Carving it means:
  * keep the head of the asm span (if the leaf does not start it),
  * emit `- [<file_off>, c, <name>]`,
  * resume `- [<file_off+size>, asm]` (if the leaf does not end the span).

The YAML owns geometry, so this is the only sanctioned way to add a row:
doing it by hand is how spans end up overlapping or swallowing a neighbour.

Usage:
  tools/build/carve_leaf.py <func_name> <size_hex> [--comment TEXT] [--dry-run]

The file offset is derived from the symbol's own VRAM (repo convention:
address-named symbols), so it cannot disagree with the name.
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
YAML = ROOT / "configs" / "USA" / "disc1.yaml"
VRAM_BASE = 0x80010000
FILE_BASE = 0x800

ROW_RE = re.compile(r"^(\s*)-\s*\[0x([0-9A-Fa-f]+),\s*(\w+)(?:,\s*([A-Za-z0-9_]+))?\]\s*$")


def rows(lines):
    """Yield (index, indent, offset, kind, symbol) for every subsegment row."""
    for i, line in enumerate(lines):
        m = ROW_RE.match(line)
        if m:
            yield i, m.group(1), int(m.group(2), 16), m.group(3), m.group(4)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("name")
    ap.add_argument("size")
    ap.add_argument("--comment", default=None)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    m = re.fullmatch(r"func_([0-9A-Fa-f]{8})", args.name)
    if not m:
        sys.exit(f"name must be func_<8 hex vram digits>, got {args.name!r}")
    vram = int(m.group(1), 16)
    size = int(args.size, 16)
    if size <= 0 or size % 4:
        sys.exit(f"size must be a positive multiple of 4, got {args.size}")
    off = vram - VRAM_BASE + FILE_BASE
    end = off + size

    lines = YAML.read_text().split("\n")
    table = list(rows(lines))
    if any(sym == args.name for _, _, _, _, sym in table):
        print(f"carve_leaf: {args.name} already carved")
        return 0

    # Locate the span containing `off`.
    host = None
    for n, (idx, indent, roff, kind, _sym) in enumerate(table):
        nxt = table[n + 1][2] if n + 1 < len(table) else None
        if roff <= off and (nxt is None or off < nxt):
            host = (idx, indent, roff, kind, nxt)
            break
    if host is None:
        sys.exit(f"no span contains file offset 0x{off:X}")
    idx, indent, roff, kind, nxt = host
    if kind != "asm":
        sys.exit(f"0x{off:X} lands in a `{kind}` span starting 0x{roff:X}, not asm")
    if nxt is not None and end > nxt:
        sys.exit(f"leaf 0x{off:X}+0x{size:X} overruns its asm span (next row 0x{nxt:X})")

    block = []
    if args.comment:
        block.append(f"{indent}# {args.comment}")
    block.append(f"{indent}- [0x{off:X}, c, {args.name}]")
    if nxt is None or end < nxt:
        block.append(f"{indent}- [0x{end:X}, asm]")

    if off == roff:
        # The leaf starts the asm span: replace the asm row outright.
        new = lines[:idx] + block + lines[idx + 1:]
    else:
        # Keep the asm head, then the leaf.
        new = lines[:idx + 1] + block + lines[idx + 1:]

    text = "\n".join(new)
    if args.dry_run:
        print("\n".join(block))
        return 0
    YAML.write_text(text)
    print(f"carve_leaf: {args.name} at 0x{off:X} size 0x{size:X} "
          f"(host asm 0x{roff:X}{'' if nxt is None else f', next 0x{nxt:X}'})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

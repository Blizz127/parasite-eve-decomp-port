#!/usr/bin/env python3
"""Prove a stripped test header restores byte-identical seeds.

For each (original, stripped) header pair: every numeric token of the
stripped file equals the original except tokens zeroed by the stripper, and
every zeroed token is covered by a RETAILFIX entry whose disc bytes equal the
original value.  Needs the disc (tools/extract/disc_cache.py populate).

  verify_retail_fixups.py ORIG_DIR HEADER...
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/analysis"))
sys.path.insert(0, str(ROOT / "tools/extract"))
import strip_retail_seeds as srs  # noqa: E402
import disc_cache  # noqa: E402


def arrays(data: bytes):
    out = {}
    for dm in srs.ROWS_DECL_RE.finditer(data):
        be = srs.find_block_end(data, dm.end())
        out[dm.group(2).decode()] = ("rows", 4, [(srs._int(r.group(1)), srs._int(r.group(2)))
                                           for r in srs.ROW_RE.finditer(data, dm.end(), be)])
    for dm in srs.FLAT_DECL_RE.finditer(data):
        be = srs.find_block_end(data, dm.end())
        out[dm.group(3).decode()] = ("flat", srs.WIDTH[dm.group(2)],
                                     [srs._int(t.group(1)) for t in srs.NUM_RE.finditer(data, dm.end(), be)])
    return out


def main(argv) -> int:
    orig_dir = Path(argv[0])
    d = disc_cache.cache_dir_if_ready()
    if d is None:
        print("ERROR: disc cache not populated", file=sys.stderr)
        return 2
    src = {"PE_RF_EXE": (d / "SLUS_006.62").read_bytes(), "PE_RF_PEIMG": None}
    peimg = d / "PE.IMG"
    bad = 0
    for h in argv[1:]:
        new = Path(h).read_bytes()
        old = (orig_dir / Path(h).name).read_bytes()
        m = srs.FIX_RE.search(new)
        fixes = [] if not m else [(k.decode(), p.decode().strip().lstrip("&").split("[")[0],
                                   int(a), int(b), s.decode(), int(o, 16))
                                  for k, p, a, b, s, o in srs.FIXROW_RE.findall(m.group(0))]
        A, B = arrays(old), arrays(new)
        restored = {n: list(v[2]) for n, v in B.items()}
        for k, name, a, b, s, o in fixes:
            if s == "PE_RF_PEIMG" and src[s] is None:
                src[s] = peimg.read_bytes()
            blob = src[s]
            kind, width, _ = B[name]
            if k == "PE_RF_ROWS":
                for i in range(b):
                    w = int.from_bytes(blob[o + 4 * i:o + 4 * i + 4], "little")
                    addr, _v = restored[name][a + i]
                    restored[name][a + i] = (addr, w)
            else:
                chunk = blob[o:o + b]
                for i in range(0, b, width):
                    restored[name][(a + i) // width] = int.from_bytes(chunk[i:i + width], "little")
        err = []
        if set(A) != set(B):
            err.append(f"array set differs: {sorted(set(A) ^ set(B))}")
        for n in A:
            if n in B and A[n][2] != restored[n]:
                diffs = sum(1 for x, y in zip(A[n][2], restored[n]) if x != y)
                err.append(f"{n}: {diffs} element(s) differ after restore "
                           f"(len {len(A[n][2])} vs {len(restored[n])})")
        # non-array text must be unchanged apart from const/pragma/fixup tail
        strip_new = srs.FIX_RE.sub(b"\n", new).replace(b"#pragma once\n", b"", 1)
        tok_old = [t for t in re.findall(rb"\b[A-Za-z_]\w*", old) if t != b"const"]
        tok_new = [t for t in re.findall(rb"\b[A-Za-z_]\w*", strip_new) if t != b"const"]
        if tok_old != tok_new:
            err.append("identifier stream differs")
        if err:
            bad += 1
            print(f"FAIL {h}: " + "; ".join(err))
        else:
            print(f"OK   {h}: {len(fixes)} fixup(s) restore the original bytes")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

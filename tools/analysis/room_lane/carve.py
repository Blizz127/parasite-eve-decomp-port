#!/usr/bin/env python3
"""carve.py <target> <func> [<func> ...]: turn asm-span functions into c rows.
Reads the function offset/size from asm/overlays/<t>/*.s nonmatching lines."""
import re, sys, glob
from pathlib import Path
ROOT = Path(__file__).resolve().parents[3]
t = sys.argv[1]
funcs = {}
for f in glob.glob(str(ROOT / f"asm/overlays/{t}/*.s")):
    txt = open(f).read()
    for m in re.finditer(r"nonmatching (func_[0-9A-F]{8}), (0x[0-9A-F]+)\s+glabel \1\s+/\* ([0-9A-F]+) ", txt):
        funcs[m.group(1)] = (int(m.group(3), 16), int(m.group(2), 16))
yp = ROOT / f"configs/USA/overlays/{t}.yaml"
lines = open(yp).read().split("\n")
row = re.compile(r"^(\s*)- \[(0x[0-9A-Fa-f]+),\s*(\w+)(?:,\s*(\w+))?\]\s*$")
rows = []  # (idx, off, kind, name)
for i, l in enumerate(lines):
    m = row.match(l)
    if m:
        rows.append([i, int(m.group(2), 16), m.group(3), m.group(4), m.group(1)])
indent = rows[0][4]
for fn in sys.argv[2:]:
    off, size = funcs[fn]
    # find containing row
    rows_sorted = sorted(rows, key=lambda r: r[1])
    cont = [r for r in rows_sorted if r[1] <= off][-1]
    assert cont[2] == "asm", (fn, cont)
    nxt = [r for r in rows_sorted if r[1] > off][0]
    new = []
    if cont[1] == off:
        cont[2], cont[3] = "c", fn
    else:
        new.append([None, off, "c", fn, indent])
    if off + size < nxt[1]:
        new.append([None, off + size, "asm", None, indent])
    else:
        assert off + size == nxt[1], (fn, hex(off+size), hex(nxt[1]))
    rows += new
# rebuild: keep non-row lines (comments) attached before their row by original index
out = []
rowset = {r[0] for r in rows if r[0] is not None}
body_start = min(rowset); body_end = max(rowset)
head = lines[:body_start]
tail = lines[body_end + 1:]
# comments between rows: keep attached to the following original row
attached = {}
pend = []
for i in range(body_start, body_end + 1):
    if i in rowset:
        attached[i] = pend; pend = []
    else:
        pend.append(lines[i])
def fmt(r):
    return f"{r[4]}- [0x{r[1]:X}, {r[2]}" + (f", {r[3]}]" if r[3] else "]")
for r in sorted(rows, key=lambda r: r[1]):
    if r[0] is not None:
        out += attached.get(r[0], [])
    out.append(fmt(r))
open(yp, "w").write("\n".join(head + out + tail))
for fn in sys.argv[2:]:
    print(fn, hex(funcs[fn][0]), hex(funcs[fn][1]))

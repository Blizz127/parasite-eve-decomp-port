#!/usr/bin/env python3
import json, re, sys
from pathlib import Path
import os
TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[2]
W = Path(os.environ.get("PORTVERIFY_WORK", ROOT / "build" / "portverify"))
HDR = re.compile(r"^[ \t]*((?:static\s+)?(?:inline\s+)?[A-Za-z_][A-Za-z0-9_ \t\*]*?)\b(func_[0-9A-Fa-f]{8})\s*\(([^;{]*?)\)\s*\{", re.M)
def strip(t): return re.sub(r"//[^\n]*", "", re.sub(r"/\*.*?\*/", "", t, flags=re.S))
def header(path, name):
    t = strip((ROOT/path).read_text(errors="replace"))
    for m in HDR.finditer(t):
        if m.group(2) == name:
            return " ".join(m.group(1).split()), [" ".join(p.split()) for p in m.group(3).split(",") if p.strip() and p.strip()!="void"]
    return None
c = json.load(open(W/"conflicts.json"))
res = {}
for n, v in c.items():
    g = header(str(W/"gen"/f"{n}_port.c"), n)
    h = header(v["hand"][0], n)
    res[n] = {"gen": g, "hand": h}
json.dump(res, open(W/"sigs.json","w"), indent=1)
if __name__ == "__main__":
    for n, r in res.items():
        if "-v" in sys.argv: print(n, r["gen"], "|", r["hand"])

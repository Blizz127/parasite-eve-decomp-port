#!/usr/bin/env python3
"""Emit driver.c: a differential table {name, gen-call, hand-call, param kinds}."""
import json, re, sys
from pathlib import Path
import os
TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[2]
W = Path(os.environ.get("PORTVERIFY_WORK", ROOT / "build" / "portverify"))
sigs = json.load(open(W/"sigs.json")); amap = json.load(open(TOOL/"argmap.json")); conf = json.load(open(W/"conflicts.json"))
names = sys.argv[1:] or sorted(p.stem for p in (W/"obj").glob("*.o"))
def ptype(p):
    p = re.sub(r"\bconst\b", "", p).strip()
    m = re.match(r"(.*?)[\s\*]*\b([A-Za-z_]\w*)$", p)
    t = p[: p.rfind(m.group(2))].strip() if m else p
    return norm(t or "int")
KNOWN = {"int", "unsigned int", "unsigned", "short", "unsigned short", "char",
         "unsigned char", "signed char", "long", "unsigned long", "void",
         "pe_addr_t", "uint32_t", "int32_t", "uint16_t", "int16_t", "uint8_t",
         "int8_t", "s8", "u8", "s16", "u16", "s32", "u32", "size_t", "uintptr_t"}
def norm(t):
    """Leaf-local typedefs (e.g. `func_8009B6D0_t`, a guest code pointer) are
    not visible to the driver: map any unknown type to a 32-bit guest word."""
    t = " ".join(w for w in t.split() if w not in ("volatile", "register"))
    if "*" in t or t in KNOWN: return t
    return "pe_addr_t" if t.startswith("func_") else "uint32_t"
def rtype(r):
    return norm(" ".join(w for w in r.split() if w not in ("static", "inline", "extern")) or "int")
out = ['#include "harness.h"']
rows = []
for n in names:
    s = sigs[n]; g = s["gen"]; h = s["hand"]
    if "static" in h[0]: continue
    gp = [ptype(p) for p in g[1]]; hp = [ptype(p) for p in h[1]]
    if any("*" in t for t in gp + hp): continue
    gr = rtype(g[0]); hr = rtype(h[0])
    k = max(len(gp), len(hp))
    types = [(gp[i] if i < len(gp) else hp[i]) for i in range(k)]
    ktypes = list(types)
    decl = ", ".join(types) or "void"
    args = ", ".join(f"({t})a[{i}]" for i, t in enumerate(types))
    if n in amap:
        types = [(hp[i] if i < len(hp) else "uint32_t") for i in range(len(amap[n]))]
        decl = ", ".join(types) or "void"
        args = ", ".join(f"({t})a[{j}]" for t, j in zip(types, amap[n]))
    out.append(f"extern {gr} pv_{n}({', '.join(gp) or 'void'});")
    out.append(f"extern {hr} {n}({decl});")
    gargs = ", ".join(f"({t})a[{i}]" for i, t in enumerate(gp))
    if gr == "void":
        out.append(f"static int64_t G_{n}(const uint32_t *a) {{ pv_{n}({gargs}); return 0; }}")
        out.append(f"static int64_t H_{n}(const uint32_t *a) {{ {n}({args}); return 0; }}")
        rk = 0
    elif hr == "void":
        out.append(f"static int64_t G_{n}(const uint32_t *a) {{ (void)pv_{n}({gargs}); return 0; }}")
        out.append(f"static int64_t H_{n}(const uint32_t *a) {{ {n}({args}); return 0; }}")
        rk = 2
    else:
        out.append(f"static int64_t G_{n}(const uint32_t *a) {{ return (int64_t)({gr})pv_{n}({gargs}); }}")
        out.append(f"static int64_t H_{n}(const uint32_t *a) {{ return (int64_t)({gr}){n}({args}); }}")
        rk = 1
    kinds = "".join("p" if t == "pe_addr_t" else ("b" if "char" in t else "h" if "short" in t else "i") for t in ktypes)
    rows.append(f'  {{"{n}", G_{n}, H_{n}, "{kinds}", {rk}}},')
out.append("const PvEntry pv_table[] = {"); out += rows; out.append("  {0,0,0,0,0}};")
(W/"driver_table.c").write_text("\n".join(out) + "\n")
print(len(rows), "entries")

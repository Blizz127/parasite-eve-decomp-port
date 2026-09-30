#!/usr/bin/env python3
"""Flag int->host-pointer casts in generated TUs: (T *)X where X roots at a
SCALAR D_ macro or an integer-typed parameter/local."""
import re, sys
from pathlib import Path
CAST = re.compile(r"\(\s*((?:const\s+|volatile\s+|unsigned\s+|signed\s+|struct\s+)*\w+)\s*(\*+)\s*\)\s*\(?\s*([A-Za-z_]\w*)")
def audit(path):
    t = re.sub(r"/\*.*?\*/", "", Path(path).read_text(), flags=re.S)
    scal = set(re.findall(r"#define (D_\w+) PE_DECOMP_SCALAR", t))
    ints = set()
    for m in re.finditer(r"\b(?:unsigned\s+|signed\s+)?(?:int|short|char|long|u32|s32|u16|s16|u8|s8|uint32_t|int32_t|pe_addr_t)\s+([A-Za-z_]\w*)\s*[,);=]", t):
        ints.add(m.group(1))
    hits = []
    for m in CAST.finditer(t):
        root = m.group(3)
        if root in scal or (root in ints and not root.startswith("host_")):
            line = t[:m.start()].count("\n") + 1
            hits.append((line, t[m.start():m.start()+60].split("\n")[0]))
    for m in re.finditer(r"\*\s*\*\s*\)", t):
        line = t[:m.start()].count("\n") + 1
        hits.append((line, t[max(0,m.start()-30):m.start()+30].split("\n")[-1]))
    return hits
bad = 0
for p in sys.argv[1:]:
    h = audit(p)
    if h:
        bad += 1
        print(Path(p).name, h[:2])
print("files flagged:", bad, file=sys.stderr)

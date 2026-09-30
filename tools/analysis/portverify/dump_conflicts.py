#!/usr/bin/env python3
"""Render the generated TU of every conflict leaf (eligible, but already hand-defined)."""
import json, sys
from pathlib import Path
import os
TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[2]
W = Path(os.environ.get("PORTVERIFY_WORK", ROOT / "build" / "portverify"))
sys.path.insert(0, str(ROOT / "tools/analysis"))
import gen_decomp_ports as g

matched = g.parse_matched()
defs = g.pc_port_definitions(); known = set(defs)
shim = g.pc_port_macros(); protos = g.canonical_protos(); sigs = g.canonical_signatures()
hp = set(protos)
dsigs = {n: s for n, s in g.derived_signatures().items() if n not in hp}
for n, s in dsigs.items():
    protos.setdefault(n, len(s["params"])); sigs.setdefault(n, {"ret": s["ret"], "params": s["params"]})
g._ADDR_SIGS.clear(); g._ADDR_SIGS.update(sigs); g._ADDR_KNOWN.clear(); g._ADDR_KNOWN.update(known)
out = W / "gen"
conf = {}
for name in sorted(matched):
    src = ROOT / "src" / f"{name}.c"
    if not src.exists(): continue
    txt = src.read_text(errors="replace")
    plan = g.analyze(name, txt, matched[name], known, shim, protos, sigs)
    if not plan["eligible"]: continue
    others = [f for f in defs.get(name, ()) if not f.startswith("pc_port/game/decomp/")]
    if not others: continue
    override = [c for c in plan["callees"] if c in dsigs and c != name and not g.same_host_decl(g.strip_comments(txt), c, dsigs[c])]
    plan["host_protos"] = [dsigs[c]["decl"] for c in override]
    plan["host_override"] = override; plan["derived_all"] = sorted(dsigs)
    (out / f"{name}_port.c").write_text(g.render(name, plan, matched, known, protos))
    conf[name] = {"hand": sorted(others), "wrap": bool(plan.get("wrap")), "words": matched[name]["words"],
                  "callees": plan["callees"], "boundaries": plan["boundaries"]}
(W / "conflicts.json").write_text(json.dumps(conf, indent=1, sort_keys=True))
print(len(conf), "conflicts")

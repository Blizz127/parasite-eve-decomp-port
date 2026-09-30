#!/usr/bin/env python3
"""
Render a standalone Parasite Eve progress dashboard from progress.json.

Usage:
    python3 tools/progress/render_dashboard.py [progress.json] [-o out.html]

Defaults to build/progress.json → build/progress.html.
Also suitable for publishing index.html to parasite-eve-progress gh-pages.
"""

from __future__ import annotations

import argparse
import datetime
import json
import os
import sys

DEFAULT_INPUT = "build/progress.json"
DEFAULT_OUTPUT = "build/progress.html"

HTML_TEMPLATE = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<title>Parasite Eve Decompilation Progress</title>
<style>
  :root {
    --bg: #0d1117; --panel: #161b22; --border: #30363d; --text: #e6edf3;
    --muted: #8b949e; --accent: #3fb950; --accent2: #58a6ff; --track: #21262d;
  }
  * { box-sizing: border-box; }
  body { margin: 0; background: var(--bg); color: var(--text);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; }
  .wrap { max-width: 1100px; margin: 0 auto; padding: 32px 20px 60px; }
  h1 { font-size: 24px; margin: 0 0 4px; }
  .sub { color: var(--muted); font-size: 13px; margin-bottom: 28px; line-height: 1.5; }
  .cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px,1fr)); gap: 16px; margin-bottom: 32px; }
  .card { background: var(--panel); border: 1px solid var(--border); border-radius: 10px; padding: 18px 20px; }
  .card .label { color: var(--muted); font-size: 12px; text-transform: uppercase; letter-spacing: .05em; }
  .card .big { font-size: 30px; font-weight: 700; margin: 6px 0 2px; }
  .card .note { color: var(--muted); font-size: 12px; }
  .bar { height: 10px; background: var(--track); border-radius: 6px; overflow: hidden; margin-top: 10px; }
  .bar > span { display:block; height:100%; background: linear-gradient(90deg,var(--accent2),var(--accent)); }
  h2 { font-size: 16px; margin: 28px 0 14px; border-bottom: 1px solid var(--border); padding-bottom: 8px; }
  .cat { background: var(--panel); border: 1px solid var(--border); border-radius: 10px; padding: 14px 18px; margin-bottom: 12px; }
  .cat .top { display:flex; justify-content: space-between; align-items:baseline; }
  .cat .name { font-weight: 600; }
  .cat .pct { font-variant-numeric: tabular-nums; }
  .cat .meta { color: var(--muted); font-size: 12px; margin-top: 4px; }
  table { width: 100%; border-collapse: collapse; font-size: 13px; }
  th, td { text-align: left; padding: 8px 10px; border-bottom: 1px solid var(--border); }
  th { color: var(--muted); font-weight: 600; cursor: pointer; user-select: none; position: sticky; top: 0; background: var(--bg); }
  td.num, th.num { text-align: right; font-variant-numeric: tabular-nums; }
  .minibar { width: 120px; height: 8px; background: var(--track); border-radius: 5px; overflow:hidden; display:inline-block; vertical-align: middle; margin-left: 8px; }
  .minibar > span { display:block; height:100%; background: linear-gradient(90deg,var(--accent2),var(--accent)); }
  input[type=search] { background: var(--panel); border:1px solid var(--border); color: var(--text);
    border-radius: 6px; padding: 7px 10px; width: 260px; margin-bottom: 12px; }
  .tablewrap { max-height: 520px; overflow:auto; border:1px solid var(--border); border-radius:10px; }
  .links { margin-top: 8px; font-size: 12px; }
  .links a { color: var(--accent2); text-decoration: none; }
  .links a:hover { text-decoration: underline; }
</style>
</head>
<body>
<div class="wrap">
  <h1>Parasite Eve Decompilation &mdash; Progress</h1>
  <div class="sub" id="sub"></div>
  <div class="links">
    <a href="https://github.com/Blizz127/Parasite-Eve-Decompilation">Repository</a>
    &middot;
    <a href="https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki">Wiki</a>
  </div>
  <div class="cards" id="cards"></div>
  <h2>Categories</h2>
  <div id="cats"></div>
  <h2>Translation units</h2>
  <input type="search" id="filter" placeholder="Filter units&hellip;"/>
  <div class="tablewrap">
    <table id="units">
      <thead><tr>
        <th data-k="name">Unit</th>
        <th data-k="cat">Category</th>
        <th class="num" data-k="funcs">Funcs</th>
        <th class="num" data-k="code">Code (bytes)</th>
        <th class="num" data-k="pct">Matched %</th>
      </tr></thead>
      <tbody></tbody>
    </table>
  </div>
</div>
<script id="data" type="application/json">__DATA__</script>
<script>
const D = JSON.parse(document.getElementById('data').textContent);
const pct = x => (x == null ? 0 : x).toFixed(1) + '%';
const m = D.measures || {};
const meta = D.metadata || {};
const note = meta.note || '';
document.getElementById('sub').textContent =
  `Generated ${D.generated} \\u00b7 SLUS-006.62 \\u00b7 ${m.matched_functions||0}/${m.total_functions||0} functions \\u00b7 ${(+m.total_units||0)} units` +
  (note ? ` \\u00b7 ${note}` : '');

function card(label, big, note, frac) {
  return `<div class="card"><div class="label">${label}</div><div class="big">${big}</div>`+
    `<div class="note">${note||''}</div>`+
    (frac!=null?`<div class="bar"><span style="width:${Math.max(0,Math.min(100,frac)).toFixed(2)}%"></span></div>`:'')+
    `</div>`;
}
document.getElementById('cards').innerHTML =
  card('Code matched', pct(m.matched_code_percent), `${(+m.matched_code||0).toLocaleString()} / ${(+m.total_code||0).toLocaleString()} bytes`, m.matched_code_percent) +
  card('Functions matched', `${m.matched_functions||0}`, `of ${m.total_functions||0} (${pct(m.matched_functions_percent)})`, m.matched_functions_percent) +
  card('Exact leaf match', pct(m.fuzzy_match_percent), 'byte-verified C leaves (objdiff N/A)', m.fuzzy_match_percent) +
  card('Units', `${m.total_units||0}`, 'C leaves + asm subsegments', null);

document.getElementById('cats').innerHTML = (D.categories||[]).map(c => {
  const cm = c.measures||{};
  return `<div class="cat"><div class="top"><span class="name">${c.name}</span>`+
    `<span class="pct">${pct(cm.matched_code_percent)}</span></div>`+
    `<div class="bar"><span style="width:${(cm.matched_code_percent||0).toFixed(2)}%"></span></div>`+
    `<div class="meta">${cm.matched_functions||0}/${cm.total_functions||0} functions \\u00b7 `+
    `${(+cm.matched_code||0).toLocaleString()}/${(+cm.total_code||0).toLocaleString()} bytes</div></div>`;
}).join('');

const units = (D.units||[]).map(u => {
  const um = u.measures||{};
  return {
    name: u.name,
    cat: ((u.metadata||{}).progress_categories||[]).join(', '),
    funcs: +um.total_functions||0,
    code: +um.total_code||0,
    pct: um.matched_code_percent==null ? 0 : um.matched_code_percent
  };
});
let sortK='pct', sortDir=-1;
const tbody = document.querySelector('#units tbody');
function render() {
  const f = document.getElementById('filter').value.toLowerCase();
  const rows = units.filter(u => u.name.toLowerCase().includes(f))
    .sort((a,b)=>{const x=a[sortK],y=b[sortK];return (x<y?-1:x>y?1:0)*sortDir;});
  tbody.innerHTML = rows.map(u =>
    `<tr><td>${u.name}</td><td>${u.cat}</td><td class="num">${u.funcs}</td>`+
    `<td class="num">${u.code.toLocaleString()}</td>`+
    `<td class="num">${u.pct.toFixed(1)}%<span class="minibar"><span style="width:${u.pct.toFixed(1)}%"></span></span></td></tr>`
  ).join('');
}
document.querySelectorAll('#units th').forEach(th => th.onclick = () => {
  const k = th.dataset.k; if (k===sortK) sortDir*=-1; else {sortK=k; sortDir=(k==='name'||k==='cat')?1:-1;} render();
});
document.getElementById('filter').oninput = render;
render();
</script>
</body>
</html>
"""


def find_repo_root() -> str:
    path = os.path.abspath(os.path.dirname(__file__))
    while path != "/":
        if os.path.isfile(os.path.join(path, "CLAUDE.md")):
            return path
        path = os.path.dirname(path)
    return os.getcwd()


def main() -> None:
    repo = find_repo_root()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", default=os.path.join(repo, DEFAULT_INPUT))
    parser.add_argument("-o", "--output", default=os.path.join(repo, DEFAULT_OUTPUT))
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(
            f"ERROR: {args.input} not found. "
            "Run tools/progress/generate_progress.py first.",
            file=sys.stderr,
        )
        sys.exit(1)

    with open(args.input, encoding="utf-8") as fh:
        data = json.load(fh)

    data["generated"] = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
    html = HTML_TEMPLATE.replace("__DATA__", json.dumps(data))

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, "w", encoding="utf-8") as fh:
        fh.write(html)

    m = data.get("measures", {})
    print(f"Wrote {args.output}")
    print(
        f"  code: {float(m.get('matched_code_percent', 0)):.2f}%  "
        f"functions: {m.get('matched_functions', 0)}/{m.get('total_functions', 0)}"
    )


if __name__ == "__main__":
    main()

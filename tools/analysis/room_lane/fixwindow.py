#!/usr/bin/env python3
"""fixwindow.py <target>: after a split, assemble the first text asm file; while `as` rejects a
word (header data disassembled as code, e.g. `tge`), move the text start just past the last
rejected word (config row, header comment and manifest text_start). Prints the new start or 'ok'.
Also drops profile assignments that name no YAML C span (ports that failed the link check)."""
import json, os, re, subprocess, sys
t = sys.argv[1]
yp = f'configs/USA/overlays/{t}.yaml'; pp = f'configs/USA/overlays/{t}_build_profiles.json'
y = open(yp).read()
rows = re.findall(r'- \[(0x[0-9A-Fa-f]+), (\w+)(?:, (\w+))?\]', y)
cs = {n for o, k, n in rows if k == 'c'}
j = json.load(open(pp)); changed = False
for pn in list(j.get('assignments', {})):
    keep = [f for f in j['assignments'][pn] if f in cs]
    if keep != j['assignments'][pn]:
        changed = True
        if keep: j['assignments'][pn] = keep
        else: del j['assignments'][pn]
for pn in list(j.get('local_profiles', {})):
    if pn not in j.get('assignments', {}):
        del j['local_profiles'][pn]; changed = True
if changed:
    open(pp, 'w').write(json.dumps(j, indent=2) + '\n'); print('pruned stale assignments')
first = next(o for o, k, n in rows if k in ('asm', 'c'))
asm = f'asm/overlays/{t}/{int(first,16):X}.s'
if not os.path.exists(asm): print('ok (first text span is C)'); sys.exit(0)
env = dict(os.environ, LD_LIBRARY_PATH='tools/mipsel-host/usr/lib/x86_64-linux-gnu')
r = subprocess.run(['tools/mipsel-host/bin/mipsel-linux-gnu-as', '-EL', '-mips1', '-mabi=32', '-I', 'include', '-o', '/dev/null', asm], capture_output=True, text=True, env=env)
if r.returncode == 0: print('ok'); sys.exit(0)
lines = open(asm).read().split('\n'); bad = []
for m in re.finditer(r':(\d+): Error', r.stderr):
    mm = re.search(r'/\* ([0-9A-F]+) ', lines[int(m.group(1)) - 1])
    if mm: bad.append(int(mm.group(1), 16))
if not bad: print('as failed, no offset:', r.stderr[:300]); sys.exit(1)
new = max(bad) + 4
if any(k == 'c' and int(o, 16) < new for o, k, n in rows):
    print('refuse: a C span lies before', hex(new)); sys.exit(1)
y = y.replace(f'- [{first}, asm]', f'- [0x{new:X}, asm]', 1)
open(yp, 'w').write(y)
m = open('configs/USA/overlays/manifest.yaml').read()
i = m.index(f'  {t}:'); k = m.index('text_start: ', i); e = m.index('\n', k)
open('configs/USA/overlays/manifest.yaml', 'w').write(m[:k] + f'text_start: 0x{new:X}' + m[e:])
print('text_start', hex(int(first, 16)), '->', hex(new))

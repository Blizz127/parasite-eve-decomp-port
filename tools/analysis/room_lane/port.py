#!/usr/bin/env python3
"""port.py <dstroom> : for every body of <dstroom> whose masked hash equals a C leaf of the
source room targets (default room_m0022i), write src/overlays/room_<dst>/func_<VRAM>.c with the
C re-targeted: own name renamed, every func_/D_ token remapped by comparing the jal/j targets and
lui/%lo addresses of the two bodies word by word. Prints '<func> <off> <size> <srcfunc>' per port;
refuses (prints SKIP) when a room-range token has no mapping."""
import sys, re, os, struct
ARGS = sys.argv[1:]
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from portlib import derive_field_base
DRY = os.environ.get('ROOM_LANE_DRY') == '1'
SRC_ROOT = os.path.join(os.environ.get('ROOM_LANE_WORK', 'build/room_lane'), 'dry') if DRY else '.'
sys.argv = [sys.argv[0], 'match'] + sys.argv[1:2]  # reuse loader
exec(open(os.path.join(os.path.dirname(__file__), 'roomscan.py')).read().split("cmd = sys.argv[1]")[0])
dst = ARGS[0]
srcs = ARGS[1:] or (['room_m0022i'] + sorted(os.path.basename(y)[:-5] for y in __import__('glob').glob('configs/USA/overlays/room_m*.yaml') if not y.endswith(f'room_{dst}.yaml') and 'm0022i' not in y))
def addrs(off, ws):
    res = []; lui = {}
    for i, w in enumerate(ws):
        pc = VB + off + 4*i
        op = w >> 26; rs = (w >> 21) & 31; rt = (w >> 16) & 31
        if op in (2, 3):
            res.append((i, ((pc + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2))); continue
        if op == 0x0F: lui[rt] = (w & 0xFFFF) << 16; continue
        if op in (0x08, 0x09, 0x0D, 0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B, 0x22, 0x26, 0x2A, 0x2E, 0x32, 0x3A) and rs in lui:
            lo = w & 0xFFFF
            a = (lui[rs] | lo) if op == 0x0D else (lui[rs] + (lo - 0x10000 if lo & 0x8000 else lo)) & 0xFFFFFFFF
            res.append((i, a))
            if rt != rs and op < 0x28: lui.pop(rt, None)
        elif op == 0 and (w & 0x3F) in (0x21, 0x23, 0x25, 0x24, 0x20):
            rd = (w >> 11) & 31
            if not ((w & 0x3F) in (0x21, 0x20) and rd in lui and rd in (rs, rt)):
                lui.pop(rd, None)
    return res
import json
def prof_env(t, fn):
    j = json.load(open(f'configs/USA/overlays/{t}_build_profiles.json'))
    for pn, fns in j.get('assignments', {}).items():
        if fn in fns:
            return pn, j.get('local_profiles', {}).get(pn)
    return None, None
dst_prof_path = f'configs/USA/overlays/room_{dst}_build_profiles.json'
dst_yaml_path = f'configs/USA/overlays/room_{dst}.yaml'
known = {}
for t in srcs:
    rn = t.replace('room_', '')
    bmap = dict(bodies(rn))
    for off, fn in c_leaves(t):
        if off in bmap: known.setdefault(hsh(bmap[off]), (t, fn, off, bmap[off]))
already = {fn for o, fn in c_leaves('room_' + dst)} if os.path.exists(f'configs/USA/overlays/room_{dst}.yaml') else set()
os.makedirs(os.path.join(SRC_ROOT, f'src/overlays/room_{dst}'), exist_ok=True)
for off, ws in bodies(dst):
    k = known.get(hsh(ws))
    if not k: continue
    t, fn, soff, sws = k
    new = f'func_{VB+off:08X}'
    if new in already: continue
    a0 = addrs(soff, sws); a1 = addrs(off, ws)
    m = {}
    ok = True
    for (i, x), (j, y) in zip(a0, a1):
        if i != j or (x in m and m[x] != y): ok = False
        m[x] = y
    src = open(f'src/overlays/{t}/{fn}.c').read()
    body = src[src.index('*/') + 2:].lstrip('\n')
    def sub(mt):
        name = mt.group(0); pre, hx = name.split('_'); a = int(hx, 16)
        if name == fn: return new
        if a in m: return f'{pre}_{m[a]:08X}'
        if VB <= a < 0x80200000:
            # A struct-field reference (&D + off) maps the field address; derive
            # the base from every mapped address within 0x100 above it, and
            # require they all imply the same delta.
            b = derive_field_base(m, a)
            if b is not None:
                return f'{pre}_{b:08X}'
            raise KeyError(name)
        return name
    try:
        body = re.sub(r'\b(?:func|D)_[0-9A-F]{8}\b', sub, body)
    except KeyError as e:
        print('SKIP', new, 'unmapped', e); continue
    if not ok: print('SKIP', new, 'address pattern differs'); continue
    pn, lp = prof_env(t, fn)
    envs = '-'
    pname = 'era_o2_g0 (default)'
    if pn is not None:
        if lp is None or set(lp.get('environment', {})) - {'MASPSX_THREE_WORD_SYMBOL_STORE', 'MASPSX_DISPATCH_FOLD'} or lp.get('flags') != ['-O2', '-G0']:
            print('SKIP', new, 'unsupported profile', pn); continue
        tabs = []
        for tb in lp['environment']['MASPSX_DISPATCH_FOLD'].split(','):
            a = int(tb[5:], 16)
            if a not in m: raise SystemExit(f'no jtbl mapping for {tb} in {new}')
            tabs.append(f'jtbl_{m[a]:08X}')
        npn = f'room_{dst}_dispatch_' + '_'.join(x[5:] for x in tabs)
        j = json.load(open(dst_prof_path))
        j.setdefault('local_profiles', {})[npn] = {'toolchain': 'era', 'flags': ['-O2', '-G0'], 'environment': {'MASPSX_THREE_WORD_SYMBOL_STORE': '1', 'MASPSX_DISPATCH_FOLD': ','.join(tabs)}}
        lst = j.setdefault('assignments', {}).setdefault(npn, [])
        if new not in lst: lst.append(new)
        if not DRY: open(dst_prof_path, 'w').write(json.dumps(j, indent=2) + '\n')
        y = open(dst_yaml_path).read()
        if ', rodata]' not in y:
            lo = min(int(x[5:], 16) for x in tabs) - VB
            y = y.replace('      - [0x0, bin]\n', f'      - [0x0, bin]\n      # header switch jump tables: MASPSX_DISPATCH_FOLD pool\n      - [0x{lo:X}, rodata]\n', 1)
            if not DRY: open(dst_yaml_path, 'w').write(y)
        else:
            mm = re.search(r'- \[(0x[0-9A-Fa-f]+), rodata\]', y)
            lo = min(int(x[5:], 16) for x in tabs) - VB
            if lo < int(mm.group(1), 16):
                y = y.replace(mm.group(0), f'- [0x{lo:X}, rodata]')
                if not DRY: open(dst_yaml_path, 'w').write(y)
        envs = 'MASPSX_THREE_WORD_SYMBOL_STORE=1 MASPSX_DISPATCH_FOLD=' + ','.join(tabs)
        pname = npn + ' (local: -O2 -G0 + dispatch fold)'
    hdr = (f'/* room_{dst} (PE.IMG room {dst} chunk 2, VRAM 0x8018EFE8)\n'
           f' * {new} — blob offset {off:#x}, {len(ws)*4:#x} bytes. Profile {pname}.\n'
           f' * Masked-body twin of {t} {fn}; C re-targeted by symbol address\n'
           f' * (docs/evidence/room_{dst}-ports-2026-09-23/REPORT.md). */\n\n')
    open(os.path.join(SRC_ROOT, f'src/overlays/room_{dst}/{new}.c'), 'w').write(hdr + body)
    print(new, hex(off), hex(len(ws)*4), fn, envs)

#!/usr/bin/env python3
"""roomscan.py: room text windows, text groups, and masked-body matches vs matched room leaves.
  roomscan.py window <room>             -> text_start/text_end, #jr, #prologues, group members
  roomscan.py match <room> [<srcroom>...] -> bodies of <room> whose masked hash equals a C leaf of srcroom targets
"""
import sys, os, struct, hashlib, collections, re, glob
sys.path.insert(0, 'tools/extract')
import peimg
root = peimg.REPO_ROOT
man = peimg.load_manifest(root, peimg.DEFAULT_MANIFEST)
exe = peimg.read_exe(root, man)
rooms = peimg.room_table(exe, man)
img = peimg.open_image(root, man, hash_image=False)
ilba = int(man['image']['disc_lba'])
VB = 0x8018EFE8; JR = 0x03E00008
def is_ptr(w): return 0x8018EFE8 <= w < 0x80200000
chunks = {}
with open(img, 'rb') as f:
    for r in rooms:
        if r['sec'][2] == 0: continue
        start = r['lba'] + r['sec'][0] + r['sec'][1] - ilba
        f.seek(start * 2048); chunks[r['name']] = f.read(r['sec'][2] * 2048)
def words(d): return struct.unpack('<%dI' % (len(d)//4), d[:len(d)//4*4])
def mask(ws):
    out = []; lui = set()
    for w in ws:
        op = w >> 26; rs = (w >> 21) & 31; rt = (w >> 16) & 31
        if op in (2, 3): out.append(w & 0xFC000000); continue
        if op == 0x0F: out.append(w & 0xFFFF0000); lui.add(rt); continue
        if op in (0x08, 0x09, 0x0D, 0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B, 0x22, 0x26, 0x2A, 0x2E, 0x32, 0x3A) and rs in lui:
            out.append(w & 0xFFFF0000)
            if op in (0x08,0x09,0x0D) or op < 0x28: lui.discard(rt) if rt != rs else None
            continue
        out.append(w)
    return tuple(out)
def window(name):
    ws = words(chunks[name])
    jr = [i for i, w in enumerate(ws) if w == JR]
    pro = [i for i, w in enumerate(ws) if (w & 0xFFFF8000) == 0x27BD8000]
    if not jr or not pro: return None
    i = jr[0]
    while i > 0 and not is_ptr(ws[i-1]) and ws[i-1] != 0xFFFFFFFF: i -= 1
    if i == 0: i = pro[0]
    return i*4, (jr[-1]+2)*4
def bodies(name):
    ws = words(chunks[name]); a, b = window(name)
    s = a//4; out = []
    for j in range(a//4, b//4):
        if ws[j] == JR:
            e = j + 2
            while s < e and ws[s] == 0: s += 1
            if s < e: out.append((s*4, list(ws[s:e])))
            s = e
    return out
def hsh(ws): return hashlib.sha1(struct.pack('<%dI' % len(ws), *mask(ws))).hexdigest()
def c_leaves(target):
    y = open(f'configs/USA/overlays/{target}.yaml').read()
    return [(int(m.group(1), 16), m.group(2)) for m in re.finditer(r'- \[(0x[0-9A-Fa-f]+), c, (func_[0-9A-F]+)\]', y)]
cmd = sys.argv[1]
if cmd == 'window':
    n = sys.argv[2]; a, b = window(n)
    tx = chunks[n][a:b]
    grp = sorted(k for k in chunks if window(k) == (a, b) and chunks[k][a:b] == tx) if window(n) else []
    ws = words(chunks[n])
    print(n, 'text', hex(a), hex(b), 'size', hex(len(chunks[n])), 'jr', sum(1 for w in ws[a//4:b//4] if w == JR),
          'prologues', sum(1 for w in ws[a//4:b//4] if (w & 0xFFFF8000) == 0x27BD8000), 'group', len(grp), grp)
elif cmd == 'match':
    n = sys.argv[2]; srcs = sys.argv[3:] or ['room_m0022i']
    known = {}
    for t in srcs:
        rn = t.replace('room_', '')
        bmap = dict(bodies(rn))
        for off, fn in c_leaves(t):
            if off in bmap: known.setdefault(hsh(bmap[off]), (t, fn, off))
    tot = 0
    for off, ws in bodies(n):
        k = known.get(hsh(ws))
        if k:
            tot += 1
            print(f'{hex(off)} func_{VB+off:08X} {len(ws)*4:#x} <= {k[0]} {k[1]}')
    print('matched', tot, 'of', len(bodies(n)), file=sys.stderr)
elif cmd == 'groups':
    tx = collections.defaultdict(list)
    for k in chunks:
        w = window(k)
        if not w: continue
        tx[(w, hashlib.sha1(chunks[k][w[0]:w[1]]).hexdigest())].append(k)
    for (w, h), v in sorted(tx.items(), key=lambda kv: (-len(kv[1]), sorted(kv[1])[0])):
        v = sorted(v); print(v[0], hex(w[0]), hex(w[1]), len(v), ' '.join(v[1:]))
elif cmd == 'top':
    leaders = [l.split()[0] for l in open(os.path.join(os.environ.get('ROOM_LANE_WORK', 'build/room_lane'), 'groups.txt'))]
    srcs = sys.argv[2:]
    known = set()
    for t in srcs:
        bmap = dict(bodies(t.replace('room_', '')))
        for off, fn in c_leaves(t):
            if off in bmap: known.add(hsh(bmap[off]))
    cnt = collections.defaultdict(list)
    for n in leaders:
        for off, ws in bodies(n):
            h = hsh(ws)
            if h in known: continue
            if not cnt[h] or cnt[h][-1][0] != n: cnt[h].append((n, off, len(ws)))
    rows = sorted(cnt.items(), key=lambda kv: (-len(kv[1]), kv[1][0][2]))
    for h, v in rows[:120]:
        n, off, L = v[0]
        print(len(v), f'{L*4:#x}', n, hex(off), f'func_{VB+off:08X}')

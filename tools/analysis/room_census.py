#!/usr/bin/env python3
"""Room chunk-2 census for the PE.IMG room tier (read-only, stdlib only).

Reads the 438-slot room package table from the extracted EXE and every room's
chunk 2 straight out of the extracted PE.IMG (nothing is written), then
reports: code rooms, framed prologues, `jr $ra`-delimited bodies (raw,
jal/lui-masked, and fully masked = jal/j targets + lui immediates + the
%lo immediate of a load/store/addiu whose base came from a lui), fan-in of
masked bodies, whether shared bodies sit at one offset, identical text
windows, and the table-order rooms that introduce new bodies.

Text window per room: from just after the last absolute pointer
(0x8018EFE8..0x80200000) or 0xFFFFFFFF word preceding the first `jr $ra`
(i.e. after the header jump tables), falling back to the first framed
prologue when the header holds no pointers; ends after the last `jr $ra` +
delay slot. Usage: python3 tools/analysis/room_census.py
"""
import sys, struct, hashlib, collections, json
sys.path.insert(0, 'tools/extract')
from pathlib import Path
import peimg
root = peimg.REPO_ROOT
man = peimg.load_manifest(root, peimg.DEFAULT_MANIFEST)
exe = peimg.read_exe(root, man)
rooms = peimg.room_table(exe, man)
img = peimg.open_image(root, man, hash_image=False)
ilba = int(man['image']['disc_lba'])
VB = 0x8018EFE8
JR = 0x03E00008
def is_ptr(w): return 0x8018EFE8 <= w < 0x80200000
chunks = {}
with open(img, 'rb') as f:
    for r in rooms:
        if r['sec'][2] == 0: continue
        start = r['lba'] + r['sec'][0] + r['sec'][1] - ilba
        f.seek(start * 2048); chunks[r['name']] = f.read(r['sec'][2] * 2048)
print('slots', len(rooms), 'with chunk2', len(chunks))
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
code = {}; funcs = []; nframed = {}  # (room, off, words)
for name, d in chunks.items():
    ws = words(d)
    jr = [i for i, w in enumerate(ws) if w == JR]
    if not jr: continue
    # text start: scan back from first jr to just after the last pointer/zero-run header word
    pro = [i for i, w in enumerate(ws) if (w & 0xFFFF8000) == 0x27BD8000]
    if not pro: continue
    i = jr[0]
    while i > 0 and not is_ptr(ws[i-1]) and ws[i-1] != 0xFFFFFFFF: i -= 1
    if i == 0:
        i = pro[0]
    tstart = i; tend = jr[-1] + 2
    nframed[name] = len([x for x in pro if x < tend])
    code[name] = (tstart*4, tend*4, len(d))
    s = tstart
    for j in jr:
        e = j + 2
        while s < e and ws[s] == 0: s += 1
        if s < e: funcs.append((name, s*4, ws[s:e]))
        s = e
print('code rooms', len(code), 'framed prologues', sum(nframed.values()))
raw_words = sum(len(f[2]) for f in funcs)
um = collections.defaultdict(set); mk = collections.defaultdict(set); mkw = {}; umw = {}
for n, o, ws in funcs:
    h = hashlib.sha1(struct.pack('<%dI' % len(ws), *ws)).hexdigest(); um[h].add((n, o)); umw[h] = len(ws)
    m = hashlib.sha1(struct.pack('<%dI' % len(ws), *mask(ws))).hexdigest(); mk[m].add((n, o)); mkw[m] = len(ws)
print('functions', len(funcs), 'words', raw_words)
print('unique unmasked', len(um), 'words', sum(umw.values()))
mk2=collections.defaultdict(set);mk2w={}
for n,o,ws in funcs:
    m=hashlib.sha1(struct.pack('<%dI'%len(ws),*[(w&0xFC000000) if (w>>26) in (2,3) else (w&0xFFFF0000) if (w>>26)==0xF else w for w in ws])).hexdigest(); mk2[m].add((n,o)); mk2w[m]=len(ws)
print('unique jal/lui-masked', len(mk2), 'words', sum(mk2w.values()))
print('unique masked', len(mk), 'words', sum(mkw.values()))
def rooms_of(s): return len({n for n, o in s})
fan = collections.Counter()
for m, s in mk.items():
    k = rooms_of(s); fan['>=10' if k >= 10 else '2-9' if k >= 2 else '1'] += 1
print('masked fan-in', dict(fan), 'words', {b: sum(mkw[m] for m, s in mk.items() if (rooms_of(s) >= 10) == (b == '>=10') and (2 <= rooms_of(s) < 10) == (b == '2-9')) for b in ('>=10', '2-9', '1')})
# same VRAM across rooms?
same = sum(1 for m, s in mk.items() if rooms_of(s) >= 2 and len({o for n, o in s}) == 1)
multi = sum(1 for m, s in mk.items() if rooms_of(s) >= 2)
print('shared masked bodies', multi, 'at one offset in every room', same)
seen = {}
for n, o, ws in funcs:
    seen[(n, o)] = hashlib.sha1(struct.pack('<%dI' % len(ws), *ws)).hexdigest()
ident = sum(1 for m, s in mk.items() if rooms_of(s) >= 2 and len({seen[x] for x in s}) == 1)
print('shared masked bodies byte-identical everywhere', ident)
# whole-chunk duplicates
ch = collections.defaultdict(list)
for n in code: ch[hashlib.sha1(chunks[n]).hexdigest()].append(n)
print('code rooms with a whole-chunk twin', sum(len(v) for v in ch.values() if len(v) > 1), 'groups', sum(1 for v in ch.values() if len(v) > 1))
tx = collections.defaultdict(list)
for n, (a, b, L) in code.items(): tx[hashlib.sha1(chunks[n][a:b]).hexdigest()].append(n)
print('code rooms sharing an identical text window', sum(len(v) for v in tx.values() if len(v) > 1), 'distinct texts', len(tx))
# rooms needed to cover all unique masked bodies (greedy table order)
cov = set(); need = 0
for n in sorted(code):
    new = {m for m, s in mk.items() if any(x[0] == n for x in s)} - cov
    if new: need += 1; cov |= new
print('rooms in table order that introduce new bodies', need)
# fan-in per room
best = sorted(code, key=lambda n: -sum(1 for m, s in mk.items() if rooms_of(s) >= 10 and any(x[0] == n for x in s)))[:5]
print('rooms with most >=10-fan-in bodies', [(n, sum(1 for m, s in mk.items() if rooms_of(s) >= 10 and any(x[0] == n for x in s))) for n in best])
starts = collections.Counter(a for a, b, L in code.values())
print('text_start histogram top', starts.most_common(8))
g = sorted(tx.values(), key=len, reverse=True)
print('text groups (size:first room):', [(len(v), sorted(v)[0], hex(code[sorted(v)[0]][0]), hex(code[sorted(v)[0]][1])) for v in g[:12]])
print('distinct text words', sum((code[v[0]][1]-code[v[0]][0])//4 for v in g))
print('m0418i group', [len(v) for v in g if 'm0418i' in v], [sorted(v)[:6] for v in g if 'm0418i' in v])
# are identical-text rooms at identical start offsets?
print('groups with differing text_start', sum(1 for v in g if len({code[n][0] for n in v}) > 1))

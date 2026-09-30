#!/usr/bin/env python3
"""newroom.py <room> <slot> <sec_start> <sec_end> <size> <sha1> <text_start> <text_end> : write config + profiles + manifest entry."""
import sys, re
r, slot, s0, s1, size, sha, ts, te = sys.argv[1:]
t = f'room_{r}'
tpl = open('configs/USA/overlays/room_m0022i.yaml').read()
head = f'''# Splat config — Parasite Eve (USA) Disc 1, PE.IMG room package {r}, chunk 2.
#
# Blob: `python3 tools/extract/peimg.py room {r}` ->
# build/extracted/disc1/peimg/{t}_c2.bin (git-ignored, NEVER committed).
# Split `scripts/split_overlay.sh {t}`; gate `scripts/exact_rebuild_overlay.sh {t}`.
#
# Facts (peimg.py room {r}, 2026-09-23): slot {slot}, PE.IMG sectors
# [{s0},{s1}), size {size}, vram 0x8018EFE8, SHA-1 {sha}.
# Geometry (room_census text-window rule, tools/analysis/room_lane/roomscan.py):
#   0x0-{ts}  header + switch jump tables (bin); {ts}-{te} .text;
#   {te}-{size} behaviour tables + room data (bin).
'''
body = tpl[tpl.index('name: Parasite Eve'):tpl.index('    subsegments:')]
body = body.replace('m0022i', r).replace('3231b287bdde6fb55bcc3734f5be6ff4906ab0c3', sha)
open(f'configs/USA/overlays/{t}.yaml', 'w').write(head + body + f'''    subsegments:
      - [0x0, bin]
      - [{ts}, asm]
      - [{te}, bin]
  - [{size}]
''')
open(f'configs/USA/overlays/{t}_build_profiles.json', 'w').write('''{
  "schema_version": 1,
  "_comment": "Build profiles for the %s room target. Profile DEFINITIONS are inherited from the EXE manifest via profiles_from; only the ASSIGNMENTS are owned here.",
  "profiles_from": "configs/USA/disc1_build_profiles.json",
  "default_profile": "era_o2_g0",
  "assignments": {}
}
''' % t)
m = open('configs/USA/overlays/manifest.yaml').read()
if f'  {t}:' not in m:
    ent = f'''  {t}:
    kind: room
    description: room {r} chunk 2 (room tier target)
    room_slot: {slot}
    sector_start: {s0}
    sector_end: {s1}
    size: {size}
    vram: 0x8018EFE8
    sha1: {sha}
    blob: build/extracted/disc1/peimg/{t}_c2.bin
    text_start: {ts}
    text_end: {te}
    config: configs/USA/overlays/{t}.yaml
    build_profiles: configs/USA/overlays/{t}_build_profiles.json
'''
    i = m.index('  room_m0022i:')
    j = m.index('build_profiles: configs/USA/overlays/room_m0022i_build_profiles.json', i)
    j = m.index('\n', j) + 1
    m = m[:j] + ent + m[j:]
    open('configs/USA/overlays/manifest.yaml', 'w').write(m)
print('ok', t)

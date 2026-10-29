#!/usr/bin/env python3
"""Battle-style census: every value func_800339A0 (battle style row) can get
from the room scripts on the user's Disc 1 (PE.IMG is byte-identical on Disc 2).

The style reaches func_800339A0 from field-VM op 0x55 (battle entry,
func_800144FC -> func_80029810(**a0)) and op 0xED key 0xAF1
(func_80016910); its other callers pass 0 or D_8009D02C, which only holds 0 or
the current style.  Read-only over the user's disc: prints each op's operand
values and argument modes, never bytes.

usage: style_census.py <repo root> <Disc 1 .bin>"""
import sys, collections
from pathlib import Path
ROOT = Path(sys.argv[1])
sys.path.insert(0, str(ROOT / "tools/research"))
sys.path.insert(0, str(ROOT / "pc_port/tools"))
from pe_pst0_scan import parse_field_table, extract_script_from_package, decode_script  # noqa
from pe_btl14_m0005i_publish_oracle import read_form1  # noqa
exe = (ROOT / "build/extracted/disc1/SLUS_006.62").read_bytes()
disc = Path(sys.argv[2])
PE_IMG_LBA = 1013
rows = parse_field_table(exe)
print("field table rows:", len(rows))
op55 = []
seen_rooms = 0; ed = 0; hits = []; modes0 = collections.Counter(); keys = collections.Counter()
for rec in rows:
    c0 = rec["meta"] & 0xFF; c1 = (rec["meta"] >> 8) & 0xFFF; c2 = rec["meta"] >> 20
    if c0 + c1 + c2 == 0:
        continue
    try:
        package = read_form1(disc, PE_IMG_LBA + rec["start"], c0 + c1 + c2)
    except Exception as exc:
        continue
    ext = extract_script_from_package(package, rec["meta"])
    if ext is None:
        continue
    seen_rooms += 1
    script = decode_script(ext[1])
    for mod in script["modules"]:
        for cmd in mod["commands"]:
            if cmd["opcode"] == 0x55 and cmd["args"]:
                op55.append((rec["name"], cmd["modes"][0], cmd["args"][0] & 0xFFFFFFFF))
            if cmd["opcode"] != 0xED or not cmd["args"]:
                continue
            ed += 1
            keys[(cmd["modes"][0], cmd["args"][0])] += 1
            if cmd["args"][0] == 0xAF1 or cmd["modes"][0] != 0:
                hits.append((rec["name"], mod["index"], cmd["offset"], cmd["modes"], [hex(a) for a in cmd["args"]]))
print("rooms with scripts:", seen_rooms, "op 0xED commands:", ed)
print("distinct (mode0,key) count:", len(keys))
print("0xAF1 / non-immediate-key hits:", len(hits))
for h in hits[:40]:
    print(" ", h)

vals = collections.Counter((m, v & 0xFF if m == 0 else v) for _, m, v in op55)
print("op 0x55 battle-entry commands:", len(op55), "distinct (mode, style):", sorted(vals.items())[:40])
print("non-immediate style operands:", [(r, m, hex(v)) for r, m, v in op55 if m != 0][:20])
print("immediate styles >= 4:", [(r, hex(v)) for r, m, v in op55 if m == 0 and (v & 0xFF) >= 4][:20])

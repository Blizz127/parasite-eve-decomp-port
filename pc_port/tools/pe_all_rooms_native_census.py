#!/usr/bin/env python3
"""All-rooms native census: every room package's code bodies vs the native registry.

EVIDENCE ONLY (read-only; writes only the --json/--markdown outputs it is given).
Runs the per-room analysis of pe_room_native_census.py over every room package
with code in chunk 2 and aggregates the result, so one run answers "which room
code would stop a native-only build if it ran".

    PE_DISC1_BIN=<disc 1 .bin> python3 pc_port/tools/pe_all_rooms_native_census.py \
        [--json out.json] [--markdown out.md] [--dispatch zz_dispatch.c] [--skips gen_verbose.txt]

Both discs carry a byte-identical PE.IMG (SHA-1 146c0ce7...) and EXE (452fb033...),
so the Disc 1 census covers Disc 2's room code too.

Statuses are the per-room tool's: native-generated / native-hand (registry entry
whose FNV-1a matches the room bytes, resolved the way PE_GuestCode_Resolve
resolves), native-direct (a host binding outside the registry), ambiguous (more
than one distinct registry match: PE_GuestCode_Resolve returns NULL, so it is a
boundary unless the overlay is declared resident), direct-addr-guarded and
not-native.  "reachable" is the static closure from the room's resource records
and script op 0x6A codes (see pe_room_native_census.py); a body that is not
reachable statically can still run (function pointers built at run time), so the
not-native total is reported both ways.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import pe_room_native_census as c  # noqa: E402


def mask_words(b: bytes) -> bytes:
    """Body bytes with relocation-dependent fields masked (tools/analysis/
    room_census.py rule): jal/j targets, lui immediates and the %lo immediate
    of a load/store/addiu whose base came from a lui.  Two bodies with equal
    masked bytes are the same code placed at different addresses."""
    import struct
    ws = struct.unpack("<%dI" % (len(b) // 4), b[:len(b) // 4 * 4])
    out, lui = [], set()
    for w in ws:
        op, rs, rt = w >> 26, (w >> 21) & 31, (w >> 16) & 31
        if op in (2, 3):
            out.append(w & 0xFC000000)
            continue
        if op == 0x0F:
            out.append(w & 0xFFFF0000)
            lui.add(rt)
            continue
        if op in (0x08, 0x09, 0x0D, 0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B,
                  0x22, 0x26, 0x2A, 0x2E, 0x32, 0x3A) and rs in lui:
            out.append(w & 0xFFFF0000)
            if (op in (0x08, 0x09, 0x0D) or op < 0x28) and rt != rs:
                lui.discard(rt)
            continue
        out.append(w)
    return struct.pack("<%dI" % len(out), *out)


def c_source_index(pkgs):
    """{(size, sha1(masked bytes)): [(room, leaf, kind)]} over every room leaf
    that has C: matched YAML `c` leaves and NON_MATCHING leaves."""
    idx: dict = {}
    nm_path = c.ROOT / "configs/USA/overlays/nonmatching.yaml"
    nonmatch = []
    if nm_path.is_file():
        for m in re.finditer(r"^\s*-\s*\[\s*(room_\w+)\s*,\s*(func_\w+)\s*,\s*(0x[0-9A-Fa-f]+)\s*,\s*(0x[0-9A-Fa-f]+)\s*\]",
                             nm_path.read_text(), re.M):
            nonmatch.append((m.group(1)[len("room_"):], m.group(2), int(m.group(3), 16), int(m.group(4), 16)))
    entries = []
    for y in sorted(c.OVL_DIR.glob("room_m*.yaml")):
        room = y.stem[len("room_"):]
        if room not in pkgs:
            continue
        _base, leaves = c.yaml_leaves(y)
        entries += [(room, n, off, size, "matched") for n, off, size in leaves]
    entries += [(r, n, off, size, "nonmatching") for r, n, off, size in nonmatch if r in pkgs]
    for room, name, off, size, kind in entries:
        blob = pkgs[room]["chunk2"]
        if off + size > len(blob):
            continue
        key = (size, hashlib.sha1(mask_words(blob[off:off + size])).hexdigest())
        idx.setdefault(key, []).append((room, name, kind))
    return idx


def dispatch_closures(path: Path) -> dict:
    """{(table, symbol, vma): closure} from the registry (0 = none)."""
    text = path.read_text(encoding="utf-8")
    out = {}
    for tm in re.finditer(r"static const PeGuestCodeEntry pe_gc_(\w+)_e\[\] = \{(.*?)\n\};", text, re.S):
        for em in re.finditer(r"\{0x([0-9A-Fa-f]+)u, 0x[0-9A-Fa-f]+u, 0x[0-9A-Fa-f]+u, \w+, \"([^\"]+)\""
                              r"(?:, 0x([0-9A-Fa-f]+)u, \w+, \d+u)?\}", tm.group(2)):
            out[(tm.group(1), em.group(2), int(em.group(1), 16))] = int(em.group(3) or "0", 16)
    return out


def census_room(name, pkgs, disp, dbind, midx, skips, cidx, clos):
    me = pkgs[name]
    win = c.text_window(me["chunk2"])
    if win is None:
        return None
    lo, hi = win
    bs = c.bodies(me["chunk2"], lo, hi)
    a = c.analyse_room(name, me, win, bs)
    chunk = me["chunk2"]
    y = c.OVL_DIR / f"room_{name}.yaml"
    c_spans = (c.yaml_leaves(y)[1] if y.is_file() else []) + c.nonmatching_leaves(name)

    def natives_at(off):
        found = []
        for (dv, dfp), ents in disp.items():
            if dv != c.VB + off:
                continue
            for t, sym, esz, hand in ents:
                if off + esz <= len(chunk) and c.fnv1a(chunk[off:off + esz]) == dfp:
                    found.append((t, sym, esz, hand, dfp))
        return found

    rows = []
    for off, size in bs:
        vma = c.VB + off
        fp = c.fnv1a(chunk[off:off + size])
        natives = natives_at(off)
        data_head = 0
        if not natives:
            # The roomscan text-window rule folds the data words that precede
            # a room's first function (a seed, colours, a string) into its
            # first body.  When a leaf (this room's YAML c / NON_MATCHING leaf,
            # or any registry entry: how a twin room resolves the same bytes)
            # starts inside the body and ends with it, the body's code is that
            # leaf: its entry is what code and records call (the head is
            # addressed as data, e.g. m0145i `RoomFxSeed8 D_8018F1CC`).
            cands = sorted({o2 for _n, o2, s2 in c_spans if off < o2 and o2 + s2 == off + size}
                           | {dv - c.VB for (dv, _fp), ents in disp.items()
                              if off < dv - c.VB < off + size
                              and any(dv - c.VB + e[2] == off + size for e in ents)})
            for o2 in cands:
                tail = [x for x in natives_at(o2) if o2 + x[2] == off + size]
                if tail:
                    natives, data_head = tail, o2 - off
                    break
        # PE_GuestCode_Resolve: one fingerprint match, where matches with the
        # same size and FNV count once if they are 8-byte stubs or share the
        # generated symbol (closure equality is not modelled here: stricter).
        distinct = {(dfp, esz) for _, _, esz, _, dfp in natives}
        syms = {s for _, s, _, _, _ in natives}
        matches = len({t for t, _, _, _, _ in natives})
        closures = {clos.get((t, s_, vma), 0) for t, s_, _, _, _ in natives}
        same = len(distinct) == 1 and (next(iter(distinct))[1] == 8 or len(syms) == 1
                                       or (len(closures) == 1 and 0 not in closures))
        if natives and (matches == 1 or same):
            status = "native-hand" if any(h for *_, h, _ in natives) else "native-generated"
        elif natives:
            status = "ambiguous"
        else:
            status = "not-native"
        direct = []
        if status == "not-native":
            for b in dbind.get(vma, []):
                if b["size"] is None:
                    direct.append(b["where"] + " (address only)")
                elif off + b["size"] <= len(chunk) and c.fnv1a(chunk[off:off + b["size"]]) == b["fnv"]:
                    direct.append(b["where"])
            if any("(address only)" not in d for d in direct):
                status = "native-direct"
            elif direct:
                status = "direct-addr-guarded"
        mc = midx.get((vma, size, fp), [])
        reason = None
        if mc and not status.startswith("native"):
            for room, leaf, _ in mc:
                reason = skips.get((f"room_{room}", leaf))
                if reason:
                    break
        twins = []
        if not status.startswith("native"):
            key = (size, hashlib.sha1(mask_words(chunk[off:off + size])).hexdigest())
            twins = [f"{r}/{l}:{k}" for r, l, k in cidx.get(key, [])]
        mine = a["reach"].get(vma)
        kind = None
        if mine is not None:
            kind = ("conditional" if any("(conditional)" in w for _, w in c.path_of(a["reach"], vma))
                    else "unconditional")
        rows.append(dict(vma=vma, size=size, fnv1a=fp, status=status, data_head=data_head,
                         reachable=mine is not None,
                         reach_kind=kind, matched_c=[f"{r}/{l}" for r, l, _ in mc],
                         gen_skip_reason=reason, direct=direct, c_twins=twins,
                         native=sorted({f"{t}:{s}" for t, s, *_ in natives})))
    return dict(room=name, slot=me["slot"], chunk2_sha1=hashlib.sha1(chunk).hexdigest(),
                window=[c.VB + lo, c.VB + hi], rows=rows,
                records=len(a["records"]), script_6a=len(a["allocs"]))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--json", type=Path)
    ap.add_argument("--markdown", type=Path)
    ap.add_argument("--dispatch", type=Path, default=c.DISPATCH)
    ap.add_argument("--skips", type=Path)
    ap.add_argument("--only", default="", help="comma list of rooms")
    ns = ap.parse_args()
    if not ns.dispatch.is_file():
        raise SystemExit(f"no registry {ns.dispatch}: run tools/analysis/gen_overlay_ports.py first")
    exe = c.load_exe()
    pkgs = c.all_packages(exe)
    disp = c.dispatch_entries(ns.dispatch)
    dbind = c.direct_bindings()
    midx = c.matched_index(pkgs)
    cidx = c_source_index(pkgs)
    clos = dispatch_closures(ns.dispatch)
    skips = {}
    if ns.skips and ns.skips.is_file():
        for ln in ns.skips.read_text().splitlines():
            m = re.match(r"\s+skip (room_\w+)/(func_\w+): (.*)", ln)
            if m:
                skips[(m.group(1), m.group(2))] = m.group(3)
    want = [r for r in ns.only.split(",") if r] or sorted(pkgs)
    out = []
    for name in want:
        if name not in pkgs:
            continue
        res = census_room(name, pkgs, disp, dbind, midx, skips, cidx, clos)
        if res is not None:
            out.append(res)
    tot = collections.Counter()
    reach_tot = collections.Counter()
    uniq_nn = {}
    uniq_all = {}
    for r in out:
        for row in r["rows"]:
            tot[row["status"]] += 1
            key = (row["vma"], row["size"], row["fnv1a"])
            uniq_all.setdefault(key, row["status"])
            if row["reachable"]:
                reach_tot[row["status"]] += 1
            if not row["status"].startswith("native"):
                uniq_nn.setdefault(key, []).append(r["room"])
    twin_keys = {(row["vma"], row["size"], row["fnv1a"]) for r in out for row in r["rows"]
                 if not row["status"].startswith("native") and row["c_twins"]}
    summary = dict(rooms=len(out), bodies=sum(len(r["rows"]) for r in out),
                   native_via_data_head=sum(1 for r in out for row in r["rows"]
                                            if row["data_head"] and row["status"].startswith("native")),
                   unique_not_native_with_c_twin=len(twin_keys),
                   by_status=dict(tot), reachable_by_status=dict(reach_tot),
                   unique_bodies=len(uniq_all),
                   unique_not_native=len(uniq_nn),
                   unique_not_native_bytes=sum(k[1] for k in uniq_nn))
    print(json.dumps(summary, indent=1))
    worst = sorted(out, key=lambda r: -sum(1 for x in r["rows"] if x["reachable"]
                                           and not x["status"].startswith("native")))
    print("rooms by reachable-not-native bodies:")
    for r in worst[:40]:
        rn = [x for x in r["rows"] if x["reachable"] and not x["status"].startswith("native")]
        nn = [x for x in r["rows"] if not x["status"].startswith("native")]
        print(f"  {r['room']}: bodies {len(r['rows'])} not-native {len(nn)} reachable-not-native {len(rn)}"
              f" ({sum(x['size'] for x in rn):#x} bytes)")
    if ns.json:
        ns.json.write_text(json.dumps(dict(summary=summary, rooms=out), indent=1))
    if ns.markdown:
        lines = ["# All-rooms native census", "",
                 f"Rooms with code: {summary['rooms']}; bodies {summary['bodies']}; "
                 f"unique (vma,size,FNV) bodies {summary['unique_bodies']}; "
                 f"unique not-native {summary['unique_not_native']} "
                 f"({summary['unique_not_native_bytes']:#x} bytes).", "",
                 "| status | bodies | reachable |", "|---|--:|--:|"]
        for s in sorted(tot):
            lines.append(f"| {s} | {tot[s]} | {reach_tot.get(s, 0)} |")
        lines += ["", "| room | bodies | not native | reachable not native | bytes |", "|---|--:|--:|--:|--:|"]
        for r in worst:
            rn = [x for x in r["rows"] if x["reachable"] and not x["status"].startswith("native")]
            nn = [x for x in r["rows"] if not x["status"].startswith("native")]
            lines.append(f"| {r['room']} | {len(r['rows'])} | {len(nn)} | {len(rn)} | {sum(x['size'] for x in rn):#x} |")
        ns.markdown.write_text("\n".join(lines) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""room_callback_census.py -- room-overlay code addresses the port cannot dispatch.

`overlay_callback_census.py` covers the six subsystem overlays.  Every room
loads its chunk 2 at one shared address, 0x8018EFE8, so a room function reaches
native code only in one of these ways:
  * a registry row whose FNV-1a fingerprint matches the resident bytes.  This
    is the `PE_GuestCode_Resolve` rule.  Production never marks a room
    resident, so every room address resolves by fingerprint, using the
    8-byte, same-symbol and same-closure equivalences;
  * an explicit hand-dispatch arm, which a room predicate gates;
  * for slot 0 of a module descriptor only, the port's `return 0` stub rule
    (`func_8006914C_module_init` in pc_port/game/boot/func_80029810_port.c).
    The module init in func_8006914C is the only reader of slot 0.  Retail
    loads D_800942E0 in nine EXE functions, and only func_8006914C calls word
    0.  No room or subsystem overlay loads the table.

Candidate references are collected for each room chunk 2 that holds a `jr $ra`:
  word -- every aligned 32-bit word of the chunk that names an address inside
          the chunk;
  lui  -- in the chunk's code area (up to its last `jr $ra` and delay slot),
          `lui rt,hi` followed within six words by `addiu|ori rt,rt,lo`.

Code and data are told apart by two things:
  * the registry rows attributed to the room, i.e. those whose fingerprint
    matches the room's bytes;
  * the gaps between those rows.  A gap counts as code when it holds a
    terminator: a `jr $ra`, or a `j` into the chunk or the EXE text.

Inside a gap, the code entries are:
  * the first word after an attributed row;
  * the first word after a terminator's delay slot (zero padding skipped);
  * every exact `return 0` stub (`jr $ra; addu $v0,$zero,$zero`).
Each entry's body runs to its next terminator and that terminator's delay slot.

Classes:
  registry        unique fingerprint resolution
  ambiguous       two or more distinct matches, so resolve returns NULL
  row_unresolved  an attributed row start that resolve still misses
  internal        strictly inside an attributed row
  stub0           a `return 0` stub named only as slot 0 of 7-pointer
                  descriptors (the module-init stub rule)
  hand            a gap entry, or a label inside a gap body, that has at least
                  one production arm literal.  Arms are listed as file:line;
                  this tool does not check an arm's room predicate.
  UNCOVERED       a gap entry or gap label with no row, no arm and no stub rule
  data            everything else

Scope:
  * own-chunk references only: no addresses computed at run time, and none
    held only in other chunks;
  * hand-arm predicates are not evaluated;
  * a row whose fingerprint window runs past the end of the chunk is never
    attributed, because the port would hash RAM the chunk does not define.

No game data is written.  Retail files are read-only inputs, and the report
prints addresses, counts and file:line sites only.
"""
from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
MAIN = REPO.parents[1] if REPO.parent.name == "build" else REPO
REGISTRY = "pc_port/game/decomp_ovl/_dispatch/zz_dispatch.c"

sys.path.insert(0, str(REPO / "tools/analysis"))
sys.path.insert(0, str(REPO / "tools/extract"))
import overlay_callback_census as occ  # noqa: E402  (fnv1a, lui_pairs, strip_comments, ARM_RE)

ROOM_VMA = 0x8018EFE8
JR_RA = 0x03E00008
MOVE_V0_ZERO = 0x00001021
EXE_TEXT = (0x80010000, 0x800E0808)
FAIL_CLASSES = ("UNCOVERED", "ambiguous", "row_unresolved")
CLASSES = ("registry", "internal", "stub0", "hand", "data") + FAIL_CLASSES

TABLE_RE = re.compile(r'static const PeGuestCodeTable pe_gc_\w+ = \{"([^"]+)", 0x([0-9A-Fa-f]+)u, '
                      r'0x([0-9A-Fa-f]+)u, [^,]+, 0x[0-9A-Fa-f]+u, (pe_gc_\w+_e), (\d+)u\};')
ENTRIES_RE = re.compile(r"static const PeGuestCodeEntry (pe_gc_\w+_e)\[\] = \{(.*?)\n\};", re.S)
ENTRY_RE = re.compile(r'\{0x([0-9A-Fa-f]{8})u,\s*0x([0-9A-Fa-f]+)u,\s*0x([0-9A-Fa-f]{8})u,\s*\w+,\s*'
                      r'(?:"([^"]*)"|0|NULL),\s*0x([0-9A-Fa-f]{8})u')
ORDER_RE = re.compile(r"pe_gc_tables\[\]\s*=\s*\{(.*?)\};", re.S)


def registry_tables(text: str) -> list[dict]:
    """The generated tables in `pe_gc_tables[]` order, entries truncated to their count."""
    entries = {}
    for name, body in ENTRIES_RE.findall(text):
        entries[name] = [{"vma": int(v, 16), "size": int(s, 16), "fp": int(f, 16),
                          "symbol": sym or "", "closure": int(c, 16)}
                         for v, s, f, sym, c in ENTRY_RE.findall(body)]
    tables = {}
    for tid, lo, hi, ename, count in TABLE_RE.findall(text):
        rows = entries.get(ename, [])[: int(count)]
        if len(rows) != int(count):
            raise ValueError(f"registry table {tid}: {len(rows)} rows parsed, count {count}")
        tables[ename[: -len("_e")]] = {"id": tid, "lo": int(lo, 16), "hi": int(hi, 16), "entries": rows}
    order = ORDER_RE.search(text)
    if not order:
        raise ValueError("registry: no pe_gc_tables[] array")
    return [tables[n] for n in re.findall(r"&(pe_gc_\w+)\s*,", order.group(1))]


def find_entry(rows: list[dict], addr: int) -> dict | None:
    """pe_guestcode.c find_entry: lower-bound binary search on vma."""
    lo, hi = 0, len(rows)
    while lo < hi:
        mid = (lo + hi) // 2
        if rows[mid]["vma"] < addr:
            lo = mid + 1
        else:
            hi = mid
    return rows[lo] if lo < len(rows) and rows[lo]["vma"] == addr else None


def fingerprint_matches(e: dict, addr: int, chunk: bytes) -> bool:
    off = addr - ROOM_VMA
    return bool(e["fp"]) and 0 <= off and off + e["size"] <= len(chunk) and \
        occ.fnv1a(chunk[off:off + e["size"]]) == e["fp"]


def resolve(tables: list[dict], addr: int, chunk: bytes) -> tuple[dict | None, int]:
    """PE_GuestCode_Resolve with no resident table, over `chunk` mounted at ROOM_VMA.
    Returns (entry or None, number of distinct fingerprint matches)."""
    by_fp, matches = None, 0
    for t in tables:
        if not t["lo"] <= addr < t["hi"]:
            continue
        e = find_entry(t["entries"], addr)
        if e is None:
            continue
        if t["id"] == "exe":
            return e, 1
        if fingerprint_matches(e, addr, chunk):
            same = (by_fp is not None and by_fp["size"] == e["size"] and by_fp["fp"] == e["fp"] and
                    (e["size"] == 8 or (by_fp["symbol"] and e["symbol"] and by_fp["symbol"] == e["symbol"]) or
                     (e["closure"] != 0 and by_fp["closure"] == e["closure"])))
            if not same:
                matches += 1
            if by_fp is None:
                by_fp = e
    return (by_fp if matches == 1 else None), matches


def arm_sites(repo: Path) -> dict[int, list[str]]:
    """Production `case 0x...:` / `== 0x...` literals, with their file:line sites."""
    sites: dict[int, list[str]] = defaultdict(list)
    for p in sorted((repo / "pc_port").rglob("*.c")):
        rel = p.relative_to(repo).as_posix()
        if "/tests/" in rel or "/_dispatch/" in rel:
            continue
        text = occ.strip_comments(p.read_text(encoding="utf-8", errors="replace"))
        for m in occ.ARM_RE.finditer(text):
            addr = int(m.group(1) or m.group(2), 16)
            if ROOM_VMA <= addr < 0x80200000:
                sites[addr].append(f"{rel}:{text.count(chr(10), 0, m.start()) + 1}")
    return dict(sites)


def census_room(name: str, chunk: bytes, tables: list[dict], arms: dict[int, list[str]],
                use_j: bool = True) -> dict | None:
    n = len(chunk) // 4
    ws = struct.unpack("<%dI" % n, chunk[: n * 4])
    end = ROOM_VMA + len(chunk)
    jrs = [i for i, w in enumerate(ws) if w == JR_RA]
    if not jrs:
        return None
    code_end = jrs[-1] + 2                       # word index past the last delay slot

    def in_chunk(w: int) -> bool:
        return ROOM_VMA <= w < end and w % 4 == 0

    # Registry rows attributed to this room: an entry whose fingerprint
    # matches these bytes, from any non-EXE table whose window covers it.
    rows = set()
    for t in tables:
        if t["id"] == "exe":
            continue
        for e in t["entries"]:
            if ROOM_VMA <= e["vma"] < end and fingerprint_matches(e, e["vma"], chunk):
                rows.add((e["vma"], e["vma"] + e["size"]))
    rows_sorted = sorted(rows)
    covered = bytearray(code_end)
    for a, b in rows_sorted:
        for i in range((a - ROOM_VMA) // 4, min((b - ROOM_VMA + 3) // 4, code_end)):
            covered[i] = 1

    def terminator(i: int, run_lo: int, run_hi: int) -> bool:
        """`jr $ra`, or a `j` that leaves its uncovered run (a tail call).  A `j`
        to a label inside the same run is a jump within one function."""
        w = ws[i]
        if w == JR_RA:
            return True
        if use_j and w >> 26 == 0x02:
            target = ((ROOM_VMA + i * 4 + 4) & 0xF0000000) | ((w & 0x03FFFFFF) << 2)
            if ROOM_VMA + run_lo * 4 <= target < ROOM_VMA + run_hi * 4:
                return False
            return ROOM_VMA <= target < end or EXE_TEXT[0] <= target < EXE_TEXT[1]
        return False

    def is_ret0(i: int) -> bool:
        return i + 1 < n and ws[i] == JR_RA and ws[i + 1] == MOVE_V0_ZERO

    # Gaps: maximal uncovered runs of the code area that hold a terminator,
    # trimmed after the last terminator's delay slot.
    gaps = []
    i = 0
    while i < code_end:
        if covered[i]:
            i += 1
            continue
        j = i
        while j < code_end and not covered[j]:
            j += 1
        terms = [k for k in range(i, j) if terminator(k, i, j)]
        if terms:
            gaps.append((i, min(terms[-1] + 2, j), terms))
        i = j

    bodies = {}                                  # entry word index -> body end (exclusive)
    for g0, g1, terms in gaps:
        starts = set()
        if g0 > 0:                               # right after an attributed row
            starts.add(g0)
        starts.update(t + 2 for t in terms)
        starts.update(k for k in range(g0, g1) if is_ret0(k))
        for s in sorted(starts):
            while s < g1 and ws[s] == 0:
                s += 1
            if s >= g1 or s in bodies:
                continue
            nxt = next((t for t in terms if t >= s), None)
            if nxt is not None:
                bodies[s] = min(nxt + 2, g1)

    def slot0(k: int) -> bool:
        if k + 7 > n or not all(in_chunk(ws[k + d]) for d in range(7)):
            return False
        return not (k > 0 and in_chunk(ws[k - 1])) and not (k + 7 < n and in_chunk(ws[k + 7]))

    refs: dict[int, list[tuple[str, int]]] = defaultdict(list)
    for k, w in enumerate(ws):
        if in_chunk(w):
            refs[w].append(("word", k))
    for addr, pc in occ.lui_pairs(chunk[: code_end * 4], ROOM_VMA):
        if in_chunk(addr):
            refs[addr].append(("lui", (pc - ROOM_VMA) // 4))

    def via(addr: int) -> list[str]:
        return [f"word+0x{k * 4:X}" if kind == "word" else f"lui@0x{ROOM_VMA + k * 4:08X}"
                for kind, k in refs[addr]]

    counts = dict.fromkeys(CLASSES, 0)
    items = []
    entry_class = {}
    by_addr = {}
    for addr in sorted(refs):
        k = (addr - ROOM_VMA) // 4
        e, matches = resolve(tables, addr, chunk)
        arm = arms.get(addr, [])
        if e is not None:
            cls = "registry"
        elif matches > 1:
            cls = "ambiguous"
        elif any(a == addr for a, _ in rows_sorted):
            cls = "row_unresolved"
        elif any(a < addr < b for a, b in rows_sorted):
            cls = "internal"
        elif k in bodies:
            if is_ret0(k) and all(kind == "word" and slot0(j) for kind, j in refs[addr]):
                cls = "stub0"
            elif arm:
                cls = "hand"
            else:
                cls = "UNCOVERED"
            entry_class[k] = cls
        else:
            owner = next((s for s, b in bodies.items() if s < k < b), None)
            if owner is None:
                cls = "data"
            else:
                owner_arms = arms.get(ROOM_VMA + owner * 4, [])
                cls = "hand" if (arm or owner_arms) else "UNCOVERED"
                arm = arm or owner_arms
        counts[cls] += 1
        by_addr[addr] = cls
        if cls not in ("registry", "internal", "data"):
            items.append({"class": cls, "addr": addr, "via": via(addr),
                          "arms": arm if cls == "hand" else []})
    gap_bodies = [{"entry": ROOM_VMA + s * 4, "size": (b - s) * 4,
                   "class": entry_class.get(s, "unreferenced"),
                   "arms": arms.get(ROOM_VMA + s * 4, [])}
                  for s, b in sorted(bodies.items())]
    return {"room": name, "attributed_rows": len(rows_sorted), "counts": counts,
            "items": items, "gap_bodies": gap_bodies, "_classes": by_addr}


def room_chunks(peimg, man: dict) -> list[tuple[str, bytes]]:
    exe = peimg.read_exe(MAIN, man)
    image = peimg.open_image(MAIN, man, hash_image=True)
    image_lba = int(man["image"]["disc_lba"])
    out = []
    for room in peimg.room_table(exe, man):
        if room["sec"][2] == 0:
            continue
        start = room["lba"] + room["sec"][0] + room["sec"][1] - image_lba
        out.append((room["name"], peimg.read_sectors(image, start, start + room["sec"][2])))
    return out


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--json", type=Path, help="write the full result here")
    ap.add_argument("--check", action="store_true",
                    help="exit 1 when any reference is UNCOVERED, ambiguous or row_unresolved")
    ap.add_argument("--rooms", nargs="*", help="limit to these rooms (m####i)")
    ns = ap.parse_args(argv)

    import peimg  # tools/extract; read-only use
    man = peimg.load_manifest(REPO, peimg.DEFAULT_MANIFEST)
    tables = registry_tables((REPO / REGISTRY).read_text(encoding="utf-8"))
    arms = arm_sites(REPO)
    results, j_refs, j_bodies = [], 0, 0
    totals = dict.fromkeys(CLASSES, 0)
    for name, chunk in room_chunks(peimg, man):
        if ns.rooms and name not in ns.rooms:
            continue
        res = census_room(name, chunk, tables, arms)
        if res is None:
            continue
        # How much the `j` (tail-call) terminators matter: references whose
        # class differs from a `jr $ra`-only run, and the gap-body difference.
        base = census_room(name, chunk, tables, arms, use_j=False)
        j_refs += sum(1 for a, c in res["_classes"].items() if base["_classes"].get(a) != c)
        j_bodies += len(res["gap_bodies"]) - len(base["gap_bodies"])
        del res["_classes"]
        results.append(res)
        for c in CLASSES:
            totals[c] += res["counts"][c]
        if res["items"] or res["gap_bodies"]:
            shown = " ".join(f"{c} {res['counts'][c]}" for c in CLASSES if res["counts"][c])
            print(f"{name}: rows {res['attributed_rows']} gap bodies {len(res['gap_bodies'])} | {shown}")
            for it in res["items"]:
                extra = f" arms {', '.join(it['arms'][:3])}" if it["arms"] else ""
                print(f"    {it['class']} 0x{it['addr']:08X} via {', '.join(it['via'][:3])}{extra}")
            for g in res["gap_bodies"]:
                print(f"    gap body 0x{g['entry']:08X} +0x{g['size']:X} {g['class']}")
    nbodies = sum(len(r["gap_bodies"]) for r in results)
    print("room_callback_census: %d code rooms, %d references (%s); %d gap bodies; "
          "`j` tail-call terminators reclassify %d references and add %d gap bodies"
          % (len(results), sum(totals.values()),
             ", ".join(f"{c} {totals[c]}" for c in CLASSES), nbodies, j_refs, j_bodies))
    if ns.json:
        ns.json.write_text(json.dumps(results, indent=1) + "\n")
    return 1 if ns.check and any(totals[c] for c in FAIL_CLASSES) else 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""overlay_callback_census.py -- subsystem-overlay code addresses the port cannot dispatch.

`dangling_callback_census.py` checks EXE function starts only.  A subsystem
overlay (manifest `kind: subsystem`) runs its own code at a fixed vram.  Its
functions reach native code in one of two ways:
  * a registry row whose FNV-1a fingerprint matches the overlay bytes, as
    `PE_GuestCode_Resolve` requires;
  * an explicit hand-dispatch arm.

Some overlay addresses fall outside every such row even though the game
materialises them, as a stored word or as a `lui`/`addiu|ori` pair.  The port
cannot run those callbacks: `PE_EffectCallback` and `PE_GuestCall` stop on
them.  The ovl_03C9 Liberate callbacks 0x801EDC44 and 0x801EEBC8 were of this
class.

Candidate code addresses are collected for each subsystem overlay that has a
recorded text window, inside [vram+text_start, vram+text_end):
  word -- every aligned 32-bit word of the overlay's own blob;
  lui  -- in the overlay's own text, `lui rt,hi` followed within six words by
          `addiu|ori rt,rt,lo`.  The search stops at another write of rt by lui.
  exe  -- every aligned word of the retail EXE image.  This is done only for
          overlays whose arena no other overlay or room package shares.  In a
          shared arena an address seen in EXE data is ambiguous.

Each address is classified against the registry rows attributed to that
overlay.  A row is attributed when its span lies inside the text window and the
FNV-1a of the overlay bytes over that span equals the row's fingerprint.
  resolved  -- a row start, or a hand-dispatch `case`/`==` arm;
  internal  -- strictly inside an attributed row (a branch or jump-table label);
  UNCOVERED -- outside every attributed row.

Scope: this sees only own-blob references, plus the EXE for unshared arenas.
It does not see references held only in other overlays' or room packages'
data, or addresses computed at run time.

No game data is written.  Retail files are read-only inputs, and the report
prints addresses and counts only.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
MAIN = REPO.parents[1] if REPO.parent.name == "build" else REPO
DEF_EXE = MAIN / "build/extracted/disc1/SLUS_006.62"
DEF_PEIMG = MAIN / "build/extracted/disc1/peimg"
REGISTRY = "pc_port/game/decomp_ovl/_dispatch/zz_dispatch.c"

sys.path.insert(0, str(REPO / "tools/build"))
import overlay_targets  # noqa: E402

ROW_RE = re.compile(r"\{0x([0-9A-F]{8})u,\s*0x([0-9A-F]+)u,\s*0x([0-9A-F]{8})u,")
ARM_RE = re.compile(r"(?:case\s+0x([0-9A-Fa-f]{8})u?\s*:|==\s*0x([0-9A-Fa-f]{8})u?)")


def fnv1a(data: bytes) -> int:
    h = 0x811C9DC5
    for b in data:
        h = ((h ^ b) * 0x01000193) & 0xFFFFFFFF
    return h


def strip_comments(t: str) -> str:
    t = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)), t, flags=re.S)
    return re.sub(r"//[^\n]*", "", t)


def registry_rows(text: str) -> list[tuple[int, int, int]]:
    return [(int(a, 16), int(s, 16), int(f, 16)) for a, s, f in ROW_RE.findall(text)]


def hand_arms(repo: Path) -> set[int]:
    arms: set[int] = set()
    for p in sorted((repo / "pc_port").rglob("*.c")):
        rel = p.relative_to(repo).as_posix()
        if "/tests/" in rel or "/_dispatch/" in rel:
            continue
        for m in ARM_RE.finditer(strip_comments(p.read_text(encoding="utf-8", errors="replace"))):
            arms.add(int(m.group(1) or m.group(2), 16))
    return arms


def lui_pairs(text: bytes, text_vma: int) -> list[tuple[int, int]]:
    """(address, instruction vma) for each lui/addiu|ori pair in `text`."""
    n = len(text) // 4
    words = struct.unpack("<%dI" % n, text[: n * 4])
    out = []
    for i, w in enumerate(words):
        if w >> 26 != 0x0F:
            continue
        rt, hi = (w >> 16) & 31, w & 0xFFFF
        for j in range(i + 1, min(i + 7, n)):
            v = words[j]
            op = v >> 26
            if op in (0x09, 0x0D) and (v >> 21) & 31 == rt and (v >> 16) & 31 == rt:
                lo = v & 0xFFFF
                if op == 0x09 and lo & 0x8000:
                    lo -= 0x10000
                out.append((((hi << 16) + lo) & 0xFFFFFFFF, text_vma + i * 4))
                break
            if op == 0x0F and (v >> 16) & 31 == rt:
                break
    return out


def census_overlay(ovl: dict, blob: bytes, rows: list[tuple[int, int, int]],
                   arms: set[int], exe: bytes | None) -> dict:
    vram, lo, hi = ovl["vram"], ovl["vram"] + ovl["text_start"], ovl["vram"] + ovl["text_end"]
    mine = sorted((a, a + s) for a, s, f in rows
                  if lo <= a and a + s <= hi and f == fnv1a(blob[a - vram:a - vram + s]))
    starts = {a for a, _ in mine}
    refs: dict[int, set[str]] = {}

    def add(addr: int, where: str) -> None:
        if lo <= addr < hi and addr % 4 == 0:
            refs.setdefault(addr, set()).add(where)

    n = len(blob) // 4
    for k, (w,) in enumerate(struct.iter_unpack("<I", blob[: n * 4])):
        add(w, f"word {ovl['id']}+0x{k * 4:X}")
    for addr, pc in lui_pairs(blob[ovl["text_start"]:ovl["text_end"]], lo):
        add(addr, f"lui 0x{pc:08X}")
    if exe is not None and ovl.get("exe_scan"):
        m = len(exe) // 4
        for k, (w,) in enumerate(struct.iter_unpack("<I", exe[: m * 4])):
            add(w, f"exe+0x{k * 4:X}")

    out = {"id": ovl["id"], "text": [lo, hi], "attributed_rows": len(mine),
           "resolved": 0, "internal": 0, "uncovered": []}
    for addr in sorted(refs):
        if addr in starts or addr in arms:
            out["resolved"] += 1
        elif any(a < addr < e for a, e in mine):
            out["internal"] += 1
        else:
            out["uncovered"].append({"addr": addr, "via": sorted(refs[addr])})
    return out


def subsystem_overlays(repo: Path) -> list[dict]:
    data = overlay_targets.load_manifest(repo)
    shared: dict[int, int] = {}
    for entry in data["overlays"].values():
        shared[int(entry["vram"])] = shared.get(int(entry["vram"]), 0) + 1
    room_arena = data["rooms"].get("chunk2_vram")
    out = []
    for ovl_id, entry in sorted(data["overlays"].items()):
        if entry["kind"] != "subsystem" or "text_start" not in entry:
            continue
        vram = int(entry["vram"])
        out.append({"id": ovl_id, "vram": vram, "size": int(entry["size"]),
                    "text_start": int(entry["text_start"]), "text_end": int(entry["text_end"]),
                    "sha1": str(entry["sha1"]).lower(), "blob": Path(str(entry["blob"])).name,
                    # An arena shared with another overlay, or the room chunk-2
                    # arena next to it, makes EXE words ambiguous.
                    "exe_scan": shared[vram] == 1 and not (
                        room_arena is not None and abs(vram - int(room_arena)) < 0x10)})
    return out


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--peimg", type=Path, default=DEF_PEIMG, help="directory of extracted overlay blobs")
    ap.add_argument("--exe", type=Path, default=DEF_EXE, help="retail SLUS_006.62")
    ap.add_argument("--json", type=Path, help="write the full result here")
    ap.add_argument("--check", action="store_true", help="exit 1 when any reference is UNCOVERED")
    ns = ap.parse_args(argv)

    rows = registry_rows((REPO / REGISTRY).read_text(encoding="utf-8"))
    arms = hand_arms(REPO)
    exe = ns.exe.read_bytes() if ns.exe.is_file() else None
    results = []
    for ovl in subsystem_overlays(REPO):
        blob = (ns.peimg / ovl["blob"]).read_bytes()
        if hashlib.sha1(blob).hexdigest() != ovl["sha1"] or len(blob) != ovl["size"]:
            print(f"overlay_callback_census: {ovl['id']} blob identity mismatch", file=sys.stderr)
            return 2
        res = census_overlay(ovl, blob, rows, arms, exe)
        results.append(res)
        print(f"{res['id']}: text 0x{res['text'][0]:08X}-0x{res['text'][1]:08X} rows {res['attributed_rows']} "
              f"resolved {res['resolved']} internal {res['internal']} UNCOVERED {len(res['uncovered'])}")
        for u in res["uncovered"]:
            print(f"    UNCOVERED 0x{u['addr']:08X} via {', '.join(u['via'][:3])}")
    total = sum(len(r["uncovered"]) for r in results)
    print(f"overlay_callback_census: {len(results)} subsystem overlays, {total} uncovered references"
          + ("" if exe is not None else " (no EXE image: exe scan skipped)"))
    if ns.json:
        ns.json.write_text(json.dumps(results, indent=1) + "\n")
    return 1 if ns.check and total else 0


if __name__ == "__main__":
    raise SystemExit(main())

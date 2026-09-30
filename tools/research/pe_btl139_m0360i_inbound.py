#!/usr/bin/env python3
"""PE-BTL139 — who 0x31-hops to m0360i? Day-2 entry provenance.

Evidence-only. Does not poke persist/scratch. Scans every field-table
script for dest-token opcode 0x31 whose packed name is M0360I, and
also records persist[0x4A] assigns that sit near those hops.
"""
from __future__ import annotations

import json
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from tools.research.pe_pst0_scan import (  # noqa: E402
    ALPHABET,
    DISC1_EXE,
    FORM1_USER_SIZE,
    PE_IMG,
    RawMode2Image,
    decode_packed_name,
    decode_script,
    extract_script_from_package,
    find_entry,
    iso_entries,
    parse_field_table,
)

LOCAL_DISC = ROOT / "local" / "pe_disc1.path"


def resolve_disc() -> Path:
    if LOCAL_DISC.is_file():
        p = Path(LOCAL_DISC.read_text().strip().splitlines()[0].strip())
        if p.is_file():
            return p
    raise SystemExit("disc image not found (local/pe_disc1.path)")


def encode_packed_name(name: str) -> int:
    name = name.lower()
    if len(name) != 6:
        raise ValueError(name)
    token = 0
    for ch, shift in zip(name, (27, 22, 17, 12, 7, 2)):
        idx = ALPHABET.index(ch)
        token |= idx << shift
    return token


def token_scene_index(token: int) -> int | None:
    low = token & 0xFFFF
    if low < 0x248:
        return None
    if (low - 0x248) % 0x80:
        return None
    return ((low - 0x248) // 0x80) + 3


def main() -> int:
    target_name = "m0360i"
    target_token = encode_packed_name(target_name)
    print(f"target name={target_name} packed=0x{target_token:08X}")
    print(f"alphabet-check decode={decode_packed_name(target_token)}")

    disc = resolve_disc()
    hops = []
    name_counts: Counter[str] = Counter()
    token_counts: Counter[int] = Counter()
    scripts_found = 0
    scripts_missing = 0
    names_360 = []

    with RawMode2Image(disc) as image:
        entries = iso_entries(image)
        pe_entry = find_entry(entries, PE_IMG)
        exe = image.read_form1_extent(
            find_entry(entries, DISC1_EXE)[1],
            find_entry(entries, DISC1_EXE)[2],
        )
        field_table = parse_field_table(exe)

        for rec in field_table:
            if rec["end"] <= rec["start"]:
                continue
            try:
                blob = image.read_form1_extent(
                    pe_entry[1] + rec["start"],
                    (rec["end"] - rec["start"]) * FORM1_USER_SIZE,
                )
            except Exception:
                continue
            extracted = extract_script_from_package(blob, rec["meta"])
            if extracted is None:
                scripts_missing += 1
                continue
            _off, script_blob = extracted
            script = decode_script(script_blob)
            if not script.get("modules"):
                scripts_missing += 1
                continue
            scripts_found += 1
            for module in script.get("modules", []):
                cmds = module["commands"]
                last_4a = None
                for cmd in cmds:
                    op = cmd["opcode"]
                    args = cmd["args"]
                    modes = cmd["modes"]
                    # persist[0x4A] assign via 0x0A
                    if op == 0x0A and len(args) >= 2 and modes and modes[0] == 2 and args[0] == 0x4A:
                        last_4a = {
                            "pc": cmd["offset"],
                            "value": args[1] if len(modes) > 1 and modes[1] == 0 else None,
                            "modes": list(modes),
                            "args": [int(a) for a in args],
                        }
                    if op != 0x31 or not args:
                        continue
                    token = int(args[0])
                    name = decode_packed_name(token)
                    name_counts[name] += 1
                    token_counts[token] += 1
                    idx = token_scene_index(token)
                    digits_idx = None
                    try:
                        digits_idx = int(name[1:5]) - 1
                    except ValueError:
                        digits_idx = None
                    if "360" in name or "0360" in name:
                        names_360.append(
                            {
                                "from": rec["name"],
                                "name": name,
                                "token": f"0x{token:08X}",
                            }
                        )
                    if name != target_name and idx != 359 and digits_idx != 359:
                        continue
                    hops.append(
                        {
                            "from": rec["name"],
                            "from_index": rec["index"],
                            "module": module["index"],
                            "script_pc": f"+0x{cmd['offset']:04X}",
                            "token": f"0x{token:08X}",
                            "packed_name": name,
                            "scene_index_formula": idx,
                            "last_persist_4a": last_4a,
                        }
                    )

    out_dir = ROOT / "docs" / "evidence" / "pe-btl139-m0360i-inbound"
    out_dir.mkdir(parents=True, exist_ok=True)
    payload = {
        "target": target_name,
        "packed_token": f"0x{target_token:08X}",
        "hop_count": len(hops),
        "hops": hops,
    }
    (out_dir / "hops.json").write_text(json.dumps(payload, indent=2) + "\n")
    print(f"scripts_found={scripts_found} scripts_missing={scripts_missing}")
    print(f"hops_to_m0360i={len(hops)}")
    print(f"names_containing_360={names_360}")
    for h in hops:
        print(
            f"  {h['from']}[{h['from_index']}] mod{h['module']} {h['script_pc']}"
            f" token={h['token']} name={h['packed_name']} idx={h['scene_index_formula']}"
            f" last4a={h['last_persist_4a']}"
        )
    print("top dest names:")
    for name, n in name_counts.most_common(20):
        print(f"  {n:4d} {name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""PE-BTL0 evidence-only field→battle handoff scanner.

Reads a registered USA Disc 1. Does not implement battle or edit
production runtime. Reuses the PE-PST0 disc/script decoder.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.research import pe_pst0_scan as pst0

OPCODE_TABLE = 0x800910A0
FIELD_LOOP_HINTS = (0x8003F000, 0x80040000)
TEXT_LO = 0x8001220C
TEXT_HI = 0x80091080
TAIL_LO = 0x800C22F8
TAIL_HI = 0x800E1000
RNG_FUNCS = (0x80070D10, 0x80070D6C, 0x80070DD0)

# Opcodes already proven on the field first-play prefix. Not battle.
KNOWN_FIELD = {
    0x00: "goto",
    0x01: "actor_flag_or_0x10_yield",
    0x02: "wait_ticks",
    0x05: "skip_if_false",
    0x08: "create_actor",
    0x09: "alu",
    0x0A: "assign",
    0x0B: "pose_or_snap",
    0x0D: "message_open",
    0x11: "button_or_facing_test",
    0x14: "set_task_pointer",
    0x1B: "actor_speed_or_costume",
    0x1C: "mailbox_send",
    0x1E: "actor_flag_or_0x80",
    0x1F: "mailbox_poll",
    0x20: "park_yield",
    0x22: "message_wait",
    0x23: "message_clear",
    0x24: "facing_poll",
    0x2E: "bind_clip",
    0x2F: "clip_target",
    0x30: "wait_clip",
    0x31: "dest_token",
    0x32: "actor_flag_andnot_0x80",
    0x35: "fmv_index",
    0x3F: "control_restore",
    0x40: "control_inhibit",
    0x4E: "clip_seek",
    0x5E: "sample_actor_to_locals",
    0x65: "zero_actor_velocity",
    0x75: "camera_select",
    0x77: "volume_quad",
    0x79: "actor_flag_or_0x20",
    0x7B: "camera_param",
    0x80: "camera_table_or",
    0x82: "cut_record_select",
    0x84: "camera_heading",
    0x85: "camera_mode2_fade",
    0x86: "camera_mode6",
    0x88: "camera_bit_clear",
    0x9C: "wait_camera_ready",
    0xAA: "set_load_bit",
    0xAB: "clear_load_bit",
    0xE1: "camera_byte",
    0xEA: "audio_or_worker_dispatch",
}


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def is_code_ptr(va: int) -> bool:
    return (TEXT_LO <= va < TEXT_HI) or (TAIL_LO <= va < TAIL_HI)


def dump_opcode_table(exe: bytes) -> list[dict]:
    rows = []
    for index in range(0x200):
        va = OPCODE_TABLE + index * 4
        try:
            handler = pst0.word_at(exe, va)
        except Exception:
            break
        if handler == 0:
            rows.append({"opcode": index, "handler": 0, "in_text": False, "zero": True})
            continue
        if not is_code_ptr(handler) and handler != 0:
            if index > 16 and not any(not r.get("zero") for r in rows[-8:]):
                break
            if index > 0x100:
                break
        rows.append(
            {
                "opcode": index,
                "handler": handler,
                "in_text": is_code_ptr(handler),
                "zero": False,
            }
        )
    # trim trailing zeros
    while rows and rows[-1].get("zero"):
        rows.pop()
    return rows


def jal_targets(exe: bytes, func_va: int, limit_bytes: int = 0x200) -> list[int]:
    targets = []
    off = pst0.va2off(func_va)
    end = min(off + limit_bytes, len(exe))
    for cursor in range(off, end, 4):
        word = struct.unpack_from("<I", exe, cursor)[0]
        if (word >> 26) == 3:  # jal
            dest = (word & 0x03FFFFFF) << 2
            dest |= func_va & 0xF0000000
            targets.append(dest)
        if word == 0x03E00008:  # jr ra
            break
    return targets


def stores_to(exe: bytes, func_va: int, limit_bytes: int = 0x200) -> list[dict]:
    hits = []
    off = pst0.va2off(func_va)
    end = min(off + limit_bytes, len(exe))
    last_lui: dict[int, int] = {}
    for cursor in range(off, end, 4):
        word = struct.unpack_from("<I", exe, cursor)[0]
        op = word >> 26
        if op == 0x0F:  # lui
            last_lui[(word >> 16) & 0x1F] = (word & 0xFFFF) << 16
        elif op in (0x2B, 0x29, 0x28):  # sw / sh / sb
            rs = (word >> 21) & 0x1F
            rt = (word >> 16) & 0x1F
            imm = word & 0xFFFF
            if imm >= 0x8000:
                imm -= 0x10000
            base = last_lui.get(rs)
            if base is not None:
                addr = (base + imm) & 0xFFFFFFFF
                hits.append(
                    {
                        "kind": {0x2B: "sw", 0x29: "sh", 0x28: "sb"}[op],
                        "rt": rt,
                        "addr": addr,
                        "pc": 0x80010000 + (cursor - 0x800),
                    }
                )
        if word == 0x03E00008:
            break
    return hits


def extract_scene(exe: bytes, pe_img: bytes, table_index: int) -> dict | None:
    table = pst0.parse_field_table(exe)
    rec = next((r for r in table if r["index"] == table_index), None)
    if rec is None:
        return None
    start = rec["start"] * 2048
    end = rec["end"] * 2048
    if end <= start or end > len(pe_img):
        return None
    package = pe_img[start:end]
    extracted = pst0.extract_script_from_package(package, rec["meta"])
    if extracted is None:
        return None
    rel, blob = extracted
    script = pst0.decode_script(blob)
    return {
        "table_index": table_index,
        "name": rec["name"],
        "start_sector": rec["start"],
        "end_sector": rec["end"],
        "meta": rec["meta"],
        "package_sha256": sha256(package),
        "package_bytes": len(package),
        "script_rel": rel,
        "script": script,
    }


def walk_first_play_graph(exe: bytes, pe_img: bytes) -> dict:
    """BFS from proven first-play prefix, following 0x31 tokens.

    persist[0x4A] starts at 0x18 after m0004i reel; persist[1] follows
    the PST0 walker. This is a static opcode walk, not a VM.
    """
    start = [
        ("m0002i", 1),
        ("m0003i", 2),
        ("m0372i", 371),
        ("m0004i", 3),
        ("m0378i", 377),
        ("m0377i", 376),
    ]
    seen = {}
    queue = list(start)
    hops = []
    while queue:
        name, idx = queue.pop(0)
        if idx in seen:
            continue
        scene = extract_scene(exe, pe_img, idx)
        if scene is None:
            seen[idx] = {"name": name, "error": "extract_failed"}
            continue
        seen[idx] = scene
        for module in scene["script"]["modules"]:
            for cmd in module["commands"]:
                if cmd["opcode"] != 0x31 or not cmd["args"]:
                    continue
                token = cmd["args"][0]
                dest_name = pst0.decode_packed_name(token)
                # digits[2:5] then -1 is table index for mNNNNi
                try:
                    digits = int(dest_name[1:5])
                    dest_idx = digits - 1
                except ValueError:
                    dest_idx = None
                hops.append(
                    {
                        "src": scene["name"],
                        "src_index": idx,
                        "module": module["index"],
                        "pc": cmd["offset"],
                        "token": token,
                        "dest": dest_name,
                        "dest_index": dest_idx,
                    }
                )
                if dest_idx is not None and dest_idx not in seen and dest_idx < 2000:
                    queue.append((dest_name, dest_idx))
    return {"scenes": seen, "hops": hops}


def opcode_census(scenes: dict) -> list[dict]:
    counts: Counter[int] = Counter()
    by_scene: dict[int, set[str]] = defaultdict(set)
    samples: dict[int, list[dict]] = defaultdict(list)
    for scene in scenes.values():
        if "script" not in scene:
            continue
        name = scene["name"]
        for module in scene["script"]["modules"]:
            for cmd in module["commands"]:
                op = cmd["opcode"]
                counts[op] += 1
                by_scene[op].add(name)
                if len(samples[op]) < 6:
                    samples[op].append(
                        {
                            "scene": name,
                            "module": module["index"],
                            "pc": cmd["offset"],
                            "argc": cmd["argc"],
                            "args": cmd["args"][:6],
                            "modes": cmd["modes"][:6],
                        }
                    )
    rows = []
    for op in sorted(counts):
        rows.append(
            {
                "opcode": op,
                "count": counts[op],
                "scenes": sorted(by_scene[op]),
                "known": KNOWN_FIELD.get(op, ""),
                "samples": samples[op],
            }
        )
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("disc")
    parser.add_argument("--out", type=Path, default=Path("local/btl0"))
    args = parser.parse_args()
    disc = Path(args.disc)
    args.out.mkdir(parents=True, exist_ok=True)

    digest = pst0.sha256_file(disc)
    if digest != pst0.DISC1_SHA256:
        raise SystemExit(f"disc sha mismatch {digest}")
    with pst0.RawMode2Image(disc) as image:
        entries = pst0.iso_entries(image)
        exe_ent = pst0.find_entry(entries, pst0.DISC1_EXE)
        pe_ent = pst0.find_entry(entries, pst0.PE_IMG)
        exe = image.read_form1_extent(exe_ent[1], exe_ent[2])
        pe_img = image.read_form1_extent(pe_ent[1], pe_ent[2])
    if pst0.sha256_bytes(exe) != pst0.DISC1_EXE_SHA256:
        raise SystemExit("exe sha mismatch")

    table = dump_opcode_table(exe)
    graph = walk_first_play_graph(exe, pe_img)
    census = opcode_census(graph["scenes"])

    handler_rows = []
    for row in table:
        op = row["opcode"]
        handler = row["handler"]
        used = next((c for c in census if c["opcode"] == op), None)
        info = {
            "opcode": f"0x{op:02X}",
            "opcode_dec": op,
            "handler": f"0x{handler:08X}" if handler else "0",
            "in_text": row["in_text"],
            "used_on_graph": bool(used),
            "use_count": used["count"] if used else 0,
            "scenes": ",".join(used["scenes"]) if used else "",
            "known": KNOWN_FIELD.get(op, ""),
            "jals": "",
            "stores": "",
        }
        if handler and row["in_text"]:
            jals = jal_targets(exe, handler)
            stores = stores_to(exe, handler)
            info["jals"] = " ".join(f"0x{t:08X}" for t in jals[:12])
            info["stores"] = " ".join(
                f"{s['kind']}@0x{s['addr']:08X}" for s in stores[:12]
            )
        handler_rows.append(info)

    # EXE lui/addiu refs to persist already known. Search jal to RNG
    # from opcode handlers used on the graph.
    rng_hits = []
    for row in handler_rows:
        if not row["used_on_graph"] or row["handler"] == "0":
            continue
        for dest in row["jals"].split():
            if dest and int(dest, 16) in RNG_FUNCS:
                rng_hits.append({"opcode": row["opcode"], "jal": dest})

    unknown_used = [r for r in handler_rows if r["used_on_graph"] and not r["known"]]

    summary = {
        "disc_sha256": digest,
        "exe_sha256": pst0.DISC1_EXE_SHA256,
        "opcode_table_entries": len(table),
        "graph_scene_count": len(graph["scenes"]),
        "graph_scenes": sorted(
            v.get("name", str(k)) for k, v in graph["scenes"].items()
        ),
        "hop_count": len(graph["hops"]),
        "unique_opcodes": len(census),
        "unknown_used_opcodes": [r["opcode"] for r in unknown_used],
        "rng_from_used_handlers": rng_hits,
    }

    (args.out / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    (args.out / "hops.json").write_text(json.dumps(graph["hops"], indent=2) + "\n")
    (args.out / "census.json").write_text(
        json.dumps(
            [
                {
                    "opcode": f"0x{c['opcode']:02X}",
                    "count": c["count"],
                    "scenes": c["scenes"],
                    "known": c["known"],
                    "samples": c["samples"],
                }
                for c in census
            ],
            indent=2,
        )
        + "\n"
    )
    with (args.out / "handlers.csv").open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(handler_rows[0]))
        writer.writeheader()
        writer.writerows(handler_rows)

    scene_meta = []
    for idx, scene in sorted(graph["scenes"].items()):
        scene_meta.append(
            {
                "index": idx,
                "name": scene.get("name"),
                "package_sha256": scene.get("package_sha256", ""),
                "package_bytes": scene.get("package_bytes", 0),
                "modules": scene.get("script", {}).get("module_count", 0),
                "error": scene.get("error", ""),
            }
        )
    (args.out / "scenes.json").write_text(json.dumps(scene_meta, indent=2) + "\n")
    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

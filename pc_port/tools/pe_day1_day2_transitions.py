#!/usr/bin/env python3
"""Evidence scan for the Day-1 completion marker and the Day-2 terminal route.

The field VM stores the story/progress word in persist slot 74
(0x800A77F0 + 74*4 = 0x800A7918, read as ``story`` by the M0351I exit
selector). This tool decodes every field-table room package and reports:

  * every writer of persist[74] (op 0x0A assign / op 0x09 ALU destination),
  * every reader of persist[74] (op 0x05 branch predicate, op 0x09 ALU
    operand, op 0x0A copy source),
  * the room transfer (op 0x31) that follows each writer in its module,

so the g74 = 0x80 Day-1 marker and the g74 = 0x88 -> M0042I handoff can be
pinned to instruction addresses with the real script bytes as evidence.

Static inventory only: no execution, no reachability claim beyond the
immediate module ordering that the bytecode states. Emits JSON.
"""
from __future__ import annotations

import hashlib
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/research"))
from pe_pst0_scan import (  # noqa: E402
    extract_script_from_package,
    parse_field_table,
    decode_packed_name,
    decode_script,
)
from pe_btl14_m0005i_publish_oracle import find_disc, read_form1  # noqa: E402

EXE = ROOT / "build/disc1.candidate.exe"
EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PE_IMG_LBA = 1013
STORY_INDEX = 74
STORY_ADDR = 0x800A7918

OP_NAMES = {
    0x00: "jump",
    0x01: "halt",
    0x05: "branch_if_zero",
    0x09: "alu",
    0x0A: "assign",
    0x20: "yield",
    0x31: "room_transfer",
}


def op_name(op: int) -> str:
    return OP_NAMES.get(op, f"op_{op:02X}")


def access_of(cmd: dict, index: int) -> list[tuple[str, int]]:
    """Return (mode, arg) list naming each binder slot that mentions `index`."""
    hits = []
    for slot, (mode, arg) in enumerate(zip(cmd["modes"], cmd["args"])):
        if mode == 2 and arg == index:
            hits.append(("read" if slot else "write", slot))
    return hits


def command_row(pc: int, cmd: dict) -> dict:
    return {
        "pc": f"{pc:08X}",
        "opcode": f"{cmd['opcode']:02X}",
        "name": op_name(cmd["opcode"]),
        "modes": list(cmd["modes"]),
        "args": [f"{a:08X}" for a in cmd["args"]],
        "args_dec": [a for a in cmd["args"]],
    }


def scan_room(rec: dict, meta: int, script: dict, base: int) -> dict:
    writers, readers, transfers = [], [], []
    per_module: dict[int, list[dict]] = {}
    for module in script["modules"]:
        cmds = module["commands"]
        module_base = base + module["start"]
        for cmd in cmds:
            # decode_script command offsets are blob-absolute, so the runtime
            # address is base + offset; module['start'] is only the branch base.
            pc = base + cmd["offset"]
            op = cmd["opcode"]
            modes, args = cmd["modes"], cmd["args"]
            row = command_row(pc, cmd)
            row["module"] = module["index"]
            if op == 0x0A:
                if modes and modes[0] == 2:
                    row["persist_dest"] = args[0]
                    if args[0] == STORY_INDEX:
                        writers.append(dict(row, kind="assign", value=f"{args[1]:08X}"))
                if len(modes) > 1 and modes[1] == 2 and len(args) > 1 and args[1] == STORY_INDEX:
                    readers.append(dict(row, kind="assign_source"))
            elif op == 0x09:
                if len(modes) > 1 and modes[1] == 2 and len(args) > 1 and args[1] == STORY_INDEX:
                    writers.append(dict(row, kind="alu_dest", sub=f"{args[0]:02X}"))
                for slot in (2, 3):
                    if (
                        len(modes) > slot
                        and modes[slot] == 2
                        and len(args) > slot
                        and args[slot] == STORY_INDEX
                    ):
                        readers.append(dict(row, kind=f"alu_operand{slot}"))
            elif op == 0x05:
                if modes and modes[0] == 2 and args and args[0] == STORY_INDEX:
                    target = module_base + ((args[1] << 1) & 0xFFFFFF)
                    readers.append(
                        dict(
                            row,
                            kind="branch_predicate",
                            branch_if_zero_to=f"{target:08X}",
                        )
                    )
                    # record the fallthrough and branch destination windows
            if op == 0x31 and args and modes == [0]:
                transfers.append(
                    dict(
                        row,
                        destination=decode_packed_name(args[0]).rstrip("\x00"),
                    )
                )
        per_module[module["index"]] = cmds
        # Annotate each transfer with the nearest preceding persist[74] writer
        # in the same module: the common "set story marker, then leave" shape.
        g74_writes = [w for w in writers if w["module"] == module["index"]]
        for transfer in transfers:
            if transfer["module"] != module["index"]:
                continue
            before = [w for w in g74_writes if int(w["pc"], 16) < int(transfer["pc"], 16)]
            if before:
                transfer["story_write_pc"] = before[-1]["pc"]
                transfer["story_value"] = before[-1].get("value", "?" + before[-1].get("sub", ""))
    return {
        "map_id": rec["map_id"],
        "name": rec["name"],
        "script_base": f"{base:08X}",
        "script_sha256": script["sha256"],
        "module_count": script["module_count"],
        "writers": writers,
        "readers": readers,
        "transfers": transfers,
        "_modules": per_module,
        "_base": base,
    }


def window(room: dict, center_pc: int, before: int, after: int) -> list[dict]:
    rows = []
    for module_index, cmds in room["_modules"].items():
        base = room["_base"]
        for cmd in cmds:
            pc = base + cmd["offset"]
            if pc < center_pc - before or pc > center_pc + after:
                continue
            rows.append(dict(command_row(pc, cmd), module=module_index))
    rows.sort(key=lambda r: int(r["pc"], 16))
    return rows


def main() -> int:
    exe = EXE.read_bytes()
    assert hashlib.sha1(exe).hexdigest() == EXE_SHA1, "EXE SHA-1"
    disc = find_disc(ROOT)
    assert disc is not None, "Disc 1 BIN required (local/pe_disc1.path)"
    rooms = []
    for rec in parse_field_table(exe):
        c0 = rec["meta"] & 0xFF
        c1 = (rec["meta"] >> 8) & 0xFFF
        c2 = rec["meta"] >> 20
        if c0 + c1 + c2 == 0:
            continue
        package = read_form1(disc, PE_IMG_LBA + rec["start"], c0 + c1 + c2)
        extracted = extract_script_from_package(package, rec["meta"])
        if extracted is None:
            continue
        off, raw = extracted
        script = decode_script(raw)
        if script.get("invalid") or not script["modules"]:
            continue
        base = 0x8018EFE8 + off - (c0 + c1) * 2048
        try:
            rooms.append(scan_room(rec, rec["meta"], script, base))
        except (KeyError, IndexError, struct.error):
            continue

    writers = [dict(room=r["name"], **w) for r in rooms for w in r["writers"]]
    readers = [dict(room=r["name"], **r2) for r in rooms for r2 in r["readers"]]
    routes = [dict(room=r["name"], **t) for r in rooms for t in r["transfers"]]

    def find_room(name: str) -> dict:
        return next(r for r in rooms if r["name"] == name)

    # Static room-transfer graph (destination token name -> room script name).
    graph: dict[str, list[dict]] = {}
    for route in routes:
        graph.setdefault(route["room"], []).append(route)
    known = {r["name"] for r in rooms}

    def reach(start: str) -> dict:
        seen = {start}
        order = [start]
        parent: dict[str, tuple[str, str]] = {}
        edge: dict[str, dict] = {}
        queue = [start]
        while queue:
            node = queue.pop(0)
            for route in graph.get(node, []):
                dest = route["destination"].lower()
                if dest in seen:
                    continue
                seen.add(dest)
                order.append(dest)
                parent[dest] = (node, route["pc"])
                edge[dest] = route
                queue.append(dest)
        return {"seen": order, "parent": parent, "edge": edge}

    reachability = {}
    for start, targets in (
        ("m0042i", ("m0191i", "m0037i", "m0374i", "m0091i", "m0092i")),
        ("m0351i", ("m0042i", "m0092i", "m0091i")),
    ):
        info = reach(start)
        chains = {}
        for target in targets:
            if target not in info["seen"]:
                chains[target] = None
                continue
            path = []
            node = target
            while node != start:
                prev, pc = info["parent"][node]
                path.append({"from": prev, "pc": pc, "to": node,
                             "story_value": info["edge"][node].get("story_value")})
                node = prev
            chains[target] = list(reversed(path))
        reachability[start] = {
            "reachable_rooms": len([n for n in info["seen"] if n in known]),
            "unresolved_destinations": sorted(
                {n for n in info["seen"] if n not in known}
            ),
            "chains": chains,
        }

    detail = {}
    for name in ("m0036i", "m0351i", "m0042i", "m0091i", "m0092i"):
        try:
            room = find_room(name)
        except StopIteration:
            continue
        centers = sorted({int(w["pc"], 16) for w in room["writers"]} |
                         {int(r2["pc"], 16) for r2 in room["readers"]} |
                         {int(t["pc"], 16) for t in room["transfers"]})
        detail[name] = {
            "script_base": room["script_base"],
            "script_sha256": room["script_sha256"],
            "windows": {f"{c:08X}": window(room, c, 0x140, 0x140) for c in centers},
        }

    payload = {
        "scope": (
            "static persist[74] (0x800A7918) reader/writer inventory and room "
            "transfer graph over all decoded field-table rooms; no execution or "
            "story-feasibility claim"
        ),
        "story_index": STORY_INDEX,
        "story_address": f"{STORY_ADDR:08X}",
        "rooms_scanned": len(rooms),
        "writer_count": len(writers),
        "reader_count": len(readers),
        "writers": writers,
        "readers": readers,
        "routes": routes,
        "reachability": reachability,
        "detail": detail,
    }

    # Pin the proven marker facts. These assertions are the fail-loud contract
    # of this oracle; see docs/ai_context/DAY1_DAY2_TRANSITIONS.md.
    def writers_of(value: str) -> list[tuple[str, str]]:
        return sorted((w["room"], w["pc"]) for w in writers if w.get("value") == value)

    assert writers_of("00000080") == [("m0036i", "801B82FC")], "Day-1 marker"
    assert writers_of("00000138") == [("m0091i", "801C3B38")], "Day-2 terminal marker"
    assert ("m0351i", "80191158") in writers_of("00000140"), "M0351I terminal hop"
    assert ("m0089i", "801DE640") in writers_of("00000140"), "M0089I terminal hop"
    assert writers_of("00000088") and ("m0351i", "80191114") in writers_of("00000088")
    assert writers_of("00000090") == [("m0042i", "801C70E4")], "Day-2 first room"
    route_index = {(r["room"], r["pc"]): r for r in routes}
    for room, pc, dest, story in (
        ("m0036i", "801B833C", "m0000i", "00000080"),
        ("m0351i", "80191124", "m0042i", "00000088"),
        ("m0091i", "801C3B58", "m0351i", "00000138"),
        ("m0351i", "80191168", "m0092i", "00000140"),
        ("m0089i", "801DE660", "m0092i", "00000140"),
    ):
        route = route_index[(room, pc)]
        assert route["destination"].lower() == dest, route
        assert route.get("story_value") == story, route
    chains = {
        start: info["chains"] for start, info in reachability.items()
    }
    assert chains["m0042i"]["m0191i"] is None, "M0191I must not be on the M0042I route"
    assert chains["m0042i"]["m0037i"] is None, "M0037I must not be on the M0042I route"
    assert chains["m0042i"]["m0374i"] is not None, "M0374I is on the M0042I route"
    assert chains["m0042i"]["m0091i"] is not None, "M0091I is on the M0042I route"
    assert chains["m0351i"]["m0042i"] is not None
    assert chains["m0351i"]["m0092i"] is not None
    out = ROOT / "local/live/day1-day2-transitions.json"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(payload, indent=2) + "\n")
    print(f"rooms scanned: {len(rooms)}")
    print(f"persist[74] writers: {len(writers)}")
    for w in writers:
        print(
            f"  {w['room']} mod{w['module']} @{w['pc']} "
            f"{w['name']}/{w['kind']} -> {w.get('value', w.get('sub',''))}"
        )
    print(f"persist[74] readers: {len(readers)}")
    print(f"room transfers: {len(routes)}")
    for start, info in reachability.items():
        print(f"reach from {start}: {info['reachable_rooms']} rooms")
        for target, chain in info["chains"].items():
            if chain is None:
                print(f"  {target}: NOT REACHABLE statically")
            else:
                hops = " -> ".join(
                    f"{h['from']}@{h['pc']}({h['story_value']})" for h in chain
                )
                print(f"  {target}: {hops} -> {target}")
    print(out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Opcodes used by the boot->Day-2 transition-critical field scripts.

Unlike devices/room/VR/people slots (a fixed executable body), **type-0 world
map slots are player data**: their script bytes come from the save file
(`D_800A77F0+0x50` spawns a 32 KB world-map arena), so the retail disc has no
type-0 bytecode to census. This tool censuses the *decodable* slots of the
rooms that the Day-1 completion marker and Day-2 terminal transition actually
pass through:

    m0004i  (field spawn; module 0 fights)   <- the mailbox -> m0005i arm
    m0377i / m0378i (the m0377i destination contract)
    m0036i  (Day-1 completion marker: persist[74]=0x80)
    m0351I  (the 0x80/0x138/0x140 selector hop)
    m0091i  (Day-2 terminal: persist[74]=0x138)
    m0191i / m0037i / m0374i (park scripts; M0191I/M0037I are NOT on the
                               M0042I route - see DAY1_DAY2_TRANSITIONS.md)

It then intersects the used opcodes with the dispatch table `D_800910A0`
(read from the SHA-1-exact EXE) and the handler VAs that
`pc_port/game/boot/func_80017018_port.c` claims to implement, so the
"transition-critical opcode boundary" can be stated as a concrete list rather
than a guess.

Output: local/live/field-vm-opcode-coverage.json
"""
from __future__ import annotations

import hashlib
import json
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/research"))
sys.path.insert(0, str(ROOT / "pc_port/tools"))
from pe_pst0_scan import (  # noqa: E402
    parse_field_table,
    extract_script_from_package,
    decode_script,
)
from pe_btl14_m0005i_publish_oracle import find_disc, read_form1  # noqa: E402

EXE = ROOT / "build/disc1.candidate.exe"
EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PE_IMG_LBA = 1013
VM_TABLE = 0x800910A0
VM_TABLE_MASK = 0x1FFF
PORT = ROOT / "pc_port/game/boot/func_80017018_port.c"

ROOMS = ("m0004i", "m0377i", "m0378i", "m0036i", "m0351i", "m0091i",
         "m0191i", "m0037i", "m0374i")


def va2off(va: int) -> int:
    return va - 0x80010000 + 0x800


def load_u32(exe: bytes, va: int) -> int:
    return struct.unpack_from("<I", exe, va2off(va))[0]


def ported_handler_vas() -> set[int]:
    """Handler VAs the port's dispatch chain compares against."""
    text = PORT.read_text()
    vas = {int(m, 16) for m in re.findall(r"0x(8001[0-9A-F]{4})u", text)}
    return vas


def main() -> int:
    exe = EXE.read_bytes()
    digest = hashlib.sha1(exe).hexdigest()
    assert digest == EXE_SHA1, f"EXE SHA-1 {digest}"

    table = {r["name"]: r for r in parse_field_table(exe)}
    disc = find_disc(ROOT)
    assert disc is not None, "Disc 1 BIN required (local/pe_disc1.path)"

    ported = ported_handler_vas()
    rooms: dict[str, dict] = {}
    used: dict[int, dict] = {}
    for name in ROOMS:
        rec = table.get(name)
        if rec is None:
            rooms[name] = {"status": "NO_TABLE_ENTRY"}
            continue
        c0 = rec["meta"] & 0xFF
        c1 = (rec["meta"] >> 8) & 0xFFF
        c2 = rec["meta"] >> 20
        if c0 + c1 + c2 == 0:
            rooms[name] = {"status": "EMPTY_PACKAGE"}
            continue
        package = read_form1(disc, PE_IMG_LBA + rec["start"], c0 + c1 + c2)
        extracted = extract_script_from_package(package, rec["meta"])
        if extracted is None:
            rooms[name] = {"status": "SCRIPT_NOT_FOUND"}
            continue
        off, raw = extracted
        script = decode_script(raw)
        ops: dict[int, int] = {}
        for mod in script["modules"]:
            for cmd in mod["commands"]:
                ops[cmd["opcode"]] = ops.get(cmd["opcode"], 0) + 1
        handler = {op: load_u32(exe, VM_TABLE + (op & VM_TABLE_MASK) * 4)
                   for op in ops}
        for op, count in ops.items():
            row = used.setdefault(op, {"op": op, "count": 0, "rooms": [],
                                       "handler": f"{handler[op]:08X}",
                                       "ported": handler[op] in ported})
            row["count"] += count
            row["rooms"].append(name)
        rooms[name] = {
            "status": "OK",
            "table_index": rec["index"],
            "map_id": rec["map_id"],
            "script_sha256": script["sha256"],
            "modules": script["module_count"],
            "opcodes": len(ops),
            "commands": sum(ops.values()),
            "unported": sorted(f"{o:02X}" for o in ops
                               if handler[o] not in ported),
        }

    # `handler in ported` is a VM-address match, not a proof the ported entry
    # handles every sub-path of that VA.  A few ported handlers are declared
    # via a GA_OP alias only, so also accept the op->VA identity from the
    # dispatch table when the port's comment/alias lists the opcode.
    port_text = PORT.read_text()
    alias_ops = {int(m, 16) for m in
                 re.findall(r"#define GA_OP([0-9A-F]+)\s", port_text)}

    unported = sorted((row for row in used.values() if not row["ported"]),
                      key=lambda r: r["op"])
    critical = [row for row in used.values()
                if row["handler"] not in ported
                and row["rooms"] and any(r in ("m0004i", "m0036i", "m0351i",
                                               "m0091i", "m0377i", "m0378i")
                                         for r in row["rooms"])]

    payload = {
        "scope": (
            "opcode census of the decodable transition-critical field scripts "
            "+ D_800910A0 handler resolution against the port's claimed VAs; "
            "type-0 world-map bytecode is player data and is not on the disc"
        ),
        "exe_sha1": digest,
        "vm_table": f"{VM_TABLE:08X}",
        "alias_op_defines": sorted(f"{o:02X}" for o in alias_ops),
        "rooms": rooms,
        "opcodes_used": sorted(used.values(), key=lambda r: -r["count"]),
        "unported_opcodes": unported,
        "transition_critical_unported": critical,
    }
    out = ROOT / "local/live/field-vm-opcode-coverage.json"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(payload, indent=2) + "\n")

    for name in ROOMS:
        row = rooms[name]
        if row["status"] == "OK":
            print(f"  {name}: {row['modules']} mods, {row['commands']} cmds, "
                  f"{row['opcodes']} ops, unported={row['unported']}")
        else:
            print(f"  {name}: {row['status']}")
    print(f"unported opcodes across critical scripts: "
          f"{[f'{r['op']:02X}' for r in unported]}")
    print(out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

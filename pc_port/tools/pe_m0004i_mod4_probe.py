#!/usr/bin/env python3
"""Decode the m0004i field script and dump the frontier region.

Read-only diagnostic used to find the m0004i module-4 gate. Prints every
command of the room with its runtime address, opcode, modes, args, plus the
raw op-77 (0x77) rectangle volumes with signed 16.16 world bounds. No claim:
this only decodes the retail script, it never executes it.
"""
import hashlib
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

EXE = ROOT / "build/extracted/disc1/SLUS_006.62"
EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PE_IMG_LBA = 1013

OP_NAMES = {
    0x00: "jump",
    0x01: "halt",
    0x05: "branch_if_zero",
    0x09: "alu",
    0x0A: "assign",
    0x20: "yield",
    0x31: "room_transfer",
    0x5E: "copy_aya_pos",
    0x77: "op77_rect",
}


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def sx(v):
    v &= 0xFFFFFFFF
    if v & 0x80000000:
        return v - 0x100000000
    return v


def dump_room(room_name: str, exe: bytes, disc) -> int:
    for rec in parse_field_table(exe):
        if rec["name"] != room_name:
            continue
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
        print(f"room={room_name} base={base:08X} "
              f"sha256={script['sha256']} modules={script['module_count']}")
        print(f"module_offsets={[f'{o:04X}' for o in script['module_offsets']]}")
        for module in script["modules"]:
            mstart = base + module["start"]
            mend = base + module["end"]
            print(f"\n=== module {module['index']} "
                  f"[{mstart:08X}..{mend:08X}) ===")
            for cmd in module["commands"]:
                pc = base + cmd["offset"]
                op = cmd["opcode"]
                name = OP_NAMES.get(op, f"op_{op:02X}")
                args = cmd["args"]
                line = (f"  {pc:08X} op={op:02X} {name:14s} "
                        f"modes={cmd['modes']} args="
                        + " ".join(f"{a:08X}" for a in args))
                # jump / branch targets: module_base + (arg << 1).
                if op == 0x00 and args:
                    line += f"  -> {mstart + ((args[0] << 1) & 0xFFFFFF):08X}"
                if op == 0x05 and len(args) >= 2:
                    line += (f"  if !arg0 -> "
                             f"{mstart + ((args[1] << 1) & 0xFFFFFF):08X}")
                if op == 0x31 and args:
                    line += f"  dest={decode_packed_name(args[0]).rstrip(chr(0))}"
                if op == 0x77 and len(args) >= 4:
                    # Best-effort: four signed 16.16 corner coords.
                    coords = [sx(a) >> 16 for a in args[:4]]
                    line += f"  coords16.16={coords}"
                print(line)
        return 0
    print(f"room {room_name} not found")
    return 1


def main() -> int:
    exe = EXE.read_bytes()
    assert hashlib.sha1(exe).hexdigest() == EXE_SHA1, "EXE SHA-1"
    disc = find_disc(ROOT)
    assert disc is not None, "Disc 1 BIN required"
    rooms = sys.argv[1:] or ["m0004i"]
    rc = 0
    for name in rooms:
        rc |= dump_room(name, exe, disc)
    return rc


if __name__ == "__main__":
    raise SystemExit(main())

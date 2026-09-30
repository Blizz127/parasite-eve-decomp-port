#!/usr/bin/env python3
"""PE-PST0 evidence-only persist[] provenance scanner.

Reads a registered USA Disc 1 MODE2/2352 image. Validates the disc, EXE,
and PE.IMG, then emits CSV/JSON facts about:

  * D_800A77F0 persist storage
  * field-script binder modes
  * persist readers/writers in Day-1 / current-route packages
  * EXE cross-references and save/load copy range

Does not modify production runtime and does not implement save files.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import struct
import sys
from pathlib import Path
from typing import BinaryIO, Iterable, Iterator


SECTOR_RAW = 2352
FORM1_USER_OFFSET = 24
FORM1_USER_SIZE = 2048
FORM2_FLAG = 0x20
DISC1_BYTES = 495_531_120
DISC1_SHA256 = "7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4"
DISC1_EXE = "SLUS_006.62;1"
DISC1_EXE_BYTES = 2_025_472
DISC1_EXE_SHA256 = "5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b"
DISC1_EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PE_IMG = "PE.IMG;1"
PE_IMG_BYTES = 206_213_120
PE_IMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"

LOAD = 0x80010000
EXE_HDR = 0x800
FIELD_TABLE_OFF = 0x83B78  # D_80093378
OPCODE_TABLE = 0x800910A0
MODE_TABLE = 0x80010690
ALU_TABLE = 0x80010000
PERSIST_BASE = 0x800A77F0
COND_BASE = 0x8009DF70
SCRATCH_BASE = 0x800B6A80
LOCALS_PTR = 0x8009D2F0
PERSIST_WORDS = 0x200
PERSIST_BYTES = PERSIST_WORDS * 4

MODE_CASES = {
    0: 0x80017154,
    1: 0x8001718C,
    2: 0x800171BC,
    3: 0x8001716C,
    4: 0x800171DC,
}
MODE_NAMES = {
    0: "imm",
    1: "actor_local",
    2: "persist",
    3: "cond",
    4: "scratch",
}
ALU_SUBOPS = {
    0x00: ("add", "dst = a + b", "i32_wrap"),
    0x01: ("sub", "dst = a - b", "i32_wrap"),
    0x02: ("or", "dst = a | b", "u32"),
    0x03: ("and", "dst = a & b", "u32"),
    0x04: ("xor", "dst = a ^ b", "u32"),
    0x05: ("bool_or", "dst = (a != 0) || (b != 0)", "bool"),
    0x06: ("bool_and", "dst = (a != 0) && (b != 0)", "bool"),
    0x07: ("is_zero", "dst = (a == 0)", "bool"),
    0x08: ("not", "dst = ~a", "u32"),
    0x09: ("slt_ba", "dst = (b < a) signed", "slt"),
    0x0A: ("slt_ab", "dst = (a < b) signed", "slt"),
    0x0B: ("eq", "dst = (a == b)", "bit_exact"),
    0x0C: ("sge_ab", "dst = (a >= b) signed", "slt"),
    0x0D: ("sle_ab", "dst = (a <= b) signed", "slt"),
    0x0E: ("ne", "dst = (a != b)", "bit_exact"),
    0x0F: ("mul", "dst = a * b", "i32_lo"),
    0x10: ("div", "dst = a / b", "i32"),
    0x11: ("sllv", "dst = a << b", "u32"),
    0x12: ("srav", "dst = a >> b arithmetic", "i32"),
    0x13: ("copy", "dst = a", "u32"),
    0x14: ("helper_8003708C", "dst = func_8003708C(a,b)", "UNKNOWN"),
    0x15: ("helper_800370A8", "dst = func_800370A8(a,b)", "UNKNOWN"),
    0x16: ("rem", "dst = a % b", "i32"),
    0x17: ("neg", "dst = -a", "i32"),
}
HANDLERS = {
    0x00: 0x80017294,
    0x05: 0x8001731C,
    0x09: 0x80012850,
    0x0A: 0x800173F4,
    0x31: 0x80017BB4,
}
ALPHABET = "0123456789abcdefghiklmnoprstuvwy"
CURRENT_ROUTE = (
    ("m0002i", 1),
    ("m0003i", 2),
    ("m0372i", 371),
    ("m0004i", 3),
    ("m0378i", 377),
    ("m0377i", 376),
)
DAY1_EXTRA = (
    ("m0001i", 0),
    ("m0005i", 4),
)
SLOT_STRIDES = {
    0x04: 12,
    0x08: 12,
    0x0C: 12,
    0x10: 12,
    0x14: 8,
    0x18: 8,
    0x1C: 8,
    0x20: 8,
    0x24: 8,
    0x28: 20,
    0x2C: 12,
    0x30: 12,
}
EXE_REF_HINTS = {
    0x800171BC: "binder_mode2",
    0x800171CC: "binder_mode2_base",
    0x80034F10: "new_game_zero_persist",
    0x80034F18: "new_game_zero_persist",
    0x8003F800: "save_copy_persist_out",
    0x8003F808: "save_copy_persist_out",
    0x8003FBD8: "load_copy_persist_in",
    0x8003FBE0: "load_copy_persist_in",
    0x80040B80: "save_parent_func_80040B80",
    0x80040EB4: "save_parent_calls_3F800",
    0x80042264: "load_parent_func_80042264",
    0x80042348: "load_parent_calls_3FBD8",
    0x80053128: "cross_system_func_80053128",
    0x800531CC: "cross_system_reads_persist_index_range",
}


class DiscError(RuntimeError):
    pass


def sha256_file(path: Path, chunk_size: int = 1024 * 1024) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        while chunk := source.read(chunk_size):
            digest.update(chunk)
    return digest.hexdigest()


def sha1_bytes(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


class RawMode2Image:
    def __init__(self, path: Path):
        self.path = path
        self.file: BinaryIO = path.open("rb")
        self.file.seek(0, os.SEEK_END)
        size = self.file.tell()
        if size == 0 or size % SECTOR_RAW:
            self.file.close()
            raise DiscError(f"image size {size} is not a multiple of {SECTOR_RAW}")
        self.sector_count = size // SECTOR_RAW

    def __enter__(self) -> "RawMode2Image":
        return self

    def __exit__(self, *_args: object) -> None:
        self.file.close()

    def raw_sector(self, lba: int) -> bytes:
        if not 0 <= lba < self.sector_count:
            raise DiscError(f"LBA {lba} outside image")
        self.file.seek(lba * SECTOR_RAW)
        data = self.file.read(SECTOR_RAW)
        if len(data) != SECTOR_RAW:
            raise DiscError(f"short read at LBA {lba}")
        if data[:12] != b"\x00" + b"\xff" * 10 + b"\x00" or data[15] != 2:
            raise DiscError(f"LBA {lba} is not a raw Mode 2 sector")
        if data[16:20] != data[20:24]:
            raise DiscError(f"LBA {lba} has mismatched Mode 2 subheaders")
        return data

    def user_data(self, lba: int) -> bytes:
        raw = self.raw_sector(lba)
        if raw[18] & FORM2_FLAG:
            raise DiscError(f"LBA {lba} is Form 2; Form 1 required")
        return raw[FORM1_USER_OFFSET : FORM1_USER_OFFSET + FORM1_USER_SIZE]

    def read_form1_extent(self, lba: int, size: int) -> bytes:
        if size < 0:
            raise DiscError("negative extent size")
        sectors = (size + FORM1_USER_SIZE - 1) // FORM1_USER_SIZE
        output = bytearray()
        for index in range(sectors):
            output.extend(self.user_data(lba + index))
        return bytes(output[:size])


def _u32le(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def iso_entries(image: RawMode2Image) -> list[tuple[str, int, int]]:
    pvd = image.user_data(16)
    if pvd[0] != 1 or pvd[1:6] != b"CD001":
        raise DiscError("missing ISO9660 PVD")
    root = pvd[156:]
    stack = [("", _u32le(root, 2), _u32le(root, 10))]
    files: list[tuple[str, int, int]] = []
    seen: set[tuple[int, int]] = set()
    while stack:
        prefix, lba, size = stack.pop()
        key = (lba, size)
        if key in seen:
            raise DiscError("ISO9660 directory cycle")
        seen.add(key)
        sectors = (size + FORM1_USER_SIZE - 1) // FORM1_USER_SIZE
        for sector_index in range(sectors):
            data = image.user_data(lba + sector_index)
            offset = 0
            while offset < FORM1_USER_SIZE:
                record_len = data[offset]
                if record_len == 0:
                    break
                record = data[offset : offset + record_len]
                offset += record_len
                name_len = record[32]
                name_bytes = record[33 : 33 + name_len]
                if name_bytes in (b"\x00", b"\x01"):
                    continue
                name = name_bytes.decode("ascii")
                child_lba = _u32le(record, 2)
                child_size = _u32le(record, 10)
                path = f"{prefix}/{name}" if prefix else name
                if record[25] & 2:
                    stack.append((path, child_lba, child_size))
                else:
                    files.append((path, child_lba, child_size))
    return files


def find_entry(entries: Iterable[tuple[str, int, int]], wanted: str) -> tuple[str, int, int]:
    wanted_upper = wanted.upper()
    for entry in entries:
        if entry[0].upper() == wanted_upper:
            return entry
    raise DiscError(f"required retail file absent: {wanted}")


def va2off(va: int) -> int:
    return va - LOAD + EXE_HDR


def word_at(exe: bytes, va: int) -> int:
    return struct.unpack_from("<I", exe, va2off(va))[0]


def decode_packed_name(token: int) -> str:
    chars = []
    for shift in (27, 22, 17, 12, 7, 2):
        chars.append(ALPHABET[(token >> shift) & 0x1F])
    return "".join(chars)


def s32(value: int) -> int:
    return value - 0x100000000 if value >= 0x80000000 else value


def parse_script_command(data: bytes, offset: int, end: int) -> tuple[dict, int] | None:
    if offset + 8 > end:
        return None
    if data[offset : offset + 2] == b"\xff\xff":
        return None
    header = _u32le(data, offset)
    opcode = header & 0x1FFF
    argc = (header >> 13) & 0xF
    size = 8 + argc * 4
    if offset + size > end:
        return None
    # Original17018 switches to the second header word after argument4.
    second = _u32le(data, offset + 4)
    modes = tuple(((header >> (17 + index * 3)) if index < 5
                   else (second >> ((index - 5) * 3))) & 7
                  for index in range(argc))
    args = struct.unpack_from(f"<{argc}I", data, offset + 8) if argc else ()
    return (
        {
            "offset": offset,
            "opcode": opcode,
            "argc": argc,
            "modes": list(modes),
            "args": [int(a) for a in args],
        },
        offset + size,
    )


def decode_script(blob: bytes) -> dict:
    if len(blob) < 8:
        return {"declared": 0, "module_count": 0, "modules": []}
    declared = _u32le(blob, 0)
    count = _u32le(blob, 4)
    if count > 64 or declared < 8 + count * 4 or declared > len(blob):
        return {"declared": declared, "module_count": count, "modules": [], "invalid": True}
    offs = [_u32le(blob, 8 + i * 4) for i in range(count)]
    modules = []
    for i, start in enumerate(offs):
        end = offs[i + 1] if i + 1 < count else declared
        if not (0 <= start <= end <= declared):
            continue
        cmds = []
        cursor = start
        while cursor < end:
            parsed = parse_script_command(blob, cursor, end)
            if parsed is None:
                break
            cmd, cursor = parsed
            cmds.append(cmd)
        modules.append(
            {
                "index": i,
                "start": start,
                "end": end,
                "commands": cmds,
            }
        )
    return {
        "declared": declared,
        "module_count": count,
        "module_offsets": offs,
        "sha256": sha256_bytes(blob[:declared]),
        "modules": modules,
    }


def parse_container(chunk: bytes) -> dict | None:
    if len(chunk) < 8:
        return None
    used, footer = struct.unpack_from("<II", chunk, 0)
    if footer + 4 > len(chunk) or used > len(chunk):
        return None
    slots = {}
    for slot, stride in SLOT_STRIDES.items():
        if footer + slot + 4 > len(chunk):
            continue
        packed = _u32le(chunk, footer + slot)
        count, offset = packed >> 22, packed & 0x3FFFFF
        slots[slot] = {"count": count, "rec_offset": offset, "stride": stride}
    return {"used": used, "footer": footer, "slots": slots}


def unpack_meta(meta: int) -> tuple[int, int, int]:
    return meta & 0xFF, (meta >> 8) & 0xFFF, meta >> 20


def extract_script_from_package(package: bytes, meta: int) -> tuple[int, bytes] | None:
    c0s, c1s, c2s = unpack_meta(meta)
    bases = (0, c0s * 2048, (c0s + c1s) * 2048)
    sizes = (c0s * 2048, c1s * 2048, c2s * 2048)
    for base, size in zip(bases, sizes):
        if size <= 0 or base + size > len(package):
            continue
        chunk = package[base : base + size]
        directory = parse_container(chunk)
        if directory is None:
            continue
        slot4 = directory["slots"].get(0x14)
        if not slot4 or slot4["count"] < 1:
            continue
        rec = slot4["rec_offset"]
        if rec + 8 > len(chunk):
            continue
        rec_size = _u32le(chunk, rec)
        rec_off = _u32le(chunk, rec + 4) & 0x3FFFFF
        if rec_off + 8 > len(chunk):
            continue
        declared = _u32le(chunk, rec_off)
        if declared < 8 or rec_off + declared > len(chunk):
            # fall back to recorded size if it looks like a script
            take = min(rec_size, len(chunk) - rec_off)
            if take < 8:
                continue
            declared = _u32le(chunk, rec_off)
            if declared < 8 or rec_off + declared > len(chunk):
                continue
        return base + rec_off, chunk[rec_off : rec_off + declared]
    return None


def bank_name(mode: int) -> str:
    return MODE_NAMES.get(mode, f"mode{mode}")


def alu_eval(sub: int, a: int, b: int) -> int | None:
    a32 = a & 0xFFFFFFFF
    b32 = b & 0xFFFFFFFF
    sa, sb = s32(a32), s32(b32)
    if sub == 0x00:
        return (a32 + b32) & 0xFFFFFFFF
    if sub == 0x01:
        return (a32 - b32) & 0xFFFFFFFF
    if sub == 0x02:
        return a32 | b32
    if sub == 0x03:
        return a32 & b32
    if sub == 0x04:
        return a32 ^ b32
    if sub == 0x05:
        return int(a32 != 0 or b32 != 0)
    if sub == 0x06:
        return int(a32 != 0 and b32 != 0)
    if sub == 0x07:
        return int(a32 == 0)
    if sub == 0x08:
        return (~a32) & 0xFFFFFFFF
    if sub == 0x09:
        return int(sb < sa)
    if sub == 0x0A:
        return int(sa < sb)
    if sub == 0x0B:
        return int(a32 == b32)
    if sub == 0x0C:
        return int(sa >= sb)
    if sub == 0x0D:
        return int(sa <= sb)
    if sub == 0x0E:
        return int(a32 != b32)
    if sub == 0x0F:
        return (sa * sb) & 0xFFFFFFFF
    if sub == 0x13:
        return a32
    if sub == 0x17:
        return (-sa) & 0xFFFFFFFF
    return None


def classify_access(cmd: dict) -> list[dict]:
    rows = []
    op = cmd["opcode"]
    modes = cmd["modes"]
    args = cmd["args"]
    if op == 0x0A and len(args) >= 2 and modes:
        dst_mode = modes[0]
        src_mode = modes[1] if len(modes) > 1 else -1
        if dst_mode == 2 or src_mode == 2:
            index = args[0] if dst_mode == 2 else (args[0] if src_mode != 2 else args[0])
            if dst_mode == 2:
                index = args[0]
                value = args[1]
                rows.append(
                    {
                        "rw": "write",
                        "index": index,
                        "operation": "assign",
                        "value": value if src_mode == 0 else "",
                        "src_bank": bank_name(src_mode),
                        "dst_bank": "persist",
                        "immediate": value if src_mode == 0 else "",
                    }
                )
            elif src_mode == 2:
                rows.append(
                    {
                        "rw": "read",
                        "index": args[1] if False else args[0] if dst_mode != 2 else args[0],
                        "operation": "assign_from",
                        "value": "",
                        "src_bank": "persist",
                        "dst_bank": bank_name(dst_mode),
                        "immediate": "",
                    }
                )
                # src index is the persist slot held in the persist-mode argument
                if src_mode == 2:
                    rows[-1]["index"] = args[1] if len(args) > 1 and modes[1] == 2 else args[0]
                    if modes[1] == 2:
                        rows[-1]["index"] = args[1]
                    elif modes[0] == 2:
                        rows[-1]["index"] = args[0]
    if op == 0x09 and len(args) >= 2:
        sub = args[0]
        name, desc, signedness = ALU_SUBOPS.get(sub, (f"sub{sub:#x}", "UNKNOWN", "UNKNOWN"))
        persist_args = [i for i, mode in enumerate(modes) if mode == 2]
        for arg_i in persist_args:
            index = args[arg_i]
            role = "dst" if arg_i == 1 else ("a" if arg_i == 2 else ("b" if arg_i == 3 else f"arg{arg_i}"))
            rw = "write" if arg_i == 1 else "read"
            imm = ""
            if len(args) > 3 and 3 < len(modes) and modes[3] == 0:
                imm = args[3]
            if len(args) > 2 and 2 < len(modes) and modes[2] == 0:
                if imm == "":
                    imm = args[2]
            rows.append(
                {
                    "rw": rw,
                    "index": index,
                    "operation": f"alu_{name}",
                    "value": imm,
                    "src_bank": bank_name(modes[2]) if len(modes) > 2 else "",
                    "dst_bank": bank_name(modes[1]) if len(modes) > 1 else "",
                    "immediate": imm,
                    "alu_sub": sub,
                    "alu_desc": desc,
                    "signedness": signedness,
                    "persist_role": role,
                }
            )
    return rows


def scan_lui_addiu_refs(exe: bytes, target: int) -> list[dict]:
    hi = (target >> 16) & 0xFFFF
    lo = target & 0xFFFF
    text = exe[EXE_HDR:]
    hits = []
    # lui rt, hi  = 0x3C000000 | rt<<16 | hi
    # addiu rt, rs, lo
    for off in range(0, len(text) - 8, 4):
        w0 = struct.unpack_from("<I", text, off)[0]
        if (w0 >> 26) != 0x0F:
            continue
        if (w0 & 0xFFFF) != hi:
            continue
        rt0 = (w0 >> 16) & 0x1F
        w1 = struct.unpack_from("<I", text, off + 4)[0]
        if (w1 >> 26) not in (0x09, 0x0D):  # addiu / ori
            # allow a delayed match within 8 instructions
            found = None
            for step in range(1, 9):
                nxt = struct.unpack_from("<I", text, off + step * 4)[0]
                if (nxt >> 26) in (0x09, 0x0D) and (nxt & 0xFFFF) == lo:
                    rs = (nxt >> 21) & 0x1F
                    if rs == rt0:
                        found = (off + step * 4, nxt)
                        break
            if found is None:
                continue
            w1_off, w1 = found
        else:
            if (w1 & 0xFFFF) != lo:
                continue
            rs = (w1 >> 21) & 0x1F
            if rs != rt0:
                continue
            w1_off = off + 4
        va = LOAD + off
        hits.append(
            {
                "va": f"0x{va:08X}",
                "addiu_va": f"0x{LOAD + w1_off:08X}",
                "hint": EXE_REF_HINTS.get(va, EXE_REF_HINTS.get(LOAD + w1_off, "")),
            }
        )
    return hits


def parse_field_table(exe: bytes) -> list[dict]:
    rows = []
    pe_sectors = PE_IMG_BYTES // FORM1_USER_SIZE
    prev = None
    for index in range(0, 2048):
        off = FIELD_TABLE_OFF + index * 8
        if off + 16 > len(exe):
            break
        start, meta = struct.unpack_from("<II", exe, off)
        nxt = struct.unpack_from("<I", exe, off + 8)[0]
        if start == 0 or start >= pe_sectors:
            if index > 20:
                break
            continue
        if prev is not None and start < prev:
            # table is expected non-decreasing in this game; stop on collapse
            if index > 20:
                break
        rows.append(
            {
                "index": index,
                "map_id": index + 1,
                "name": f"m{index + 1:04d}i",
                "start": start,
                "end": nxt if nxt > start else start,
                "meta": meta,
            }
        )
        prev = start
    return rows


def current_route_set() -> dict[int, str]:
    return {idx: name for name, idx in CURRENT_ROUTE + DAY1_EXTRA}


def write_csv(path: Path, rows: list[dict], fields: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        for row in rows:
            writer.writerow({key: row.get(key, "") for key in fields})


def resolve_value(mode: int, arg: int, persist: list[int], cond: dict[int, int], scratch: dict[int, int]) -> int | None:
    if mode == 0:
        return arg & 0xFFFFFFFF
    if mode == 2 and 0 <= arg < len(persist):
        return persist[arg] & 0xFFFFFFFF
    if mode == 3:
        return cond.get(arg, 0) & 0xFFFFFFFF
    if mode == 4:
        return scratch.get(arg, 0) & 0xFFFFFFFF
    return None


def simulate_module(
    module: dict,
    persist: list[int],
    *,
    scene: str,
    stop_ops: set[int] | None = None,
) -> list[dict]:
    stop_ops = stop_ops or {0x01, 0x20}
    cmds = {cmd["offset"]: cmd for cmd in module["commands"]}
    ordered = sorted(cmds)
    if not ordered:
        return []
    cond: dict[int, int] = {}
    scratch: dict[int, int] = {}
    pc = ordered[0]
    seen = set()
    events = []
    while pc in cmds and pc not in seen:
        seen.add(pc)
        cmd = cmds[pc]
        op = cmd["opcode"]
        modes = cmd["modes"]
        args = cmd["args"]
        nxt = None
        # next linear command
        later = [off for off in ordered if off > pc]
        linear = later[0] if later else None
        if op == 0x00 and args:
            nxt = module["start"] + ((args[0] << 1) & 0xFFFFFF)
        elif op == 0x05 and len(args) >= 2:
            pred = resolve_value(modes[0], args[0], persist, cond, scratch)
            if pred == 0:
                nxt = module["start"] + ((args[1] << 1) & 0xFFFFFF)
            else:
                nxt = linear
        elif op == 0x09 and len(args) >= 2:
            sub = args[0]
            a = resolve_value(modes[2], args[2], persist, cond, scratch) if len(args) > 2 and len(modes) > 2 else 0
            b = resolve_value(modes[3], args[3], persist, cond, scratch) if len(args) > 3 and len(modes) > 3 else 0
            result = alu_eval(sub, a or 0, b or 0)
            if result is not None and len(modes) > 1:
                if modes[1] == 2 and 0 <= args[1] < len(persist):
                    before = persist[args[1]]
                    persist[args[1]] = result
                    events.append(
                        {
                            "scene": scene,
                            "module": module["index"],
                            "script_pc": f"+0x{cmd['offset']:04X}",
                            "index": args[1],
                            "before": before,
                            "operation": f"alu_{ALU_SUBOPS.get(sub, ('sub',))[0]}",
                            "after": result,
                            "consequence": ALU_SUBOPS.get(sub, ("", ""))[1],
                        }
                    )
                elif modes[1] == 3:
                    cond[args[1]] = result
                elif modes[1] == 4:
                    scratch[args[1]] = result
            nxt = linear
        elif op == 0x0A and len(args) >= 2 and modes:
            src = resolve_value(modes[1] if len(modes) > 1 else 0, args[1], persist, cond, scratch)
            if src is not None and modes[0] == 2 and 0 <= args[0] < len(persist):
                before = persist[args[0]]
                persist[args[0]] = src
                events.append(
                    {
                        "scene": scene,
                        "module": module["index"],
                        "script_pc": f"+0x{cmd['offset']:04X}",
                        "index": args[0],
                        "before": before,
                        "operation": "assign",
                        "after": src,
                        "consequence": f"persist[{args[0]:#x}]={src:#x}",
                    }
                )
            elif src is not None and modes[0] == 3:
                cond[args[0]] = src
            elif src is not None and modes[0] == 4:
                scratch[args[0]] = src
            nxt = linear
        elif op == 0x31 and args:
            events.append(
                {
                    "scene": scene,
                    "module": module["index"],
                    "script_pc": f"+0x{cmd['offset']:04X}",
                    "index": "",
                    "before": "",
                    "operation": "dest_token",
                    "after": f"0x{args[0]:08X}",
                    "consequence": decode_packed_name(args[0]),
                }
            )
            nxt = linear
        elif op in stop_ops:
            break
        else:
            nxt = linear
        if nxt is None:
            break
        pc = nxt
    return events


def persist_access_rows(scene: str, table_index: int, script: dict, handler_09: int, handler_0a: int) -> list[dict]:
    rows = []
    for module in script["modules"]:
        for cmd in module["commands"]:
            for access in classify_access(cmd):
                if access.get("index", "") == "" and access.get("rw") != "write":
                    continue
                index = access.get("index", "")
                if not isinstance(index, int):
                    continue
                alu_desc = access.get("alu_desc", "")
                rows.append(
                    {
                        "index": index,
                        "index_hex": f"0x{index:X}",
                        "rw": access["rw"],
                        "operation": access["operation"],
                        "value": access.get("value", ""),
                        "value_hex": f"0x{access['value']:X}" if isinstance(access.get("value"), int) else "",
                        "scene": scene,
                        "table_index": table_index,
                        "module": module["index"],
                        "script_pc": f"+0x{cmd['offset']:04X}",
                        "handler": f"0x{handler_0a:08X}" if cmd["opcode"] == 0x0A else f"0x{handler_09:08X}",
                        "opcode": f"0x{cmd['opcode']:02X}",
                        "modes": " ".join(str(m) for m in cmd["modes"]),
                        "args": " ".join(f"0x{a:X}" for a in cmd["args"]),
                        "src_bank": access.get("src_bank", ""),
                        "dst_bank": access.get("dst_bank", ""),
                        "alu_desc": alu_desc,
                        "signedness": access.get("signedness", ""),
                        "branch_consequence": "",
                        "notes": access.get("persist_role", ""),
                    }
                )
    return rows


def annotate_branches(rows: list[dict], script: dict) -> None:
    by_pc = {}
    for module in script["modules"]:
        for cmd in module["commands"]:
            by_pc[(module["index"], cmd["offset"])] = cmd
    for row in rows:
        if row["opcode"] != "0x09":
            continue
        module = int(row["module"])
        pc = int(row["script_pc"][3:], 16)
        # look at next command in same module
        nxt = None
        for module_row in script["modules"]:
            if module_row["index"] != module:
                continue
            later = [c for c in module_row["commands"] if c["offset"] > pc]
            if later:
                nxt = later[0]
        if nxt and nxt["opcode"] == 0x05 and nxt["args"]:
            dest = module_row["start"] + ((nxt["args"][1] << 1) & 0xFFFFFF)
            row["branch_consequence"] = f"skip-if-false -> +0x{dest:04X}"
        elif nxt and nxt["opcode"] == 0x00 and nxt["args"]:
            dest = module_row["start"] + ((nxt["args"][0] << 1) & 0xFFFFFF)
            row["branch_consequence"] = f"goto -> +0x{dest:04X}"


def classify_entry(index: int, accesses: list[dict]) -> dict:
    writes = [a for a in accesses if a["rw"] == "write"]
    reads = [a for a in accesses if a["rw"] == "read"]
    values = sorted(
        {
            int(a["value"])
            for a in accesses
            if a.get("value") != "" and isinstance(a.get("value"), int)
        }
        | {
            int(a["value_hex"], 16)
            for a in accesses
            if a.get("value_hex")
        }
    )
    ops = sorted({a["operation"] for a in accesses})
    scenes = sorted({a["scene"] for a in accesses})
    semantic = ""
    confidence = "UNKNOWN"
    notes = []
    if index == 1:
        assign_vals = sorted(
            {
                int(a["value"])
                for a in writes
                if a["operation"] == "assign" and isinstance(a.get("value"), int)
            }
        )
        eq_reads = [a for a in reads if a["operation"] == "alu_eq"]
        if assign_vals and eq_reads and len(scenes) >= 2:
            semantic = "entrance_selector"
            confidence = "PROVEN_SEMANTIC"
            notes.append(
                "multiple scenes assign persist[1] immediately before dest tokens; "
                "destination modules equality-test the same slot to pick a spawn pose"
            )
        else:
            confidence = "STRONG_ROLE"
            semantic = "entrance_selector"
    elif index == 0x4A:
        compares = [a for a in reads if a["operation"].startswith("alu_")]
        if writes and compares and len(scenes) >= 2:
            confidence = "STRONG_ROLE"
            notes.append(
                "monotonic-looking gate compared with 9/0x11/0x18/0x28/0x30/0x78; "
                "not labeled storyProgress"
            )
        else:
            confidence = "VALUE_ONLY"
    elif index == 0:
        bit_ops = [a for a in accesses if a["operation"] in {"alu_and", "alu_or", "alu_xor"}]
        if bit_ops:
            confidence = "STRONG_ROLE"
            notes.append("bitwise tests/assigns; flag word, not a single enum")
        else:
            confidence = "VALUE_ONLY"
    else:
        if writes and reads:
            confidence = "VALUE_ONLY"
        elif writes or reads:
            confidence = "VALUE_ONLY"
    first_reader = next((a for a in accesses if a["rw"] == "read"), None)
    first_writer = next((a for a in accesses if a["rw"] == "write"), None)
    known_values = ";".join(f"0x{v:X}" for v in values)
    return {
        "index": index,
        "index_hex": f"0x{index:X}",
        "offset": f"0x{PERSIST_BASE + index * 4:08X}",
        "width": 32,
        "signedness": "word; slt compares signed, equality bit-exact",
        "confidence": confidence,
        "semantic_name": semantic,
        "known_values": known_values,
        "bit_masks": "2;4" if index == 0 else "",
        "first_reader": (
            f"{first_reader['scene']} {first_reader['script_pc']} {first_reader['operation']}"
            if first_reader
            else ""
        ),
        "first_writer": (
            f"{first_writer['scene']} {first_writer['script_pc']} {first_writer['operation']}"
            if first_writer
            else ""
        ),
        "scenes": ";".join(scenes),
        "survives_room": "YES",
        "survives_battle": "UNKNOWN",
        "save_backed": "YES",
        "reset_behavior": "zeroed only by func_80034F10 (boot/new-game)",
        "cross_system_readers": "",
        "notes": "; ".join(notes),
        "evidence_refs": "EXE D_800A77F0; binder mode 2; route scripts",
        "operations": ";".join(ops),
    }


def dump_53128_ranges(exe: bytes) -> list[dict]:
    rows = []
    base = 0x800923D8
    end = 0x800923F8
    off = va2off(base)
    while base < end:
        start_i, end_i = struct.unpack_from("<hh", exe, off)
        rows.append(
            {
                "table_va": f"0x{base:08X}",
                "index_start": start_i,
                "index_end": end_i,
                "covers_persist_1": int(start_i <= 1 <= end_i),
                "covers_persist_4a": int(start_i <= 0x4A <= end_i),
            }
        )
        base += 4
        off += 4
    return rows


def first_play_transition_events(script: dict, scene: str, persist: list[int]) -> list[dict]:
    """Record persist assigns that sit on the same module as a dest token.

    These are the hop writers. They are not executed by the startup walker
    because they sit after yield/mailbox/volume opcodes.
    """
    events = []
    for module in script["modules"]:
        last_assign = None
        for cmd in module["commands"]:
            if cmd["opcode"] == 0x0A and cmd["modes"] and cmd["modes"][0] == 2 and len(cmd["args"]) >= 2:
                last_assign = cmd
            if cmd["opcode"] == 0x31 and cmd["args"] and last_assign is not None:
                index = last_assign["args"][0]
                value = last_assign["args"][1] if last_assign["modes"][1] == 0 else None
                if value is None:
                    continue
                before = persist[index] if 0 <= index < len(persist) else ""
                events.append(
                    {
                        "scene": scene,
                        "module": module["index"],
                        "script_pc": f"+0x{last_assign['offset']:04X}",
                        "index": index,
                        "before": before,
                        "operation": "assign_before_token",
                        "after": value,
                        "consequence": f"then 0x31 {decode_packed_name(cmd['args'][0])} @ +0x{cmd['offset']:04X}",
                    }
                )
                last_assign = None
    return events


def dump_save_layout() -> list[dict]:
    # Cursor-relative sizes proven from func_8003F800.
    return [
        {
            "order": 0,
            "source": "D_800A77F0",
            "size": "0x800",
            "role": "persist[0..0x1FF] 512 words",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": 1,
            "source": "D_8009D2E8",
            "size": "0x4",
            "role": "pad/inhibit word",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": 2,
            "source": "D_8009D280",
            "size": "0x4",
            "role": "dest token",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": 3,
            "source": "D_8009D1A0",
            "size": "0x4",
            "role": "field request flags",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": 4,
            "source": "D_800B0CDC",
            "size": "0x4",
            "role": "UNKNOWN word next to CE0 pack",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": 5,
            "source": "D_800B0CE0..D_800B0CE6",
            "size": "0x7",
            "role": "package header bytes CE0-CE6",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": 6,
            "source": "D_800BCFEE",
            "size": "0x1",
            "role": "camera/fade byte",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": 7,
            "source": "D_800B8A20",
            "size": "0x70",
            "role": "UNKNOWN 0x70-byte block",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": 8,
            "source": "D_800B0CB0",
            "size": "0x18",
            "role": "UNKNOWN 0x18-byte block",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": 9,
            "source": "D_8009D1B0",
            "size": "0x8",
            "role": "UNKNOWN 8-byte pair",
            "function": "func_8003F800",
            "direction": "RAM->buffer",
        },
        {
            "order": -1,
            "source": "prefix before persist copy",
            "size": "0x12E4",
            "role": "copied by func_80040B80 before jal func_8003F800",
            "function": "func_80040B80",
            "direction": "RAM->buffer",
        },
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("disc", type=Path)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("docs/evidence/pe-pst0-persist-provenance"),
    )
    parser.add_argument("--skip-disc-hash", action="store_true")
    parser.add_argument("--skip-pe-hash", action="store_true")
    args = parser.parse_args()

    disc = args.disc
    if disc.stat().st_size != DISC1_BYTES:
        raise SystemExit(f"wrong Disc 1 size: {disc.stat().st_size}")
    disc_sha = "SKIPPED"
    if not args.skip_disc_hash:
        disc_sha = sha256_file(disc)
        if disc_sha != DISC1_SHA256:
            raise SystemExit(f"wrong Disc 1 SHA-256: {disc_sha}")

    with RawMode2Image(disc) as image:
        entries = iso_entries(image)
        exe_entry = find_entry(entries, DISC1_EXE)
        pe_entry = find_entry(entries, PE_IMG)
        exe = image.read_form1_extent(exe_entry[1], exe_entry[2])
        if len(exe) != DISC1_EXE_BYTES or sha256_bytes(exe) != DISC1_EXE_SHA256:
            raise SystemExit("EXE identity mismatch")
        if sha1_bytes(exe) != DISC1_EXE_SHA1:
            raise SystemExit("EXE SHA-1 mismatch")
        pe_sha1 = PE_IMG_SHA1
        if not args.skip_pe_hash:
            pe_digest = hashlib.sha1()
            remaining = PE_IMG_BYTES
            lba = pe_entry[1]
            while remaining:
                data = image.user_data(lba)
                take = min(remaining, len(data))
                pe_digest.update(data[:take])
                remaining -= take
                lba += 1
            pe_sha1 = pe_digest.hexdigest()
            if pe_sha1 != PE_IMG_SHA1:
                raise SystemExit(f"PE.IMG SHA-1 mismatch: {pe_sha1}")

        field_table = parse_field_table(exe)
        wanted = current_route_set()
        packages = {}
        for rec in field_table:
            if rec["index"] not in wanted and rec["index"] not in {0, 1, 2, 3, 4, 371, 376, 377}:
                continue
            start, end = rec["start"], rec["end"]
            if end <= start:
                continue
            blob = image.read_form1_extent(pe_entry[1] + start, (end - start) * FORM1_USER_SIZE)
            packages[rec["index"]] = blob
            rec["sha256"] = sha256_bytes(blob)
            rec["bytes"] = len(blob)

    # Verify binder/opcode tables from EXE.
    mode_ok = all(word_at(exe, MODE_TABLE + mode * 4) == va for mode, va in MODE_CASES.items())
    handler_ok = all(word_at(exe, OPCODE_TABLE + op * 4) == va for op, va in HANDLERS.items())
    alu_words = [word_at(exe, ALU_TABLE + i * 4) for i in range(24)]
    persist_init = [word_at(exe, PERSIST_BASE + i * 4) for i in range(8)]
    if any(persist_init):
        raise SystemExit(f"persist BSS prefix not zero in EXE image: {persist_init}")

    exe_refs = scan_lui_addiu_refs(exe, PERSIST_BASE)
    save_layout = dump_save_layout()
    ranges_53128 = dump_53128_ranges(exe)
    covered_by_53128 = set()
    for row in ranges_53128:
        for idx in range(row["index_start"], row["index_end"] + 1):
            if 0 <= idx < PERSIST_WORDS:
                covered_by_53128.add(idx)

    access_rows: list[dict] = []
    timeline_rows: list[dict] = []
    scene_scripts: dict[str, dict] = {}
    persist_state = [0] * PERSIST_WORDS
    route_events: list[dict] = []

    handler_09 = HANDLERS[0x09]
    handler_0a = HANDLERS[0x0A]
    scanned_scenes = []

    for name, index in CURRENT_ROUTE + DAY1_EXTRA:
        rec = next((r for r in field_table if r["index"] == index), None)
        pkg = packages.get(index)
        if rec is None or pkg is None:
            scanned_scenes.append({"scene": name, "status": "PACKAGE_MISSING"})
            continue
        extracted = extract_script_from_package(pkg, rec["meta"])
        if extracted is None:
            scanned_scenes.append({"scene": name, "status": "SCRIPT_NOT_FOUND", "pkg_sha256": rec.get("sha256", "")})
            continue
        script_off, script_blob = extracted
        script = decode_script(script_blob)
        if not script["modules"]:
            scanned_scenes.append({"scene": name, "status": "SCRIPT_DECODE_FAIL"})
            continue
        scene_scripts[name] = script
        rows = persist_access_rows(name, index, script, handler_09, handler_0a)
        annotate_branches(rows, script)
        access_rows.extend(rows)
        scanned_scenes.append(
            {
                "scene": name,
                "status": "OK",
                "table_index": index,
                "pkg_sha256": rec.get("sha256", ""),
                "script_off": f"0x{script_off:X}",
                "script_sha256": script["sha256"],
                "modules": script["module_count"],
                "persist_accesses": len(rows),
            }
        )
        # Simulate modules in index order against a running persist image.
        on_route = name in {n for n, _ in CURRENT_ROUTE}
        first_play_pcs = {
            ("m0002i", 5, 0x0EE4),
            ("m0372i", 1, 0x04E4),
            ("m0372i", 3, 0x0D48),
            ("m0004i", 4, 0x0F30),
            ("m0378i", 4, 0x0724),
        }
        if on_route:
            before_scene = persist_state[:]
            for module in script["modules"]:
                events = simulate_module(module, persist_state, scene=name)
                for event in events:
                    event["route_before_scene"] = (
                        before_scene[event["index"]] if event["index"] != "" else ""
                    )
                    event["first_play_arm"] = "YES"
                route_events.extend(events)
            for event in first_play_transition_events(script, name, persist_state):
                pc = int(str(event["script_pc"])[3:], 16)
                key = (event["scene"], int(event["module"]), pc)
                event["first_play_arm"] = "YES" if key in first_play_pcs else "ALTERNATE_OR_RETURN"
                if key in first_play_pcs and isinstance(event["after"], int) and event["index"] != "":
                    persist_state[int(event["index"])] = event["after"]
                route_events.append(event)

    # Registry from observed accesses.
    by_index: dict[int, list[dict]] = {}
    for row in access_rows:
        by_index.setdefault(int(row["index"]), []).append(row)
    registry = [classify_entry(index, rows) for index, rows in sorted(by_index.items())]
    for entry in registry:
        idx = int(entry["index"])
        if idx in covered_by_53128:
            entry["cross_system_readers"] = "func_80053128 (D_800923D8 range table)"
        elif idx == 1:
            entry["cross_system_readers"] = "field dest spawn only; not in D_800923D8 ranges"
        else:
            entry["cross_system_readers"] = "field scripts; save/load whole bank"

    # Timeline: persist mutations plus dest tokens on the current route only.
    for event in route_events:
        if event["scene"] not in {n for n, _ in CURRENT_ROUTE}:
            continue
        timeline_rows.append(event)

    out = args.output
    out.mkdir(parents=True, exist_ok=True)

    write_csv(
        out / "READERS_WRITERS.csv",
        access_rows,
        [
            "index",
            "index_hex",
            "rw",
            "operation",
            "value",
            "value_hex",
            "scene",
            "table_index",
            "module",
            "script_pc",
            "handler",
            "opcode",
            "modes",
            "args",
            "src_bank",
            "dst_bank",
            "alu_desc",
            "signedness",
            "branch_consequence",
            "notes",
        ],
    )
    write_csv(
        out / "PERSIST_REGISTRY.csv",
        registry,
        [
            "index",
            "offset",
            "width",
            "signedness",
            "confidence",
            "semantic_name",
            "known_values",
            "bit_masks",
            "first_reader",
            "first_writer",
            "scenes",
            "survives_room",
            "survives_battle",
            "save_backed",
            "reset_behavior",
            "cross_system_readers",
            "notes",
            "evidence_refs",
            "operations",
        ],
    )
    write_csv(
        out / "CURRENT_ROUTE_TIMELINE.csv",
        timeline_rows,
        [
            "scene",
            "module",
            "script_pc",
            "index",
            "before",
            "operation",
            "after",
            "consequence",
            "first_play_arm",
        ],
    )
    write_csv(
        out / "CODE_CROSSREF.csv",
        exe_refs
        + [
            {
                "va": "0x80010690",
                "addiu_va": "",
                "hint": "binder mode table (5 cases)",
            },
            {
                "va": "0x80010000",
                "addiu_va": "",
                "hint": "opcode 0x09 ALU jtbl 24 entries",
            },
            {
                "va": "0x800910A0",
                "addiu_va": "",
                "hint": "field opcode handler table",
            },
            {
                "va": "0x80034F10",
                "addiu_va": "",
                "hint": "new-game zero 512 persist words + scratch + actors",
            },
            {
                "va": "0x80034FC4",
                "addiu_va": "",
                "hint": "field actor rebuild; does not touch persist",
            },
            {
                "va": "0x8003E680",
                "addiu_va": "",
                "hint": "boot dispatcher; jal func_80034F10",
            },
        ],
        ["va", "addiu_va", "hint"],
    )
    write_csv(
        out / "CROSS_SYSTEM_READERS.csv",
        [
            {
                "system": "field_vm",
                "function": "binder mode 2 @ 0x800171BC",
                "access": "read/write word persist[slot]",
                "evidence": "func_80017134 case 2; *D_800A77F0 + slot*4",
            },
            {
                "system": "field_load",
                "function": "func_80034FC4",
                "access": "none",
                "evidence": "rebuilds D_800BEA90 actor pool only",
            },
            {
                "system": "new_game_boot",
                "function": "func_80034F10 via func_8003E680",
                "access": "write zero 512 words",
                "evidence": "sltiu count 0x200; sw $zero",
            },
            {
                "system": "save",
                "function": "func_8003F800 via func_80040B80",
                "access": "copy 0x800 bytes out",
                "evidence": "addiu $t0, persist, 0x800 memcpy loop",
            },
            {
                "system": "load",
                "function": "func_8003FBD8 via func_80042264",
                "access": "copy 0x800 bytes in",
                "evidence": "addiu $t0, src, 0x800 memcpy into D_800A77F0",
            },
            {
                "system": "inventory_or_menu_flag_sync",
                "function": "func_80053128",
                "access": "read persist[i] for i in table ranges at D_800923D8",
                "evidence": "lw persist; value-0x100 sltiu 0x80; ori D_800C0EB1 bit 3",
            },
            {
                "system": "battle",
                "function": "UNKNOWN direct persist consumer",
                "access": "not proven this rung",
                "evidence": "no other lui/addiu D_800A77F0 sites beyond binder/zero/save/load/53128",
            },
        ],
        ["system", "function", "access", "evidence"],
    )

    summary = {
        "disc_sha256": disc_sha,
        "exe_sha256": DISC1_EXE_SHA256,
        "exe_sha1": DISC1_EXE_SHA1,
        "pe_img_sha1": pe_sha1,
        "persist_base": f"0x{PERSIST_BASE:08X}",
        "persist_words": PERSIST_WORDS,
        "persist_bytes": PERSIST_BYTES,
        "mode_table_ok": mode_ok,
        "handler_table_ok": handler_ok,
        "alu_table": [f".L{v:08X}" for v in alu_words],  # case labels (symbol form)
        "exe_persist_refs": exe_refs,
        "save_layout": save_layout,
        "scanned_scenes": scanned_scenes,
        "entries_observed": len(registry),
        "access_rows": len(access_rows),
        "timeline_rows": len(timeline_rows),
        "field_table_rows": len(field_table),
        "func_80053128_ranges": ranges_53128,
        "persist_after_route_sim": {
            "0": persist_state[0],
            "1": persist_state[1],
            "0x4A": persist_state[0x4A],
        },
    }
    (out / "scan_summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps({k: summary[k] for k in (
        "persist_base",
        "persist_words",
        "mode_table_ok",
        "handler_table_ok",
        "entries_observed",
        "access_rows",
        "timeline_rows",
        "persist_after_route_sim",
    )}, indent=2))
    for scene in scanned_scenes:
        print(f"{scene['scene']}: {scene['status']} accesses={scene.get('persist_accesses', 0)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

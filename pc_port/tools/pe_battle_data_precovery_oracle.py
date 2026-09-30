#!/usr/bin/env python3
"""PE battle-data precovery oracle.

Recomputes NYPD / m0005i / CE2=14 / M0367I data contracts from the
SHA-1-exact EXE and Disc 1 PE.IMG. Writes or verifies:

  docs/evidence/pe-battle-data-precovery/
    ACTOR_RESOURCES.csv
    BATTLE_SCRIPTS.csv
    RESOURCE_PUBLICATION.csv
    PEIMG_PACKAGES.csv
    UNKNOWN_DEPENDENCIES.csv
    COMMAND_BINDS.csv
    WRITER_A_CLIPS.csv
    SPAWN_DESCRIPTORS.csv
    DEST_HOPS.csv
    DEST_PACKAGE_HEADS.csv
    REPORT.md

Does not import production C. Does not modify UE5. Does not name
actors from appearance.
"""
from __future__ import annotations

import csv
import hashlib
import os
import pathlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
HDR = 0x800
PE_IMG_LBA = 1013
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048

M0005I_TOKEN = 0xA80002C8
M0005I_REL = 0x266A
M0005I_PACKED = 0x0600A921
M0005I_CHUNK2 = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
M0005I_FULL = "fc48530a84811c31ebf4cde06bb12c9c7bcc815348db8dd8c2bb7dacd3410724"
M0367I_TOKEN = 0xA80663C8
M0367I_REL = 0x15050
M0367I_PACKED = 0x04E0AA21
M0367I_CHUNK2 = "ab9af4f446a6f9a1f8f79f1f80b862c4d516beddbede1229afab2dfc30b44b1e"
CE214_SHA = "db785a5eea1f78f945284e57955605326f5856adda1ebc83d0a95d7a0142b1b2"
CLIP4_SHA = "6207fbca2fe44a3549bf0b7fbcf1ce3e979a0a12e8b130e606ea4a985b1885e4"
WIN_6B35C = "1106cb2a94fa877af5a067e36a3c24d08f186694b112c2139d2869f7cf341f24"
WIN_B0E70 = "f2f5b2382224b409acea599d5bdf1572a082fe30d76d19731ff712b4aef0e2d0"
WIN_6C118 = "59c4bd179f4f37cbc881c9d847d80e469f4311f69c5efa904b2c583169cf70f2"
WIN_362B8 = "c1d9429303ec09b5ebea35272182d10d64b061ede1dee1c34a637581827db78d"
WIN_WRITER_A = tuple(exe_words(0x8006B84C, 13))
LIST_OFF = 0x202A4
DESC_OFF = 0x25014
M367_LIST_OFF = 0x1B314
M367_DESC_OFF = 0x1C6D4
M367_B0E70_2 = "c55bd4c8c80895452ae3c6d786e4cbfa403311cee384aa87e63360231bf86015"
M367_B0E70_4 = "acec62834711c862d1abf5e50e1efc39ce5268ec159106487c79c811fac80b24"
KNOWN_OPS = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
    0x0D, 0x0E, 0x11, 0x12, 0x14, 0x1A, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x22, 0x24, 0x2A, 0x2E, 0x2F, 0x30, 0x31, 0x3F, 0x40, 0x41, 0x43,
    0x4B, 0x4E, 0x52, 0x53, 0x54, 0x59, 0x5A, 0x5E, 0x64, 0x65, 0x6A,
    0x6B, 0x6F, 0x70, 0x77, 0x79, 0x82, 0x84, 0x85, 0x86, 0x87, 0x88,
    0x89, 0x8B, 0x94, 0x9B, 0x9C, 0xA6, 0xAA, 0xAB, 0xAD, 0xB7, 0xB8,
    0xC1, 0xC7, 0xCE, 0xD9, 0xDC, 0xE1, 0xEA, 0xED,
    0x9D,
}
YIELD_OPS = {0x01, 0x02, 0x1F, 0x20, 0x30, 0x64, 0x9C}
BRANCH_OPS = {0x00, 0x05, 0x12}
ROOT = pathlib.Path(__file__).resolve().parents[2]
EVIDENCE = ROOT / "docs" / "evidence" / "pe-battle-data-precovery"
CSV_NAMES = (
    "ACTOR_RESOURCES.csv",
    "BATTLE_SCRIPTS.csv",
    "RESOURCE_PUBLICATION.csv",
    "PEIMG_PACKAGES.csv",
    "UNKNOWN_DEPENDENCIES.csv",
    "COMMAND_BINDS.csv",
    "WRITER_A_CLIPS.csv",
    "SPAWN_DESCRIPTORS.csv",
    "DEST_HOPS.csv",
    "DEST_PACKAGE_HEADS.csv",
)
CMD15_SHA = "194a37c66679857b8a4ca3e8df72b3812cb5fafb2e03dbeb45c9e79b3fda1537"
CMD1C_SHA = "cabc0c490153091a3d83b5f79da71c108d1cbe5004276ccae70707397e25e85e"
WA18_SHA = "03f7338305ade7941ff5115bbacc5dd83481dc3cdc4058f5ea043a65617fd24c"
M367_FIRST8_08 = (2, 2, 4, 3, 4, 2, 3, 4)
TYPE0_2E_CMDS = (0x15, 0x1C, 0x1E, 0x1F, 0x20)


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def load_u16(blob: bytes, va: int) -> int:
    return struct.unpack_from("<H", blob, exe_off(va))[0]


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def find_disc() -> pathlib.Path:
    pointer = ROOT / "local" / "pe_disc1.path"
    if pointer.is_file():
        path = pathlib.Path(pointer.read_text().strip().splitlines()[0].strip())
        if path.is_file():
            return path
    env = os.environ.get("PE_DISC1_BIN", "").strip()
    if env:
        path = pathlib.Path(env)
        if path.is_file():
            return path
    raise SystemExit("FAIL: missing Disc 1 (local/pe_disc1.path or PE_DISC1_BIN)")


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            chunk = fh.read(FORM1_USER)
            require(len(chunk) == FORM1_USER, f"disc read {lba + i}")
            out += chunk
    return bytes(out)


def decode_token(exe: bytes, token: int) -> str:
    out = []
    for i in range(6):
        idx = (token >> ((5 - i) * 5 + 2)) & 0x1F
        ch = exe[exe_off(0x800930B4) + idx]
        if 97 <= ch < 123:
            ch -= 32
        out.append(chr(ch))
    return "".join(out)


def packed_secs(packed: int) -> tuple[int, int, int]:
    return packed & 0xFF, (packed >> 8) & 0xFFF, packed >> 20


def rel24(blob: bytes, packed: int) -> int:
    return struct.unpack_from("<I", blob, (packed & 0x3FFFFF) + 4)[0] & 0x00FFFFFF


def walk12(blob: bytes, packed: int) -> list[dict]:
    count = packed >> 22
    rec = packed & 0x3FFFFF
    rows = []
    for i in range(count):
        off = rec + i * 12
        size, ptrw, w8 = struct.unpack_from("<III", blob, off)
        ptr = ptrw & 0xFFFFFF
        payload = blob[ptr : ptr + size] if ptr + size <= len(blob) else b""
        rows.append(
            {
                "i": i,
                "off": off,
                "size": size,
                "ptr": ptr,
                "idb": blob[off + 7],
                "ida": blob[off + 0xB],
                "sha256": hashlib.sha256(payload).hexdigest() if payload else "",
                "b0": blob[ptr] if ptr < len(blob) else None,
                "b1": blob[ptr + 1] if ptr + 1 < len(blob) else None,
                "b2": blob[ptr + 2] if ptr + 2 < len(blob) else None,
            }
        )
    return rows


def walk_script(blob: bytes, base: int, limit: int = 32) -> list[dict]:
    """17018 fetch: word @pc, word2 @pc+4, imms @pc+8, span=8+argc*4."""
    rows = []
    pc = base
    for _ in range(limit):
        if pc + 8 > len(blob):
            break
        word = struct.unpack_from("<I", blob, pc)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = word >> 17
        span = 8 + argc * 4
        if pc + span > len(blob):
            break
        imms = [
            struct.unpack_from("<I", blob, pc + 8 + i * 4)[0] for i in range(argc)
        ]
        rows.append(
            {
                "rel": pc - base,
                "word": word,
                "op": op,
                "argc": argc,
                "kinds": kinds,
                "imms": imms,
                "span": span,
            }
        )
        if op not in KNOWN_OPS:
            break
        pc += span
    return rows


def script_base(blob: bytes, list_off: int, typ: int) -> int:
    rel = struct.unpack_from("<I", blob, list_off + 8 + typ * 4)[0]
    return list_off + rel


def token_table_index(name: str) -> int:
    digits = "".join(ch for ch in name if ch.isdigit())
    require(digits != "", f"token name {name}")
    return int(digits) - 1


def csv_write(path: pathlib.Path, fieldnames: list[str], rows: list[dict]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="\n", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        for row in rows:
            writer.writerow({k: row.get(k, "") for k in fieldnames})


def csv_read(path: pathlib.Path) -> tuple[list[str], list[dict]]:
    with path.open("r", encoding="utf-8", newline="") as fh:
        reader = csv.DictReader(fh)
        rows = list(reader)
        return list(reader.fieldnames or []), rows


def hex_list(values: list[int], width: int = 2) -> str:
    return "|".join(f"0x{v:0{width}X}" for v in values)


class World:
    def __init__(self) -> None:
        exe_path = ROOT / "build" / "disc1.candidate.exe"
        require(exe_path.is_file(), f"missing {exe_path}")
        self.exe = exe_path.read_bytes()
        require(hashlib.sha1(self.exe).hexdigest() == SHA1, "EXE SHA-1")
        require(struct.unpack_from("<I", self.exe, 0x1C)[0] == 0x1EE000, "tsize")
        self.disc = find_disc()
        s0, s1, s2 = packed_secs(M0005I_PACKED)
        self.m5_c0 = read_form1(self.disc, PE_IMG_LBA + M0005I_REL, s0)
        self.m5_c1 = read_form1(self.disc, PE_IMG_LBA + M0005I_REL + s0, s1)
        self.m5_c2 = read_form1(self.disc, PE_IMG_LBA + M0005I_REL + s0 + s1, s2)
        self.m5_full = self.m5_c0 + self.m5_c1 + self.m5_c2
        require(hashlib.sha256(self.m5_c2).hexdigest() == M0005I_CHUNK2, "m0005i chunk2")
        require(hashlib.sha256(self.m5_full).hexdigest() == M0005I_FULL, "m0005i full")
        self.ce214 = read_form1(self.disc, PE_IMG_LBA + 396, 32)
        require(hashlib.sha256(self.ce214).hexdigest() == CE214_SHA, "CE2=14 bank")
        t0, t1, t2 = packed_secs(M0367I_PACKED)
        self.m367_c0 = read_form1(self.disc, PE_IMG_LBA + M0367I_REL, t0)
        self.m367_c1 = read_form1(self.disc, PE_IMG_LBA + M0367I_REL + t0, t1)
        self.m367_c2 = read_form1(self.disc, PE_IMG_LBA + M0367I_REL + t0 + t1, t2)
        self.m367_full = self.m367_c0 + self.m367_c1 + self.m367_c2
        require(hashlib.sha256(self.m367_c2).hexdigest() == M0367I_CHUNK2, "M0367I chunk2")
        self.m5_hdr = struct.unpack_from("<I", self.m5_c2, 4)[0] & 0x3FFFFF
        self.ce_hdr = struct.unpack_from("<I", self.ce214, 4)[0] & 0x3FFFFF
        self.m367_hdr = struct.unpack_from("<I", self.m367_c2, 4)[0] & 0x3FFFFF

    def pin_exe(self) -> None:
        require(decode_token(self.exe, M0005I_TOKEN) == "M0005I", "M0005I token")
        require(decode_token(self.exe, M0367I_TOKEN) == "M0367I", "M0367I token")
        require(load_u32(self.exe, 0x80093378 + 4 * 8) == M0005I_REL, "table[4] rel")
        require(load_u32(self.exe, 0x80093378 + 4 * 8 + 4) == M0005I_PACKED, "table[4] packed")
        require(load_u32(self.exe, 0x80093378 + 366 * 8) == M0367I_REL, "table[366] rel")
        require(load_u32(self.exe, 0x80093378 + 366 * 8 + 4) == M0367I_PACKED, "table[366] packed")
        require(load_u16(self.exe, 0x800930D8 + 22 * 2) == 396, "CE2=14 start")
        require(load_u16(self.exe, 0x800930D8 + 23 * 2) == 428, "CE2=14 end")
        require(jal_target(load_u32(self.exe, 0x8003F07C)) == 0x8006B35C, "3F074 jal 6B35C")
        require(jal_target(load_u32(self.exe, 0x8003F088)) == 0x8006B4F8, "3F074 jal 6B4F8")
        require((0x8006B4F8 - 0x8006B35C) // 4 == 103, "6B35C 103w")
        require(window_sha(self.exe, 0x8006B35C, 0x8006B4F8) == WIN_6B35C, "6B35C sha")
        require(load_u32(self.exe, 0x8006B378) == 0xAC400198, "6B35C sw B0E70 via +0x198")
        require(load_u32(self.exe, 0x8006B3F0) == 0xAC400948, "6B35C zero +0x948")
        require(load_u32(self.exe, 0x8006B7C8) == 0x8EA2000C, "hdr+0x0C lw")
        require(load_u32(self.exe, 0x8006B804) == 0xAC620198, "hdr+0x0C sw B0E70[idB]")
        require(window_sha(self.exe, 0x8006B7C8, 0x8006B820) == WIN_B0E70, "hdr+0x0C window")
        for i, word in enumerate(WIN_WRITER_A):
            require(load_u32(self.exe, 0x8006B84C + i * 4) == word, f"Writer A[{i}]")
        require(load_u32(self.exe, 0x8006C118) == 0xAE820198, "6C118 sw B0E70[0]")
        require(load_u32(self.exe, 0x8006C158) == 0xAC6201C0, "Writer B sw +0x1C0")
        require(window_sha(self.exe, 0x8006C0E4, 0x8006C174) == WIN_6C118, "6C118 window")
        require(window_sha(self.exe, 0x800362B8, 0x800363F4) == WIN_362B8, "362B8 sha")
        require(load_u32(self.exe, 0x800351A8) == 0x8C220E70, "35038 lw B0E70[type]")
        require(load_u32(self.exe, 0x8001A6DC) == 0xAE2201B0, "1A680 sw +0x1B0")
        require(load_u32(self.exe, 0x800910A0 + 0xCE * 4) == 0x800181CC, "table[0xCE]")
        require(load_u32(self.exe, 0x800910A0 + 0x9B * 4) == 0x80015240, "table[0x9B]")
        require(load_u32(self.exe, 0x800910A0 + 0x08 * 4) == 0x8001735C, "table[0x08]")
        require(load_u32(self.exe, 0x800910A0 + 0x2E * 4) == 0x80017AE8, "table[0x2E]")
        require(load_u32(self.exe, 0x800910A0 + 0x2F * 4) == 0x80017B34, "table[0x2F]")
        require(load_u32(self.exe, 0x800910A0 + 0xC1 * 4) == 0x80019AC0, "table[0xC1]")
        require(load_u32(self.exe, 0x800910A0 + 0x9D * 4) == 0x80019450, "table[0x9D]")
        require(load_u32(self.exe, 0x80019AD4) == 0x34420400, "0xC1 ori +0x98 0x400")
        require(load_u32(self.exe, 0x80017B60) == 0xA4830012, "0x2F sh +0x12")
        require(load_u32(self.exe, 0x8006B8C4) == 0xAEC20944, "sw +0x944")
        require(load_u32(self.exe, 0x8006B8E8) == 0xAEC20948, "sw +0x948")
        require(load_u32(self.exe, 0x8006B90C) == 0xAEC2094C, "sw +0x94C")
        require(load_u32(self.exe, 0x8006B94C) == 0xAC620950, "sw +0x950")

    def packages(self) -> list[dict]:
        s0, s1, s2 = packed_secs(M0005I_PACKED)
        t0, t1, t2 = packed_secs(M0367I_PACKED)
        rows = [
            {
                "package_id": "m0005i",
                "token": f"0x{M0005I_TOKEN:08X}",
                "decoded_name": "M0005I",
                "table_index": 4,
                "rel": f"0x{M0005I_REL:X}",
                "packed": f"0x{M0005I_PACKED:08X}",
                "sectors": f"{s0}+{s1}+{s2}",
                "peimg_lba_range": f"[{PE_IMG_LBA + M0005I_REL},{PE_IMG_LBA + M0005I_REL + s0 + s1 + s2})",
                "size": len(self.m5_full),
                "sha256": hashlib.sha256(self.m5_full).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "3-chunk field package; chunk2 hdr@+4",
                "consumer": "func_8006B4F8",
                "writer": "PE.IMG table D_80093378[4]",
                "confidence": "PROVEN",
                "notes": "NYPD/Eve-intro room; first 0x89 at module 6 +0x350C",
            },
            {
                "package_id": "m0005i_chunk0",
                "token": f"0x{M0005I_TOKEN:08X}",
                "decoded_name": "M0005I",
                "table_index": 4,
                "rel": f"0x{M0005I_REL:X}",
                "packed": f"0x{M0005I_PACKED:08X}",
                "sectors": str(s0),
                "peimg_lba_range": f"[{PE_IMG_LBA + M0005I_REL},{PE_IMG_LBA + M0005I_REL + s0})",
                "size": len(self.m5_c0),
                "sha256": hashlib.sha256(self.m5_c0).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "6B4F8 dest overlay+0x194",
                "consumer": "func_8006B4F8 / 6E6A8",
                "writer": "PE.IMG",
                "confidence": "PROVEN",
                "notes": "",
            },
            {
                "package_id": "m0005i_chunk1",
                "token": f"0x{M0005I_TOKEN:08X}",
                "decoded_name": "M0005I",
                "table_index": 4,
                "rel": f"0x{M0005I_REL:X}",
                "packed": f"0x{M0005I_PACKED:08X}",
                "sectors": str(s1),
                "peimg_lba_range": f"[{PE_IMG_LBA + M0005I_REL + s0},{PE_IMG_LBA + M0005I_REL + s0 + s1})",
                "size": len(self.m5_c1),
                "sha256": hashlib.sha256(self.m5_c1).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "6B4F8 dest overlay+0x168",
                "consumer": "func_8006B4F8 / 6E6A8",
                "writer": "PE.IMG",
                "confidence": "PROVEN",
                "notes": "",
            },
            {
                "package_id": "m0005i_chunk2",
                "token": f"0x{M0005I_TOKEN:08X}",
                "decoded_name": "M0005I",
                "table_index": 4,
                "rel": f"0x{M0005I_REL:X}",
                "packed": f"0x{M0005I_PACKED:08X}",
                "sectors": str(s2),
                "peimg_lba_range": f"[{PE_IMG_LBA + M0005I_REL + s0 + s1},{PE_IMG_LBA + M0005I_REL + s0 + s1 + s2})",
                "size": len(self.m5_c2),
                "sha256": hashlib.sha256(self.m5_c2).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "scripts+Writer A+B0E70+12574; dest overlay+0x18C",
                "consumer": "func_8006B4F8 publish cut",
                "writer": "PE.IMG",
                "confidence": "PROVEN",
                "notes": f"hdr@{self.m5_hdr:#x}",
            },
            {
                "package_id": "ce2_14_clip_bank",
                "token": "CE2=14",
                "decoded_name": "",
                "table_index": "D_800930D8[22..23]",
                "rel": "396",
                "packed": "",
                "sectors": "32",
                "peimg_lba_range": f"[{PE_IMG_LBA + 396},{PE_IMG_LBA + 428})",
                "size": len(self.ce214),
                "sha256": hashlib.sha256(self.ce214).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "Writer B 12-byte dir; type-0 commands",
                "consumer": "func_8001A680 via D_800B0E98",
                "writer": "func_8006BECC state6 / 6C140",
                "confidence": "PROVEN",
                "notes": "6C1CC(a0=1) sets CE2=14; 144FC/29810 do not jal it",
            },
            {
                "package_id": "m0367i",
                "token": f"0x{M0367I_TOKEN:08X}",
                "decoded_name": "M0367I",
                "table_index": 366,
                "rel": f"0x{M0367I_REL:X}",
                "packed": f"0x{M0367I_PACKED:08X}",
                "sectors": f"{t0}+{t1}+{t2}",
                "peimg_lba_range": f"[{PE_IMG_LBA + M0367I_REL},{PE_IMG_LBA + M0367I_REL + t0 + t1 + t2})",
                "size": len(self.m367_full),
                "sha256": hashlib.sha256(self.m367_full).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "3-chunk dest package; 6B4F8 reload",
                "consumer": "func_8006B4F8",
                "writer": "0x31 store D_8009D280",
                "confidence": "PROVEN",
                "notes": "watch-arm dest after m0005i; do not infer story meaning",
            },
            {
                "package_id": "m0367i_chunk2",
                "token": f"0x{M0367I_TOKEN:08X}",
                "decoded_name": "M0367I",
                "table_index": 366,
                "rel": f"0x{M0367I_REL:X}",
                "packed": f"0x{M0367I_PACKED:08X}",
                "sectors": str(t2),
                "peimg_lba_range": f"[{PE_IMG_LBA + M0367I_REL + t0 + t1},{PE_IMG_LBA + M0367I_REL + t0 + t1 + t2})",
                "size": len(self.m367_c2),
                "sha256": hashlib.sha256(self.m367_c2).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": f"hdr@{self.m367_hdr:#x}; Writer A count 36; hdr+0x0C count 3",
                "consumer": "func_8006B4F8 publish cut",
                "writer": "PE.IMG",
                "confidence": "PROVEN",
                "notes": "",
            },
        ]
        seen: set[str] = set()
        ce_dir = walk12(
            self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x10)[0]
        )
        for rec in ce_dir:
            if rec["sha256"] in seen:
                continue
            seen.add(rec["sha256"])
            idbs = [r["idb"] for r in ce_dir if r["sha256"] == rec["sha256"]]
            rows.append(
                {
                    "package_id": f"ce2_14_clip_ptr_{rec['ptr']:X}",
                    "token": "CE2=14",
                    "decoded_name": "",
                    "table_index": f"idB={','.join(str(x) for x in idbs)}",
                    "rel": f"0x{rec['ptr']:X}",
                    "packed": "",
                    "sectors": "",
                    "peimg_lba_range": f"[{PE_IMG_LBA + 396},{PE_IMG_LBA + 428})",
                    "size": rec["size"],
                    "sha256": rec["sha256"],
                    "compression": "none",
                    "decoded_structure": (
                        f"clip enc={rec['b0']} bones-1={rec['b1']} frames={rec['b2']}"
                    ),
                    "consumer": "func_8001A680 +0x1B0",
                    "writer": "func_8006C140 Writer B",
                    "confidence": "PROVEN",
                    "notes": "type-0 command bank; do not name clip",
                }
            )
        m5_dir = walk12(
            self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x10)[0]
        )
        seen.clear()
        for rec in m5_dir:
            if rec["sha256"] in seen:
                continue
            seen.add(rec["sha256"])
            keys = [f"t{r['ida']}_c{r['idb']:02X}" for r in m5_dir if r["sha256"] == rec["sha256"]]
            rows.append(
                {
                    "package_id": f"m0005i_writera_ptr_{rec['ptr']:X}",
                    "token": f"0x{M0005I_TOKEN:08X}",
                    "decoded_name": "M0005I",
                    "table_index": ",".join(keys),
                    "rel": f"0x{rec['ptr']:X}",
                    "packed": "",
                    "sectors": "",
                    "peimg_lba_range": f"m0005i_chunk2+0x{rec['ptr']:X}",
                    "size": rec["size"],
                    "sha256": rec["sha256"],
                    "compression": "none",
                    "decoded_structure": (
                        f"Writer A payload b0={rec['b0']} b1={rec['b1']} b2={rec['b2']}"
                    ),
                    "consumer": "func_8001A680 +0x1B0",
                    "writer": "func_8006B84C Writer A",
                    "confidence": "PROVEN",
                    "notes": "room package clips; type-0 has no cmd 4 here",
                }
            )
        m367_0c = walk12(
            self.m367_c2,
            struct.unpack_from("<I", self.m367_c2, self.m367_hdr + 0x0C)[0],
        )
        seen.clear()
        for rec in m367_0c:
            if rec["sha256"] in seen:
                continue
            seen.add(rec["sha256"])
            idbs = [r["idb"] for r in m367_0c if r["sha256"] == rec["sha256"]]
            rows.append(
                {
                    "package_id": f"m0367i_b0e70_ptr_{rec['ptr']:X}",
                    "token": f"0x{M0367I_TOKEN:08X}",
                    "decoded_name": "M0367I",
                    "table_index": f"idB={','.join(str(x) for x in idbs)}",
                    "rel": f"0x{rec['ptr']:X}",
                    "packed": "",
                    "sectors": "",
                    "peimg_lba_range": f"m0367i_chunk2+0x{rec['ptr']:X}",
                    "size": rec["size"],
                    "sha256": rec["sha256"],
                    "compression": "none",
                    "decoded_structure": f"hdr+0x0C resource object b={rec['b0']},{rec['b1']},{rec['b2']}",
                    "consumer": "func_80035038 +0x1AC",
                    "writer": "func_8006B4F8 6B804",
                    "confidence": "PROVEN",
                    "notes": "not the 125E0 type-1 object; type 1 B0E70 stays empty",
                }
            )
        m367_dir = walk12(
            self.m367_c2,
            struct.unpack_from("<I", self.m367_c2, self.m367_hdr + 0x10)[0],
        )
        seen.clear()
        for rec in m367_dir:
            if rec["sha256"] in seen:
                continue
            seen.add(rec["sha256"])
            keys = [
                f"t{r['ida']}_c{r['idb']:02X}"
                for r in m367_dir
                if r["sha256"] == rec["sha256"]
            ]
            rows.append(
                {
                    "package_id": f"m0367i_writera_ptr_{rec['ptr']:X}",
                    "token": f"0x{M0367I_TOKEN:08X}",
                    "decoded_name": "M0367I",
                    "table_index": ",".join(keys),
                    "rel": f"0x{rec['ptr']:X}",
                    "packed": "",
                    "sectors": "",
                    "peimg_lba_range": f"m0367i_chunk2+0x{rec['ptr']:X}",
                    "size": rec["size"],
                    "sha256": rec["sha256"],
                    "compression": "none",
                    "decoded_structure": (
                        f"Writer A payload enc={rec['b0']} bones-1={rec['b1']} frames={rec['b2']}"
                    ),
                    "consumer": "func_8001A680 +0x1B0 after type1 0x08",
                    "writer": "func_8006B84C Writer A",
                    "confidence": "PROVEN",
                    "notes": "no type-1 Writer A row; types 2/3 share cmds 0x00-0x0A",
                }
            )
        m319_rel = load_u32(self.exe, 0x80093378 + 318 * 8)
        m319_packed = load_u32(self.exe, 0x80093378 + 318 * 8 + 4)
        t0, t1, t2 = packed_secs(m319_packed)
        m319_c2 = read_form1(self.disc, PE_IMG_LBA + m319_rel + t0 + t1, t2)
        m319_hdr = struct.unpack_from("<I", m319_c2, 4)[0] & 0x3FFFFF
        m319_dir = walk12(m319_c2, struct.unpack_from("<I", m319_c2, m319_hdr + 0x10)[0])
        seen.clear()
        for rec in m319_dir:
            if rec["sha256"] in seen:
                continue
            seen.add(rec["sha256"])
            keys = [
                f"t{r['ida']}_c{r['idb']:02X}"
                for r in m319_dir
                if r["sha256"] == rec["sha256"]
            ]
            rows.append(
                {
                    "package_id": f"m0319i_writera_ptr_{rec['ptr']:X}",
                    "token": "0xA80614C8",
                    "decoded_name": "M0319I",
                    "table_index": ",".join(keys),
                    "rel": f"0x{rec['ptr']:X}",
                    "packed": "",
                    "sectors": "",
                    "peimg_lba_range": f"m0319i_chunk2+0x{rec['ptr']:X}",
                    "size": rec["size"],
                    "sha256": rec["sha256"],
                    "compression": "none",
                    "decoded_structure": (
                        f"Writer A payload enc={rec['b0']} bones-1={rec['b1']} frames={rec['b2']}"
                    ),
                    "consumer": "func_8001A680 after persist 0x31",
                    "writer": "func_8006B84C Writer A",
                    "confidence": "PROVEN",
                    "notes": "persist-gated dest; do not hop runtime",
                }
            )
        return rows

    def publication(self) -> list[dict]:
        m5_0c = walk12(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x0C)[0])
        m5_10 = walk12(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x10)[0])
        ce_0c = walk12(self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x0C)[0])
        ce_10 = walk12(self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x10)[0])
        obj948 = rel24(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x18)[0])
        obj94c = rel24(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x1C)[0])
        rows = [
            {
                "object": "D_800B0E70[0..9]",
                "field_or_global": "overlay+0x198",
                "writer": "func_8006B35C",
                "writer_va": "0x8006B378",
                "source_data": "zero",
                "source_offset": "",
                "lifetime": "dest_enter_clear",
                "publication_order": 1,
                "consumer": "func_80035038 +0x1AC",
                "consumer_va": "0x800351A8",
                "actor_type": "0-9",
                "confidence": "PROVEN",
                "notes": "10-word countdown from overlay+0x1BC to +0x198",
            },
            {
                "object": "D_800B0E98 type rows",
                "field_or_global": "overlay+0x1C0",
                "writer": "func_8006B35C",
                "writer_va": "0x8006B398",
                "source_data": "zero",
                "source_offset": "",
                "lifetime": "dest_enter_clear",
                "publication_order": 2,
                "consumer": "func_8001A680 +0x1B0",
                "consumer_va": "0x8001A6C8",
                "actor_type": "0-9",
                "confidence": "PROVEN",
                "notes": "10 rows x 48 words; stride 192",
            },
            {
                "object": "D_800B161C/20/24/28",
                "field_or_global": "overlay+0x944..+0x958",
                "writer": "func_8006B35C",
                "writer_va": "0x8006B3C0",
                "source_data": "zero",
                "source_offset": "",
                "lifetime": "dest_enter_clear",
                "publication_order": 3,
                "consumer": "12574/1A918/371B0",
                "consumer_va": "0x80012574",
                "actor_type": "",
                "confidence": "PROVEN",
                "notes": "zeroer of 6B4F8 publish slots",
            },
        ]
        for rec in m5_0c:
            rows.append(
                {
                    "object": f"D_800B0E70[{rec['idb']}]",
                    "field_or_global": "actor+0x1AC",
                    "writer": "func_8006B4F8 hdr+0x0C @ 0x8006B804",
                    "writer_va": "0x8006B804",
                    "source_data": "m0005i_chunk2",
                    "source_offset": f"0x{rec['ptr']:X}",
                    "lifetime": "until_next_6B35C",
                    "publication_order": 4,
                    "consumer": "func_80035038",
                    "consumer_va": "0x800351A8",
                    "actor_type": str(rec["idb"]),
                    "confidence": "PROVEN",
                    "notes": f"idA={rec['ida']} size={rec['size']} sha256={rec['sha256']}",
                }
            )
        rows.append(
            {
                "object": "D_800B0E98[type*192+cmd*4]",
                "field_or_global": "actor+0x1B0",
                "writer": "func_8006B4F8 hdr+0x10 Writer A @ 0x8006B84C",
                "writer_va": "0x8006B84C",
                "source_data": "m0005i_chunk2",
                "source_offset": f"0x{m5_10[0]['off']:X}" if m5_10 else "",
                "lifetime": "until_next_6B35C",
                "publication_order": 5,
                "consumer": "func_8001A680",
                "consumer_va": "0x8001A6C8",
                "actor_type": "mixed",
                "confidence": "PROVEN",
                "notes": f"21 records; type2 cmd=0x17 ptr=0x1F3A8 byte2=51",
            }
        )
        rows.append(
            {
                "object": "D_800B161C",
                "field_or_global": "overlay+0x944",
                "writer": "func_8006B4F8 hdr+0x14 / 12574",
                "writer_va": "0x8006B8C4",
                "source_data": "m0005i_chunk2",
                "source_offset": f"0x{LIST_OFF:X}",
                "lifetime": "until_next_6B35C",
                "publication_order": 6,
                "consumer": "func_800125E0",
                "consumer_va": "0x800125E0",
                "actor_type": "list",
                "confidence": "PROVEN",
                "notes": "count=7 type entries; 125E0 desc is type1+type6 only",
            }
        )
        rows.append(
            {
                "object": "D_800B1620",
                "field_or_global": "overlay+0x948",
                "writer": "func_8006B4F8 hdr+0x18",
                "writer_va": "0x8006B8E8",
                "source_data": "m0005i_chunk2",
                "source_offset": f"0x{obj948:X}",
                "lifetime": "until_next_6B35C",
                "publication_order": 7,
                "consumer": "func_8001A918",
                "consumer_va": "0x8001A918",
                "actor_type": "",
                "confidence": "PROVEN",
                "notes": f"lhu+2=1 +0x20=0x760; sha256={hashlib.sha256(self.m5_c2[obj948:obj948+0x800]).hexdigest()}",
            }
        )
        rows.append(
            {
                "object": "D_800B1624",
                "field_or_global": "overlay+0x94C",
                "writer": "func_8006B4F8 hdr+0x1C",
                "writer_va": "0x8006B90C",
                "source_data": "m0005i_chunk2",
                "source_offset": f"0x{obj94c:X}",
                "lifetime": "until_next_6B35C",
                "publication_order": 8,
                "consumer": "func_800371B0 / view apply",
                "consumer_va": "0x800371B0",
                "actor_type": "",
                "confidence": "PROVEN",
                "notes": f"sha256={hashlib.sha256(self.m5_c2[obj94c:obj94c+0x422C]).hexdigest() if obj94c+0x422C<=len(self.m5_c2) else ''}",
            }
        )
        require(len(ce_0c) == 1, "CE2=14 hdr+0x0C count")
        rows.append(
            {
                "object": "D_800B0E70[0]",
                "field_or_global": "actor+0x1AC type0",
                "writer": "func_8006C118 inside 6BECC",
                "writer_va": "0x8006C118",
                "source_data": "ce2_14_clip_bank",
                "source_offset": f"0x{ce_0c[0]['ptr']:X}",
                "lifetime": "until_next_6B35C",
                "publication_order": 9,
                "consumer": "func_80035038",
                "consumer_va": "0x800351A8",
                "actor_type": "0",
                "confidence": "PROVEN",
                "notes": (
                    f"rel24 of hdr+0x0C; size={ce_0c[0]['size']} "
                    f"sha256={ce_0c[0]['sha256']}; not m0005i hdr+0x0C"
                ),
            }
        )
        clip4 = next(r for r in ce_10 if r["ida"] == 0 and r["idb"] == 4)
        require(clip4["sha256"] == CLIP4_SHA, "clip4 sha")
        rows.append(
            {
                "object": "D_800B0E98[4]",
                "field_or_global": "actor+0x1B0 type0 cmd4",
                "writer": "func_8006C140 Writer B",
                "writer_va": "0x8006C158",
                "source_data": "ce2_14_clip_bank",
                "source_offset": "0x6C14",
                "lifetime": "until_next_6B35C",
                "publication_order": 10,
                "consumer": "func_8001A680 / 29810",
                "consumer_va": "0x8001A6DC",
                "actor_type": "0",
                "confidence": "PROVEN",
                "notes": f"1700B enc=1 bones-1=30 frames=18 sha256={CLIP4_SHA}",
            }
        )
        rows.append(
            {
                "object": "actor+0x1B4 dest",
                "field_or_global": "actor+0x1B4",
                "writer": "in-actor dest object; not a table pointer",
                "writer_va": "",
                "source_data": "actor body",
                "source_offset": "+0x1B4",
                "lifetime": "actor",
                "publication_order": 11,
                "consumer": "func_80015240 / func_8006CC68",
                "consumer_va": "0x80015240",
                "actor_type": "0",
                "confidence": "PROVEN",
                "notes": "15240 dest=actor+0x1B4; 6CC68 publishes D254+0x1B4 to D_800B0D10; 362B8/3D050 init only if +0x1AC!=0",
            }
        )
        m367_0c = walk12(
            self.m367_c2,
            struct.unpack_from("<I", self.m367_c2, self.m367_hdr + 0x0C)[0],
        )
        for rec in m367_0c:
            rows.append(
                {
                    "object": f"D_800B0E70[{rec['idb']}]",
                    "field_or_global": "actor+0x1AC",
                    "writer": "func_8006B4F8 hdr+0x0C @ 0x8006B804",
                    "writer_va": "0x8006B804",
                    "source_data": "m0367i_chunk2",
                    "source_offset": f"0x{rec['ptr']:X}",
                    "lifetime": "until_next_6B35C",
                    "publication_order": 12,
                    "consumer": "func_80035038",
                    "consumer_va": "0x800351A8",
                    "actor_type": str(rec["idb"]),
                    "confidence": "PROVEN",
                    "notes": f"M0367I idA={rec['ida']} size={rec['size']} sha256={rec['sha256']}",
                }
            )
        rows.append(
            {
                "object": "D_800B0E98 M0367I Writer A",
                "field_or_global": "overlay+0x1C0+type*192+cmd*4",
                "writer": "func_8006B4F8 hdr+0x10 Writer A @ 0x8006B84C",
                "writer_va": "0x8006B84C",
                "source_data": "m0367i_chunk2",
                "source_offset": "hdr+0x10",
                "lifetime": "until_next_6B35C",
                "publication_order": 13,
                "consumer": "func_8001A680 after type1 0x08 types 2/3/4",
                "consumer_va": "0x8001A6C8",
                "actor_type": "0,2,3,4",
                "confidence": "PROVEN",
                "notes": "36 rows; type0 cmd 0x18 only; no type1 row; 6B35C clears CE2=14 first",
            }
        )
        return rows

    def _script_rows(
        self,
        blob: bytes,
        list_off: int,
        scene: str,
        source: str,
        note_for,
    ) -> list[dict]:
        n = struct.unpack_from("<I", blob, list_off + 4)[0]
        rows = []
        for typ in range(n):
            rel = struct.unpack_from("<I", blob, list_off + 8 + typ * 4)[0]
            base = list_off + rel
            insns = walk_script(blob, base, 24)
            require(insns, f"{scene} type {typ} empty")
            window = blob[base : base + sum(x["span"] for x in insns)]
            ops = [x["op"] for x in insns]
            known = [f"0x{op:02X}" for op in ops if op in KNOWN_OPS]
            unknown = [f"0x{op:02X}" for op in ops if op not in KNOWN_OPS]
            branches = []
            yields = []
            for insn in insns:
                if insn["op"] in BRANCH_OPS and insn["imms"]:
                    branches.append(
                        f"+{insn['rel']:X}->+{(insn['imms'][0] << 1) & 0xFFFFFFFF:X}"
                    )
                if insn["op"] in YIELD_OPS:
                    yields.append(f"+{insn['rel']:X}/0x{insn['op']:02X}")
            rows.append(
                {
                    "actor_type": typ,
                    "scene": scene,
                    "source": source,
                    "base_offset": f"0x{base:X}",
                    "window_size": len(window),
                    "sha256": hashlib.sha256(window).hexdigest(),
                    "first_word": f"0x{insns[0]['word']:08X}",
                    "first_opcode": f"0x{insns[0]['op']:02X}",
                    "opcode_sequence": hex_list(ops),
                    "branch_targets": ";".join(branches),
                    "yield_points": ";".join(yields),
                    "known_opcodes": "|".join(known),
                    "unknown_opcodes": "|".join(unknown),
                    "confidence": "PROVEN",
                    "notes": "17018 word@pc word2@pc+4 imms@pc+8; " + note_for(typ),
                }
            )
        return rows

    def scripts(self) -> list[dict]:
        n = struct.unpack_from("<I", self.m5_c2, LIST_OFF + 4)[0]
        require(n == 7, "m0005i 12574 list count")
        rows = self._script_rows(
            self.m5_c2,
            LIST_OFF,
            "m0005i",
            "m0005i_chunk2",
            lambda typ: (
                "125E0-spawned"
                if typ in (1, 6)
                else "0x08-spawned"
                if typ in (0, 3, 5)
                else "listed_not_125E0_desc"
            ),
        )
        n367 = struct.unpack_from("<I", self.m367_c2, M367_LIST_OFF + 4)[0]
        require(n367 == 5, "M0367I 12574 list count")
        list_word0 = struct.unpack_from("<I", self.m367_c2, M367_LIST_OFF)[0]
        require(M367_LIST_OFF + list_word0 == M367_DESC_OFF, "M0367I desc = list+word0")
        desc = self.m367_c2[M367_DESC_OFF : M367_DESC_OFF + 4]
        require(desc[0] == 1 and desc[1] == 1 and desc[2] == 0, "M0367I 125E0 desc")
        rows.extend(
            self._script_rows(
                self.m367_c2,
                M367_LIST_OFF,
                "m0367i",
                "m0367i_chunk2",
                lambda typ: "125E0-spawned" if typ == 1 else "listed_not_125E0_desc",
            )
        )
        return rows

    def actors(self) -> list[dict]:
        m5_0c = {r["idb"]: r for r in walk12(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x0C)[0])}
        ce_0c = walk12(self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x0C)[0])[0]
        desc = self.m5_c2[DESC_OFF : DESC_OFF + 8]
        require(desc[0] == 2 and desc[1] == 1 and desc[3] == 6, "125E0 desc")
        scripts = {(r["scene"], int(r["actor_type"])): r for r in self.scripts()}
        vtable = {0: "func_80035C84", **{t: "func_80035E04" for t in range(1, 10)}}
        spawn = {
            0: "type1_0x08_second",
            1: "125E0_desc[0]",
            2: "listed_only",
            3: "type1_0x08_first",
            4: "listed_only",
            5: "type1_0x08_third",
            6: "125E0_desc[1]_D20C_head",
        }
        rows = []
        for typ in range(7):
            model = ""
            model_off = ""
            model_size = ""
            model_sha = ""
            model_src = ""
            if typ == 0:
                model_src = "ce2_14_clip_bank"
                model_off = f"0x{ce_0c['ptr']:X}"
                model_size = ce_0c["size"]
                model_sha = ce_0c["sha256"]
                model = "B0E70[0] via 6C118"
            elif typ in m5_0c:
                rec = m5_0c[typ]
                model_src = "m0005i_chunk2"
                model_off = f"0x{rec['ptr']:X}"
                model_size = rec["size"]
                model_sha = rec["sha256"]
                model = f"B0E70[{typ}] via 6B804"
            else:
                model = "B0E70 empty on this path"
            anim = "D_800B0E98 Writer A+B"
            if typ == 0:
                anim = "CE2=14 Writer B + m0005i Writer A cmds 0x18/0x1D-0x20"
            elif typ == 2:
                anim = "m0005i Writer A (cmd 0x17 byte2=51)"
            elif typ == 5:
                anim = "m0005i Writer A cmds 0-3 (60/88B payloads)"
            rows.append(
                {
                    "actor_type": typ,
                    "scene": "m0005i",
                    "descriptor_source": "m0005i_chunk2+0x25014" if typ in (1, 6) else "type1_0x08_desc",
                    "descriptor_offset": "0x25014" if typ in (1, 6) else "runtime",
                    "script_source": scripts[("m0005i", typ)]["source"],
                    "script_offset": scripts[("m0005i", typ)]["base_offset"],
                    "script_sha256": scripts[("m0005i", typ)]["sha256"],
                    "first_vm_pc": scripts[("m0005i", typ)]["base_offset"],
                    "first_opcode": scripts[("m0005i", typ)]["first_opcode"],
                    "model_resource_source": model_src,
                    "model_offset": model_off,
                    "model_size": model_size,
                    "model_sha256": model_sha,
                    "animation_bank": anim,
                    "publication_global": "D_800B0E70+D_800B0E98+D_800B161C",
                    "resource_object_writer": "6B35C; 6B4F8 6B804/6B84C; 6C118",
                    "update_function": vtable[typ],
                    "known_field_offsets": "+0x0C type; +0x0D idB; +0x1AC B0E70; +0x1B0 B0E98; +0x1B4 dest; +0x98 flags; +0x190 jalr",
                    "spawn_path": spawn[typ],
                    "confidence": "PROVEN" if typ in (0, 1, 3, 5, 6) else "PROVEN_LISTED_NOT_125E0",
                    "notes": model,
                }
            )
        m367_0c = {
            r["idb"]: r
            for r in walk12(
                self.m367_c2,
                struct.unpack_from("<I", self.m367_c2, self.m367_hdr + 0x0C)[0],
            )
        }
        require(m367_0c[2]["sha256"] == M367_B0E70_2, "M0367I B0E70[2]")
        require(m367_0c[4]["sha256"] == M367_B0E70_4, "M0367I B0E70[4]")
        for typ in range(5):
            rec = m367_0c.get(typ)
            rows.append(
                {
                    "actor_type": typ,
                    "scene": "m0367i",
                    "descriptor_source": "m0367i_chunk2+0x1C6D4"
                    if typ == 1
                    else "listed_only",
                    "descriptor_offset": "0x1C6D4" if typ == 1 else "",
                    "script_source": scripts[("m0367i", typ)]["source"],
                    "script_offset": scripts[("m0367i", typ)]["base_offset"],
                    "script_sha256": scripts[("m0367i", typ)]["sha256"],
                    "first_vm_pc": scripts[("m0367i", typ)]["base_offset"],
                    "first_opcode": scripts[("m0367i", typ)]["first_opcode"],
                    "model_resource_source": "m0367i_chunk2" if rec else "",
                    "model_offset": f"0x{rec['ptr']:X}" if rec else "",
                    "model_size": rec["size"] if rec else "",
                    "model_sha256": rec["sha256"] if rec else "",
                    "animation_bank": "m0367i Writer A 36 rows",
                    "publication_global": "D_800B0E70+D_800B0E98+D_800B161C",
                    "resource_object_writer": "6B35C; 6B4F8 6B804/6B84C",
                    "update_function": vtable[typ],
                    "known_field_offsets": "+0x0C type; +0x0D idB; +0x1AC B0E70; +0x1B0 B0E98; +0x1B4 dest",
                    "spawn_path": (
                        "125E0_desc[0]"
                        if typ == 1
                        else "type1_0x08"
                        if typ in (2, 3, 4)
                        else "listed_not_125E0_desc"
                    ),
                    "confidence": "PROVEN" if typ in (1, 2, 3, 4) else "PROVEN_LISTED_NOT_125E0",
                    "notes": f"B0E70[{typ}] via 6B804" if rec else "B0E70 empty on 125E0 type1",
                }
            )
        return rows

    def unknowns(self) -> list[dict]:
        return [
            {
                "id": "B0E70_type0_body",
                "needed_by": "35038 +0x1AC!=0 tail / 362B8 / 3D050",
                "surface": "CE2=14 +0x8 size 23864",
                "what_is_known": "writer 6C118; hash pinned; 35038 copies to +0x1AC",
                "what_is_missing": "decoded TMD/skeleton fields; 3D050 tail still gated",
                "blocker": "do_not_publish_into_runtime_until_3D050_tail",
                "confidence": "SOURCE_PROVEN_STRUCTURE_PARTIAL",
                "notes": "not named; type-0 resource object only",
            },
            {
                "id": "actor_plus_1B4_init",
                "needed_by": "15240 / 6CC68 / 3A088",
                "surface": "actor+0x1B4 dest object",
                "what_is_known": "in-actor dest; 15240 dest=actor+0x1B4; empty when +0x1AC==0",
                "what_is_missing": "35038 +0x1AC!=0 stores that fill dest+0 / dest+0x24",
                "blocker": "362B8/3D050 not this lane",
                "confidence": "CONSUMER_PROVEN_WRITER_PARTIAL",
                "notes": "do not invent host-side dest",
            },
            {
                "id": "type1_type3_type6_B0E70",
                "needed_by": "1AA78 / 362B8",
                "surface": "D_800B0E70[1,3,6]",
                "what_is_known": "m0005i hdr+0x0C writes idB 2 and 5 only; 6C118 writes [0] only",
                "what_is_missing": "any later writer for types 1/3/6 on this encounter",
                "blocker": "empty_+0x1AC_is_authentic",
                "confidence": "PROVEN_ABSENT_ON_THIS_PATH",
                "notes": "35038 ORs +0x98 0xE0; 1AA78 0x80 no-op",
            },
            {
                "id": "slot_ids_1332_1333_1334",
                "needed_by": "m0005i module2 0x5A",
                "surface": "tags 50/51/52",
                "what_is_known": "numeric IDs only; written by 2FF78",
                "what_is_missing": "table that maps 1332/1333/1334 to a model/script",
                "blocker": "do_not_name_from_appearance",
                "confidence": "NUMERIC_ONLY",
                "notes": "see pe-btl0-field-battle-handoff",
            },
            {
                "id": "ce2_14_clip_semantics",
                "needed_by": "1A680 / 1A4AC ticker",
                "surface": "CE2=14 idB 0-23,28",
                "what_is_known": "sizes/frames/hashes; cmd4 is first 29810 bind",
                "what_is_missing": "clip meaning; do not name from appearance",
                "blocker": "semantics_out_of_scope",
                "confidence": "BANK_PROVEN",
                "notes": "type0 0x2E uses 0x15/0x1C here; 0x1E/0x1F/0x20 are Writer A",
            },
            {
                "id": "m0367i_type0_2e_15_after_dest",
                "needed_by": "M0367I listed type-0 0x2E(0x15)",
                "surface": "D_800B0E98[21]",
                "what_is_known": "6B35C zeros B0E98 on dest enter; M0367I Writer A type0 is cmd 0x18 only",
                "what_is_missing": "whether 6BECC/Writer B republishes CE2=14 after M0367I load",
                "blocker": "slot_empty_unless_Writer_B_reruns",
                "confidence": "PROVEN_GAP",
                "notes": "m0005i 0x2E(0x15) is CE2=14 idB=21; that row does not survive 6B35C",
            },
            {
                "id": "m0367i_type2_3_op_c1",
                "needed_by": "M0367I spawned type 2/3/4 first visit",
                "surface": "0xC1 / 0x9D then 0x2E",
                "what_is_known": "0xC1=19AC0 actor+0x98|=0x400 ret1; type2/3 then 0x2E(0x09); type4 0x9D then 0x2E(0x07/0x01)",
                "what_is_missing": "0x9D field write (lh +0x224 / sh +0x14); type4 later 0x44 at 0x800136C0",
                "blocker": "0x9D/0x44_not_this_lane",
                "confidence": "HANDLER_AND_BIND_PROVEN",
                "notes": "do not implement the VM here",
            },
            {
                "id": "m0367i_type1_B0E70",
                "needed_by": "M0367I 125E0 type 1",
                "surface": "D_800B0E70[1]",
                "what_is_known": "125E0 desc count=1 type=1 idB=0; first op 0x40; hdr+0x0C is idB 2/3/4 only; type1 0x08 first8 types 2,2,4,3,4,2,3,4 idB=0",
                "what_is_missing": "persist 0x09 tag 74 gate on first visit; type1 itself has no Writer A / no 0x2E",
                "blocker": "empty_+0x1AC_authentic_until_spawn",
                "confidence": "DESC_AND_08_PROVEN",
                "notes": "do not infer story meaning from M0367I",
            },
            {
                "id": "m0367i_hdr20_rec1",
                "needed_by": "6B4F8 overlay+0x950 second record",
                "surface": "hdr+0x20 rec[1]",
                "what_is_known": "count=2; rec[0] is 6486B sha 467bf214… shared with m0005i",
                "what_is_missing": "rec[1] size 16921664 is not a payload; encoding",
                "blocker": "sentinel_or_packed_not_a_blob",
                "confidence": "PROVEN_GAP",
                "notes": "m0005i rec[1] is the same shape (size ~16MB, empty hash)",
            },
        ]

    def _wa_ce(self, scene: str) -> tuple[bytes, list[dict], list[dict] | None]:
        if scene == "m0005i":
            blob = self.m5_c2
            wa = walk12(blob, struct.unpack_from("<I", blob, self.m5_hdr + 0x10)[0])
            ce = walk12(self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x10)[0])
            return blob, wa, ce
        blob = self.m367_c2
        wa = walk12(blob, struct.unpack_from("<I", blob, self.m367_hdr + 0x10)[0])
        ce = walk12(self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x10)[0])
        return blob, wa, ce

    def _resolve_cmd(self, typ: int, cmd: int, wa: list[dict], ce: list[dict] | None, scene: str):
        rec = next((r for r in wa if r["ida"] == typ and r["idb"] == cmd), None)
        if rec is not None:
            return "WriterA", rec, "room_hdr+0x10", "0x8006B84C"
        if typ == 0 and ce is not None:
            rec = next((r for r in ce if r["ida"] == 0 and r["idb"] == cmd), None)
            if rec is not None:
                bank = "CE2=14"
                if scene == "m0367i":
                    bank = "CE2=14_cleared_by_6B35C"
                return bank, rec, "ce2_14_clip_bank", "0x8006C158"
        return "UNRESOLVED", None, "", ""

    def command_binds(self) -> list[dict]:
        rows = []
        jobs = (
            ("m0005i", self.m5_c2, LIST_OFF, 7),
            ("m0367i", self.m367_c2, M367_LIST_OFF, 5),
        )
        for scene, blob, list_off, ntyp in jobs:
            _, wa, ce = self._wa_ce(scene)
            for typ in range(ntyp):
                base = script_base(blob, list_off, typ)
                insns = walk_script(blob, base, 800)
                last_cmd = ""
                last_bank = ""
                for insn in insns:
                    if insn["op"] not in (0x2E, 0x2F):
                        continue
                    imm = insn["imms"][0] if insn["imms"] else 0
                    if insn["op"] == 0x2E:
                        bank, rec, pkg, writer_va = self._resolve_cmd(typ, imm, wa, ce, scene)
                        last_cmd = f"0x{imm:02X}"
                        last_bank = bank
                        dest_field = "actor+0x1B0"
                        dest_slot = f"D_800B0E98[{typ}*192+{imm}*4]"
                        consumer = "func_80017AE8 -> func_8001A680"
                        path = (
                            "6C140 Writer B"
                            if bank.startswith("CE2=14")
                            else "6B84C Writer A"
                            if bank == "WriterA"
                            else ""
                        )
                        rows.append(
                            {
                                "scene": scene,
                                "actor_type": typ,
                                "script_rel": f"0x{insn['rel']:X}",
                                "opcode": "0x2E",
                                "imm": f"0x{imm:02X}",
                                "bank": bank,
                                "package": pkg if rec else "",
                                "ida": rec["ida"] if rec else "",
                                "idb": rec["idb"] if rec else imm,
                                "source_offset": f"0x{rec['ptr']:X}" if rec else "",
                                "size": rec["size"] if rec else "",
                                "frames": rec["b2"] if rec else "",
                                "sha256": rec["sha256"] if rec else "",
                                "dest_slot": dest_slot,
                                "dest_field": dest_field,
                                "writer": "func_8006C140" if bank.startswith("CE2=14") else "func_8006B84C" if bank == "WriterA" else "",
                                "writer_va": writer_va,
                                "first_consumer": consumer,
                                "publication_path": path,
                                "confidence": "PROVEN_GAP" if "cleared" in bank else "PROVEN",
                                "notes": (
                                    "first live type0 bind after 0xAA/0x40/0x65"
                                    if scene == "m0005i" and typ == 0 and insn["rel"] == 0x334
                                    else "0x2F is a frame cap, not a command"
                                    if False
                                    else "do not name clip from appearance"
                                ),
                            }
                        )
                    else:
                        rows.append(
                            {
                                "scene": scene,
                                "actor_type": typ,
                                "script_rel": f"0x{insn['rel']:X}",
                                "opcode": "0x2F",
                                "imm": f"0x{imm:X}",
                                "bank": "frame_cap",
                                "package": "",
                                "ida": typ,
                                "idb": last_cmd,
                                "source_offset": "",
                                "size": "",
                                "frames": imm,
                                "sha256": "",
                                "dest_slot": "",
                                "dest_field": "actor+0x12",
                                "writer": "func_80017B34",
                                "writer_va": "0x80017B34",
                                "first_consumer": "func_80017B34 min(+0x0F, imm)",
                                "publication_path": f"after_0x2E {last_cmd} {last_bank}",
                                "confidence": "PROVEN",
                                "notes": "cap vs +0x0F=frames-1; not a command ID",
                            }
                        )
        return rows

    def writer_a_clips(self) -> list[dict]:
        rows = []
        used_m5 = {(0, 0x1E), (0, 0x1F), (0, 0x20), (2, 0x02), (2, 0x06), (2, 0x13), (2, 0x14), (2, 0x15), (2, 0x16), (2, 0x17), (5, 0x00), (5, 0x02)}
        specs = (
            ("m0005i", self.m5_c2, self.m5_hdr, used_m5, "m0005i_chunk2"),
            ("m0367i", self.m367_c2, self.m367_hdr, None, "m0367i_chunk2"),
        )
        first_cons = {
            ("m0005i", 0, 0x1E): "type0 0x2E +0x3B0",
            ("m0005i", 0, 0x1F): "type0 0x2E +0x498",
            ("m0005i", 0, 0x20): "type0 0x2E +0xC4C",
            ("m0005i", 2, 0x06): "listed type2 0x2E +0xE4",
            ("m0005i", 5, 0x02): "type5 0x2E +0x80 after type1 0x08",
            ("m0005i", 5, 0x00): "type5 0x2E +0xB8 after type1 0x08",
            ("m0367i", 0, 0x18): "listed type0; no 0x2E of 0x18 in prefix",
            ("m0367i", 2, 0x09): "type2 0x2E +0x94 after 0xC1",
            ("m0367i", 3, 0x09): "type3 0x2E +0x84 after 0xC1",
            ("m0367i", 4, 0x07): "type4 0x2E +0xF0 after 0x9D",
            ("m0367i", 4, 0x01): "type4 0x2E +0x1E8 after 0x9D",
        }
        m319 = next(r for r in self.dest_hops() if r["decoded_name"] == "M0319I")
        rel = int(m319["rel"], 16)
        packed = int(m319["packed"], 16)
        s0, s1, s2 = packed_secs(packed)
        m319_c2 = read_form1(self.disc, PE_IMG_LBA + rel + s0 + s1, s2)
        m319_hdr = struct.unpack_from("<I", m319_c2, 4)[0] & 0x3FFFFF
        specs = specs + (("m0319i", m319_c2, m319_hdr, None, "m0319i_chunk2"),)
        for scene, blob, hdr, filt, pkg in specs:
            wa = walk12(blob, struct.unpack_from("<I", blob, hdr + 0x10)[0])
            for rec in wa:
                if filt is not None and (rec["ida"], rec["idb"]) not in filt:
                    continue
                if scene == "m0319i":
                    reach = "m0367i_type1_0x31_persist"
                elif scene == "m0367i":
                    reach = (
                        "type1_0x08"
                        if rec["ida"] in (2, 3, 4)
                        else "listed_type0"
                        if rec["ida"] == 0
                        else "unreferenced"
                    )
                elif rec["ida"] == 0:
                    reach = "type0_0x2E"
                elif rec["ida"] == 2:
                    reach = "listed_type2_0x2E"
                else:
                    reach = "type1_0x08_type5_0x2E"
                rows.append(
                    {
                        "scene": scene,
                        "ida": rec["ida"],
                        "idb": f"0x{rec['idb']:02X}",
                        "source_package": pkg,
                        "source_offset": f"0x{rec['ptr']:X}",
                        "size": rec["size"],
                        "sha256": rec["sha256"],
                        "frames": rec["b2"],
                        "enc": rec["b0"],
                        "bones_minus_1": rec["b1"],
                        "dest_slot": f"D_800B0E98[{rec['ida']}*192+{rec['idb']}*4]",
                        "writer_pc": "0x8006B84C",
                        "reachable_via": reach,
                        "first_known_consumer": first_cons.get(
                            (scene, rec["ida"], rec["idb"]),
                            "type1_0x08 spawn; no 0x2E in walked prefix"
                            if scene == "m0367i" and rec["ida"] in (2, 3, 4)
                            else "M0319I Writer A; persist-gated dest"
                            if scene == "m0319i"
                            else "listed_stream_0x2E",
                        ),
                        "confidence": "PROVEN",
                        "notes": "do not name clip from appearance",
                    }
                )
        return rows

    def spawn_descriptors(self) -> list[dict]:
        rows = []
        jobs = (
            ("m0005i", self.m5_c2, LIST_OFF, 1),
            ("m0367i", self.m367_c2, M367_LIST_OFF, 1),
        )
        models = {
            "m0005i": {
                r["idb"]: r
                for r in walk12(
                    self.m5_c2,
                    struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x0C)[0],
                )
            },
            "m0367i": {
                r["idb"]: r
                for r in walk12(
                    self.m367_c2,
                    struct.unpack_from("<I", self.m367_c2, self.m367_hdr + 0x0C)[0],
                )
            },
        }
        scripts = {(r["scene"], int(r["actor_type"])): r for r in self.scripts()}
        for scene, blob, list_off, parent in jobs:
            base = script_base(blob, list_off, parent)
            insns = walk_script(blob, base, 400)
            order = 0
            for insn in insns:
                if insn["op"] != 0x08 or len(insn["imms"]) < 5:
                    continue
                order += 1
                typ, idb, x, y, z = insn["imms"][:5]
                rec = models[scene].get(typ)
                if scene == "m0005i" and typ == 0:
                    src = "ce2_14_plus_8_via_6C118"
                    sha = ""
                elif rec:
                    src = f"B0E70[{typ}]"
                    sha = rec["sha256"]
                else:
                    src = "B0E70 empty"
                    sha = ""
                rows.append(
                    {
                        "scene": scene,
                        "parent_type": parent,
                        "script_rel": f"0x{insn['rel']:X}",
                        "spawn_order": order,
                        "actor_type": typ,
                        "idb": idb,
                        "pose_x": f"0x{x:X}",
                        "pose_y": f"0x{y:X}",
                        "pose_z": f"0x{z:X}",
                        "model_source": src,
                        "model_sha256": sha,
                        "first_opcode": scripts.get((scene, typ), {}).get("first_opcode", ""),
                        "confidence": "PROVEN",
                        "notes": (
                            "1735C argc5; persist 0x09 tag 74"
                            if scene == "m0367i"
                            else "1735C argc5; first three match BTL22, then types 2 and 4"
                        ),
                    }
                )
        return rows

    def dest_hops(self) -> list[dict]:
        rows = []
        base = script_base(self.m367_c2, M367_LIST_OFF, 1)
        insns = walk_script(self.m367_c2, base, 400)
        seen: set[int] = set()
        for insn in insns:
            if insn["op"] != 0x31 or not insn["imms"]:
                continue
            token = insn["imms"][0]
            name = decode_token(self.exe, token)
            idx = token_table_index(name)
            rel = load_u32(self.exe, 0x80093378 + idx * 8)
            packed = load_u32(self.exe, 0x80093378 + idx * 8 + 4)
            s0, s1, s2 = packed_secs(packed)
            sectors = s0 + s1 + s2
            key = token
            sha = ""
            size = sectors * FORM1_USER
            if key not in seen:
                seen.add(key)
                if token == M0005I_TOKEN:
                    sha = hashlib.sha256(self.m5_full).hexdigest()
                    size = len(self.m5_full)
                elif token == M0367I_TOKEN:
                    sha = hashlib.sha256(self.m367_full).hexdigest()
                    size = len(self.m367_full)
                else:
                    blob = read_form1(self.disc, PE_IMG_LBA + rel, sectors)
                    sha = hashlib.sha256(blob).hexdigest()
                    size = len(blob)
            rows.append(
                {
                    "scene": "m0367i",
                    "parent_type": 1,
                    "script_rel": f"0x{insn['rel']:X}",
                    "token": f"0x{token:08X}",
                    "decoded_name": name,
                    "table_index": idx,
                    "rel": f"0x{rel:X}",
                    "packed": f"0x{packed:08X}",
                    "sectors": f"{s0}+{s1}+{s2}",
                    "size": size if key in seen else "",
                    "sha256": sha,
                    "confidence": "PROVEN",
                    "notes": "0x31 stores D_8009D280; persist-gated; do not hop the runtime",
                }
            )
        # fill sha on later duplicate tokens
        by_tok = {r["token"]: r for r in rows if r["sha256"]}
        for r in rows:
            if not r["sha256"] and r["token"] in by_tok:
                r["sha256"] = by_tok[r["token"]]["sha256"]
                r["size"] = by_tok[r["token"]]["size"]
        return rows

    def dest_package_heads(self) -> list[dict]:
        rows = []
        seen: set[str] = set()
        for hop in self.dest_hops():
            name = hop["decoded_name"]
            if name in seen:
                continue
            seen.add(name)
            idx = int(hop["table_index"])
            rel = int(hop["rel"], 16)
            packed = int(hop["packed"], 16)
            s0, s1, s2 = packed_secs(packed)
            if name == "M0005I":
                c2 = self.m5_c2
                hdr = self.m5_hdr
            elif name == "M0367I":
                c2 = self.m367_c2
                hdr = self.m367_hdr
            else:
                c2 = read_form1(self.disc, PE_IMG_LBA + rel + s0 + s1, s2)
                hdr = struct.unpack_from("<I", c2, 4)[0] & 0x3FFFFF
            recs0c = walk12(c2, struct.unpack_from("<I", c2, hdr + 0x0C)[0])
            recs10 = walk12(c2, struct.unpack_from("<I", c2, hdr + 0x10)[0])
            recs14 = walk12(c2, struct.unpack_from("<I", c2, hdr + 0x14)[0])
            list_off = recs14[0]["ptr"] if recs14 else 0
            nlist = struct.unpack_from("<I", c2, list_off + 4)[0] if list_off else 0
            word0 = struct.unpack_from("<I", c2, list_off)[0] if list_off else 0
            desc = list_off + word0 if list_off else 0
            desc_n = c2[desc] if desc and desc < len(c2) else 0
            desc_types = [c2[desc + 1 + i * 2] for i in range(desc_n)] if desc_n else []
            t1_op = ""
            if nlist > 1:
                base = list_off + struct.unpack_from("<I", c2, list_off + 12)[0]
                ins = walk_script(c2, base, 1)
                if ins:
                    t1_op = f"0x{ins[0]['op']:02X}"
            rows.append(
                {
                    "decoded_name": name,
                    "token": hop["token"],
                    "table_index": idx,
                    "chunk2_size": len(c2),
                    "chunk2_sha256": hashlib.sha256(c2).hexdigest(),
                    "hdr_0c_idbs": ",".join(str(r["idb"]) for r in recs0c),
                    "writera_count": len(recs10),
                    "list_count": nlist,
                    "desc_count": desc_n,
                    "desc_types": ",".join(str(x) for x in desc_types),
                    "type1_first_opcode": t1_op,
                    "confidence": "PROVEN",
                    "notes": "persist-gated 0x31 from M0367I type1; do not hop runtime",
                }
            )
        return rows


def build() -> tuple[World, dict[str, tuple[list[str], list[dict]]]]:
    world = World()
    world.pin_exe()
    tables = {
        "ACTOR_RESOURCES.csv": (
            [
                "actor_type",
                "scene",
                "descriptor_source",
                "descriptor_offset",
                "script_source",
                "script_offset",
                "script_sha256",
                "first_vm_pc",
                "first_opcode",
                "model_resource_source",
                "model_offset",
                "model_size",
                "model_sha256",
                "animation_bank",
                "publication_global",
                "resource_object_writer",
                "update_function",
                "known_field_offsets",
                "spawn_path",
                "confidence",
                "notes",
            ],
            world.actors(),
        ),
        "BATTLE_SCRIPTS.csv": (
            [
                "actor_type",
                "scene",
                "source",
                "base_offset",
                "window_size",
                "sha256",
                "first_word",
                "first_opcode",
                "opcode_sequence",
                "branch_targets",
                "yield_points",
                "known_opcodes",
                "unknown_opcodes",
                "confidence",
                "notes",
            ],
            world.scripts(),
        ),
        "RESOURCE_PUBLICATION.csv": (
            [
                "object",
                "field_or_global",
                "writer",
                "writer_va",
                "source_data",
                "source_offset",
                "lifetime",
                "publication_order",
                "consumer",
                "consumer_va",
                "actor_type",
                "confidence",
                "notes",
            ],
            world.publication(),
        ),
        "PEIMG_PACKAGES.csv": (
            [
                "package_id",
                "token",
                "decoded_name",
                "table_index",
                "rel",
                "packed",
                "sectors",
                "peimg_lba_range",
                "size",
                "sha256",
                "compression",
                "decoded_structure",
                "consumer",
                "writer",
                "confidence",
                "notes",
            ],
            world.packages(),
        ),
        "UNKNOWN_DEPENDENCIES.csv": (
            [
                "id",
                "needed_by",
                "surface",
                "what_is_known",
                "what_is_missing",
                "blocker",
                "confidence",
                "notes",
            ],
            world.unknowns(),
        ),
        "COMMAND_BINDS.csv": (
            [
                "scene",
                "actor_type",
                "script_rel",
                "opcode",
                "imm",
                "bank",
                "package",
                "ida",
                "idb",
                "source_offset",
                "size",
                "frames",
                "sha256",
                "dest_slot",
                "dest_field",
                "writer",
                "writer_va",
                "first_consumer",
                "publication_path",
                "confidence",
                "notes",
            ],
            world.command_binds(),
        ),
        "WRITER_A_CLIPS.csv": (
            [
                "scene",
                "ida",
                "idb",
                "source_package",
                "source_offset",
                "size",
                "sha256",
                "frames",
                "enc",
                "bones_minus_1",
                "dest_slot",
                "writer_pc",
                "reachable_via",
                "first_known_consumer",
                "confidence",
                "notes",
            ],
            world.writer_a_clips(),
        ),
        "SPAWN_DESCRIPTORS.csv": (
            [
                "scene",
                "parent_type",
                "script_rel",
                "spawn_order",
                "actor_type",
                "idb",
                "pose_x",
                "pose_y",
                "pose_z",
                "model_source",
                "model_sha256",
                "first_opcode",
                "confidence",
                "notes",
            ],
            world.spawn_descriptors(),
        ),
        "DEST_HOPS.csv": (
            [
                "scene",
                "parent_type",
                "script_rel",
                "token",
                "decoded_name",
                "table_index",
                "rel",
                "packed",
                "sectors",
                "size",
                "sha256",
                "confidence",
                "notes",
            ],
            world.dest_hops(),
        ),
        "DEST_PACKAGE_HEADS.csv": (
            [
                "decoded_name",
                "token",
                "table_index",
                "chunk2_size",
                "chunk2_sha256",
                "hdr_0c_idbs",
                "writera_count",
                "list_count",
                "desc_count",
                "desc_types",
                "type1_first_opcode",
                "confidence",
                "notes",
            ],
            world.dest_package_heads(),
        ),
    }
    return world, tables


def write_tables(tables: dict[str, tuple[list[str], list[dict]]]) -> None:
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    for name, (fields, rows) in tables.items():
        csv_write(EVIDENCE / name, fields, rows)


def verify_tables(tables: dict[str, tuple[list[str], list[dict]]]) -> None:
    for name, (fields, rows) in tables.items():
        path = EVIDENCE / name
        require(path.is_file(), f"missing {path}")
        got_fields, got_rows = csv_read(path)
        require(got_fields == fields, f"{name} columns")
        require(len(got_rows) == len(rows), f"{name} row count {len(got_rows)}!={len(rows)}")
        for i, (want, got) in enumerate(zip(rows, got_rows)):
            for key in fields:
                require(str(want[key]) == got[key], f"{name}[{i}].{key}")


def verify_report(tables: dict[str, tuple[list[str], list[dict]]]) -> None:
    path = EVIDENCE / "REPORT.md"
    require(path.is_file(), "missing REPORT.md")
    text = path.read_text(encoding="utf-8")
    for needle in (
        "PE battle-data precovery",
        SHA1,
        M0005I_CHUNK2,
        M0005I_FULL,
        CE214_SHA,
        CLIP4_SHA,
        M0367I_CHUNK2,
        "D_800B0E70",
        "D_800B0E98",
        "func_8006B35C",
        "func_8006C118",
        "RUNTIME_HANDOFF",
        "0x2E",
        "0x2F",
        CMD15_SHA,
        CMD1C_SHA,
        "fafb08d77f8d9f0c79b44835c8910fdca6fb0a1fe37e82cc406bdc9362995d85",
    ):
        require(needle in text, f"REPORT missing {needle}")
    pkgs = {r["package_id"]: r for r in tables["PEIMG_PACKAGES.csv"][1]}
    require(pkgs["m0005i"]["sha256"] in text, "REPORT m0005i sha")
    require(pkgs["ce2_14_clip_bank"]["sha256"] in text, "REPORT CE2=14 sha")


def main() -> int:
    write = "--write" in sys.argv
    world, tables = build()
    type0 = next(r for r in tables["BATTLE_SCRIPTS.csv"][1] if r["actor_type"] == 0)
    type6 = next(r for r in tables["BATTLE_SCRIPTS.csv"][1] if r["actor_type"] == 6)
    require(type0["first_opcode"] == "0x9B", "type0 first 0x9B")
    require(type6["first_opcode"] == "0xCE", "type6 first 0xCE")
    require(type6["first_word"] == "0x000080CE", "type6 word")
    m367_t1 = next(
        r
        for r in tables["BATTLE_SCRIPTS.csv"][1]
        if r["scene"] == "m0367i" and r["actor_type"] == 1
    )
    require(m367_t1["first_opcode"] == "0x40", "M0367I type1 0x40")
    binds = tables["COMMAND_BINDS.csv"][1]
    t0_2e = [
        r
        for r in binds
        if r["scene"] == "m0005i" and int(r["actor_type"]) == 0 and r["opcode"] == "0x2E"
    ]
    cmds = tuple(sorted({int(r["imm"], 16) for r in t0_2e}))
    require(cmds == TYPE0_2E_CMDS, f"type0 0x2E cmds {cmds}")
    first = next(r for r in t0_2e if r["script_rel"] == "0x334")
    require(first["imm"] == "0x15" and first["sha256"] == CMD15_SHA, "first 0x2E 0x15")
    require(first["bank"] == "CE2=14", "first 0x2E bank")
    cmd1c = next(r for r in t0_2e if r["imm"] == "0x1C")
    require(cmd1c["sha256"] == CMD1C_SHA and cmd1c["bank"] == "CE2=14", "0x2E 0x1C")
    for imm, bank in (("0x1E", "WriterA"), ("0x1F", "WriterA"), ("0x20", "WriterA")):
        row = next(r for r in t0_2e if r["imm"] == imm)
        require(row["bank"] == bank, f"type0 {imm} bank")
    wa = tables["WRITER_A_CLIPS.csv"][1]
    m367_wa = [r for r in wa if r["scene"] == "m0367i"]
    require(len(m367_wa) == 36, f"M0367I Writer A {len(m367_wa)}")
    m319_wa = [r for r in wa if r["scene"] == "m0319i"]
    require(len(m319_wa) == 6, f"M0319I Writer A {len(m319_wa)}")
    require(not any(int(r["ida"]) == 1 for r in m367_wa), "no type1 Writer A")
    t0_18 = next(r for r in m367_wa if int(r["ida"]) == 0 and r["idb"] == "0x18")
    require(t0_18["sha256"] == WA18_SHA, "M0367I type0 cmd 0x18")
    spawns = [
        r
        for r in tables["SPAWN_DESCRIPTORS.csv"][1]
        if r["scene"] == "m0367i"
    ]
    first8 = tuple(int(r["actor_type"]) for r in spawns[:8])
    require(first8 == M367_FIRST8_08, f"M0367I first8 0x08 {first8}")
    hops = tables["DEST_HOPS.csv"][1]
    require(any(r["decoded_name"] == "M0319I" for r in hops), "M0319I hop")
    m367_t2_2e = next(
        r
        for r in binds
        if r["scene"] == "m0367i" and int(r["actor_type"]) == 2 and r["opcode"] == "0x2E"
    )
    require(m367_t2_2e["imm"] == "0x09" and m367_t2_2e["bank"] == "WriterA", "M0367I t2 0x2E")
    heads = tables["DEST_PACKAGE_HEADS.csv"][1]
    m319 = next(r for r in heads if r["decoded_name"] == "M0319I")
    require(m319["desc_types"] == "1" and int(m319["writera_count"]) == 6, "M0319I head")
    m5head = next(r for r in heads if r["decoded_name"] == "M0005I")
    require(m5head["desc_types"] == "1,6", "M0005I desc types")
    if write:
        write_tables(tables)
    verify_tables(tables)
    verify_report(tables)
    print(
        "PASS: battle-data precovery; "
        f"m0005i {len(world.m5_full)}B; CE2=14 25 clips; "
        f"B0E70 idB {sorted(r['idb'] for r in walk12(world.m5_c2, struct.unpack_from('<I', world.m5_c2, world.m5_hdr + 0x0C)[0]))}; "
        "scripts 0-6; 6B35C/6B804/6C118 pinned; "
        f"type0 0x2E {len(t0_2e)}; M0367I WA {len(m367_wa)}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

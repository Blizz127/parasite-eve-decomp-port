#!/usr/bin/env python3
"""PE-BTL126 — post-1266C scratch[0]|=4 provenance.

EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
m0005i chunk2 SHA-256 01a64ba3…7e3b.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
HDR = 0x800
PE_IMG_LBA = 1013
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048
M0005I_REL = 0x266A
M0005I_PACKED = 0x0600A921
M0005I_CHUNK2 = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
LIST_OFF = 0x202A4
ROOT = pathlib.Path(__file__).resolve().parents[2]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


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
    raise SystemExit("FAIL: missing Disc 1")


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            chunk = fh.read(FORM1_USER)
            require(len(chunk) == FORM1_USER, f"disc read {lba + i}")
            out += chunk
    return bytes(out)


def packed_secs(packed: int) -> tuple[int, int, int]:
    return packed & 0xFF, (packed >> 8) & 0xFFF, packed >> 20


def decode_insn(blob: bytes, pc: int) -> dict | None:
    if pc + 8 > len(blob):
        return None
    word, word2 = struct.unpack_from("<II", blob, pc)
    op = word & 0x1FFF
    argc = (word >> 13) & 0xF
    span = 8 + argc * 4
    if pc + span > len(blob):
        return None
    kinds_bits = word >> 17
    kinds = []
    imms = []
    for i in range(argc):
        if i == 5:
            kinds_bits = word2
        kinds.append(kinds_bits & 7)
        kinds_bits >>= 3
        imms.append(struct.unpack_from("<I", blob, pc + 8 + i * 4)[0])
    return {
        "word": word,
        "op": op,
        "argc": argc,
        "kinds": kinds,
        "imms": imms,
        "span": span,
    }


def linear_walk(blob: bytes, base: int, end: int) -> list[dict]:
    rows = []
    pc = base
    while pc + 8 <= end:
        insn = decode_insn(blob, pc)
        if insn is None or insn["op"] > 0x1FF:
            break
        insn["rel"] = pc - base
        rows.append(insn)
        pc += insn["span"]
    return rows


def script_base(blob: bytes, list_off: int, typ: int) -> int:
    rel = struct.unpack_from("<I", blob, list_off + 8 + typ * 4)[0]
    return list_off + rel


def load_exe() -> bytes:
    for p in (
        ROOT / "build" / "disc1.candidate.exe",
        ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62",
    ):
        if p.is_file():
            blob = p.read_bytes()
            require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
            return blob
    raise SystemExit("FAIL: missing retail EXE")


def exe_imm_6a80(exe: bytes) -> list[int]:
    hits = []
    text = exe[HDR:]
    for i in range(0, len(text) - 4, 4):
        w = struct.unpack_from("<I", text, i)[0]
        if (w & 0xFFFF) == 0x6A80:
            hits.append(0x80010000 + i)
    return hits


def main() -> int:
    exe = load_exe()
    hits = exe_imm_6a80(exe)
    require(hits == [0x800126D0, 0x800171F4, 0x80034F3C], f"EXE 0x6A80 {hits}")
    require(jal_target(load_u32(exe, 0x8003F0B8)) == 0x8001266C, "3F074 jal 1266C")
    require(load_u32(exe, 0x80017A68) == 0x00431025, "0x2A or dest")
    require(load_u32(exe, 0x800910A0 + 0x2A * 4) == 0x80017A50, "jtbl 0x2A")
    require(load_u32(exe, 0x800910A0 + 0x1F * 4) == 0x800177AC, "jtbl 0x1F")
    require(load_u32(exe, 0x800910A0 + 0x20 * 4) == 0x800172FC, "jtbl 0x20")
    require(load_u32(exe, 0x8001731C) == 0x8C820000, "0x05 lw arg0")
    require(load_u32(exe, 0x8001732C) == 0x14400009, "0x05 bne skip-if-false")

    disc = find_disc()
    s0, s1, s2 = packed_secs(M0005I_PACKED)
    chunk2 = read_form1(disc, PE_IMG_LBA + M0005I_REL + s0 + s1, s2)
    require(hashlib.sha256(chunk2).hexdigest() == M0005I_CHUNK2, "chunk2 sha")

    bases = [script_base(chunk2, LIST_OFF, t) for t in range(7)]
    ends = bases[1:] + [0x25014]
    bit2 = []
    scratch0_stores = []
    for typ, base, end in zip(range(7), bases, ends):
        for insn in linear_walk(chunk2, base, end):
            op = insn["op"]
            kinds = insn["kinds"]
            imms = insn["imms"]
            dest0 = False
            if op in (0x0A, 0x2A, 0x28, 0x1F) and kinds and kinds[0] == 4 and imms[0] == 0:
                dest0 = True
            if op == 0x09 and len(kinds) > 1 and kinds[1] == 4 and imms[1] == 0:
                dest0 = True
            if dest0:
                scratch0_stores.append((typ, insn["rel"], op, kinds, imms))
            if (
                op == 0x2A
                and kinds[:2] == [4, 0]
                and imms[:2] == [0, 2]
            ):
                bit2.append((typ, insn["rel"]))

    require(bit2 == [(6, 0x1850)], f"bit2 setters {bit2}")

    t6 = {i["rel"]: i for i in linear_walk(chunk2, bases[6], ends[6])}
    require(t6[0x190]["word"] == 0x02308009, "wait AND scratch[0]&4")
    require(t6[0x190]["kinds"] == [0, 3, 4, 0], "wait dest is cond")
    require(t6[0x1A8]["imms"][0] == 7, "wait is_zero")
    require(t6[0x1C0]["imms"][1] == 244, "skip imm 0xF4 -> +0x1E8")
    require(t6[0x1850]["word"] == 0x0008402A, "type-6 +0x1850 0x2A[0,2]")
    require(t6[0x180]["imms"] == [2, 1668], "mailbox label +0xD08")
    require(t6[0x3F8]["imms"] == [2, 0, 125], "0x1C(2,0,0x7D)")
    require(t6[0x394]["op"] == 0x89, "0x89 after wait release")

    t2 = {i["rel"]: i for i in linear_walk(chunk2, bases[2], ends[2])}
    require(t2[0x3C]["imms"] == [2, 84], "type-2 +0x19C -> +0xA8")
    require(t2[0xA8]["op"] == 0x1F, "mailbox starts at 0x1F")
    require(t2[0x98]["op"] == 0x20, "main task parks 0x20")
    require(t2[0x4B8]["imms"][3] == 0x7D, "0x7D compare")
    require(t2[0x484]["imms"][3] == 0x7F, "0x7F compare")
    require(t2[0x4AC]["op"] == 0x00 and t2[0x4AC]["imms"][0] == 902, "0x7F goto +0x70C")
    require(t2[0x804]["op"] == 0x6F, "0x6F at +0x804")
    require(t2[0x534]["imms"] == [6, 0, 0x84], "0x7D arm sends 0x84 to type-6")

    t0 = {i["rel"]: i for i in linear_walk(chunk2, bases[0], ends[0])}
    require(t0[0x1150]["word"] == 0x0008402A, "type-0 +0x1150 is also 0x2A kind4,0")
    require(t0[0x1150]["imms"][1] == 4, "type-0 0x2A is bit 4, not bit 2")
    require(t0[0x608]["op"] == 0x20, "new-game type-0 parks 0x20")
    require(t0[0x35C]["imms"] == [2, 0, 11], "persist-39 0x1C(2,0,0xB)")
    require(t0[0x1128]["imms"] == [2, 0, 0x7F], "type-0 0x7F is the 0x6F wake")

    print(
        "PASS: EXE B6A80 sites 1266C/17018/34F10; only m0005i "
        "scratch[0]|=4 is type-6 +0x1850; type-2 0x20 parks, mailbox "
        "0x1F at +0xA8; 0x7D handshake != 0x7F/0x6F"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

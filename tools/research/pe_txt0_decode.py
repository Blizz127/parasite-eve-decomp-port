#!/usr/bin/env python3
"""PE-TXT0 evidence-only retail text decoder.

Validates a registered USA Disc 1 MODE2/2352 image, locates the shared
field slot-7 string bank, and decodes a message ID through the retail
FF/F9 FE <id> lookup used by func_80037870.

Research only. Not a production text runtime. A --png write is an
explicitly non-production diagnostic that uses a host 5x7 font.

Example:
  python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" --message 0x21

Decoded text goes to stdout, or with --png to a file under build/ only
(git-ignored); never paste it into tracked files.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
import zlib
from pathlib import Path
from typing import BinaryIO, Iterable

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
PE_IMG_LBA = 1013
PE_IMG_BYTES = 206_213_120

LOAD = 0x80010000
EXE_HDR = 0x800
FIELD_TABLE_OFF = 0x83B78
DISPATCH = 0x800910A0
HANDLER_0D = 0x80017410
HANDLER_22 = 0x800177C8
OPEN_ALLOC = 0x800375E0
POLL = 0x80037548
WINDOW_UPDATE = 0x80037870
RECORD_BASE = 0x800BCEA8
STREAM_PTR_0 = 0x800B1628
STREAM_PTR_1 = 0x800B162C
LANG_WORD = 0x800B0CD8
WIDTH_TABLE = 0x800916A0
NAME_BUF = 0x80091694
NAME_COUNT = 0x8009169D
GP_TEXT = 0x8009CD90  # 0x120($gp) with gp = 0x8009CD70

# Shared bank copied into every first-play field package.
STREAM0_SHA256 = "467bf214b60e2b6d2972d0ce8d8cd8898ec2908f70ed65ed63802b11ad88eba1"
STREAM1_SHA256 = "231da6258a69d8c44e49509164973ef2236cabf4cd40bcce4c973ce7b91c262a"
STREAM0_SIZE = 0x1956
STREAM1_SIZE = 0x2A25

# USA boot ORs 0x40000000 into D_800B0CD8, selecting stream 1.
USA_STREAM_INDEX = 1

CURRENT_SLICE = tuple(range(0x14, 0x24))
SCENES = (
    ("m0002i", 1),
    ("m0003i", 2),
    ("m0004i", 3),
    ("m0372i", 371),
)

# Letters: retail code + 0x31 == ASCII. Punctuation is NOT that formula.
LETTER_BASE = 0x31
PUNCT = {
    0x0F: ("SPACE", " "),
    0x2B: ("EXCLAMATION", "!"),
    0x2E: ("APOSTROPHE", "'"),
    0x2F: ("PERIOD", "."),
    0x4A: ("COMMA", ","),
}

# 5x7 diagnostic host font. NOT retail. Used only for --png.
_HOST_FONT = {
    " ": [0, 0, 0, 0, 0, 0, 0],
    "!": [0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00],
    "'": [0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00],
    ",": [0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08],
    ".": [0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00],
    "?": [0x0E, 0x11, 0x02, 0x04, 0x00, 0x04, 0x00],
    "A": [0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00],
    "B": [0x1E, 0x11, 0x1E, 0x11, 0x11, 0x1E, 0x00],
    "C": [0x0E, 0x11, 0x10, 0x10, 0x11, 0x0E, 0x00],
    "D": [0x1E, 0x11, 0x11, 0x11, 0x11, 0x1E, 0x00],
    "E": [0x1F, 0x10, 0x1E, 0x10, 0x10, 0x1F, 0x00],
    "F": [0x1F, 0x10, 0x1E, 0x10, 0x10, 0x10, 0x00],
    "G": [0x0E, 0x11, 0x10, 0x13, 0x11, 0x0E, 0x00],
    "H": [0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x00],
    "I": [0x0E, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00],
    "J": [0x01, 0x01, 0x01, 0x01, 0x11, 0x0E, 0x00],
    "K": [0x11, 0x12, 0x1C, 0x12, 0x11, 0x11, 0x00],
    "L": [0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x00],
    "M": [0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x00],
    "N": [0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x00],
    "O": [0x0E, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00],
    "P": [0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x00],
    "Q": [0x0E, 0x11, 0x11, 0x15, 0x12, 0x0D, 0x00],
    "R": [0x1E, 0x11, 0x11, 0x1E, 0x12, 0x11, 0x00],
    "S": [0x0E, 0x11, 0x0C, 0x02, 0x11, 0x0E, 0x00],
    "T": [0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00],
    "U": [0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00],
    "V": [0x11, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00],
    "W": [0x11, 0x11, 0x11, 0x15, 0x1B, 0x11, 0x00],
    "X": [0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00],
    "Y": [0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x00],
    "Z": [0x1F, 0x01, 0x02, 0x04, 0x08, 0x1F, 0x00],
    "a": [0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F, 0x00],
    "b": [0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00],
    "c": [0x00, 0x0E, 0x11, 0x10, 0x11, 0x0E, 0x00],
    "d": [0x01, 0x01, 0x0F, 0x11, 0x11, 0x0F, 0x00],
    "e": [0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E, 0x00],
    "f": [0x06, 0x08, 0x1C, 0x08, 0x08, 0x08, 0x00],
    "g": [0x00, 0x0F, 0x11, 0x0F, 0x01, 0x0E, 0x00],
    "h": [0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x00],
    "i": [0x04, 0x00, 0x0C, 0x04, 0x04, 0x0E, 0x00],
    "j": [0x02, 0x00, 0x06, 0x02, 0x12, 0x0C, 0x00],
    "k": [0x10, 0x12, 0x14, 0x18, 0x14, 0x12, 0x00],
    "l": [0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00],
    "m": [0x00, 0x1A, 0x15, 0x15, 0x15, 0x15, 0x00],
    "n": [0x00, 0x1E, 0x11, 0x11, 0x11, 0x11, 0x00],
    "o": [0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00],
    "p": [0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10, 0x00],
    "q": [0x00, 0x0F, 0x11, 0x0F, 0x01, 0x01, 0x00],
    "r": [0x00, 0x16, 0x19, 0x10, 0x10, 0x10, 0x00],
    "s": [0x00, 0x0F, 0x10, 0x0E, 0x01, 0x1E, 0x00],
    "t": [0x08, 0x1C, 0x08, 0x08, 0x08, 0x06, 0x00],
    "u": [0x00, 0x11, 0x11, 0x11, 0x11, 0x0F, 0x00],
    "v": [0x00, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00],
    "w": [0x00, 0x11, 0x11, 0x15, 0x15, 0x0A, 0x00],
    "x": [0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00],
    "y": [0x00, 0x11, 0x11, 0x0F, 0x01, 0x0E, 0x00],
    "z": [0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F, 0x00],
}


class DiscError(RuntimeError):
    pass


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        while chunk := source.read(1024 * 1024):
            digest.update(chunk)
    return digest.hexdigest()


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha1_bytes(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()


class RawMode2Image:
    def __init__(self, path: Path):
        self.path = path
        self.file: BinaryIO = path.open("rb")
        self.file.seek(0, 2)
        size = self.file.tell()
        if size == 0 or size % SECTOR_RAW:
            self.file.close()
            raise DiscError(f"image size {size} is not a multiple of {SECTOR_RAW}")
        self.sector_count = size // SECTOR_RAW

    def __enter__(self) -> "RawMode2Image":
        return self

    def __exit__(self, *_args: object) -> None:
        self.file.close()

    def user_data(self, lba: int) -> bytes:
        if not 0 <= lba < self.sector_count:
            raise DiscError(f"LBA {lba} outside image")
        self.file.seek(lba * SECTOR_RAW)
        raw = self.file.read(SECTOR_RAW)
        if len(raw) != SECTOR_RAW:
            raise DiscError(f"short read at LBA {lba}")
        if raw[18] & FORM2_FLAG:
            raise DiscError(f"LBA {lba} is Form 2; Form 1 required")
        return raw[FORM1_USER_OFFSET : FORM1_USER_OFFSET + FORM1_USER_SIZE]

    def read_form1_extent(self, lba: int, size: int) -> bytes:
        sectors = (size + FORM1_USER_SIZE - 1) // FORM1_USER_SIZE
        out = bytearray()
        for i in range(sectors):
            out.extend(self.user_data(lba + i))
        return bytes(out[:size])


def iso_entries(image: RawMode2Image) -> list[tuple[str, int, int]]:
    pvd = image.user_data(16)
    if pvd[0] != 1 or pvd[1:6] != b"CD001":
        raise DiscError("missing ISO9660 PVD")
    root = pvd[156:]
    stack = [("", struct.unpack_from("<I", root, 2)[0], struct.unpack_from("<I", root, 10)[0])]
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
                rec_len = data[offset]
                if rec_len == 0:
                    break
                rec = data[offset : offset + rec_len]
                offset += rec_len
                name_len = rec[32]
                name_bytes = rec[33 : 33 + name_len]
                if name_bytes in (b"\x00", b"\x01"):
                    continue
                name = name_bytes.decode("ascii")
                child_lba = struct.unpack_from("<I", rec, 2)[0]
                child_size = struct.unpack_from("<I", rec, 10)[0]
                path = f"{prefix}/{name}" if prefix else name
                if rec[25] & 2:
                    stack.append((path, child_lba, child_size))
                else:
                    files.append((path, child_lba, child_size))
    return files


def find_entry(entries: Iterable[tuple[str, int, int]], wanted: str) -> tuple[str, int, int]:
    want = wanted.upper()
    for entry in entries:
        if entry[0].upper() == want:
            return entry
    raise DiscError(f"required retail file absent: {wanted}")


def u16(data: bytes, off: int) -> int:
    return struct.unpack_from("<H", data, off)[0]


def u32(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def va2off(va: int) -> int:
    return va - LOAD + EXE_HDR


def word_at(exe: bytes, va: int) -> int:
    return u32(exe, va2off(va))


def validate_disc(path: Path) -> str:
    size = path.stat().st_size
    if size != DISC1_BYTES:
        raise DiscError(f"wrong Disc 1 size: {size}")
    digest = sha256_file(path)
    if digest != DISC1_SHA256:
        raise DiscError(f"wrong Disc 1 SHA-256: {digest}")
    return digest


def load_retail(disc: Path) -> tuple[bytes, bytes]:
    validate_disc(disc)
    with RawMode2Image(disc) as image:
        entries = iso_entries(image)
        exe_ent = find_entry(entries, DISC1_EXE)
        pe_ent = find_entry(entries, PE_IMG)
        if (exe_ent[1], exe_ent[2]) != (24, DISC1_EXE_BYTES):
            raise DiscError("EXE ISO location/size mismatch")
        if (pe_ent[1], pe_ent[2]) != (PE_IMG_LBA, PE_IMG_BYTES):
            raise DiscError("PE.IMG ISO location/size mismatch")
        exe = image.read_form1_extent(exe_ent[1], exe_ent[2])
        if sha256_bytes(exe) != DISC1_EXE_SHA256:
            raise DiscError("EXE SHA-256 mismatch")
        return exe, image.read_form1_extent(pe_ent[1], pe_ent[2])


def read_package(pe_img: bytes, exe: bytes, table_index: int) -> bytes:
    base = FIELD_TABLE_OFF + table_index * 8
    start = u32(exe, base)
    nxt = u32(exe, base + 8)
    return pe_img[start * FORM1_USER_SIZE : nxt * FORM1_USER_SIZE]


def slot7_streams(package: bytes, meta: int) -> list[dict]:
    c0s, c1s, c2s = meta & 0xFF, (meta >> 8) & 0xFFF, meta >> 20
    base2 = (c0s + c1s) * 2048
    chunk2 = package[base2 : base2 + c2s * 2048]
    footer = u32(chunk2, 4)
    packed = u32(chunk2, footer + 0x20)
    count, rec_off = packed >> 22, packed & 0x3FFFFF
    rows = []
    for i in range(count):
        size = u32(chunk2, rec_off + i * 8)
        rel = u32(chunk2, rec_off + i * 8 + 4)
        store = (rel >> 24) & 0xFF
        offb = rel & 0xFFFFFF
        blob = chunk2[offb : offb + size]
        rows.append(
            {
                "index": i,
                "store": store,
                "pkg_offset": base2 + offb,
                "size": size,
                "sha256": sha256_bytes(blob),
                "bytes": blob,
            }
        )
    return rows


def find_marker(blob: bytes, mid: int) -> tuple[int, bytes] | None:
    for pref in (bytes((0xF9, 0xFE, mid)), bytes((0xFF, 0xFE, mid))):
        pos = blob.find(pref)
        if pos >= 0:
            return pos, pref
    return None


def extract_record(blob: bytes, mid: int) -> dict | None:
    found = find_marker(blob, mid)
    if found is None:
        return None
    pos, pref = found
    i = pos + 3
    while i < len(blob):
        b = blob[i]
        if b == 0xFF:
            i += 1
            break
        if b == 0xF9:
            break
        if b == 0xFB:
            i += 1
            if i < len(blob):
                sub = blob[i]
                i += 1
                if sub == 7 and i < len(blob):
                    i += 1
            continue
        if b in (0xFC, 0xFD) and i + 1 < len(blob):
            i += 2
            continue
        i += 1
    rec = blob[pos:i]
    return {
        "id": mid,
        "marker_offset": pos,
        "marker": pref,
        "raw": rec,
        "body": rec[3:],
    }


def glyph_uv(code: int) -> tuple[int, int]:
    return (code % 21) * 12, (code // 21) * 12


def width_of(exe: bytes, code: int) -> int:
    return exe[va2off(WIDTH_TABLE) + code * 2 + 1]


def decode_tokens(body: bytes, exe: bytes) -> list[dict]:
    tokens: list[dict] = []
    i = 0
    while i < len(body):
        b = body[i]
        if b == 0xFF:
            tokens.append({"kind": "TERMINAL", "byte": 0xFF, "text": None})
            break
        if b == 0xF9:
            tokens.append({"kind": "CHAIN_F9", "byte": 0xF9, "text": None})
            break
        if b == 0xF7:
            tokens.append({"kind": "NEWLINE", "byte": 0xF7, "text": "\n"})
            i += 1
            continue
        if b == 0xF8:
            tokens.append({"kind": "PAGE_WAIT", "byte": 0xF8, "text": None})
            i += 1
            continue
        if b == 0xFA:
            tokens.append({"kind": "NAME_FA", "byte": 0xFA, "text": None})
            i += 1
            continue
        if b == 0xFB:
            sub = body[i + 1] if i + 1 < len(body) else None
            extra = None
            consumed = 2
            if sub == 7 and i + 2 < len(body):
                extra = body[i + 2]
                consumed = 3
            tokens.append(
                {
                    "kind": f"FB_{sub:02X}" if sub is not None else "FB",
                    "byte": 0xFB,
                    "sub": sub,
                    "extra": extra,
                    "text": None,
                }
            )
            i += consumed
            continue
        if b == 0xFC:
            arg = body[i + 1] if i + 1 < len(body) else None
            tokens.append({"kind": "FC_EXT", "byte": 0xFC, "arg": arg, "text": None})
            i += 2 if arg is not None else 1
            continue
        if b == 0xFD:
            arg = body[i + 1] if i + 1 < len(body) else None
            tokens.append({"kind": "FD_EXT", "byte": 0xFD, "arg": arg, "text": None})
            i += 2 if arg is not None else 1
            continue
        if b in PUNCT:
            kind, ch = PUNCT[b]
            u, v = glyph_uv(b)
            tokens.append(
                {
                    "kind": kind,
                    "byte": b,
                    "text": ch,
                    "u": u,
                    "v": v,
                    "width": width_of(exe, b),
                }
            )
            i += 1
            continue
        if 0x10 <= b <= 0x29 or 0x30 <= b <= 0x49:
            ch = chr(b + LETTER_BASE)
            u, v = glyph_uv(b)
            tokens.append(
                {
                    "kind": "LETTER",
                    "byte": b,
                    "text": ch,
                    "u": u,
                    "v": v,
                    "width": width_of(exe, b),
                }
            )
            i += 1
            continue
        u, v = glyph_uv(b)
        tokens.append(
            {
                "kind": "UNKNOWN_GLYPH",
                "byte": b,
                "text": None,
                "u": u,
                "v": v,
                "width": width_of(exe, b) if b < 0x80 else None,
            }
        )
        i += 1
    return tokens


def tokens_to_text(tokens: list[dict]) -> str:
    out: list[str] = []
    for tok in tokens:
        if tok["kind"] == "NAME_FA":
            out.append("[NAME]")
        elif tok["kind"].startswith("FB_"):
            extra = tok.get("extra")
            if tok.get("sub") == 7 and extra is not None:
                out.append(f"[PAUSE {extra}]")
            else:
                out.append(f"[{tok['kind']}]")
        elif tok["kind"] in ("TERMINAL", "CHAIN_F9", "PAGE_WAIT", "FC_EXT", "FD_EXT"):
            continue
        elif tok["kind"] == "UNKNOWN_GLYPH":
            out.append(f"<{tok['byte']:02X}>")
        elif tok.get("text") is not None:
            out.append(tok["text"])
    return "".join(out)


def script_0d_sites(package: bytes, meta: int) -> list[str]:
    c0s, c1s, c2s = meta & 0xFF, (meta >> 8) & 0xFFF, meta >> 20
    base2 = (c0s + c1s) * 2048
    chunk2 = package[base2 : base2 + c2s * 2048]
    footer = u32(chunk2, 4)
    packed = u32(chunk2, footer + 0x14)
    count, rec_off = packed >> 22, packed & 0x3FFFFF
    if count < 1:
        return []
    rec_size = u32(chunk2, rec_off)
    rec_rel = u32(chunk2, rec_off + 4) & 0xFFFFFF
    script = chunk2[rec_rel : rec_rel + rec_size]
    if len(script) < 8:
        return []
    declared = u32(script, 0)
    nmod = u32(script, 4)
    if nmod > 64 or declared > len(script):
        return []
    offs = [u32(script, 8 + i * 4) for i in range(nmod)]
    hits = []
    for mi, start in enumerate(offs):
        end = offs[mi + 1] if mi + 1 < nmod else declared
        cur = start
        while cur + 8 <= end:
            if script[cur : cur + 2] == b"\xff\xff":
                break
            header = u32(script, cur)
            opcode = header & 0x1FFF
            argc = (header >> 13) & 0xF
            size = 8 + argc * 4
            if cur + size > end:
                break
            args = struct.unpack_from(f"<{argc}I", script, cur + 8) if argc else ()
            if opcode in (0x0D, 0x22) and args:
                hits.append(
                    f"module{mi}+0x{cur:04X} op=0x{opcode:02X} argc={argc} id=0x{args[0]:X}"
                )
            cur += size
    return hits


def _png_chunk(tag: bytes, data: bytes) -> bytes:
    return (
        struct.pack(">I", len(data))
        + tag
        + data
        + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
    )


def write_diagnostic_png(path: Path, title: str, lines: list[str]) -> None:
    # 320x54 retail window rectangle, host 5x7 font, labeled non-production.
    w, h = 320, 70
    bg = (16, 16, 32)
    fg = (220, 220, 200)
    note = (180, 80, 80)
    pix = [bg] * (w * h)

    def plot(x: int, y: int, color: tuple[int, int, int]) -> None:
        if 0 <= x < w and 0 <= y < h:
            pix[y * w + x] = color

    def draw_text(x: int, y: int, text: str, color: tuple[int, int, int]) -> None:
        cx = x
        for ch in text:
            bits = _HOST_FONT.get(ch, _HOST_FONT.get("?", _HOST_FONT[" "]))
            for row, rowbits in enumerate(bits):
                for col in range(5):
                    if rowbits & (0x10 >> col):
                        plot(cx + col, y + row, color)
            cx += 6

    for x in range(w):
        plot(x, 0, (80, 80, 100))
        plot(x, 53, (80, 80, 100))
        plot(x, h - 1, note)
    for y in range(54):
        plot(0, y, (80, 80, 100))
        plot(w - 1, y, (80, 80, 100))
    draw_text(4, 2, "NON-PRODUCTION DIAGNOSTIC", note)
    draw_text(4, 12, title[:50], fg)
    yy = 22
    for line in lines[:3]:
        draw_text(20, yy, line[:46], fg)
        yy += 12
    draw_text(4, 56, "host 5x7 font  not retail glyphs", note)

    raw = bytearray()
    for y in range(h):
        raw.append(0)
        for x in range(w):
            raw.extend(pix[y * w + x])
    png = b"\x89PNG\r\n\x1a\n"
    png += _png_chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += _png_chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += _png_chunk(b"IEND", b"")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(png)


def format_tokens(tokens: list[dict]) -> str:
    parts = []
    for tok in tokens:
        if tok["kind"] == "LETTER" or tok["kind"] in {
            "SPACE",
            "EXCLAMATION",
            "APOSTROPHE",
            "PERIOD",
            "COMMA",
            "NEWLINE",
        }:
            parts.append(repr(tok["text"])[1:-1] if tok["text"] != "\n" else "\\n")
        elif tok["kind"] == "NAME_FA":
            parts.append("FA:name")
        elif tok["kind"] == "TERMINAL":
            parts.append("FF:term")
        elif tok["kind"] == "CHAIN_F9":
            parts.append("F9:chain")
        elif tok["kind"] == "PAGE_WAIT":
            parts.append("F8:page")
        elif tok["kind"].startswith("FB_"):
            extra = tok.get("extra")
            if extra is not None:
                parts.append(f"FB {tok['sub']:02X} {extra:02X}")
            else:
                parts.append(f"FB {tok.get('sub')}")
        elif tok["kind"] == "UNKNOWN_GLYPH":
            parts.append(f"U/{tok['byte']:02X}")
        else:
            parts.append(tok["kind"])
    return " ".join(parts)


def print_message(rec: dict, tokens: list[dict], stream: dict, scene: str) -> None:
    mid = rec["id"]
    print(f"message_id=0x{mid:02X}")
    print(f"scene_example={scene}")
    print(f"stream_index={stream['index']}")
    print(f"stream_store={stream['store']}")
    print(f"stream_pkg_offset=0x{stream['pkg_offset']:X}")
    print(f"stream_size=0x{stream['size']:X}")
    print(f"stream_sha256={stream['sha256']}")
    print(f"marker_offset=+0x{rec['marker_offset']:X}")
    print(f"marker_bytes={rec['marker'].hex()}")
    print(f"raw_bytes={rec['raw'].hex()}")
    print(f"raw_len={len(rec['raw'])}")
    print(f"token_stream={format_tokens(tokens)}")
    print(f"decoded_text={tokens_to_text(tokens)!r}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("disc", type=Path)
    parser.add_argument("--message", default=None, help="message id, e.g. 0x21 or 33")
    parser.add_argument("--all-slice", action="store_true", help="decode 0x14..0x23")
    parser.add_argument("--png", type=Path, default=None, help="non-production diagnostic PNG")
    parser.add_argument("--scene", default="m0004i", help="package used for provenance offsets")
    args = parser.parse_args()
    # Decoded game text never goes into the tracked tree (owner decision
    # 2026-09-28): file output is confined to the git-ignored build/.
    if args.png is not None:
        build = (Path(__file__).resolve().parents[2] / "build").resolve()
        if build not in args.png.resolve().parents:
            parser.error(f"--png must be under {build} (decoded game text stays out of git)")

    exe, pe_img = load_retail(args.disc)
    print(f"disc_sha256={DISC1_SHA256}")
    print(f"exe_sha256={DISC1_EXE_SHA256}")
    print(f"exe_sha1={sha1_bytes(exe)}")
    print(f"op_0D_handler={word_at(exe, DISPATCH + 0x0D * 4):#010x}")
    print(f"op_0x22_handler={word_at(exe, DISPATCH + 0x22 * 4):#010x}")
    if word_at(exe, DISPATCH + 0x0D * 4) != HANDLER_0D:
        raise DiscError("opcode 0x0D handler drifted")
    if word_at(exe, DISPATCH + 0x22 * 4) != HANDLER_22:
        raise DiscError("opcode 0x22 handler drifted")

    name = exe[va2off(NAME_BUF) : va2off(NAME_BUF) + 8]
    nlen = exe[va2off(NAME_COUNT)]
    print(f"rom_name_bytes={name[:4].hex()} count={nlen}")
    print(f"record_base={RECORD_BASE:#010x}")
    print(f"usa_stream_select=D_800B0CD8&0x40000000 -> {STREAM_PTR_1:#010x}")

    scene_name = args.scene
    scene_idx = next(i for n, i in SCENES if n == scene_name)
    meta = u32(exe, FIELD_TABLE_OFF + scene_idx * 8 + 4)
    pkg = read_package(pe_img, exe, scene_idx)
    streams = slot7_streams(pkg, meta)
    print(f"package={scene_name} bytes={len(pkg)} sha256={sha256_bytes(pkg)}")
    for row in streams:
        print(
            f"slot7[{row['index']}] store={row['store']} "
            f"pkg+0x{row['pkg_offset']:X} size=0x{row['size']:X} sha256={row['sha256']}"
        )
        if row["index"] == 0 and row["sha256"] != STREAM0_SHA256:
            raise DiscError("stream 0 identity mismatch")
        if row["index"] == 1 and row["sha256"] != STREAM1_SHA256:
            raise DiscError("stream 1 identity mismatch")

    usa = streams[USA_STREAM_INDEX]
    sites = script_0d_sites(pkg, meta)
    if sites:
        print("script_message_ops:")
        for line in sites:
            print(f"  {line}")

    ids: list[int] = []
    if args.message is not None:
        ids.append(int(args.message, 0))
    if args.all_slice or args.message is None:
        if args.message is None:
            ids.extend(CURRENT_SLICE)
    ids = list(dict.fromkeys(ids))

    first_rec = None
    first_tokens = None
    for mid in ids:
        rec = extract_record(usa["bytes"], mid)
        if rec is None:
            print(f"message_id=0x{mid:02X} status=NOT_FOUND")
            continue
        tokens = decode_tokens(rec["body"], exe)
        print_message(rec, tokens, usa, scene_name)
        if first_rec is None:
            first_rec, first_tokens = rec, tokens

    if args.png and first_rec is not None and first_tokens is not None:
        text = tokens_to_text(first_tokens)
        lines = [ln for ln in text.replace("[NAME]", "Aya").split("\n")]
        write_diagnostic_png(
            args.png,
            f"id 0x{first_rec['id']:02X} NON-PROD",
            lines,
        )
        print(f"diagnostic_png={args.png} (NON-PRODUCTION host font)")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except DiscError as exc:
        print(f"FATAL: {exc}", file=sys.stderr)
        raise SystemExit(2)

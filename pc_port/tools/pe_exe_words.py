#!/usr/bin/env python3
"""Retail EXE words for the pc_port oracles, read from the user's disc.

Policy (CLAUDE.md hard rule 3, owner decision 2026-09-28 "run off the
disc"): no retail instruction words or tables are committed.  The oracles
name only the spans they model -- (vaddr, word count) -- and fetch the words
here from SLUS_006.62 in the git-ignored disc cache that
tools/extract/disc_cache.py builds from the user's own disc image.

    from pe_exe_words import exe_words, exe_word_map
    W = exe_words(0x80042C78, 16)            # [u32, ...]
    WORDS = exe_word_map(0x80052C6C, 148)    # {vaddr: u32}

Override the EXE with PE_EXE_PATH=<SLUS_006.62>.  Fails loudly (SystemExit)
when no verified USA Disc 1 EXE is available.
"""
from __future__ import annotations

import hashlib
import os
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
EXE_VRAM = 0x80010000
EXE_HDR = 0x800

_EXE: bytes | None = None


def exe_path() -> Path:
    env = os.environ.get("PE_EXE_PATH")
    if env:
        return Path(env)
    sys.path.insert(0, str(ROOT / "tools/extract"))
    try:
        import disc_cache  # tools/extract/disc_cache.py
    finally:
        sys.path.pop(0)
    d = disc_cache.cache_dir_if_ready()
    if d is None:
        raise SystemExit("FATAL: no disc cache; run: python3 tools/extract/disc_cache.py "
                         "populate --image <your Parasite Eve USA Disc 1 image>")
    return d / "SLUS_006.62"


def exe_bytes() -> bytes:
    """The verified retail SLUS_006.62 (SHA-1 checked once per process)."""
    global _EXE
    if _EXE is None:
        p = exe_path()
        data = p.read_bytes()
        got = hashlib.sha1(data).hexdigest()
        if got != EXE_SHA1:
            raise SystemExit(f"FATAL: {p}: SHA-1 {got} != {EXE_SHA1} (USA Disc 1 EXE)")
        _EXE = data
    return _EXE


def exe_off(vaddr: int) -> int:
    return vaddr - EXE_VRAM + EXE_HDR


def exe_words(vaddr: int, n: int) -> list[int]:
    """n little-endian u32 words starting at vaddr."""
    return list(struct.unpack_from(f"<{n}I", exe_bytes(), exe_off(vaddr)))


def exe_word(vaddr: int) -> int:
    return struct.unpack_from("<I", exe_bytes(), exe_off(vaddr))[0]


def exe_word_map(vaddr: int, n: int) -> dict[int, int]:
    """{address: word} for n consecutive words starting at vaddr."""
    return {vaddr + 4 * i: w for i, w in enumerate(exe_words(vaddr, n))}


def exe_halves(vaddr: int, n: int) -> list[int]:
    return list(struct.unpack_from(f"<{n}H", exe_bytes(), exe_off(vaddr)))


def exe_slice(vaddr: int, n: int) -> bytes:
    off = exe_off(vaddr)
    return exe_bytes()[off:off + n]


_PEIMG: Path | None = None


def peimg_path() -> Path:
    """PE.IMG from the disc cache (or PE_PEIMG_PATH)."""
    global _PEIMG
    if _PEIMG is None:
        env = os.environ.get("PE_PEIMG_PATH")
        p = Path(env) if env else exe_path().parent / "PE.IMG"
        if not p.exists():
            raise SystemExit(f"FATAL: {p} missing; run tools/extract/disc_cache.py populate")
        _PEIMG = p
    return _PEIMG


def peimg_slice(offset: int, n: int) -> bytes:
    with open(peimg_path(), "rb") as f:
        f.seek(offset)
        return f.read(n)


def exe_hex_list(vaddr: int, n: int, order: str = "value", upper: bool = True) -> list[str]:
    """n words as 8-digit hex strings: order "value" (the u32, as objdump
    prints it) or "le" (memory byte order, as the split's comment column)."""
    ws = exe_words(vaddr, n)
    out = [f"{w:08X}" if order == "value" else struct.pack("<I", w).hex().upper() for w in ws]
    return out if upper else [h.lower() for h in out]


def exe_hex_text(vaddr: int, n: int, order: str = "value", upper: bool = True) -> str:
    """Whitespace-separated form of exe_hex_list (for words(\"\"\"...\"\"\") parsers)."""
    return "\n" + " ".join(exe_hex_list(vaddr, n, order, upper)) + "\n"

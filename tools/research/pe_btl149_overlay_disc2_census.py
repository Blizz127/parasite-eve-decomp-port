#!/usr/bin/env python3
"""PE-BTL149: census PE.IMG-loaded ranges and Disc 2 executable coverage.

This is an evidence-only scanner.  It does not modify the native runtime or
interpret an overlay's story meaning.  The PE.IMG ranges are the retail LBA
pairs in D_8009315E..D_80093174 used by the executable's image-load loops.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path


M0360_TOKEN = 0xA8066048
# MIPS %hi carries because 0xD280 is negative when used as a signed
# 16-bit offset: lui 0x800A + signed 0xD280 = 0x8009D280.
DEST_HI = 0x800A
DEST_LO = 0xD280
ALT_DEST_HI = 0x800B

# D_8009315E..D_8009317A, as recovered from the retail executable.  The last
# value is the end marker used by the adjacent table; only increasing pairs
# are scanned.
IMAGE_LBAS = [
    0x039F, 0x03C5, 0x03C9, 0x03D2, 0x0457, 0x04FC, 0x0516,
    0x060B, 0x0700, 0x07B7, 0x07E0, 0x0839, 0x089D, 0x08B0,
]


def u32(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def sha1(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()


def word_hits(data: bytes, word: int) -> list[int]:
    needle = struct.pack("<I", word)
    return [off for off in range(0, len(data) - 3, 4)
            if data[off : off + 4] == needle]


def address_store_candidates(data: bytes, hi: int, lo: int) -> list[dict]:
    """Find direct `lui reg,hi` -> memory(op)(reg,lo) windows.

    The window is deliberately conservative evidence screening, not CFG
    recovery.  Results are therefore reported as candidates, never semantic
    writers without disassembly/context.
    """
    rows: list[dict] = []
    def writes_register(ins: int, reg: int) -> bool:
        opcode = ins >> 26
        rt = (ins >> 16) & 0x1F
        rd = (ins >> 11) & 0x1F
        if opcode in {0x00, 0x10, 0x12, 0x13}:
            return rd == reg and rd != 0
        if opcode in {0x02, 0x03, 0x0F}:
            return rt == reg and rt != 0
        if 0x04 <= opcode <= 0x07 or opcode in {0x14, 0x15}:
            return False
        return rt == reg and rt != 0

    for off in range(0, len(data) - 4, 4):
        ins = u32(data, off)
        if (ins >> 26) != 0x0F or (ins & 0xFFFF) != hi:
            continue
        reg = (ins >> 16) & 0x1F
        for distance in range(1, 17):
            target = off + distance * 4
            if target + 4 > len(data):
                break
            if distance > 1 and any(
                    writes_register(u32(data, mid), reg)
                    for mid in range(off + 4, target, 4)):
                break
            mem = u32(data, target)
            opcode = mem >> 26
            base = (mem >> 21) & 0x1F
            # All MIPS integer loads/stores, including byte/halfword forms.
            if base != reg or opcode not in {0x20, 0x21, 0x23, 0x24, 0x25,
                                             0x28, 0x29, 0x2B}:
                continue
            if (mem & 0xFFFF) != lo:
                continue
            kind = "store" if opcode in {0x28, 0x29, 0x2B} else "load"
            rows.append({"lui_offset": off, "memory_offset": target,
                         "lui": ins, "memory": mem,
                         "distance_words": distance, "kind": kind})
    return rows


def token_construction_candidates(data: bytes) -> list[dict]:
    rows: list[dict] = []
    for off in range(0, len(data) - 4, 4):
        ins = u32(data, off)
        if (ins >> 26) != 0x0F or (ins & 0xFFFF) != 0xA806:
            continue
        reg = (ins >> 16) & 0x1F
        for distance in range(1, 17):
            target = off + distance * 4
            if target + 4 > len(data):
                break
            follow = u32(data, target)
            opcode = follow >> 26
            rs = (follow >> 21) & 0x1F
            rt = (follow >> 16) & 0x1F
            # ori/addiu rt, reg, 0x6048; both are plausible constant
            # materialization forms and are reported for manual review.
            if ((opcode == 0x0D and rs == reg and rt == reg) or
                    (opcode == 0x09 and rs == reg and rt == reg)) and \
                    (follow & 0xFFFF) == 0x6048:
                rows.append({"lui_offset": off, "follow_offset": target,
                             "lui": ins, "follow": follow,
                             "distance_words": distance})
    return rows


def scan_blob(data: bytes) -> dict:
    d280 = address_store_candidates(data, DEST_HI, DEST_LO)
    ad280 = address_store_candidates(data, ALT_DEST_HI, DEST_LO)
    return {
        "bytes": len(data),
        "sha1": sha1(data),
        "raw_m0360_token_offsets": word_hits(data, M0360_TOKEN),
        "direct_D_8009D280_candidates": d280,
        "direct_D_8009D280_writers": [row for row in d280
                                      if row["kind"] == "store"],
        "direct_800AD280_candidates": ad280,
        "m0360_token_construction_candidates": token_construction_candidates(data),
    }


def scan_ranges(peimg: bytes) -> list[dict]:
    rows = []
    for start, end in zip(IMAGE_LBAS, IMAGE_LBAS[1:]):
        blob = peimg[start * 2048 : end * 2048]
        row = scan_blob(blob)
        row.update({"lba_start": start, "lba_end": end,
                    "byte_start": start * 2048, "byte_end": end * 2048})
        rows.append(row)
    return rows


def fast_full_peimg_scan(peimg: bytes) -> dict:
    """Full-archive negative scan without decoding arbitrary data as code."""
    raw = []
    pos = 0
    needle = struct.pack("<I", M0360_TOKEN)
    while True:
        off = peimg.find(needle, pos)
        if off < 0:
            break
        raw.append(off)
        pos = off + 1

    writers = []
    pos = 0
    store_low = b"\x80\xD2"  # little-endian low half 0xD280
    while True:
        off = peimg.find(store_low, pos)
        if off < 0:
            break
        if off % 4 == 0:
            ins = u32(peimg, off)
            if (ins >> 26) in {0x28, 0x29, 0x2B}:
                base = (ins >> 21) & 0x1F
                for lui_off in range(max(0, off - 64), off, 4):
                    lui = u32(peimg, lui_off)
                    if ((lui >> 26) == 0x0F and (lui & 0xFFFF) == DEST_HI
                            and ((lui >> 16) & 0x1F) == base):
                        writers.append({"lui_offset": lui_off,
                                        "memory_offset": off,
                                        "lui": lui, "memory": ins})
                        break
        pos = off + 1

    constructs = []
    pos = 0
    lui_low = b"\x06\xA8"  # little-endian low half 0xA806
    while True:
        off = peimg.find(lui_low, pos)
        if off < 0:
            break
        if off % 4 == 0:
            lui = u32(peimg, off)
            if (lui >> 26) == 0x0F:
                reg = (lui >> 16) & 0x1F
                for follow_off in range(off + 4, min(len(peimg), off + 68), 4):
                    follow = u32(peimg, follow_off)
                    if ((follow >> 26) in {0x09, 0x0D}
                            and ((follow >> 21) & 0x1F) == reg
                            and ((follow >> 16) & 0x1F) == reg
                            and (follow & 0xFFFF) == 0x6048):
                        constructs.append({"lui_offset": off,
                                           "follow_offset": follow_off,
                                           "lui": lui, "follow": follow})
        pos = off + 1
    return {"raw_m0360_token_offsets": raw,
            "direct_D_8009D280_writers": writers,
            "m0360_token_construction_candidates": constructs}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--disc1-exe", type=Path, required=True)
    ap.add_argument("--disc2-exe", type=Path, required=True)
    ap.add_argument("--disc1-peimg", type=Path, required=True)
    ap.add_argument("--disc2-peimg", type=Path, required=True)
    ap.add_argument("--output", type=Path)
    args = ap.parse_args()

    exe1 = args.disc1_exe.read_bytes()
    exe2 = args.disc2_exe.read_bytes()
    img1 = args.disc1_peimg.read_bytes()
    img2 = args.disc2_peimg.read_bytes()
    result = {
        "scanner": "pe_btl149_overlay_disc2_census.py",
        "target_token": f"0x{M0360_TOKEN:08X}",
        "destination_state": "D_8009D280",
        "overlay_ranges_source": "retail D_8009315E..D_8009317A",
        "disc1_executable": scan_blob(exe1),
        "disc2_executable": scan_blob(exe2),
        "disc1_peimg": {"bytes": len(img1), "sha1": sha1(img1),
                        "ranges": scan_ranges(img1),
                        "full_archive_scan": fast_full_peimg_scan(img1)},
        "disc2_peimg": {"bytes": len(img2), "sha1": sha1(img2),
                        "ranges": scan_ranges(img2),
                        "full_archive_scan": fast_full_peimg_scan(img2)},
        "disc2_executable_byte_identical": exe1 == exe2,
        "peimg_byte_identical": img1 == img2,
    }
    encoded = json.dumps(result, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.write_text(encoded)
    else:
        print(encoded, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

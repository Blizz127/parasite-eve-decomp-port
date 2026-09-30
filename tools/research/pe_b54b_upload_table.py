#!/usr/bin/env python3
"""PE-B54B evidence-only VRAM upload-table enumerator.

Finds the canonical Disc 1 channel-1 texture packet in PE.IMG
(header 0x0340B5B8, count 13) and decodes every 0x14-byte
func_8006E1C0 entry. Does not modify production C.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.research import pe_pst0_scan as pst0

HEADER = 0x0340B5B8
ENTRY0_IMG = 0x080B0020
ENTRY0_CLUT = 0x39040040
PLAYER_TEX = (960, 256, 64, 255)
PLAYER_TEX_TAIL = (960, 511, 64, 1)
PLAYER_CLUT = (0, 448, 256, 2)
ATLAS_MIN_W = 21 * 12  # 252


def decode_rect(packed: int, h: int) -> tuple[int, int, int, int]:
    x = (packed >> 10) & 0x7FF
    y = packed >> 21
    w = packed & 0x3FF
    return x, y, w, h


def find_packet(peimg: bytes) -> int:
    needle = struct.pack("<I", HEADER)
    start = 0
    while True:
        hit = peimg.find(needle, start)
        if hit < 0:
            raise SystemExit("texture header 0x0340B5B8 not found in PE.IMG")
        # header lives at metadata+0x28; metadata = base + 0xB578
        meta = hit - 0x28
        base = meta - 0xB578
        if base >= 0 and struct.unpack_from("<I", peimg, base + 4)[0] == 0xB578:
            entry0 = base + (HEADER & 0x3FFFFF)
            img = struct.unpack_from("<I", peimg, entry0 + 8)[0]
            clut = struct.unpack_from("<I", peimg, entry0 + 0x10)[0]
            if img == ENTRY0_IMG and clut == ENTRY0_CLUT:
                return base
        start = hit + 1


def enumerate_entries(peimg: bytes, base: int) -> list[dict]:
    header = struct.unpack_from("<I", peimg, base + 0xB578 + 0x28)[0]
    count = header >> 22
    table = base + (header & 0x3FFFFF)
    rows = []
    for index in range(count):
        at = table + index * 0x14
        raw = peimg[at : at + 0x14]
        size_word, img_off, img_pack, clut_off, clut_pack = struct.unpack_from("<5I", raw)
        img_off &= 0xFFFFFF
        clut_off &= 0xFFFFFF
        img_h = raw[7] or 0x100
        clut_h = raw[15]
        ix, iy, iw, ih = decode_rect(img_pack, img_h)
        if clut_off:
            cx, cy, cw, ch = decode_rect(clut_pack, clut_h)
            clut_src = 0x801229A0 + img_off + clut_off
        else:
            cx = cy = cw = ch = 0
            clut_src = 0
        palettes = (cw // 16) if cw else 0
        rows.append(
            {
                "index": index,
                "size_word": size_word,
                "image_off": img_off,
                "image": (ix, iy, iw, ih),
                "clut_off": clut_off,
                "clut": (cx, cy, cw, ch),
                "clut_src": clut_src,
                "bpp": 4 if palettes else 0,
                "eff_w": iw * 4,
                "eff_h": ih,
                "palettes": palettes,
                "raw": raw.hex(),
            }
        )
    return rows


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("disc", nargs="?")
    parser.add_argument("--peimg", type=Path)
    parser.add_argument("--exe", type=Path)
    args = parser.parse_args()
    if args.peimg:
        peimg = args.peimg.read_bytes()
    elif args.disc:
        with pst0.RawMode2Image(Path(args.disc)) as image:
            entries = pst0.iso_entries(image)
            img_ent = pst0.find_entry(entries, pst0.PE_IMG)
            peimg = image.read_form1_extent(img_ent[1], img_ent[2])
    else:
        print("need disc or --peimg", file=sys.stderr)
        return 2
    print("peimg_sha1", hashlib.sha1(peimg).hexdigest())
    base = find_packet(peimg)
    print(f"packet_peimg_off={base:#x} sector={base // 2048}")
    rows = enumerate_entries(peimg, base)
    print(f"count={len(rows)}")
    for row in rows:
        ix, iy, iw, ih = row["image"]
        cx, cy, cw, ch = row["clut"]
        atlas = row["eff_w"] >= ATLAS_MIN_W and row["eff_h"] >= 12
        print(
            f"{row['index']:2d} I({ix},{iy},{iw},{ih}) C({cx},{cy},{cw},{ch}) "
            f"src={row['clut_src']:#x} bpp={row['bpp']} eff={row['eff_w']}x{row['eff_h']} "
            f"pals={row['palettes']} atlas_row={atlas}"
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())

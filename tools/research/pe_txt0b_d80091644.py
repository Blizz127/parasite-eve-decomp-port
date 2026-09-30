#!/usr/bin/env python3
"""PE-TXT0-B evidence-only D_80091644 / font-atlas locator.

Reads PE.IMG sectors 180-197 (D_800930EC range) as the TIM uploaded
by func_800718D0 after the 0x8006AF54 poll, and checks it against
record-0 GetTPage/GetClut packing. Does not modify production C.
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

TIM_SHA256 = "864088aed4b2bde72436404c6f6b6bd86302acba9d4e2671f07e7f5c16e27821"
IMAGE_SHA256 = "2156e25d353704ba31b568f7f19c3124492899f54fac09f79d6cd8b3c6d0e883"
CLUT_SHA256 = "52b1e9ebc16d0c0ad5280c1a9386dc2750277f6322c62516ddd3c2d2fb04bf86"


def pack_record0() -> tuple[int, int]:
    a, b, c, d = 0x0140, 0x0000, 0x0140, 0x00FC
    dest1 = ((a & 0x3FF) >> 6) | 0x20 | ((b & 0x100) >> 4) | ((b & 0x200) << 2)
    dest2 = (d << 6) | ((c >> 4) & 0x3F)
    return dest1, dest2


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("disc", nargs="?")
    parser.add_argument("--peimg", type=Path)
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
    bank = peimg[180 * 2048 : 197 * 2048]
    print("tim_sha256", hashlib.sha256(bank).hexdigest())
    print("tim_sha_ok", hashlib.sha256(bank).hexdigest() == TIM_SHA256)
    magic, flag, clut_len = struct.unpack_from("<III", bank, 0)
    cx, cy, cw, ch = struct.unpack_from("<HHHH", bank, 12)
    img_at = 8 + clut_len
    img_len, ix, iy, iw, ih = struct.unpack_from("<IHHHH", bank, img_at)
    clut = bank[20 : 8 + clut_len]
    image = bank[img_at + 12 : img_at + img_len]
    print(f"flag={flag:#x} clut=({cx},{cy},{cw},{ch}) image=({ix},{iy},{iw},{ih})")
    print("clut_sha256", hashlib.sha256(clut).hexdigest())
    print("image_sha256", hashlib.sha256(image).hexdigest())
    print("eff_4bpp", iw * 4, "x", ih, "atlas_row", iw * 4 >= 252)
    dest1, dest2 = pack_record0()
    print(f"packed_tpage={dest1:#x} packed_clut={dest2:#x}")
    print("tpage_matches_rect", dest1 == 0x25 and ix == 320 and iy == 0)
    print("clut_matches_rect", dest2 == 0x3F14 and cx == 320 and cy == 252)
    return 0


if __name__ == "__main__":
    sys.exit(main())

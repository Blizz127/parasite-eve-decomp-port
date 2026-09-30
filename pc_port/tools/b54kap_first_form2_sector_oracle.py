#!/usr/bin/env python3
"""Independent B54K-AP Disc-1 multiplexed stream-sector oracle."""
from pathlib import Path
import hashlib
import struct
import sys

RAW_SIZE = 2352
FORM1_SIZE = 2048
FORM2_SIZE = 2324
FIRST_LBA = 189742
SYNC = bytes.fromhex("00ffffffffffffffffffff00")


def need(ok, message):
    if not ok:
        raise SystemExit("FAIL: " + message)


def read_sector(fp, lba):
    fp.seek(lba * RAW_SIZE)
    data = fp.read(RAW_SIZE)
    need(len(data) == RAW_SIZE, f"short raw sector at LBA {lba}")
    return data


def main():
    need(len(sys.argv) == 2, "usage: b54kap_first_form2_sector_oracle.py DISC1.bin")
    image = Path(sys.argv[1])
    need(image.stat().st_size % RAW_SIZE == 0, "image is not 2352-byte sector aligned")

    with image.open("rb") as fp:
        sectors = [read_sector(fp, FIRST_LBA + i) for i in range(10)]

    first = sectors[0]
    need(hashlib.sha256(first).hexdigest() ==
         "84210c5a23b682cf8972f477cd0065568a85482f8391c353c27c1cd5bf15c5a3",
         "first raw sector identity")
    need(first[:12] == SYNC and first[15] == 2, "Mode-2 sync/header")
    need(first[16:20] == bytes((1, 1, 0x48, 0)), "first subheader tuple")
    need(first[16:20] == first[20:24], "duplicated video subheader")
    need((first[18] & 0x20) == 0, "video sector must be Form 1")

    video = first[24:24 + FORM1_SIZE]
    need(hashlib.sha256(video).hexdigest() ==
         "810cc410d14efad506d757aab639f09c4aeb3591391fada7afd9a4b67e821a56",
         "first Form-1 data identity")
    magic, chunk, chunks, frame, size, width, height = struct.unpack_from(
        "<IHHIIHH", video)
    need((magic, chunk, chunks, frame, size, width, height) ==
         (0x80010160, 0, 9, 1, 0xA98, 320, 240), "first STR header")

    # Chunks 0..6, one Form-2 XA sector, then chunks 7..8.
    for i in range(7):
        data = sectors[i][24:24 + FORM1_SIZE]
        need(sectors[i][16:20] == bytes((1, 1, 0x48, 0)),
             f"video subheader at +{i}")
        need((sectors[i][18] & 0x20) == 0, f"video Form bit at +{i}")
        need(struct.unpack_from("<IHHI", data) == (0x80010160, i, 9, 1),
             f"video chunk {i}")

    xa = sectors[7]
    need(hashlib.sha256(xa).hexdigest() ==
         "7b4a69c55db31ddf2c61069793855ffb08d3181b64780c7a68dcd2e18bb6741c",
         "interleaved XA raw identity")
    need(xa[16:20] == bytes((1, 1, 0x64, 1)) and xa[16:20] == xa[20:24],
         "interleaved XA subheader")
    need((xa[18] & 0x20) != 0, "XA sector must be Form 2")
    xa_data = xa[24:24 + FORM2_SIZE]
    need(hashlib.sha256(xa_data).hexdigest() ==
         "8a6094b8f60717e897286181ad0c372e3d3dba83ae9679d87b28c108cf316187",
         "XA Form-2 data identity")
    need(struct.unpack_from("<I", xa_data)[0] != 0x80010160,
         "XA sector is not a video chunk")
    for i, sector in ((7, sectors[8]), (8, sectors[9])):
        need(struct.unpack_from("<IHHI", sector, 24) ==
             (0x80010160, i, 9, 1), f"resumed video chunk {i}")

    root = Path(__file__).resolve().parents[2]
    header = (root / "pc_port/platform/pe_disc.h").read_text()
    source = (root / "pc_port/platform/pe_disc.c").read_text()
    boot = (root / "pc_port/game/boot/func_80081314_port.c").read_text()
    need("#define PE_DISC_USER_SECTOR  2048u" in header and
         "#define PE_DISC_USER_OFFSET  24u" in header,
         "public ISO-sector geometry")
    need("memcpy(out, raw + PE_DISC_USER_OFFSET, PE_DISC_USER_SECTOR);" in source,
         "2048-byte extraction implementation")
    need("static bool PE_Disc_ReadRaw" in source and
         "PE_Disc_ReadRaw" not in header,
         "raw stream reader remains private")
    libcd=(root/"pc_port/platform/pe_libcd.c").read_text()
    need("func_80081314_func_8007F0C8_cut" not in boot and
         "func_8007F0C8_completion_selector" in libcd,
         "strict frontier moved without delivery implementation")

    print("  OK retail video: Mode-2 Form-1 / 2048-byte data")
    print("  OK retail XA: Mode-2 Form-2 / 2324-byte data")
    print("  OK multiplex: video chunks 0..6, XA sector, video chunks 7..8")
    print("  OK native: ISO-only reader lacks subheader/variable geometry")
    print("\nB54K-AP multiplexed stream sectors: PASS.")


if __name__ == "__main__":
    main()

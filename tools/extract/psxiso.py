#!/usr/bin/env python3
"""Minimal ISO9660 reader for raw PS1 disc images (MODE2/2352, single track).

Parses the ISO9660 filesystem directly from a raw 2352-byte-per-sector BIN
image without mounting it. User data is taken from the Mode 2 Form 1 payload
(offset 24, 2048 bytes per sector). Stdlib only.

Commands:
    info    <image.bin>                       Print primary volume descriptor fields.
    list    <image.bin>                       Print all files: LBA, size, path.
    extract <image.bin> <iso_path> <out>      Extract one file to <out>.

`extract` is only valid for Mode 2 Form 1 files (executables, plain data).
It fails loudly if it hits a Form 2 sector (XA audio / STR video payloads).
"""

import struct
import sys

SECTOR_RAW = 2352
USER_OFF = 24  # sync(12) + header(4) + subheader(8)
USER_LEN = 2048
SUBHEADER_OFF = 16
FORM2_FLAG = 0x20  # submode bit 5


class IsoError(RuntimeError):
    pass


class RawImage:
    def __init__(self, path):
        self.f = open(path, "rb")
        self.f.seek(0, 2)
        size = self.f.tell()
        if size % SECTOR_RAW != 0:
            raise IsoError(
                f"image size {size} is not a multiple of {SECTOR_RAW}; "
                "not a raw MODE2/2352 image?"
            )
        self.sector_count = size // SECTOR_RAW

    def user_data(self, lba, check_form=False):
        if lba >= self.sector_count:
            raise IsoError(f"LBA {lba} beyond end of image ({self.sector_count})")
        self.f.seek(lba * SECTOR_RAW)
        raw = self.f.read(SECTOR_RAW)
        if check_form and raw[SUBHEADER_OFF + 2] & FORM2_FLAG:
            raise IsoError(
                f"sector at LBA {lba} is Mode 2 Form 2 (XA/STR); "
                "plain-file extraction is not valid for it"
            )
        return raw[USER_OFF : USER_OFF + USER_LEN]


def u32_le(buf, off):
    return struct.unpack_from("<I", buf, off)[0]


def read_pvd(img):
    pvd = img.user_data(16)
    if pvd[0] != 1 or pvd[1:6] != b"CD001":
        raise IsoError("no primary volume descriptor at LBA 16 (CD001 missing)")
    return pvd


def parse_dir_records(img, extent_lba, data_len):
    """Yield (name, lba, size, is_dir) for one directory extent."""
    n_sectors = (data_len + USER_LEN - 1) // USER_LEN
    for s in range(n_sectors):
        data = img.user_data(extent_lba + s)
        off = 0
        while off < USER_LEN:
            rec_len = data[off]
            if rec_len == 0:  # records never span sectors; rest is padding
                break
            rec = data[off : off + rec_len]
            off += rec_len
            name_len = rec[32]
            name = rec[33 : 33 + name_len]
            if name in (b"\x00", b"\x01"):  # self / parent
                continue
            yield (
                name.decode("ascii", "replace"),
                u32_le(rec, 2),
                u32_le(rec, 10),
                bool(rec[25] & 0x02),
            )


def walk(img):
    """Yield (path, lba, size) for every file on the disc."""
    pvd = read_pvd(img)
    root_lba = u32_le(pvd, 156 + 2)
    root_len = u32_le(pvd, 156 + 10)
    stack = [("", root_lba, root_len)]
    while stack:
        prefix, lba, length = stack.pop()
        for name, ext_lba, size, is_dir in parse_dir_records(img, lba, length):
            path = f"{prefix}/{name}" if prefix else name
            if is_dir:
                stack.append((path, ext_lba, size))
            else:
                yield path, ext_lba, size


def find_file(img, iso_path):
    want = iso_path.upper().lstrip("/")
    for path, lba, size in walk(img):
        if path.upper() in (want, want + ";1") or path.upper().split(";")[0] == want:
            return path, lba, size
    raise IsoError(f"file not found on disc: {iso_path}")


def cmd_info(img):
    pvd = read_pvd(img)
    print(f"system_id:    {pvd[8:40].decode('ascii', 'replace').strip()}")
    print(f"volume_id:    {pvd[40:72].decode('ascii', 'replace').strip()}")
    print(f"volume_space: {u32_le(pvd, 80)} sectors")
    print(f"image_sectors:{img.sector_count}")


def cmd_list(img):
    for path, lba, size in sorted(walk(img), key=lambda t: t[1]):
        print(f"{lba:7d} {size:10d} {path}")


def cmd_extract(img, iso_path, out_path):
    path, lba, size = find_file(img, iso_path)
    remaining = size
    with open(out_path, "wb") as out:
        sector = lba
        while remaining > 0:
            data = img.user_data(sector, check_form=True)
            out.write(data[: min(USER_LEN, remaining)])
            remaining -= USER_LEN
            sector += 1
    print(f"extracted {path} (LBA {lba}, {size} bytes) -> {out_path}")


def main(argv):
    if len(argv) < 3:
        print(__doc__, file=sys.stderr)
        return 2
    cmd, image = argv[1], argv[2]
    img = RawImage(image)
    if cmd == "info":
        cmd_info(img)
    elif cmd == "list":
        cmd_list(img)
    elif cmd == "extract" and len(argv) == 5:
        cmd_extract(img, argv[3], argv[4])
    else:
        print(__doc__, file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

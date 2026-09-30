#!/usr/bin/env python3
"""Print size, CRC32, MD5, and SHA-1 for each given file (redump-style).

Usage: hashfile.py <file> [<file> ...]
"""

import hashlib
import sys
import zlib


def hash_file(path):
    crc = 0
    md5 = hashlib.md5()
    sha1 = hashlib.sha1()
    size = 0
    with open(path, "rb") as f:
        while chunk := f.read(1 << 20):
            crc = zlib.crc32(chunk, crc)
            md5.update(chunk)
            sha1.update(chunk)
            size += len(chunk)
    return size, f"{crc:08x}", md5.hexdigest(), sha1.hexdigest()


def main(argv):
    if len(argv) < 2:
        print(__doc__, file=sys.stderr)
        return 2
    for path in argv[1:]:
        size, crc, md5, sha1 = hash_file(path)
        print(f"file:  {path}")
        print(f"size:  {size}")
        print(f"crc32: {crc}")
        print(f"md5:   {md5}")
        print(f"sha1:  {sha1}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

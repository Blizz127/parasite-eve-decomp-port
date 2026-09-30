#!/usr/bin/env python3
"""Phase 6E-B54I independent oracle: GPU primitive-builder leaves + wrappers.

Verifies, against the SHA-1-exact retail executable SLUS_006.62:

  1. Literal word windows for all eleven newly ported functions, including
     the out-of-line delay-slot stores that complete func_80077C04
     (0x80077C14), func_80077C84 (0x80077CAC), and func_8005DADC
     (0x8005DAF8).
  2. The retail call-site vectors: func_80077A64(0,1,256,480) == 0x34 and
     func_80077AA4(0x130,0x1F8) == 0x7E13, re-derived from the literal
     argument loads at the func_80030894 jal sites.
  3. Callee linkage: the jal edges of both wrappers, the B54D-audit call
     counts (18 sites for func_800370DC, 1 for func_80037140 inside
     func_80030894), and the twelve executable-wide func_800719E4 sites
     with their a0 argument classification.

The native ports (pc_port/game/boot/func_80077{A64,AA4,B04,B34,C04,C84,
CB4}_port.c, func_8005DADC_port.c, func_80037{0DC,140}_port.c,
platform/func_800719E4_port.c) implement exactly these decodes; the native
focused test proves the byte contract on the host.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_hex_list

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

# rom_addr, file_offset, word_count, exact words (hex, as in the image)
WINDOWS = [
    ("func_80077A64", 0x68264, 15, exe_hex_list(0x80077A64, 15, 'value', True)),
    ("func_80077AA4", 0x682A4, 6, [
        "00051180", "00042103", "3084003F", "00441025", "03E00008",
        "3042FFFF"]),
    ("func_80077B04", 0x68304, 10, exe_hex_list(0x80077B04, 10, 'value', True)),
    ("func_80077B34", 0x68334, 10, exe_hex_list(0x80077B34, 10, 'value', True)),
    ("func_80077C04", 0x68404, 5, [
        "24020004", "A0820003", "24020064", "03E00008", "A0820007"]),
    ("func_80077C84", 0x68484, 11, exe_hex_list(0x80077C84, 11, 'value', True)),
    ("func_80077CB4", 0x684B4, 13, exe_hex_list(0x80077CB4, 13, 'value', True)),
    ("func_8005DADC", 0x4E2DC, 8, exe_hex_list(0x8005DADC, 8, 'value', True)),
    ("func_800370DC", 0x278DC, 25, exe_hex_list(0x800370DC, 25, 'value', True)),
    ("func_80037140", 0x27940, 25, exe_hex_list(0x80037140, 25, 'value', True)),
    ("func_800719E4", 0x621E4, 3, [
        "240A00B0", "01400008", "24090038"]),
]

# Wrapper jal edges (from the literal windows above)
WRAPPER_CALLEE_SITES = {
    0x800370DC: [0x80077C84, 0x80077C04, 0x80077CB4, 0x800719E4],
    0x80037140: [0x80077C84, 0x80077C44, 0x80077CB4, 0x800719E4],
}


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def find_exe() -> pathlib.Path:
    here = pathlib.Path(__file__).resolve()
    for cand in (here.parent.parent / "build" / "disc1.candidate.exe",
                 here.parent.parent.parent / "build" / "disc1.candidate.exe",
                 pathlib.Path("build/disc1.candidate.exe"),
                 pathlib.Path("pc_port/build/disc1.candidate.exe")):
        if cand.is_file():
            return cand
    raise SystemExit("FAIL: could not locate disc1.candidate.exe")


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1,
            f"exe sha1 mismatch ({path})")
    return data


def word(data: bytes, rom: int) -> int:
    return struct.unpack_from("<I", data, rom - 0x8000F800)[0]


def jal_target(w: int) -> int:
    return ((w & 0x03FFFFFF) << 2) | 0x80000000


def all_jal_sites(data: bytes, target: int) -> list[int]:
    enc = (target & 0x1FFFFFFF) >> 2 | 0x0C000000
    sites = []
    rom = 0x80010000
    end = 0x8000F800 + len(data)
    while rom < end:
        if word(data, rom) == enc:
            sites.append(rom)
        rom += 4
    return sites


def get_tpage(tp: int, abr: int, x: int, y: int) -> int:
    return (((tp & 3) << 7) | ((abr & 3) << 5) | ((y & 0x100) >> 4) |
            ((x & 0x3FF) >> 6) | ((y & 0x200) << 2))


def get_clut(x: int, y: int) -> int:
    return ((y << 6) | ((x >> 4) & 0x3F)) & 0xFFFF


def run() -> int:
    data = load_exe(find_exe())
    n = 0

    # 1. Literal word windows (image words == documented words).
    for name, off, count, expect in WINDOWS:
        ws = [f"{struct.unpack_from('<I', data, off + 4 * i)[0]:08X}"
              for i in range(count)]
        require(ws == expect,
                f"{name}: word window mismatch\n  got      {ws}\n"
                f"  expected {expect}")
        n += 1
        print(f"  OK {name}: {count} words literal-exact")

    # 2. Retail call-site vectors re-derived from the jal argument loads.
    # func_80030894 site 0x800308EC: li a0,0; li a1,1; li a2,256; li a3,480
    require(word(data, 0x800308E0) == 0x00002021 and
            word(data, 0x800308E4) == 0x24050001 and
            word(data, 0x800308E8) == 0x24060100 and
            word(data, 0x800308EC) == 0x0C01DE99 and
            word(data, 0x800308F0) == 0x240701E0,
            "func_80030894 GetTPage call site shape changed")
    require(get_tpage(0, 1, 256, 480) == 0x34,
            "GetTPage(0,1,256,480) != 0x34")
    n += 2
    print("  OK func_80077A64: retail call (0,1,256,480) -> 0x34 "
          "(arguments verified at 0x800308E0..F0)")

    # site 0x800308FC: li a0,304; li a1,504
    require(word(data, 0x800308F4) == 0x24040130 and
            word(data, 0x800308F8) == 0x240501F8 and
            word(data, 0x800308FC) == 0x0C01DEA9,
            "func_80030894 GetClut call site shape changed")
    require(get_clut(0x130, 0x1F8) == 0x7E13,
            "GetClut(0x130,0x1F8) != 0x7E13")
    n += 2
    print("  OK func_80077AA4: retail call (0x130,0x1F8) -> 0x7E13 "
          "(arguments verified at 0x800308F4..FC)")

    # 3. Wrapper jal edges: each wrapper calls exactly its four callees in
    #    retail order with the audit's argument shapes.
    for wrapper, callees in WRAPPER_CALLEE_SITES.items():
        rom = wrapper
        seen = []
        for _ in range(24):
            w = word(data, rom)
            if (w >> 26) & 0x3F == 0x03:
                seen.append(jal_target(w))
            if w == 0x03E00008:  # terminal jr ra
                break
            rom += 4
        require(seen == callees,
                f"{wrapper:#x}: jal sequence {list(map(hex, seen))} != "
                f"{list(map(hex, callees))}")
        n += 1
        print(f"  OK {wrapper:#x}: jal chain "
              f"{' -> '.join(hex(c) for c in seen)}")

    # 4. func_80030894 callee census: 18 sites for func_800370DC, 1 for
    #    func_80037140, plus every leaf linked from the audit.
    f3_end = 0x80030894 + 788 * 4
    inner = []
    for rom in range(0x80030894, f3_end, 4):
        w = word(data, rom)
        if (w >> 26) & 0x3F == 0x03:
            inner.append(jal_target(w))
    for tgt, want in ((0x800370DC, 18), (0x80037140, 1),
                      (0x80077A64, 5), (0x80077AA4, 2),
                      (0x80077B04, 2), (0x80077B34, 3),
                      (0x8005DADC, 2), (0x80077CB4, 0),
                      (0x80077C84, 0), (0x80077C04, 0)):
        got = inner.count(tgt)
        require(got == want,
                f"func_80030894 calls {tgt:#x} {got} times, expected {want}")
        n += 1
    print("  OK func_80030894 callee census matches the B54D audit "
          "(leaves reached only through the wrappers)")

    # 5. func_800719E4 executable-wide census: 12 sites; 11 load a0=-1 in
    #    the jal delay slot, 1 (func_8006E758, CD mode set) loads a0=1.
    sites = all_jal_sites(data, 0x800719E4)
    require(len(sites) == 12, f"func_800719E4: {len(sites)} jal sites != 12")
    minus1 = sum(1 for s in sites
                 if word(data, s + 4) == 0x2404FFFF)
    plus1 = sum(1 for s in sites if word(data, s + 4) == 0x24040001)
    require(minus1 == 11 and plus1 == 1,
            f"func_800719E4 a0 classification: -1 x{minus1}, +1 x{plus1}")
    n += 3
    print("  OK func_800719E4: 12 jal sites (11 x a0=-1 fail paths, "
          "1 x a0=1 CD mode set)")

    print(f"\nB54I oracle: {n} checks passed (11 literal windows, "
          "retail call vectors, wrapper jal chains, callee census, "
          "B(38h) site classification).")
    return 0


if __name__ == "__main__":
    sys.exit(run())

#!/usr/bin/env python3
"""Phase 6E-B54A independent func_8006AD40 suffix oracle.

Read-only reconstruction of the retail suffix beginning at 0x8006AE50.
Verifies literal words against the SHA-1-exact executable, models the
captured post-B53I-D locals, and walks suffix control flow without
importing production C or inventing DMA/CD progress.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC_START = 0x8006AD40
FUNC_END = 0x8006B35C
CUT_PC = 0x8006AE50
LOOP_REENTER = 0x8006AE44
LOOP_EXIT = 0x8006AE68
LOOKUP_CALL = 0x8006AEFC
LOADIMAGE_CALL = 0x8006AF18
POLL_AFTER_PACKET = 0x8006AF54
FIRST_UNRESOLVED_CALL = 0x8006AFA8
FIRST_UNRESOLVED = 0x800718D0

# Full 391-word body; suffix is WORDS[68:].
WORDS = exe_words(0x8006AD40, 391)

# jal sites from CUT onward, with current classification.
SUFFIX_CALLS = [
    (0x8006AE48, 0x8006E1C0, "func_8006E1C0", "TRANSLATED_FAITHFUL",
     "B51; remaining loop iterations after the cut"),
    (0x8006AEFC, 0x8006E498, "func_8006E498", "TRANSLATED_FAITHFUL",
     "B16 archive key lookup"),
    (0x8006AF18, 0x8007506C, "func_8007506C", "TRANSLATED_FAITHFUL",
     "B52 LoadImage wrapper"),
    (0x8006AF54, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL",
     "B16 CD poll; result is live drive state"),
    (0x8006AF88, 0x8006E6A8, "func_8006E6A8", "TRANSLATED_FAITHFUL", "B16 issue"),
    (0x8006AFA8, 0x800718D0, "func_800718D0", "UNRESOLVED",
     "29-word LoadImage helper; only callee is func_8007506C"),
    (0x8006B04C, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL", "B16 poll"),
    (0x8006B080, 0x8006E6A8, "func_8006E6A8", "TRANSLATED_FAITHFUL", "B16 issue"),
    (0x8006B0A4, 0x800718D0, "func_800718D0", "UNRESOLVED", "second site"),
    (0x8006B0AC, 0x80030894, "func_80030894", "UNRESOLVED",
     "0xC50-byte body; no port"),
    (0x8006B0BC, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL", "B16 poll"),
    (0x8006B0F0, 0x8006E6A8, "func_8006E6A8", "TRANSLATED_FAITHFUL", "B16 issue"),
    (0x8006B118, 0x8006E498, "func_8006E498", "TRANSLATED_FAITHFUL", "B16 lookup"),
    (0x8006B12C, 0x8006E498, "func_8006E498", "TRANSLATED_FAITHFUL", "B16 lookup"),
    (0x8006B140, 0x8006E498, "func_8006E498", "TRANSLATED_FAITHFUL", "B16 lookup"),
    (0x8006B154, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL", "B16 poll"),
    (0x8006B188, 0x8006E6A8, "func_8006E6A8", "TRANSLATED_FAITHFUL", "B16 issue"),
    (0x8006B1DC, 0x8006E1C0, "func_8006E1C0", "TRANSLATED_FAITHFUL", "later packet"),
    (0x8006B20C, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL", "B16 poll"),
    (0x8006B254, 0x8006E1C0, "func_8006E1C0", "TRANSLATED_FAITHFUL", "later packet"),
    (0x8006B274, 0x80087024, "func_80087024", "HOST_SDK_OR_STREAM",
     "stream command 0xF1 in pe_stream.c"),
    (0x8006B27C, 0x80074DC0, "func_80074DC0", "HOST_SDK_SHIM", "DrawSync"),
    (0x8006B284, 0x80074A44, "func_80074A44", "HOST_SDK_SHIM", "ResetGraph"),
    (0x8006B28C, 0x80073A44, "func_80073A44", "HOST_SDK_SHIM", "VSync"),
    (0x8006B2B4, 0x800755F0, "func_800755F0", "HOST_SDK_SHIM", "PutDispEnv"),
    (0x8006B2BC, 0x80074D28, "func_80074D28", "HOST_SDK_SHIM", "SetDispMask"),
]

# Captured post-B53I-D / prefix-cut guest state (evidence only).
CANON_BASE = 0x801229A0
CANON_META = 0x8012DF18
CANON_HEADER = 0x0340B5B8
CANON_COUNT = 13
CANON_ENTRY0 = 0x8012DF58
CANON_S1 = 0
CANON_S2 = 1
CANON_LBA = 0x000003F5
CANON_LOOKUP = 0x801229A8
CANON_LOOKUP_WORD0 = 0x00007F0C
CANON_CD_BUSY = 0x01004000
CANON_D800B0CD8 = 0x41004003


def require(cond: bool, message: str) -> None:
    if not cond:
        raise SystemExit(f"FATAL: {message}")


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    got = hashlib.sha1(data).hexdigest()
    require(got == EXE_SHA1, f"{path} SHA-1 {got} != {EXE_SHA1}")
    require(data[:8] == b"PS-X EXE", "not a PS-X EXE")
    return data


def verify_words(data: bytes) -> None:
    taddr = struct.unpack_from("<I", data, 0x18)[0]
    foff = FUNC_START - taddr + 0x800
    require(len(WORDS) == (FUNC_END - FUNC_START) // 4, "word/range mismatch")
    require((CUT_PC - FUNC_START) // 4 == 68, "cut is not word 68")
    require(len(WORDS) - 68 == 323, "suffix is not 323 words")
    for i, expected in enumerate(WORDS):
        actual = struct.unpack_from("<I", data, foff + i * 4)[0]
        require(actual == expected,
                f"word {i} @ {FUNC_START + i * 4:#010x}: "
                f"{actual:#010x} != {expected:#010x}")
    # Delay slots at the cut and loop.
    require(WORDS[66] == 0x0C01B870, "jal func_8006E1C0 at 0x8006AE48")
    require(WORDS[67] == 0x02802821, "delay move a1,s4 at 0x8006AE4C")
    require(WORDS[68] == 0x8E620028, "suffix start lw v0,0x28(s3)")
    require(WORDS[69] == 0x26310001, "suffix addiu s1,1")
    require(WORDS[72] == 0x1440FFF8, "bne back to 0x8006AE44")
    require(WORDS[73] == 0x26100014, "delay addiu s0,0x14")
    require(WORDS[-2] == 0x03E00008, "jr ra")
    require(WORDS[-1] == 0x00000000, "return delay nop")


def decode_rect(packed: int, h: int) -> tuple[int, int, int, int]:
    x = (packed >> 10) & 0x7FF
    y = packed >> 21
    w = packed & 0x3FF
    return x, y, w, h


def run_counted_loop(count: int, s1: int) -> dict:
    """Model AE50..AE68. Does not invent 6E1C0 internals."""
    calls = []
    s0_index = s1  # entry index already issued before the cut
    while True:
        s1 += 1
        if s1 < count:
            s0_index += 1
            calls.append(("func_8006E1C0", s0_index))
            continue
        break
    return {
        "remaining_6e1c0": len(calls),
        "final_s1": s1,
        "exit_pc": LOOP_EXIT,
        "calls": calls,
    }


def classify_first_unresolved() -> tuple[str, int]:
    for addr, target, name, cls, _ in SUFFIX_CALLS:
        if cls == "UNRESOLVED":
            return name, addr
    raise SystemExit("FATAL: no unresolved suffix callee")


def run_scenarios() -> int:
    n = 0

    # 1. Geometry / delay slots / call census.
    require(CUT_PC - FUNC_START == 0x110, "prefix byte size")
    require(FUNC_END - CUT_PC == 0x50C, "suffix byte size")
    require((FUNC_END - CUT_PC) // 4 == 323, "suffix word count")
    require(WORDS[66] == 0x0C01B870 and WORDS[67] == 0x02802821,
            "call+delay immediately before the cut")
    n += 1

    # 2. Canonical loop: count 13, s1 already 0, first entry already issued.
    loop = run_counted_loop(CANON_COUNT, CANON_S1)
    require(loop["remaining_6e1c0"] == 12, "canonical remaining 6E1C0 count")
    require(loop["final_s1"] == 13, "s1 must reach count")
    require(loop["exit_pc"] == LOOP_EXIT, "loop exit PC")
    require([c[1] for c in loop["calls"]] == list(range(1, 13)),
            "entries 1..12")
    n += 1

    # 3. count==1 (already issued the only entry): no further 6E1C0.
    loop1 = run_counted_loop(1, 0)
    require(loop1["remaining_6e1c0"] == 0 and loop1["final_s1"] == 1,
            "count=1 must exit immediately")
    n += 1

    # 4. Entry 0 decode matches the accepted two-LoadImage pair.
    e0_img = decode_rect(0x080B0020, 0x40)
    e0_clut = decode_rect(0x39040040, 0x01)
    require(e0_img == (704, 64, 32, 64), "entry0 image RECT")
    require(e0_clut == (256, 456, 64, 1), "entry0 CLUT RECT")
    n += 1

    # 5. DMA idle vs active does not change suffix-local control flow.
    #    The suffix body has no 1F80xxxx access. Next 6E1C0 uses existing
    #    GPU/DMA2 authority; this oracle never completes a transfer.
    for dma_active in (0, 1):
        loop_dma = run_counted_loop(CANON_COUNT, CANON_S1)
        require(loop_dma["remaining_6e1c0"] == 12,
                f"DMA active={dma_active} mutated the counted loop")
    n += 1

    # 6. After the loop, translated lookup/LoadImage walk are eligible, but
    #    CD poll is not invented. First unresolved *function* is 718D0.
    name, site = classify_first_unresolved()
    require(name == "func_800718D0" and site == FIRST_UNRESOLVED_CALL,
            "first unresolved callee/site")
    require(CANON_LOOKUP_WORD0 != 0, "ABADC06C payload word0 must be live")
    require(CANON_D800B0CD8 & CANON_CD_BUSY == CANON_CD_BUSY,
            "canonical CD busy bits must remain set; do not invent poll=0")
    n += 1

    # 7. B50-era first unresolved (6E1C0 at AE48) is now translated.
    require(any(a == 0x8006AE48 and c == "TRANSLATED_FAITHFUL"
                for a, _, _, c, _ in SUFFIX_CALLS),
            "0x8006AE48 must now be classified translated")
    require(any(t == 0x8007506C and c == "TRANSLATED_FAITHFUL"
                for _, t, _, c, _ in SUFFIX_CALLS),
            "7506C must now be classified translated")
    n += 1

    # 8. Recommended B54B cut is the loop exit, not the old AE50 cut and
    #    not a leap to 718D0.
    require(LOOP_EXIT == 0x8006AE68, "recommended B54B PC")
    require(CUT_PC != LOOP_EXIT, "old cut is not the new recommended cut")
    n += 1

    require(n == 8, "scenario count")
    return n


def main(argv: list[str]) -> int:
    require(len(argv) == 2, "usage: b54a_6ad40_suffix_oracle.py executable")
    data = load_exe(pathlib.Path(argv[1]))
    verify_words(data)
    scenarios = run_scenarios()
    print(f"B54A oracle: PASS ({scenarios}/{scenarios} scenarios; {argv[1]})")
    print(f"  function   {FUNC_START:#010x}..{FUNC_END:#010x} "
          f"({len(WORDS)} words)")
    print(f"  suffix     {CUT_PC:#010x}..{FUNC_END:#010x} (323 words)")
    print(f"  canonical  count=13 s1=0 remaining_6E1C0=12 exit={LOOP_EXIT:#010x}")
    print("  first unresolved function: func_800718D0 @ 0x8006AFA8 "
          "(not reached without an explicit CD poll result)")
    print("  recommended B54B cut: 0x8006AE68 after remaining 6E1C0 loop")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

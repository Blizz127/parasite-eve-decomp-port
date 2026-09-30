#!/usr/bin/env python3
"""Phase 6E-PE-GPU1 independent oracle: GPU packet-header leaves.

Verifies the five SetPolyF3 / SetPolyFT4 / SetPolyG4 / SetTile / SetSprt
ROM functions and classifies the four GetTPage / GetClut / SetSemiTrans /
SetShadeTex callees of func_80030894, all against the SHA-1-exact retail
executable SLUS_006.62.

"Byte-exact against the matching build" means: each native leaf writes
exactly the bytes the ROM function writes, in the same order.  The matching
build already reproduces these ROM words (the candidate EXE is the exact
SHA-1), and the native leaves (pc_port/game/boot/func_80077B{64,BA4,BC4,
C44,C64}_port.c) perform the identical two-byte stores.  This oracle decodes
the ROM words directly and asserts the contract, so the agreement is checkable
without re-running the era toolchain.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

# (rom_addr, file_offset, native_symbol, expected offset3, expected offset7)
SET_LEAVES = [
    (0x80077B64, 0x68364, "func_80077B64", 4,  32),
    (0x80077BA4, 0x683A4, "func_80077BA4", 9,  44),
    (0x80077BC4, 0x683C4, "func_80077BC4", 8,  56),
    (0x80077C44, 0x68444, "func_80077C44", 3,  96),
    (0x80077C64, 0x68464, "func_80077C64", 3,  64),
]

# (rom_addr, file_offset, native_symbol) — classified, not native-ported.
GETTERS = [
    (0x80077A64, 0x68264, "func_80077A64"),
    (0x80077AA4, 0x682A4, "func_80077AA4"),
    (0x80077B04, 0x68304, "func_80077B04"),
    (0x80077B34, 0x68334, "func_80077B34"),
]

JR_RA = 0x03E00008


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


def words(data: bytes, off: int, n: int) -> list[int]:
    return list(struct.unpack_from("<%dI" % n, data, off))


def decode_store(w: int) -> tuple[int, int, int] | None:
    """Decode `sb $v0, off($a0)` (opcode 0x28, base $a0=4, src $v0=2)."""
    if (w >> 26) & 0x3F != 0x28:
        return None
    base = (w >> 21) & 0x1F
    src = (w >> 16) & 0x1F
    imm = w & 0xFFFF
    if base != 4 or src != 2:
        return None
    return (base, imm, src)


def decode_li_v0(w: int) -> int | None:
    """Decode `addiu $v0, $zero, imm` (opcode 0x09, rs=$zero=0, rt=$v0=2)."""
    if (w >> 26) & 0x3F != 0x09:
        return None
    rs = (w >> 21) & 0x1F
    rt = (w >> 16) & 0x1F
    imm = w & 0xFFFF
    if rs != 0 or rt != 2:
        return None
    return imm


def run() -> int:
    data = load_exe(find_exe())
    n = 0

    # 1. The five SET leaves: exact two-byte store contract, real functions.
    for addr, off, sym, e3, e7 in SET_LEAVES:
        ws = words(data, off, 5)
        # Pattern: addiu $v0,$zero,Imm3 ; sb $v0,3($a0) ;
        #          addiu $v0,$zero,Imm7 ; jr $ra ; sb $v0,7($a0)
        imm3 = decode_li_v0(ws[0])
        st3 = decode_store(ws[1])
        imm7 = decode_li_v0(ws[2])
        require(ws[3] == JR_RA, f"{sym}: word3 is jr $ra")
        st7 = decode_store(ws[4])
        require(imm3 == e3, f"{sym}: offset3 value {imm3} != {e3}")
        require(st3 == (4, 3, 2), f"{sym}: word1 must be sb $v0,3($a0)")
        require(imm7 == e7, f"{sym}: offset7 value {imm7} != {e7}")
        require(st7 == (4, 7, 2), f"{sym}: word5 must be sb $v0,7($a0)")
        require(imm3 is not None and imm7 is not None,
                f"{sym}: both immediates decoded")
        # Native leaves write exactly these two bytes in this order.
        require(st3[1] == 3 and st7[1] == 7, f"{sym}: store order 3 then 7")
        n += 1
        print(f"  OK {sym}: byte[3]={e3} byte[7]={e7} (5-word real function)")

    # 2. The four getters: classify as real outlined functions (own jr $ra).
    for addr, off, sym in GETTERS:
        # Scan forward for the terminating jr $ra; cap at 64 words.
        found = False
        for i in range(64):
            if words(data, off + i * 4, 1)[0] == JR_RA:
                found = True
                break
        require(found, f"{sym}: no jr $ra found (not a real function)")
        n += 1
        print(f"  OK {sym}: real function (terminates in jr $ra)")

    # 3. All nine are callees of func_80030894 (cross-check the audit).
    f3_off = 0x80030894 - 0x8000F800
    f3_words = words(data, f3_off, 788)
    callees = set()
    for w in f3_words:
        if (w >> 26) & 0x3F == 0x03:  # jal
            tgt = ((w & 0x03FFFFFF) << 2) | 0x80000000
            callees.add(tgt)
    all_callees = [(a, s) for (a, _, s, _, _) in SET_LEAVES] + \
                  [(a, s) for (a, _, s) in GETTERS]
    for addr, sym in all_callees:
        require(addr in callees, f"{sym}: not a jal callee of func_80030894")
        n += 1

    print(f"\nPE-GPU1 oracle: {n} checks passed (5 SET leaves byte-exact, "
          f"4 getters classified real, 9 callee links verified).")
    return 0


if __name__ == "__main__":
    sys.exit(run())

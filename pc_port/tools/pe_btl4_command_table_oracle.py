#!/usr/bin/env python3
"""PE-BTL4 independent oracle: Writer A vs 29810 non-producers.

Checks SHA-1-exact EXE words without importing production C.

Writer A (func_8006B4F8 clip loop at 0x8006B84C, idA*192 + 0x1C0)
is real. It is NOT the unique table writer: Writer B at 0x8006C140
stores overlay+0x1C0+idB*4 without idA*192. See
pe_btl3_command_table_oracle.py for that producer.

Also pins:
* func_8001A680 / func_8001A784 are the only text materializations of
  D_800B0E98 (0x0E98). They read the table; they do not fill it.
* func_800339A0 and func_80030640 (the 29810 callees before 1A680)
  do not touch that table.
* On the 0x55 -> 29810 -> 1A680 path, *D_8009D254+0x0C is still the
  type-0 ctor byte. The real table row is type 0 / command 4.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TEXT_LO = 0x8001220C
TEXT_HI = 0x80091080
TAIL_LO = 0x800C22F8
TAIL_HI = 0x800E1000

S6_BASE = [
    0x3C16800B,  # lui $s6, 0x800B
    0x26D60CD8,  # addiu $s6, $s6, 0xCD8  -> D_800B0CD8
]
CLIP_STORE = exe_words(0x8006B84C, 13)
    # lbu $a1, 11($a2)   idA
    # lbu $a0, 7($a2)    idB
    # lw  $v1, 4($a2)    pointer word
    # sll $v0, $a1, 1
    # addu $v0, $v0, $a1
    # sll $v0, $v0, 6    idA * 192
    # addu $v0, $v0, $s6
    # addiu $v0, $v0, 0x1C0  -> D_800B0E98 row
    # sll $a0, $a0, 2
    # addu $a0, $a0, $v0
    # and $v1, $v1, $a3  (0x00FFFFFF)
    # addu $v1, $s4, $v1
    # sw  $v1, 0($a0)
FN_339A0 = exe_words(0x800339A0, 31)
FN_30640_HEAD = [
    0x3C04800A, 0x8C84D278, 0x27BDFFE8, 0xAFBF0014,
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def signed_imm(word: int) -> int:
    imm = word & 0xFFFF
    return imm - 0x10000 if imm & 0x8000 else imm


def opcode(word: int) -> int:
    return word >> 26


def rs(word: int) -> int:
    return (word >> 21) & 31


def fn(word: int) -> int:
    return word & 63


def words_at(data: bytes, start: int, count: int) -> list[int]:
    return [load_u32(data, start + i * 4) for i in range(count)]


def iter_text_and_tail():
    yield from range(TEXT_LO, TEXT_HI, 4)
    yield from range(TAIL_LO, TAIL_HI, 4)


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require(words_at(data, 0x8006B548, 2) == S6_BASE, "6B4F8 $s6 = D_800B0CD8")
    require(words_at(data, 0x8006B84C, 13) == CLIP_STORE,
            "6B84C clip-table store loop")
    require(load_u32(data, 0x8006B84C) == 0x90C5000B, "idA at record+0xB")
    require(load_u32(data, 0x8006B850) == 0x90C40007, "idB at record+0x7")
    require(load_u32(data, 0x8006B87C) == 0xAC830000, "sw pointer to slot")

    e98 = []
    star_1c0 = []
    for va in iter_text_and_tail():
        word = load_u32(data, va)
        op = word >> 26
        imm = word & 0xFFFF
        if imm == 0x0E98 and op in (8, 9, 12, 13, 15, 32, 33, 35, 36, 37, 40, 41, 43):
            e98.append(va)
        if op in (8, 9) and imm == 0x01C0 and ((word >> 21) & 31) != 0:
            nearby_sll6 = False
            for delta in range(-24, 12, 4):
                other = va + delta
                if not (TEXT_LO <= other < TEXT_HI or TAIL_LO <= other < TAIL_HI):
                    continue
                ww = load_u32(data, other)
                if (ww >> 26) == 0 and (ww & 63) == 0 and ((ww >> 6) & 31) == 6:
                    nearby_sll6 = True
            if nearby_sll6:
                star_1c0.append(va)

    require(e98 == [0x8001A6A8, 0x8001A7BC],
            f"D_800B0E98 materializations: {[hex(x) for x in e98]}")
    require(star_1c0 == [0x8006B868],
            f"*192+0x1C0 Writer A site: {[hex(x) for x in star_1c0]}")

    require(words_at(data, 0x800339A0, 31) == FN_339A0, "339A0 window")
    require(jal_target(load_u32(data, 0x80029998)) == 0x800339A0,
            "29810 still jals 339A0")
    require(load_u32(data, 0x800339A8) == 0x24C60E38, "339A0 src 0x80010E38")
    require(load_u32(data, 0x80033A04) == 0xA7820114, "339A0 sh gp+0x114")
    require(load_u32(data, 0x80033A0C) == 0xA3840110, "339A0 sb gp+0x110")
    require(load_u32(data, 0x80033A10) == 0xA7820116, "339A0 sh gp+0x116")
    require(0x0E98 not in {w & 0xFFFF for w in FN_339A0},
            "339A0 must not mention 0x0E98")

    require(words_at(data, 0x80030640, 4) == FN_30640_HEAD, "30640 head")
    require(jal_target(load_u32(data, 0x80029960)) == 0x80030640,
            "29810 still jals 30640")
    require(load_u32(data, 0x80030640) == 0x3C04800A, "30640 uses D_8009D278")
    require(load_u32(data, 0x8001A6A8) == 0x24840E98, "1A680 addiu 0x0E98")
    require(load_u32(data, 0x8001A7BC) == 0x24840E98, "1A784 addiu 0x0E98")
    require(load_u32(data, 0x8001A6C8) == 0x8C620000, "1A680 lw table slot")

    gp = 0x8009CD70
    require(gp + 0x4E4 == 0x8009D254, "D_8009D254 is 0x4E4($gp)")
    require(load_u32(data, 0x800910C0) == 0x8001735C, "opcode 0x08 -> 1735C")
    require(jal_target(load_u32(data, 0x80017394)) == 0x80035038,
            "0x08 jals ctor 35038")
    require(load_u32(data, 0x80035100) == 0x92020000, "ctor lbu desc[0]")
    require(load_u32(data, 0x80035148) == 0x14400008,
            "bne desc[0]!=0 skips D254 publish")
    require(load_u32(data, 0x80035158) == 0xAF9104E4,
            "type0 sw actor -> 0x4E4($gp) D_8009D254")
    require(load_u32(data, 0x80035174) == 0x92020000, "reload desc[0]")
    require(load_u32(data, 0x8003517C) == 0xA222000C,
            "sb desc[0] actor+0x0C")
    require(load_u32(data, 0x80035180) == 0x92020001, "lbu desc[1]")
    require(load_u32(data, 0x80035188) == 0xA222000D, "sb desc[1] actor+0x0D")
    require(load_u32(data, 0x8001A6A0) == 0x9223000C, "1A680 lbu type actor+0x0C")
    require(load_u32(data, 0x800299A4) == 0x3C04800A, "29810 lui D_800Axxxx")
    require(load_u32(data, 0x800299A8) == 0x8C84D254, "29810 a0=*D_8009D254")
    require(jal_target(load_u32(data, 0x800299B0)) == 0x8001A680,
            "29810 jal 1A680")

    d254_ptr_sw = []
    for va in range(TEXT_LO, TEXT_HI, 4):
        word = load_u32(data, va)
        if opcode(word) == 43 and rs(word) == 28 and signed_imm(word) == 0x4E4:
            d254_ptr_sw.append(va)
    require(d254_ptr_sw == [0x80034FB0, 0x8003502C, 0x80035158, 0x80036124],
            f"D_8009D254 pointer stores: {[hex(x) for x in d254_ptr_sw]}")
    require(load_u32(data, 0x80034FB0) == 0xAF8004E4, "ctor-pool zero D254")
    require(load_u32(data, 0x8003502C) == 0xAF8004E4, "ctor fail zero D254")
    require(load_u32(data, 0x80036124) == 0xAF8004E4, "dtor zero D254")

    for start, end, label in (
        (0x800144FC, 0x80014680, "144FC"),
        (0x80029810, 0x800299B0, "29810-before-1A680"),
        (0x800293F4, 0x80029450, "293F4"),
        (0x80030640, 0x80030740, "30640"),
        (0x800339A0, 0x80033A20, "339A0"),
    ):
        for va in range(start, end, 4):
            word = load_u32(data, va)
            if opcode(word) == 3:
                tgt = jal_target(word)
                require(tgt not in (0x8002FF78, 0x80018164, 0x80035038),
                        f"{label} jal type-writer {tgt:#x} at {va:#x}")
            if opcode(word) in (40, 41, 43) and signed_imm(word) == 0x0C:
                require(label == "293F4" and va == 0x80029418,
                        f"{label} store +0x0C at {va:#x} (HP record only)")

    require(load_u32(data, 0x80020A78) == 0x8CC5000C, "209F0 lw stack copy +0xC")
    require(load_u32(data, 0x80020A88) == 0xACE5000C, "209F0 sw stack dest +0xC")
    require(load_u32(data, 0x80020ACC) == 0xA0E5000C, "209F0 sb tail of memcpy")
    require(load_u32(data, 0x80020AD4) == 0x8C84D278,
            "209F0 then uses D_8009D278 not D254")
    require(load_u32(data, 0x80029404) == 0x84A3000C, "293F4 lh record+0x0C")
    require(load_u32(data, 0x80029418) == 0xA4A6000C, "293F4 sh HP into record")

    print(
        "PASS: Writer A *192+0x1C0 store is 6B4F8/6B84C; "
        "339A0/30640 are not producers (Writer B is 6C140, see BTL3); "
        "1A680 type byte is ctor desc[0]==0 via D_8009D254"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

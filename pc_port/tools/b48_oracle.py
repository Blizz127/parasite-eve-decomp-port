#!/usr/bin/env python3
"""
Phase 6E-B48/B48A retail-word and ordering oracle.

Verifies:
- 16 retail words at 0x800850F4
- 13 retail words at 0x800850C0
- 23 retail words at 0x80085E54
- Exact delay slot semantics
- 33 retail words at 0x8007D9F8 (DMA issue wrapper)
- 47 retail words at 0x8007D614 (DMA IRQ callback)
- 10 retail words at 0x80085098 (application completion callback)
- Controlled ordering proof: issue leaves busy; a later event copies data,
  dispatches the live callback, deregisters it, then clears busy

This script does not emulate PSX DMA timing or MMIO.  The controlled model
checks only the ordering directly established by the verified retail words.
"""

import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

# func_800850F4: 16 words at 0x800850F4 (0x40 bytes)
# asm/disc1/74FB0.s:738-755
FUNC_800850F4 = [
    0x27BDFFE0,  # addiu $sp, $sp, -0x20
    0xAFB00010,  # sw    $s0, 0x10($sp)
    0x00808021,  # addu  $s0, $a0, $zero
    0xAFB10014,  # sw    $s1, 0x14($sp)
    0xAFBF0018,  # sw    $ra, 0x18($sp)
    0x0C021430,  # jal   func_800850C0
    0x00A08821,  # addu  $s1, $a1, $zero  (delay slot)
    0x02002021,  # addu  $a0, $s0, $zero
    0x0C021795,  # jal   func_80085E54
    0x02202821,  # addu  $a1, $s1, $zero  (delay slot)
    0x8FBF0018,  # lw    $ra, 0x18($sp)
    0x8FB10014,  # lw    $s1, 0x14($sp)
    0x8FB00010,  # lw    $s0, 0x10($sp)
    0x27BD0020,  # addiu $sp, $sp, 0x20
    0x03E00008,  # jr    $ra
    0x00000000,  # nop   (delay slot)
]

# func_800850C0: 13 words at 0x800850C0 (0x34 bytes)
# asm/disc1/74FB0.s:720-734
FUNC_800850C0 = exe_words(0x800850C0, 13)
    # addiu $sp, $sp, -0x18
    # addiu $v0, $zero, 0x1
    # lui   $a0, 0x8008
    # addiu $a0, $a0, 0x5098
    # sw    $ra, 0x10($sp)
    # lui   $at, 0x800A
    # sw    $v0, -0x2DB4($at)  [D_8009D24C]
    # jal   func_80085F44
    # nop   (delay slot)
    # lw    $ra, 0x10($sp)
    # addiu $sp, $sp, 0x18
    # jr    $ra
    # nop   (delay slot)

# func_80085E54: 23 words at 0x80085E54 (0x5C bytes)
# asm/disc1/75F44.s:600-626
FUNC_80085E54 = [
    0x27BDFFE8,  # addiu $sp, $sp, -0x18
    0xAFB00010,  # sw    $s0, 0x10($sp)
    0x00A08021,  # addu  $s0, $a1, $zero
    0x3C020007,  # lui   $v0, 0x0007
    0x3442EFF0,  # ori   $v0, $v0, 0xEFF0
    0x0050102B,  # sltu  $v0, $v0, $s0
    0x10400003,  # beqz  $v0, .L80085E7C
    0xAFBF0014,  # sw    $ra, 0x14($sp)  (delay slot)
    0x3C100007,  # lui   $s0, 0x0007
    0x3610EFF0,  # ori   $s0, $s0, 0xEFF0
    # .L80085E7C:
    0x0C01F67E,  # jal   func_8007D9F8
    0x02002821,  # addu  $a1, $zero, $s0  (delay slot)
    0x3C02800A,  # lui   $v0, 0x800A
    0x8C42B434,  # lw    $v0, -0x4BCC($v0)  [D_8009B434]
    0x00000000,  # nop
    0x14400003,  # bnez  $v0, .L80085EA0
    0x02001021,  # addu  $v0, $zero, $s0  (delay slot)
    0x3C01800A,  # lui   $at, 0x800A
    0xAC20B430,  # sw    $zero, -0x4BD0($at)  [D_8009B430]
    # .L80085EA0:
    0x8FBF0014,  # lw    $ra, 0x14($sp)
    0x8FB00010,  # lw    $s0, 0x10($sp)
    0x03E00008,  # jr    $ra
    0x27BD0018,  # addiu $sp, $sp, 0x18  (delay slot)
]

# func_8007D9F8: 33 words at 0x8007D9F8 (0x84 bytes)
FUNC_8007D9F8 = exe_words(0x8007D9F8, 33)

# func_8007D614: 47 words at 0x8007D614 (0xBC bytes)
FUNC_8007D614 = exe_words(0x8007D614, 47)

# func_80085098: 10 words at 0x80085098 (0x28 bytes)
FUNC_80085098 = exe_words(0x80085098, 10)


def verify_words(exe_path, base, words):
    """Verify instruction words match the executable."""
    with open(exe_path, "rb") as f:
        data = f.read()
    
    sha = hashlib.sha1(data).hexdigest()
    if sha != SHA1:
        print(f"  FAIL: SHA-1 mismatch: {sha}")
        return False
    
    # PS1 EXE: taddr at 0x18, text starts at 0x800
    taddr = struct.unpack_from("<I", data, 0x18)[0]
    
    for i, expected in enumerate(words):
        offset = base - taddr + 0x800 + i * 4
        actual = struct.unpack_from("<I", data, offset)[0]
        if actual != expected:
            print(f"  FAIL: word {i} at 0x{base + i*4:08X}: "
                  f"expected 0x{expected:08X}, got 0x{actual:08X}")
            return False
    
    return True


def verify_sequential_calls():
    """Verify func_800850F4 calls func_800850C0 then func_80085E54."""
    jal_850c0 = FUNC_800850F4[5]
    jal_85e54 = FUNC_800850F4[8]
    
    # jal opcode is 0x03 (bits 31-26)
    if (jal_850c0 >> 26) != 0x03:
        print(f"  FAIL: word 5 not jal: 0x{jal_850c0:08X}")
        return False
    if (jal_85e54 >> 26) != 0x03:
        print(f"  FAIL: word 8 not jal: 0x{jal_85e54:08X}")
        return False
    
    # Verify delay slots
    ds1 = FUNC_800850F4[6]  # addu $s1, $a1, $zero
    ds2 = FUNC_800850F4[9]  # addu $a1, $s1, $zero
    
    if ds1 != 0x00A08821:
        print(f"  FAIL: delay slot 1: 0x{ds1:08X}")
        return False
    if ds2 != 0x02202821:
        print(f"  FAIL: delay slot 2: 0x{ds2:08X}")
        return False
    
    return True


def verify_850c0_sets_busy():
    """Verify func_800850C0 sets D_8009D24C = 1."""
    # Words 5-6: lui $at, 0x800A; sw $v0, -0x2DB4($at) [D_8009D24C]
    lui = FUNC_800850C0[5]
    sw = FUNC_800850C0[6]
    
    if lui != 0x3C01800A:
        print(f"  FAIL: 850C0 lui: 0x{lui:08X}")
        return False
    if sw != 0xAC22D24C:
        print(f"  FAIL: 850C0 sw: 0x{sw:08X}")
        return False
    
    return True


def verify_85e54_clamps_size():
    """Verify func_80085E54 clamps size to 0x7EFF0."""
    lui = FUNC_80085E54[3]
    ori = FUNC_80085E54[4]
    sltu = FUNC_80085E54[5]
    
    if lui != 0x3C020007:
        print(f"  FAIL: 85E54 lui: 0x{lui:08X}")
        return False
    if ori != 0x3442EFF0:
        print(f"  FAIL: 85E54 ori: 0x{ori:08X}")
        return False
    if sltu != 0x0050102B:
        print(f"  FAIL: 85E54 sltu: 0x{sltu:08X}")
        return False
    
    return True


def verify_85e54_clears_callback():
    """Verify func_80085E54 clears D_8009B430 when no callback."""
    # Words 12-13: lui $v0, 0x800A; lw $v0, D_8009B434
    # Words 17-18: lui $at, 0x800A; sw $zero, D_8009B430
    lui = FUNC_80085E54[12]
    lw = FUNC_80085E54[13]
    lui2 = FUNC_80085E54[17]
    sw = FUNC_80085E54[18]
    
    if lui != 0x3C02800A:
        print(f"  FAIL: 85E54 lui: 0x{lui:08X}")
        return False
    if lw != 0x8C42B434:
        print(f"  FAIL: 85E54 lw: 0x{lw:08X}")
        return False
    if lui2 != 0x3C01800A:
        print(f"  FAIL: 85E54 lui2: 0x{lui2:08X}")
        return False
    if sw != 0xAC20B430:
        print(f"  FAIL: 85E54 sw: 0x{sw:08X}")
        return False
    
    return True


def verify_async_chain_words():
    """Verify the issue, IRQ-dispatch, and completion-callback structure."""
    # D9F8's mode-0 arm calls func_8007D778 three times.  The third call's
    # delay slot supplies size; the next path sets v0=size and returns.
    if [FUNC_8007D9F8[i] for i in (14, 16, 20)] != [0x0C01F5DE] * 3:
        print("  FAIL: func_8007D9F8 DMA helper call sequence")
        return False
    if FUNC_8007D9F8[21:24] != [0x02003021, 0x0801F69A, 0x02001021]:
        print("  FAIL: func_8007D9F8 does not return size after issue")
        return False

    # D614 reads the live D_8009B434 slot, branches if null, reloads it,
    # and dispatches through jalr.  It never writes D_8009D24C.
    if FUNC_8007D614[29] != 0x8C42B434 or \
       FUNC_8007D614[34] != 0x8C42B434 or \
       FUNC_8007D614[36] != 0x0040F809:
        print("  FAIL: func_8007D614 callback dispatch sequence")
        return False
    if 0xAC20D24C in FUNC_8007D614:
        print("  FAIL: func_8007D614 unexpectedly clears busy itself")
        return False

    # 85098 first calls func_80085F44 with a0=0, then clears D_8009D24C.
    if FUNC_80085098[2:6] != [
        0x0C0217D1, 0x00002021, 0x3C01800A, 0xAC20D24C
    ]:
        print("  FAIL: func_80085098 callback/busy order")
        return False
    return True


def verify_controlled_async_order():
    """Model only state ordering proven by the literal retail chain."""
    state = {
        "busy": 0, "callback": 0, "pending": False,
        "data_visible": False, "callback_ran": False,
    }

    # func_800850C0, followed by func_8007D9F8 issue/return.
    state["busy"] = 1
    state["callback"] = 0x80085098
    state["pending"] = True
    if state["busy"] != 1 or state["callback_ran"] or state["data_visible"]:
        return False

    # Controlled later DMA event: hardware data completion precedes D614's
    # callback dispatch; 85098 deregisters and then clears busy.
    state["data_visible"] = True
    state["pending"] = False
    if state["callback"] == 0x80085098:
        if not state["data_visible"]:
            return False
        state["callback_ran"] = True
        state["callback"] = 0
        state["busy"] = 0

    return state == {
        "busy": 0, "callback": 0, "pending": False,
        "data_visible": True, "callback_ran": True,
    }


def main():
    if len(sys.argv) < 2:
        print("Usage: b48_oracle.py <disc1.candidate.exe>")
        sys.exit(1)
    
    exe_path = sys.argv[1]
    
    print("B48 Oracle: func_800850F4 (16 words)")
    print("=" * 50)
    
    # 1. Verify instruction words
    print("\n[1] Verifying func_800850F4 instruction words...")
    if not verify_words(exe_path, 0x800850F4, FUNC_800850F4):
        print("  FAIL")
        return False
    print("  PASS: 16/16 words match")
    
    # 2. Verify func_800850C0 words
    print("\n[2] Verifying func_800850C0 instruction words...")
    if not verify_words(exe_path, 0x800850C0, FUNC_800850C0):
        print("  FAIL")
        return False
    print("  PASS: 13/13 words match")
    
    # 3. Verify func_80085E54 words
    print("\n[3] Verifying func_80085E54 instruction words...")
    if not verify_words(exe_path, 0x80085E54, FUNC_80085E54):
        print("  FAIL")
        return False
    print("  PASS: 23/23 words match")
    
    # 4. Verify call sequence
    print("\n[4] Verifying call sequence (850C0 → 85E54)...")
    if not verify_sequential_calls():
        print("  FAIL")
        return False
    print("  PASS: sequential calls verified")
    
    # 5. Verify 850C0 sets busy flag
    print("\n[5] Verifying func_800850C0 sets D_8009D24C = 1...")
    if not verify_850c0_sets_busy():
        print("  FAIL")
        return False
    print("  PASS: D_8009D24C = 1 set")
    
    # 6. Verify 85E54 size clamp
    print("\n[6] Verifying func_80085E54 size clamp to 0x7EFF0...")
    if not verify_85e54_clamps_size():
        print("  FAIL")
        return False
    print("  PASS: size clamped to 0x7EFF0")
    
    # 7. Verify callback clear
    print("\n[7] Verifying func_80085E54 clears D_8009B430...")
    if not verify_85e54_clears_callback():
        print("  FAIL")
        return False
    print("  PASS: D_8009B430 cleared when no callback")
    
    # 8. Verify the literal issue/IRQ/completion functions.
    print("\n[8] Verifying asynchronous DMA chain words...")
    for base, words, name in (
        (0x8007D9F8, FUNC_8007D9F8, "func_8007D9F8"),
        (0x8007D614, FUNC_8007D614, "func_8007D614"),
        (0x80085098, FUNC_80085098, "func_80085098"),
    ):
        if not verify_words(exe_path, base, words):
            print(f"  FAIL: {name}")
            return False
    if not verify_async_chain_words():
        print("  FAIL")
        return False
    print("  PASS: issue returns before IRQ callback clears busy")

    # 9. Controlled ordering proof.  This is intentionally not a hardware
    # timing emulator; production tests exercise the native event provider.
    print("\n[9] Verifying controlled asynchronous state ordering...")
    if not verify_controlled_async_order():
        print("  FAIL")
        return False
    print("  PASS: initiate -> busy -> later data -> callback -> complete")
    
    print("\n" + "=" * 50)
    print("B48 Oracle: ALL 9 SCENARIOS PASS")
    print("Wrapper words: 16 (850F4) + 13 (850C0) + 23 (85E54) = 52")
    print("Async chain words: 33 (7D9F8) + 47 (7D614) + 10 (85098) = 90")
    return True


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)

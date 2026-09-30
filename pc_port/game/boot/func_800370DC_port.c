/*
 * Phase 6E-B54I — func_800370DC: primitive add/sort wrapper (sprite twin)
 * (translated retail logic, classification 1).
 *
 * Complete retail body (25 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * file 0x278DC).  Decoded from build/disc1.candidate.exe @ file 0x278DC:
 *
 *   0x800370DC: 0x27BDFFE0  addiu sp, sp, -32
 *   0x800370E0: 0xAFB00010  sw    s0, 16(sp)
 *   0x800370E4: 0x00808021  move  s0, a0            ; s0 = packet
 *   0x800370E8: 0x00A03821  move  a3, a1            ; a3 = tpage mode
 *   0x800370EC: 0x00002821  move  a1, zero          ; a1 = 0
 *   0x800370F0: 0x24060001  li    a2, 1             ; a2 = 1
 *   0x800370F4: 0xAFBF0018  sw    ra, 24(sp)
 *   0x800370F8: 0x0C01DF21  jal   0x80077C84        ; draw-mode word
 *   0x800370FC: 0xAFB10014  sw    s1, 20(sp)        ; delay slot
 *   0x80037100: 0x26110008  addiu s1, s0, 8         ; s1 = packet + 8
 *   0x80037104: 0x0C01DF01  jal   0x80077C04        ; setSprt header
 *   0x80037108: 0x02202021  move  a0, s1            ; delay slot
 *   0x8003710C: 0x02002021  move  a0, s0
 *   0x80037110: 0x0C01DF2D  jal   0x80077CB4        ; length-budget append
 *   0x80037114: 0x02202821  move  a1, s1            ; delay slot
 *   0x80037118: 0x10400003  beq   v0, zero, 0x80037128
 *   0x8003711C: 0x00000000  nop
 *   0x80037120: 0x0C01C679  jal   0x800719E4        ; B(38h) fail path
 *   0x80037124: 0x2404FFFF  li    a0, -1            ; delay slot
 *   0x80037128..0x8003713C: epilogue (lw x3, addiu sp, jr ra, nop)
 *
 * Semantics, with p = a0 and mode = a1:
 *   func_80077C84(p, 0, 1, mode);      ; p[3]=1; p+4 = 0xE1000200|(mode&0x9FF)
 *   s = p + 8;
 *   func_80077C04(s);                  ; s[3]=4; s[7]=0x64 (setSprt)
 *   if (func_80077CB4(p, s) != 0)      ; p[3] = 6, *(u32*)s = 0 on success
 *       func_800719E4((uint32_t)-1);   ; B(38h) CD-mode fail path
 *
 * The func_80077C84 return value is not consumed.  Void return.
 *
 * Caller census: 18 jal sites in func_80030894 (words 41/77/97/…/470),
 * first at 0x80030A78 with a1 = s8 (the masked GetTPage result).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_800370DC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800370DC_port.c (src/func_800370DC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

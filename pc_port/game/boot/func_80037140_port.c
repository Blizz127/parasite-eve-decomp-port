/*
 * Phase 6E-B54I — func_80037140: primitive add/sort wrapper (tile twin)
 * (translated retail logic, classification 1).
 *
 * Complete retail body (25 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * file 0x27940).  Decoded from build/disc1.candidate.exe @ file 0x27940:
 *
 *   0x80037140: 0x27BDFFE0  addiu sp, sp, -32
 *   0x80037144: 0xAFB00010  sw    s0, 16(sp)
 *   0x80037148: 0x00808021  move  s0, a0            ; s0 = packet
 *   0x8003714C: 0x00A03821  move  a3, a1            ; a3 = tpage mode
 *   0x80037150: 0x00002821  move  a1, zero          ; a1 = 0
 *   0x80037154: 0x24060001  li    a2, 1             ; a2 = 1
 *   0x80037158: 0xAFBF0018  sw    ra, 24(sp)
 *   0x8003715C: 0x0C01DF21  jal   0x80077C84        ; draw-mode word
 *   0x80037160: 0xAFB10014  sw    s1, 20(sp)        ; delay slot
 *   0x80037164: 0x26110008  addiu s1, s0, 8         ; s1 = packet + 8
 *   0x80037168: 0x0C01DF11  jal   0x80077C44        ; SetTile header
 *   0x8003716C: 0x02202021  move  a0, s1            ; delay slot
 *   0x80037170: 0x02002021  move  a0, s0
 *   0x80037174: 0x0C01DF2D  jal   0x80077CB4        ; length-budget append
 *   0x80037178: 0x02202821  move  a1, s1            ; delay slot
 *   0x8003717C: 0x10400003  beq   v0, zero, 0x8003718C
 *   0x80037180: 0x00000000  nop
 *   0x80037184: 0x0C01C679  jal   0x800719E4        ; B(38h) fail path
 *   0x80037188: 0x2404FFFF  li    a0, -1            ; delay slot
 *   0x8003718C..0x800371A0: epilogue (lw x3, addiu sp, jr ra, nop)
 *
 * Identical to func_800370DC except the second callee is the SetTile
 * header leaf func_80077C44 (length 3, code 0x60), so the successful
 * append computes p[3] = 1 + 3 + 1 = 5.
 *
 * Caller census: 1 jal site in func_80030894 (0x80030AF4), with a1 =
 * v0 & 0xFFFF (masked).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80037140: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80037140_port.c (src/func_80037140.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

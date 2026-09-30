/*
 * Phase 6E-B54I — func_80077A64: GetTPage texture-page value builder
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body (15 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * file 0x68264).  Decoded from build/disc1.candidate.exe @ file 0x68264:
 *
 *   0x80077A64: 0x30820003  andi v0, a0, 0x03
 *   0x80077A68: 0x000211C0  sll  v0, v0, 7
 *   0x80077A6C: 0x30A50003  andi a1, a1, 0x03
 *   0x80077A70: 0x00052940  sll  a1, a1, 5
 *   0x80077A74: 0x00451025  or   v0, v0, a1
 *   0x80077A78: 0x30E30100  andi v1, a3, 0x0100
 *   0x80077A7C: 0x00031903  sra  v1, v1, 4
 *   0x80077A80: 0x00431025  or   v0, v0, v1
 *   0x80077A84: 0x30C603FF  andi a2, a2, 0x03FF
 *   0x80077A88: 0x00063183  sra  v1, a2, 6
 *   0x80077A8C: 0x00461025  or   v0, v0, v1
 *   0x80077A90: 0x30E70200  andi a3, a3, 0x0200
 *   0x80077A94: 0x00073880  sll  a3, a3, 2
 *   0x80077A98: 0x03E00008  jr   ra
 *   0x80077A9C: 0x00471025  or   v0, v0, a3    ; delay slot
 *
 * Psy-Q libgpu origin: GetTPage(tp, abr, x, y), outlined by the retail
 * compiler into a standalone ROM function.  ABI proven from the first
 * jal in func_80030894 (word 8 @ 0x800308EC): a0=0, a1=1, a2=0x100 (256),
 * a3=0x1E0 (480) produces exactly 0x34.
 *
 * return = ((tp & 3) << 7) | ((abr & 3) << 5)
 *        | ((y & 0x100) >> 4)          ; both sra inputs are pre-masked
 *        | ((x & 0x3FF) >> 6)          ; positive, so sra == srl
 *        | ((y & 0x200) << 2);
 *
 * Pure computation: no loads, no stores, no callees.  Retail returns the
 * full 32-bit v0 with no final mask (the delay slot is the last OR); the
 * canonical caller masks itself (`andi s8, v0, 0xffff` at 0x80030900).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077A64: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077A64_port.c (src/func_80077A64.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

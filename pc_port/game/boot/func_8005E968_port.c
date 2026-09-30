/*
 * Phase 6E-B19a — func_8005E968: pack-color halver leaf.
 *
 * Raw body: 8 words / 0x20, exe 0x8005E968–0x8005E987, file 0x4F168,
 * live split asm/disc1/4F0A4.s:80–89; all 8 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * Sole call site in func_8005E588: jal @0x8005E6B4, $a0 = 0x80808080
 * (ori 0x8080,$0 → 0x80808080 in the delay slot).
 *
 * Operation:
 *   1. Mask = 0x007F7F7F
 *   2. sw $a0 → D_8009D110  ($gp + 0x3A0) — store original color
 *   3. $a0 = ($a0 >> 1) & 0x007F7F7F  (arithmetic sra + and mask)
 *   4. sw $a0 → D_8009D114  ($gp + 0x3A4) — store halved color
 *   5. jr $ra; nop
 *
 * The arithmetic shift preserves the sign of each component byte;
 * the mask strips the top 9 bits, zeroing the sign-extended portion
 * for packed 3-byte RGB values.
 *
 * void func_8005E968(uint32_t packed); — return ignored.
 *
 * Classification: 1 — translated retail logic.
 */
#include "psx_compat.h"

#define GA_COLOR_ORIG   (0x8009CD70u + 0x3A0u)  /* D_8009D110 */
#define GA_COLOR_HALF   (0x8009CD70u + 0x3A4u)  /* D_8009D114 */

/* func_8005E968: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005E968_port.c (src/func_8005E968.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

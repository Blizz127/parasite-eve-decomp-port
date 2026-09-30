/*
 * Phase 6E-B19b — func_8005F844: conditional constant-store leaf.
 *
 * Raw body: 12 words / 0x30, exe 0x8005F844–0x8005F873, file 0x50044,
 * live split asm/disc1/4F6D4.s:714–729; all 12 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * Sole call site in func_8005E588: jal @0x8005E6C0, $a0 = 0
 * (addu $a0,$0,$0 in the delay slot).
 *
 * Operation (ROM order):
 *   1. if (a0 != 0) $v0 = 0x3A1C  else $v0 = 0x395D
 *      sw $v0 → D_8009D13C  ($gp + 0x3CC)
 *   2. if (a0 != 0) $v0 = 0xCC    else $v0 = 0x84
 *      sw $v0 → D_8009D140  ($gp + 0x3D0)
 *   3. $v0 = 0xA4
 *      sw $v0 → D_8009D144  ($gp + 0x3D4)
 *   4. jr $ra; nop
 *
 * With a0=0 (the dispatcher call arg): stores 0x395D, 0x84, 0xA4.
 *
 * void func_8005F844(int a0); — return ignored.
 *
 * Classification: 1 — translated retail logic.
 */
#include "psx_compat.h"

#define GA_F844_A   (0x8009CD70u + 0x3CCu)   /* D_8009D13C */
#define GA_F844_B   (0x8009CD70u + 0x3D0u)   /* D_8009D140 */
#define GA_F844_C   (0x8009CD70u + 0x3D4u)   /* D_8009D144 */

/* func_8005F844: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005F844_port.c (src/func_8005F844.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

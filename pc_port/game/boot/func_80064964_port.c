/*
 * Phase 6E-B21 — func_80064964: bzero + flag initialization.
 *
 * Raw body: 27 words / 0x6C, exe 0x80064964–0x800649CF, file 0x55164,
 * live split asm/disc1/539C0.s:1743–1771; all 27 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * Sole call site: func_800527C8 @0x8005280C (nop delay slot, no args,
 * return ignored).  Now the B20 strict frontier.
 *
 * Operation (ROM order):
 *   1. Call func_80071A24(0x800A3060, 0x120) — BIOS A(28h) bzero trampoline.
 *      This is a 3-word Psy-Q kernel entry stub (addiu $t2,0xA0;
 *      jr $t2; addiu $t1,0x28), adapted as a checked guest-memory fill.
 *   2. After the kernel call returns, set 8 byte flags to 0xFF (-1):
 *      D_800A3078, D_800A30A0, D_800A30B0, D_800A30B8,
 *      D_800A30C0, D_800A30C4, D_800A3124, D_800A3134.
 *
 * No arguments.  No return value (jr $ra, nop delay, no $v0 set).
 * The -1 sentinel at the end of the $v0 register ($v0 = 0xFFFFFFFF
 * from addiu $v0,$zero,-1) is never consumed — it's a scratch value
 * for the subsequent sb operations (only the low byte matters).
 *
 * Classification: 1 — translated retail logic with one unresolved SDK
 * dependency (func_80071A24, BIOS A(28h) bzero trampoline).
 */
#include "psx_compat.h"

/* func_80064964: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80064964_port.c (src/func_80064964.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

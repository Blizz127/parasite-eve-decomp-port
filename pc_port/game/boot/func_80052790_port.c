/*
 * Phase 6E-B36 — func_80052790: boolean-state store + notify wrapper.
 *
 * Retail body: 9 instructions / 0x24 bytes,
 * 0x80052790..0x800527B0 (exclusive end 0x800527B4), file offset
 * 0x42F90, live split [0x42F90, c].  All 9 words exe-verified against
 * SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Stores the argument at D_8009D020 ($gp+0x2B0), converts it to a
 * boolean (a0 < 1 → 1, else 0) in the jal delay slot, and calls
 * func_80086728(bool).
 * Stack frame: addiu $sp,-0x18 / sw $ra,0x10($sp) / lw $ra,0x10($sp) /
 * addiu $sp,+0x18 / jr $ra / nop.
 *
 * func_80086728 is TRANSLATED (Phase 6E-B38).
 * No return value is consumed (void wrapper).
 *
 * Executable call sites (3):
 *   func_8004AF38 @ 0x8004AFEC  a0=func_80063428 ret; addu slot
 *   func_8005C310 @ 0x8005C438  a0=lb(0x800C0DFF)&3; andi slot
 *   func_8005D6F4 @ 0x8005D910  a0=1; addiu slot
 *
 * Classification: 1 — translated retail logic (thin wrapper).
 */
#include "psx_compat.h"

#define GA_52790_STATE 0x8009D020u  /* $gp + 0x2B0 */

extern void func_80086728(int a0);

/* func_80052790: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052790_port.c (src/func_80052790.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

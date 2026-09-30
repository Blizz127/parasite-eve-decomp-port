/*
 * Phase 6E-B34 — func_8005E850: alarm-timer setter wrapper.
 *
 * Retail body: 13 instructions / 0x34 bytes,
 * 0x8005E850..0x8005E880 (exclusive end 0x8005E884), file offset
 * 0x4F050, live split [0x4F050, c].  All 13 words exe-verified against
 * SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Non-leaf void wrapper: reads D_800B0DB0 (signed byte, always 0 —
 * never written anywhere in the retail executable) and D_800B0DB1
 * (signed byte, alarm timer value set by func_8006A2E8), adds each to
 * the corresponding argument, and calls func_8006A2E8(a0 + D_800B0DB0,
 * a1 + D_800B0DB1).  Stack frame: addiu $sp,-0x18 / sw $ra,0x10($sp) /
 * lw $ra,0x10($sp) / addiu $sp,+0x18 / jr $ra / nop.
 *
 * func_8006A2E8 is now REAL (B37).  Its return value is discarded by
 * all callers of func_8005E850.
 *
 * Executable call sites (4):
 *   func_8004B61C @ 0x8004B684  a0=0, a1=1; nop slot; ret discarded
 *   func_8004B674 @ 0x8004B6DC  a0=0, a1=lw(gp+0x274)-r; subu slot; ret discarded
 *   func_8005C310 @ 0x8005C428  a0=0, a1=lb(0x800C0DFF)-r; subu slot; ret discarded
 *   func_8005D6F4 @ 0x8005D900  a0=0, a1=8-r; subu slot; ret discarded
 *
 * During boot D_800B0DB0 = 0 (unwritten BSS) and D_800B0DB1 = 0
 * (unwritten BSS until func_8006A2E8 stores to it), so the call
 * degenerates to func_8006A2E8(a0, a1).
 *
 * Classification: 1 — translated retail logic (all callees now REAL).
 */
#include "psx_compat.h"



/* func_8005E850: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005E850_port.c (src/func_8005E850.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

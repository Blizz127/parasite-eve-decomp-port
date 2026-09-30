/*
 * Phase 6E-B33 — func_8005E884: signed-byte alarm-timer query leaf.
 *
 * Retail body: 4 instructions / 0x10 bytes,
 * 0x8005E884..0x8005E890 (exclusive end 0x8005E894), file offset
 * 0x4F084, live split [0x4F084, c].  All 4 words exe-verified against
 * SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Loads the signed byte at D_800B0DB1 and returns it.  No callees,
 * no guest writes, no SDK/GPU/disc/audio/input activity.  The sole
 * writer to D_800B0DB1 is func_8006A2E8 (a value-validated setter:
 * stores only when a1 < 16).  During boot D_800B0DB1 is unwritten
 * BSS, so the retail return is 0.
 *
 * Executable call sites:
 *   func_8005D6F4 @ 0x8005D8F0  → r; func_8005E850(0, 8-r)
 *   func_8004B5A4 @ 0x8004B5C0  → sw $v0,628($gp); epilogue
 *   func_8004B5DC @ 0x8004B5F0  → subu a0,a0,v0; func_8005FCAC(8-r)
 *   func_8004B6CC @ 0x8004B6CC  → lw $gp+0x274; subu a1,a1,v0;
 *                                  func_8005E850(0, stored-r)
 *   func_8005C150 @ 0x8005C300  → sb $v0,0x800C0DFF
 *   func_8005C310 @ 0x8005C414  → lb a1,0x800C0DFF; subu a1,a1,v0;
 *                                  func_8005E850(0, stored-r)
 *
 * Classification: 1 — translated retail logic (trivial leaf).
 */
#include "psx_compat.h"

/* func_8005E884: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005E884_port.c (src/func_8005E884.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

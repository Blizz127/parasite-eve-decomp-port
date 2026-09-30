/*
 * Phase 6E-B37 — func_8006A2E8: conditional alarm-timer setter.
 *
 * Retail body: 5 instructions / 0x14 bytes,
 * 0x8006A2E8..0x8006A2F8 (exclusive end 0x8006A2FC), file offset
 * 0x5AAE8.  All 5 words exe-verified against SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * If a1 < 16: stores a1 (as halfword) at 0x800BCE9E and 0x800BCE8A,
 * stores a1 (as signed byte) at D_800B0DB1, and returns a1.
 * If a1 >= 16: returns 0 without writing.
 *
 * The delay slot of the caller (func_8005E850 @ 0x8005E86C) is
 * `addu $a1, $v1, $a1` which forms the effective second argument.
 *
 * Sole call site: func_8005E850 @ 0x8005E86C (delay slot:
 * addu $a1,$v1,$a1; return discarded by all callers of 5E850).
 *
 * Classification: 1 — translated retail logic (conditional leaf).
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"

#define GA_A2E8_HW1  0x800BCE9Eu   /* sh $v0 target 1 */
#define GA_A2E8_HW2  0x800BCE8Au   /* sh $v0 target 2 */
#define GA_A2E8_SB   0x800B0DB1u   /* sb $v0 target 3 */

/* func_8006A2E8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006A2E8_port.c (src/func_8006A2E8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

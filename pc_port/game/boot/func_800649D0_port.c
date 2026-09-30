/*
 * Phase 6E-B35 — func_800649D0: resource-state reset/initializer.
 *
 * Retail body: 30 instructions / 0x78 bytes,
 * 0x800649D0..0x80064A44 (exclusive end 0x80064A48), file offset
 * 0x551D0, live split [0x551D0, c].  All 30 words exe-verified against
 * SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Stores the argument at D_8009D16C ($gp+0x3FC).  If a0 is zero:
 * bzero(0x800A3060, 0x120) via REAL func_80071A24, then stores 0xFF
 * (as sb) to eight specific bytes: 0x800A3078, 0x800A30A0, 0x800A30B0,
 * 0x800A30B8, 0x800A30C0, 0x800A30C4, 0x800A3124, 0x800A3134.
 * If a0 is non-zero: returns immediately after the store.
 *
 * No other callees, no SDK/GPU/disc/audio/input activity.  Void return.
 *
 * Executable call sites (3):
 *   func_8004AF38 @ 0x8004B0EC  a0=func_80063428 ret; addu slot; ret discarded
 *   func_8005C310 @ 0x8005C450  a0=lb(0x800C0DFF)&1; andi slot; ret discarded
 *   func_8005D6F4 @ 0x8005D908  a0=0; addu slot; ret discarded
 *
 * Classification: 1 — translated retail logic.
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"

extern pe_addr_t func_80071A24(pe_addr_t dst, uint32_t len);

#define GA_649D0_STATE   0x8009D16Cu  /* $gp + 0x3FC */
#define GA_649D0_BASE    0x800A3060u
#define GA_649D0_SIZE    0x120u
#define GA_649D0_BYTE0   0x800A3078u
#define GA_649D0_BYTE1   0x800A30A0u
#define GA_649D0_BYTE2   0x800A30B0u
#define GA_649D0_BYTE3   0x800A30B8u
#define GA_649D0_BYTE4   0x800A30C0u
#define GA_649D0_BYTE5   0x800A30C4u
#define GA_649D0_BYTE6   0x800A3124u
#define GA_649D0_BYTE7   0x800A3134u

/* func_800649D0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800649D0_port.c (src/func_800649D0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

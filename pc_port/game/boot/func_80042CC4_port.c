/*
 * Phase 6E-B31 — func_80042CC4: retail byte-ramp initializer.
 *
 * Retail body: 31 instructions / 0x7C bytes,
 * 0x80042CC4..0x80042D3C (exclusive end 0x80042D40), file offset
 * 0x334C4, live split asm/disc1/334C4.s.  The independent B31 oracle
 * transcribes and verifies every word before executing its own model.
 *
 * The function is a leaf.  It clears the first byte of D_800A1878, then
 * repeatedly interpolates the next byte from the current byte while the
 * signed byte/threshold comparison succeeds.  The final byte count is
 * written to $gp+0x170 (D_8009CEE0).  The caller's a2/a3 are overwritten
 * before either is used; no host pointer or hardware provider is involved.
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"

#define GA_42CC4_TABLE 0x800A1878u
#define GA_42CC4_COUNT 0x8009CEE0u /* retail gp 0x8009CD70 + 0x170 */

/* func_80042CC4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042CC4_port.c (src/func_80042CC4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

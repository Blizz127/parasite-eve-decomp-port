/*
 * Phase 6E-B17 — func_8005BC98: $gp-relative flag setter.
 *
 * Raw body: 4 words / 0x10, exe 0x8005BC98–0x8005BCA7, file 0x4C498;
 * the matching decomp C leaf (src/func_8005BC98.c, Phase 5DD) replaced
 * the asm split, so the body was decoded directly from the SHA-exact
 * retail executable and all 4 words verified:
 *   addiu $v0, $zero, 1
 *   sw $v0, 0x4A8($gp)   → D_8009D218 (retail $gp 0x8009CD70)
 *   jr $ra; nop
 *
 * The leaf IGNORES its argument register: func_800527C8 calls it twice
 * with a0=0 and a0=1 (both delay-slot setups are dead), and both calls
 * store 1.  v0=1 is set but never consumed at either call site.
 * Idempotent; PE_RamReset clears the destination.
 *
 * Classification: 1 — translated retail logic (matching decomp leaf).
 */
#include "psx_compat.h"

/* func_8005BC98: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005BC98_port.c (src/func_8005BC98.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

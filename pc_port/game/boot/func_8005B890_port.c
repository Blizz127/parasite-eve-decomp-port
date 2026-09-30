/*
 * Phase 6E-B17 — func_8005B890: $gp-relative word setter.
 *
 * Raw body: 3 words / 0xC, exe 0x8005B890–0x8005B89B, file 0x4C090;
 * the matching decomp C leaf (src/func_8005B890.c, Phase 5DC) replaced
 * the asm split, so the body was decoded directly from the SHA-exact
 * retail executable and all 3 words verified:
 *   sw $a0, 0x2B8($gp)   → D_8009D028 (retail $gp 0x8009CD70)
 *   jr $ra; nop
 *
 * Sole caller in func_800527C8 passes a0=0 (delay-slot addu).  No v0
 * assignment → void.  Idempotent; safe to repeat; PE_RamReset clears
 * the destination.
 *
 * Classification: 1 — translated retail logic (matching decomp leaf).
 */
#include "psx_compat.h"

/* func_8005B890: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005B890_port.c (src/func_8005B890.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

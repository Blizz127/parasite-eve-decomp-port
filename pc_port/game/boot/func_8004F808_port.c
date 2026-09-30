/*
 * Phase 6E-B17 — func_8004F808: ten-word $gp-relative clear.
 *
 * Raw body: 12 words / 0x30, exe 0x8004F808–0x8004F837, file 0x40008;
 * the matching decomp C leaf (src/func_8004F808.c, Phase 5DH) replaced
 * the asm split, so the body was decoded directly from the SHA-exact
 * retail executable and all 12 words verified.  Ten sw $zero stores in
 * exact ROM order (retail $gp 0x8009CD70), then jr $ra / nop:
 *   0x8009CF1C, 0x8009CF30, 0x8009CF3C, 0x8009CEFC, 0x8009CF00,
 *   0x8009CF0C, 0x8009CF34, 0x8009CF38, 0x8009CFB0, 0x8009CFF8
 *
 * No reads, no callees, void.  Idempotent incl. after PE_RamReset.
 *
 * Classification: 1 — translated retail logic (matching decomp leaf).
 */
#include "psx_compat.h"

/* func_8004F808: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004F808_port.c (src/func_8004F808.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

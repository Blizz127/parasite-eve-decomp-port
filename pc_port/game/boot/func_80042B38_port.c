/*
 * Phase 6E-B17 — func_80042B38: callback-slot dual clear.
 *
 * Raw body: 6 words / 0x18, exe 0x80042B38–0x80042B4F, file 0x33338,
 * live split asm/disc1/33338.s; all 6 words verified against the
 * SHA-exact retail executable:
 *   sw $zero → D_800A1870   (function-pointer slot — the adjacent
 *                            setter func_80042B50 stores the pointer
 *                            and readers dispatch with jalr)
 *   sw $zero → D_800A1874   (invocation counter)
 *   jr $ra; nop
 *
 * Matching decomp C leaf (src/func_80042B38.c, Phase 5EI).  No reads,
 * no callees, void.  Idempotent incl. after PE_RamReset.  The cleared
 * slot holds a GUEST callback identity, never a host pointer.
 *
 * Classification: 1 — translated retail logic (matching decomp leaf).
 */
#include "psx_compat.h"

/* func_80042B38: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042B38_port.c (src/func_80042B38.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * Phase 6E-B54K-Q — complete func_80042538 state reset.
 *
 * Retail 0x80042538..0x800425DC clears the 0x830-byte record block at
 * D_800A0ED4, writes the two -1 sentinels, and clears thirteen scalar
 * owners.  Classification: complete translated retail function.
 */
#include "psx_compat.h"

/* func_80042538: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042538_port.c (src/func_80042538.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

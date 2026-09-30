/*
 * Phase 6E-A — func_8003E944: save-manager bring-up (boot path).
 *
 * ROM: asm/disc1/2EF54.s (12 words): two calls, no frame.
 *   func_800844E4(&D_800BE9A0, &D_800BE9A0 + 0x22)
 *   func_80082534()
 * Classification: 1 (translated game logic); callees real in pe_save.c.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

/* func_8003E944: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003E944_port.c (src/func_8003E944.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

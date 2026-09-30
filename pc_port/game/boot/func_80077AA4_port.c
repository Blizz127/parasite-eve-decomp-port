/*
 * Phase 6E-B54I — func_80077AA4: CLUT value builder (GetClut variant)
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body (6 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * file 0x682A4).  Decoded from build/disc1.candidate.exe @ file 0x682A4:
 *
 *   0x80077AA4: 0x00051180  sll  v0, a1, 6
 *   0x80077AA8: 0x00042103  sra  a0, a0, 4
 *   0x80077AAC: 0x3084003F  andi a0, a0, 0x3F
 *   0x80077AB0: 0x00441025  or   v0, v0, a0
 *   0x80077AB4: 0x03E00008  jr   ra
 *   0x80077AB8: 0x3042FFFF  andi v0, v0, 0xFFFF  ; delay slot
 *
 * NOTE the operand order: the sra writes $a0 (NOT $v0), so the (y<<6)
 * term survives.  This game's variant is ((y << 6) | ((x >>a 4) & 0x3F))
 * & 0xFFFF — Psy-Q libgpu's GetClut(x,y) is (x & 0x3F) | (y << 6); this
 * binary instead folds the CLUT x through a signed >>4 first.
 *
 * ABI proven from the second jal in func_80030894 (word 20 @ 0x800308FC):
 * a0=0x130 (304), a1=0x1F8 (504) produces exactly 0x7E13, which the
 * caller stores at sp+0x20 (`sh v0,32(sp)` at 0x80030908).
 *
 * Pure computation: no loads, no stores, no callees.  The sra is
 * arithmetic on the signed incoming x.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077AA4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077AA4_port.c (src/func_80077AA4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

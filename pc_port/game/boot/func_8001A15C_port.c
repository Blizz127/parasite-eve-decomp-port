/*
 * PE-BTL28 — opcode 0xD9 (1A15C) and ratan2 79FB4.
 * Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_8001A15C — 19 words 0x8001A15C..0x8001A1A8, SHA-256
 * f53fb5e6…9760. D_800910A0[0xD9].
 * *arg2 = 79FB4(*arg0, *arg1); v0=1.
 *
 * func_80079FB4 — 93 words 0x80079FB4..0x8007A128, SHA-256
 * e5b0edc7…f820. Zero jal. Signed ratan2 into
 * D_8009A6EC (4096-circle). Both-zero returns 0.
 * Live type-5 after 0x0C: ratan2(localE-localB, localD-localA).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009A6EC 0x8009A6ECu

/* func_80079FB4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80079FB4_port.c (src/func_80079FB4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 7). */

/* func_8001A15C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001A15C_port.c (src/func_8001A15C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8001A214: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001A214_port.c (src/func_8001A214.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

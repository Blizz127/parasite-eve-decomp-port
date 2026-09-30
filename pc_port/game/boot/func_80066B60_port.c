/*
 * PE-BTL41 — func_80066B60 fade-in (translated retail, not
 * matching src/). Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 30 words 0x80066B60..0x80066BD8, SHA-256 862ff618…e45e.
 * Zero jal. v0=0.
 *
 * Callers: 18EB4 / opcode 0x85 (type-3 HIT imm 0x1E) and
 * 6E9A0 @ 0x8006EB20 (boot, a0=2). Snapshot CFE8/EA/EC,
 * write 0xFF into those three, sb 2→CFEE and CFEF,
 * CFF6=a0, CFF8=0, CFF0/F2/F4=snapshot.
 *
 * Do not invent a type-3 region hit. 6E9A0 is the live
 * caller; 68E24 needs 6A8D4 B0E38 before the OT link.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BCFE8 0x800BCFE8u
#define GA_D_800BCFEA 0x800BCFEAu
#define GA_D_800BCFEC 0x800BCFECu
#define GA_D_800BCFEE 0x800BCFEEu
#define GA_D_800BCFEF 0x800BCFEFu
#define GA_D_800BCFF0 0x800BCFF0u
#define GA_D_800BCFF2 0x800BCFF2u
#define GA_D_800BCFF4 0x800BCFF4u
#define GA_D_800BCFF6 0x800BCFF6u
#define GA_D_800BCFF8 0x800BCFF8u

/* func_80066B60: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80066B60_port.c (src/func_80066B60.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80066BD8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80066BD8_port.c (src/func_80066BD8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

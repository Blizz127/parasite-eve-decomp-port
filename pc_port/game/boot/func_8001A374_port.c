/*
 * PE-BTL21 — live type-1 0xE1 / 0x84 / 0x88 (translated retail,
 * not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_8001A374 — 7 words 0x8001A374..0x8001A390. sb *arg0 →
 * D_800BCFFC. v0=1. Live imm 0x54.
 *
 * func_80018E84 — 12 words 0x80018E84..0x80018EB4. sh *arg0 →
 * D_800BD020, sh *arg1 → D_800BD022. v0=1. Live 0x800, 0x800.
 *
 * func_80018F54 — 8 words 0x80018F54..0x80018F74.
 * D_800BCFEE &= ~0x40. v0=1.
 *
 * After these, type-1 0x14/0x02 are already ported and yield.
 * Next visit is 0x08/1735C (type-0 spawn) — not this cut.
 *
 * PE-BTL31 — opcode 0x86 / 18EE0 / 66C7C.
 * 18EE0 11 words 0x80018EE0..0x80018F0C, SHA-256 d042e2ad…0c.
 * D_800910A0[0x86]. jal 66C7C(lhu *arg0); v0=1.
 * Live type-1 persist[0x4A]!=39: imm 0x1E.
 *
 * 66C7C 27 words 0x80066C7C..0x80066CE8, SHA-256 ce26b53b…ef0.
 * Zero jal. v0=0. Snapshot CFE8/EA/EC, sb 6→CFEE, zero those
 * three, CFF6=a0, CFF8=0, CFF0/F2/F4=snapshot.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BCFE8 0x800BCFE8u
#define GA_D_800BCFEA 0x800BCFEAu
#define GA_D_800BCFEC 0x800BCFECu
#define GA_D_800BCFEE 0x800BCFEEu
#define GA_D_800BCFF0 0x800BCFF0u
#define GA_D_800BCFF2 0x800BCFF2u
#define GA_D_800BCFF4 0x800BCFF4u
#define GA_D_800BCFF6 0x800BCFF6u
#define GA_D_800BCFF8 0x800BCFF8u

/* func_8001A374: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001A374_port.c (src/func_8001A374.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8001A390: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001A390_port.c (src/func_8001A390.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80018E84: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018E84_port.c (src/func_80018E84.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80018F54: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018F54_port.c (src/func_80018F54.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80066C7C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80066C7C_port.c (src/func_80066C7C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80018EE0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018EE0_port.c (src/func_80018EE0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80019618: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019618_port.c (src/func_80019618.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

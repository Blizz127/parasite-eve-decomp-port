/*
 * Phase 6E-B5 — func_80036DC8: timer-record initialization dispatcher.
 * TRANSLATED retail initialization logic (category 1), with its three
 * direct callees — complete audited leaves, not unresolved dependencies.
 *
 * Raw audit (asm/disc1/26C48.s:677-738 — the ONLY split containing these
 * symbols; live per configs/USA/disc1.yaml [0x26C48, asm]):
 *
 *   func_80036DC8  exe 0x80036DC8-0x80036DF7  10 words / 0x28  off 0x275C8
 *     prologue, jal func_80036DF8, jal func_80036E34, jal func_80036E58
 *     (nop delay slots), epilogue.  No args, no return, no other effects.
 *   func_80036DF8  exe 0x80036DF8-0x80036E33  15 words / 0x3C  off 0x275F8
 *   func_80036E34  exe 0x80036E34-0x80036E57   9 words / 0x24  off 0x27634
 *   func_80036E58  exe 0x80036E58-0x80036E7B   9 words / 0x24  off 0x27658
 *
 * ROM-order store map (11 word stores; the two zero-stores immediately
 * overwritten by the same leaf are RETAIL behavior, reproduced verbatim):
 *   0x80036E04  D_800A76A8 = 0
 *   0x80036E0C  D_800A76A8 = 0x1499700   (21,600,000)
 *   0x80036E18  D_800A76A4 = 0
 *   0x80036E20  D_800A76A0 = 0
 *   0x80036E28  D_800A76A0 = 1
 *   0x80036E3C  D_800A76BC = 0
 *   0x80036E44  D_800A76C0 = 0
 *   0x80036E4C  D_800A76B8 = 1
 *   0x80036E60  D_800A76B0 = 0
 *   0x80036E64  D_800A76B4 = 0
 *   0x80036E70  D_800A76AC = 1
 *
 * Net state: three 12-byte records at 0x800A76A0/AC/B8, each
 * { field0 = 1, field1 = 0, field2 = x }, with record 0 field2 preloaded
 * to 21,600,000.  Consumer evidence (not needed for the contract, cited
 * for subsystem role only): func_80019DB8 and func_80052894 read
 * field1[i] via base + i*12; func_80052894 returns field1[i]/60
 * (unsigned /60 magic 0x88888889, mfhi >> 5); func_800528C4 writes
 * field1[i]; the following function func_80036E7C packs field1-derived
 * /60 quotients into bitfields.  Consistent with 60 Hz tick counters;
 * any stronger subsystem name would be speculation.
 *
 * Call-site audit: func_80036DC8 has exactly ONE distinct call site
 * (func_8003E680 @ 0x8003E6E0-ish startup path; the jal appears in the
 * overlapping stale splits 2E7D0.s/2EE80.s — counted once).  The three
 * leaves are called ONLY from func_80036DC8 (one site each, live
 * 26C48.s).  No arguments, no return value consumed anywhere.
 *
 * No SDK/Psy-Q calls, no GTE/coprocessor ops, no hardware registers, no
 * loops, no callbacks, no $gp-relative access, no deeper calls of any
 * kind.  Idempotent: absolute stores replay the same values; safe on
 * repeated invocation and after PE_RamReset.
 *
 * All state is guest-RAM-backed via PE_StoreU32 — no host copies.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_36DC8_REC0  0x800A76A0u   /* record 0: +0 flag, +4 counter, +8 preload */
#define GA_36DC8_REC1  0x800A76ACu   /* record 1 */
#define GA_36DC8_REC2  0x800A76B8u   /* record 2 */

/* func_80036DF8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80036DF8_port.c (src/func_80036DF8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80036E34: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80036E34_port.c (src/func_80036E34.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80036E58: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80036E58_port.c (src/func_80036E58.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80036DC8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80036DC8_port.c (src/func_80036DC8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

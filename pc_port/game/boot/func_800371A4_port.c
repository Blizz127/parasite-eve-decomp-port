/*
 * Phase 6E-B7 — func_800371A4: $gp-relative byte setter (translated retail
 * logic, classification 1).
 *
 * Complete retail body (3 words, verified against the SHA-1-exact retail
 * executable; live split configs/USA/disc1.yaml [0x279A4, c] — already a
 * matching C leaf in the decomp):
 *
 *   0x800371A4: 0xA3840124   sb   $a0, 0x124($gp)
 *   0x800371A8: 0x03E00008   jr   $ra
 *   0x800371AC: 0x00000000   nop  (delay slot)
 *
 * Retail $gp = 0x8009CD70, so the destination is 0x8009CD70 + 0x124 =
 * 0x8009CE94 (D_8009CE94).  sb stores the low 8 bits of a0 regardless of
 * signedness; $v0 is untouched (callers treat the function as void).
 *
 * Exactly two distinct exe-wide call sites:
 *   func_8003E680  @0x8003E6F8: func_800371A4(0)  — boot-time clear
 *   func_800527C8  @0x80052874: func_800371A4(1)  — later init path
 *     (delay slot sw completes D_800B0CD8 |= 0x40000000; return unused)
 * Sole reader found: func_80037870 @0x80038704 (lbu, equality compare) —
 * not on the boot path.  First-call and repeated-call behavior identical;
 * trivially idempotent.  No reads, no calls, no hardware, no GTE.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009CE94 0x8009CE94u

/* func_800371A4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800371A4_port.c (src/func_800371A4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

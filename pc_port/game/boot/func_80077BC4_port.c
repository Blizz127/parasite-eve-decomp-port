/*
 * Phase 6E-PE-GPU1 — func_80077BC4: SetPolyG4 GPU packet-header setter
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body (5 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * live split configs/USA/disc1.yaml [0x683C4, c, func_80077BC4] — already a
 * matching C leaf in the decomp since Phase 5CE).  Decoded from
 * build/disc1.candidate.exe @ file 0x683C4:
 *
 *   0x80077BC4: 0x24020008   addiu $v0, $zero, 8
 *   0x80077BC8: 0xA0820003   sb    $v0, 3($a0)        ; *arg0[3] = 8
 *   0x80077BCC: 0x24020038   addiu $v0, $zero, 0x38
 *   0x80077BD0: 0x03E00008   jr    $ra
 *   0x80077BD4: 0xA0820007   sb    $v0, 7($a0)        ; *arg0[7] = 0x38
 *
 * Matching C (src/func_80077BC4.c):
 *   void func_80077BC4(unsigned char *arg0) {
 *       arg0[3] = 8;
 *       arg0[7] = 56;
 *   }
 *
 * Psy-Q libgpu origin: the `setPolyG4(p)` macro (setlen 8 + code 0x38),
 * outlined into a standalone ROM function, called via jal from
 * func_80030894 (words 185/298/302 @ 0x80030B78/0x80030D3C/0x80030D4C).
 * REAL function (own `jr $ra`), not a pure inline expansion.
 *
 * ABI: void func_80077BC4(pe_addr_t p).  Writes exactly byte offset 3 = 8
 * and byte offset 7 = 56; no other effects.  Idempotent; order preserved.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077BC4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077BC4_port.c (src/func_80077BC4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

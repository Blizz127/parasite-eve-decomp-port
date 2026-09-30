/*
 * Phase 6E-PE-GPU1 — func_80077C44: SetTile GPU packet-header setter
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body (5 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * live split configs/USA/disc1.yaml [0x68444, c, func_80077C44] — already a
 * matching C leaf in the decomp since Phase 5CI).  Decoded from
 * build/disc1.candidate.exe @ file 0x68444:
 *
 *   0x80077C44: 0x24020003   addiu $v0, $zero, 3
 *   0x80077C48: 0xA0820003   sb    $v0, 3($a0)        ; *arg0[3] = 3
 *   0x80077C4C: 0x24020060   addiu $v0, $zero, 0x60
 *   0x80077C50: 0x03E00008   jr    $ra
 *   0x80077C54: 0xA0820007   sb    $v0, 7($a0)        ; *arg0[7] = 0x60
 *
 * Matching C (src/func_80077C44.c):
 *   void func_80077C44(unsigned char *arg0) {
 *       arg0[3] = 3;
 *       arg0[7] = 96;
 *   }
 *
 * Psy-Q libgpu origin: the `setTile(p)` macro (setlen 3 + code 0x60),
 * outlined into a standalone ROM function, called via jal from
 * func_80030894 (words 167/424 @ 0x80030B30/0x80030F34).  REAL function
 * (own `jr $ra`), not a pure inline expansion.
 *
 * ABI: void func_80077C44(pe_addr_t p).  Writes exactly byte offset 3 = 3
 * and byte offset 7 = 96; no other effects.  Idempotent; order preserved.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077C44: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077C44_port.c (src/func_80077C44.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

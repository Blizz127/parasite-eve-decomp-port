/*
 * Phase 6E-PE-GPU1 — func_80077BA4: SetPolyFT4 GPU packet-header setter
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body (5 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * live split configs/USA/disc1.yaml [0x683A4, c, func_80077BA4] — already a
 * matching C leaf in the decomp since Phase 5CD).  Decoded from
 * build/disc1.candidate.exe @ file 0x683A4:
 *
 *   0x80077BA4: 0x24020009   addiu $v0, $zero, 9
 *   0x80077BA8: 0xA0820003   sb    $v0, 3($a0)        ; *arg0[3] = 9
 *   0x80077BAC: 0x2402002C   addiu $v0, $zero, 0x2C
 *   0x80077BB0: 0x03E00008   jr    $ra
 *   0x80077BB4: 0xA0820007   sb    $v0, 7($a0)        ; *arg0[7] = 0x2C
 *
 * Matching C (src/func_80077BA4.c):
 *   void func_80077BA4(unsigned char *arg0) {
 *       arg0[3] = 9;
 *       arg0[7] = 44;
 *   }
 *
 * Psy-Q libgpu origin: the `setPolyFT4(p)` macro (setlen 9 + code 0x2C),
 * outlined into a standalone ROM function, called via jal from
 * func_80030894 (word 41 @ 0x80030938).  REAL function (own `jr $ra`),
 * not a pure inline expansion.
 *
 * ABI: void func_80077BA4(pe_addr_t p).  Writes exactly byte offset 3 = 9
 * and byte offset 7 = 44; no other effects.  Idempotent; order preserved.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077BA4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077BA4_port.c (src/func_80077BA4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

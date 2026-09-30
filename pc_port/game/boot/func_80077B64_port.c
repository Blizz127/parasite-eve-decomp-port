/*
 * Phase 6E-PE-GPU1 — func_80077B64: SetPolyF3 GPU packet-header setter
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body (5 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * live split configs/USA/disc1.yaml [0x68364, c, func_80077B64] — already a
 * matching C leaf in the decomp since Phase 5CB).  Decoded from
 * build/disc1.candidate.exe @ file 0x68364:
 *
 *   0x80077B64: 0x24020004   addiu $v0, $zero, 4
 *   0x80077B68: 0xA0820003   sb    $v0, 3($a0)        ; *arg0[3] = 4
 *   0x80077B6C: 0x24020020   addiu $v0, $zero, 0x20
 *   0x80077B70: 0x03E00008   jr    $ra
 *   0x80077B74: 0xA0820007   sb    $v0, 7($a0)        ; *arg0[7] = 0x20
 *
 * Matching C (src/func_80077B64.c):
 *   void func_80077B64(unsigned char *arg0) {
 *       arg0[3] = 4;
 *       arg0[7] = 32;
 *   }
 *
 * Psy-Q libgpu origin: the `setPolyF3(p)` macro (setlen 4 + code 0x20),
 * outlined by the retail compiler into a standalone ROM function and called
 * via jal from func_80030894 (boot GPU-primitive builder).  It is therefore
 * a REAL function in this binary, not a pure inline expansion: it owns its
 * own `jr $ra` and a distinct caller jal site (word 483 @ 0x80031020).
 *
 * ABI: void func_80077B64(pe_addr_t p) — p is the guest address of the
 * primitive packet.  Writes exactly two bytes and nothing else: no reads,
 * no other stores, no callees, no hardware.  Idempotent; first/repeated
 * call identical; order preserved (offset 3 then offset 7).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077B64: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077B64_port.c (src/func_80077B64.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

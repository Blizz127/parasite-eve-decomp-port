/*
 * Phase 6E-B54I — func_8005DADC: guest pointer-table lookup
 * (translated retail logic, classification 1).
 *
 * Complete retail body including its return delay slot (8 words, verified
 * against the SHA-1-exact retail executable SLUS_006.62 / SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b; file 0x4E2DC).  Decoded from
 * build/disc1.candidate.exe @ file 0x4E2DC:
 *
 *   0x8005DADC: 0x3C03800B  lui   v1, 0x800B
 *   0x8005DAE0: 0x24638030  addiu v1, v1, -32720  ; v1 = 0x800A8030
 *   0x8005DAE4: 0x8C620000  lw    v0, 0(v1)       ; v0 = *(u32*)0x800A8030
 *   0x8005DAE8: 0x2463FFF8  addiu v1, v1, -8      ; v1 = 0x800A8028
 *   0x8005DAEC: 0x000420C0  sll   a0, a0, 3
 *   0x8005DAF0: 0x00431021  addu  v0, v0, v1
 *   0x8005DAF4: 0x03E00008  jr    ra
 *   0x8005DAF8: 0x00441021  addu  v0, v0, a0      ; delay slot
 *
 * The delay-slot add at 0x8005DAF8 is the final index add and belongs to
 * this call (verified word in the SHA-exact image; the next function
 * begins at 0x8005DAFC with `lui v0, 0x800B`).
 *
 * return = *(uint32_t *)0x800A8030 + 0x800A8028 + (a0 << 3);
 *
 * ABI proven from the func_80030894 call sites (words 31/330 @ 0x80030910
 * with a0=139, and 0x80031394); the caller immediately dereferences the
 * returned guest pointer (`lbu v0, 0(s3)` @ 0x80030940).  0x800A8028 is
 * the streaming material area loaded by func_8006A9E4; D_800A8030 holds
 * a pointer into it.
 *
 * Reads exactly one guest word; writes nothing.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

pe_addr_t func_8005DADC(uint32_t index)
{
    return (pe_addr_t)(PE_LoadU32(0x800A8030u) + 0x800A8028u + (index << 3));
}

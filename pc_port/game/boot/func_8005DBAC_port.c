/*
 * Phase 6E-B28 — func_8005DBAC: clamped table base computation.
 *
 * Raw body: 20 words / 0x50, executable 0x8005DBAC..0x8005DBFB, file
 * offset 0x4E3AC, live split asm/disc1/4CC98.s:1717-1739.  All 20
 * words verified exact against the SHA-exact retail executable (SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * ROM-order operation map (retail $gp = 0x8009CD70, sign-extended
 * lui 0x800B + addiu 0x803C → D_800A803C):
 *   1. $v1 = arg            (addu from $a0)
 *   2. bgez → clamp branch  (if arg < 0, $v1 = 0)
 *   3. slti $v0, $v1, 99   (if arg < 99, keep; else clamp to 98)
 *   4. $a0 = D_800A803C    (lui/addiu, sign-extended)
 *   5. $v0 = clamped << 1   (sll)
 *   6. $v0 = clamped * 3    (addu: $v0 + $v1)
 *   7. $v0 = clamped * 24   (sll by 3)
 *   8. $v1 = D_800A803C - 0x14  (addiu -20)
 *   9. $a0 = *D_800A803C    (lw, dereference pointer)
 *  10. $v0 = (clamped * 24) + (D_800A803C - 0x14)  (addu)
 *  11. return *D_800A803C + $v0  (jr/addu delay slot)
 *
 * Computes: *D_800A803C + (clamped * 24) + (D_800A803C - 0x14)
 * When *D_800A803C == 0 (BSS/unwritten), returns 0x800A8028 + (clamped * 24).
 *
 * Signature: pe_addr_t func_8005DBAC(int arg).  Returns a guest pointer.
 * Leaf function, no stack frame, no callees.
 *
 * Sole call site: func_8005CCA4 @ 0x8005CCE8 (addu $a0,$zero,$zero delay).
 */
#include "psx_compat.h"

#define GA_5DBAC_PTR  0x800A803Cu   /* sign-extended lui 0x800B + addiu 0x803C */

pe_addr_t func_8005DBAC(int arg)
{
    int clamped = arg;
    if (clamped < 0)
        clamped = 0;
    else if (clamped >= 99)
        clamped = 98;

    pe_addr_t ptr_loc = GA_5DBAC_PTR;
    pe_addr_t base_val = PE_LoadU32(ptr_loc);  /* *D_800A803C */
    pe_addr_t const_part = ((pe_addr_t)clamped * 24u) + (ptr_loc - 0x14u);
    return base_val + const_part;
}

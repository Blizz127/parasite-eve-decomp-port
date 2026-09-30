/*
 * Phase 6E-B28 — func_8005DB8C: table base computation.
 *
 * Raw body: 12 words / 0x30, executable 0x8005DB8C..0x8005DBBB, file
 * offset 0x4E38C, live split asm/disc1/4CC98.s:1704-1713.  All 12
 * words verified exact against the SHA-exact retail executable (SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * ROM-order operation map (retail $gp = 0x8009CD70, sign-extended
 * lui 0x800B + addiu 0x8038 → D_800A8038):
 *   1. $v0 = D_800A8038  (lui/addiu, sign-extended)
 *   2. $a0 = idx << 9    (sll)
 *   3. $v1 = D_800A8038 - 0x10  (addiu -16)
 *   4. $v0 = *D_800A8038  (lw, dereference pointer)
 *   5. $a0 = (idx << 9) + (D_800A8038 - 0x10)  (addu)
 *   6. return $v0 + $a0   (jr/addu delay slot)
 *
 * Computes: *D_800A8038 + (idx << 9) + (D_800A8038 - 0x10)
 * When *D_800A8038 == 0 (BSS/unwritten), returns 0x800A8028 + (idx<<9).
 *
 * Signature: pe_addr_t func_8005DB8C(int idx).  Returns a guest pointer.
 * Leaf function, no stack frame, no callees.
 *
 * Sole call site: func_8005CCA4 @ 0x8005CCC8 (addu $a0,$s0,$zero delay).
 */
#include "psx_compat.h"

#define GA_5DB8C_PTR  0x800A8038u   /* sign-extended lui 0x800B + addiu 0x8038 */

pe_addr_t func_8005DB8C(int idx)
{
    pe_addr_t ptr_loc = GA_5DB8C_PTR;
    pe_addr_t base_val = PE_LoadU32(ptr_loc);  /* *D_800A8038 */
    pe_addr_t const_part = ((pe_addr_t)idx << 9) + (ptr_loc - 0x10u);
    return base_val + const_part;
}

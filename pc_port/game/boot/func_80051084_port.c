/*
 * Phase 6E-B17 — func_80051084: pool-cursor address store.
 *
 * Raw body: 5 words / 0x14, exe 0x80051084–0x80051097, file 0x41884;
 * the matching decomp C leaf (src/func_80051084.c, Phase 5DF) replaced
 * the asm split, so the body was decoded directly from the SHA-exact
 * retail executable and all 5 words verified:
 *   lui $v0, 0x800A; addiu $v0, $v0, 0x1AA0   → v0 = 0x800A1AA0
 *   sw $v0, 0x2A4($gp)   → D_8009D014 (retail $gp 0x8009CD70)
 *   jr $ra; nop
 *
 * The stored value is a GUEST address (the D_800A1AA0 array base —
 * the adjacent allocator advances the cursor by 0x24 per allocation);
 * it is kept as pe_addr_t end to end and is never a host pointer.
 * No reads, no callees, void.  Idempotent incl. after PE_RamReset.
 *
 * Classification: 1 — translated retail logic (matching decomp leaf).
 */
#include "psx_compat.h"

void func_80051084(void)
{
    PE_StoreU32(0x8009D014u, 0x800A1AA0u);
}

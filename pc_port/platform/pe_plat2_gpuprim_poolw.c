/* Record-pool worker 0x8007A244, written clean-room from the black-box vector
 * set gpu2c (docs/evidence/native-gpu2-20261126), the port's host GPU/DMA model
 * API and psx-spx; no SDK source, decompiled source, tables or disassembly were
 * read.
 *
 * Derivation (traced to the vectors): both arguments are ignored.  The word
 * 0x800C20C4 is read; the words 0x800BE9EC, 0x800BE9E4, 0x800BE998 and
 * 0x800B89F4 are cleared; the ring-status clear 0x8007C444 is called with
 * (0, the word read); then the word 0x800B0CD0, the halfword 0x800A8018 and
 * the word 0x800A5D54 are cleared.  No value is used. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/gpuprim.h"
#include "pe_plat/cd.h"

#define POOL_COUNT_WORD 0x800C20C4u

void pe_plat_gpuprim_pool_clear(uint32_t record_base, uint32_t record_count)
{
    uint32_t count;
    (void)record_base;
    (void)record_count;
    count = PE_LoadU32(POOL_COUNT_WORD);
    PE_StoreU32(0x800BE9ECu, 0u);
    PE_StoreU32(0x800BE9E4u, 0u);
    PE_StoreU32(0x800BE998u, 0u);
    PE_StoreU32(0x800B89F4u, 0u);
    func_8007C444(0, count);
    PE_StoreU32(0x800B0CD0u, 0u);
    PE_StoreU16(0x800A8018u, 0u);
    PE_StoreU32(0x800A5D54u, 0u);
}

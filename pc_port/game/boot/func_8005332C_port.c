/*
 * Phase 6E-B40 — func_8005332C: resource-ID to record lookup.
 *
 * Retail body: 42 instructions / 0xA8 bytes at
 * 0x8005332C..0x800533D3 (exclusive end 0x800533D4), executable file
 * offset 0x43B2C.  The live split is asm/disc1/43724.s:331-378 selected
 * by configs/USA/disc1.yaml:337.  tools/b40_oracle.py contains the
 * complete executable-verified word transcription and delay-slot model.
 *
 * True ABI:
 *
 *     pe_addr_t func_8005332C(int32_t resource_id)
 *
 * The signed resource ID indexes a signed-halfword indirection table at
 * authoritative D_8009D048, bounded by the signed word D_8009D050.  The
 * halfword partitions into four exact classes:
 *
 *   0x0100..0x017F -> D_800BEEAC + value * 32
 *   0x0001..0x00FF -> func_8005DB44(value - 1), returned unchanged
 *   0x0200..0x0208 -> D_8009DE64 + value * 32
 *   every other value, negative IDs, and IDs >= signed count -> NULL
 *
 * D_8009D048/D_8009D050 are the existing host-side authoritative copies
 * of the retail $gp+0x2D8/$gp+0x2E0 words (retail $gp=0x8009CD70).  The
 * selected table halfword and func_8005DB44's two words at
 * 0x800A8038/0x800A8034 remain checked little-endian guest accesses. This
 * function performs no stores,
 * does not retain a host pointer, does not clamp malformed state, and has no
 * hardware, SDK, polling, callback, or environment-dependent behavior.
 *
 * Classification: 1 — translated retail resource/record lookup logic.
 */
#include "psx_compat.h"

extern pe_addr_t func_8005DB44(unsigned int index);

pe_addr_t func_8005332C(int32_t resource_id)
{
    int32_t entry;
    uint32_t delegated_index;
    uint32_t alternate_test;

    if (resource_id < 0)
        return 0u;
    if (resource_id >= (int32_t)D_8009D050)
        return 0u;

    entry = (int32_t)(int16_t)PE_LoadU16(
        (pe_addr_t)(D_8009D048 + ((uint32_t)resource_id << 1u)));

    if ((uint32_t)(entry - 0x100) < 0x80u)
        return (pe_addr_t)(0x800BEEACu + ((uint32_t)entry << 5u));

    delegated_index = (uint32_t)(entry - 1);
    alternate_test = (uint32_t)(entry - 0x200);
    if (delegated_index < 0xFFu)
        return func_8005DB44(delegated_index);

    if (alternate_test < 9u)
        return (pe_addr_t)(0x8009DE64u + ((uint32_t)entry << 5u));

    return 0u;
}

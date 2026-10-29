/* Native adapters of matched 80039B74 / 80039D24 / 80039ED4.
 * Guest SVECTOR rows occupy 8 bytes in the persistent 1 KiB scratchpad.
 * Padding and undecoded rows retain their previous bytes, as retail.
 * The native SDK rotation replacement consumes those guest addresses.
 * Retail differential with the real rotation backend:
 * scripts/run_native_oracle_harnesses.sh (117 cases).
 */
#include "pe_port_compat.h"
#include "game_port.h"

#define ANIMATION_ANGLES 0x1F800000u

static pe_addr_t translation_tracks(pe_addr_t dest, pe_addr_t clip, short frame,
                                    pe_addr_t *matrices, uint32_t *stride)
{
    pe_addr_t cursor = clip + 12u;
    const uint32_t offset = (uint32_t)(int32_t)frame << 1;
    *matrices = PE_LoadU32(dest + 0x84u);
    PE_StoreU32(dest + 0x58u, *matrices);
    *stride = ((PE_LoadU8(clip + 2u) >> 1) + 1u) << 2;
    for (unsigned axis = 0; axis < 3u; ++axis) {
        const int constant = PE_LoadU16(cursor) != 0;
        const int32_t value = (int16_t)PE_LoadU16(cursor + 2u + (constant ? 0u : offset));
        cursor += constant ? 4u : *stride;
        PE_StoreU32(*matrices + 20u + axis * 4u, (uint32_t)value);
    }
    return cursor;
}

void func_80039D24(pe_addr_t dest, pe_addr_t clip, short frame)
{
    pe_addr_t matrices;
    uint32_t translation_stride;
    pe_addr_t cursor = translation_tracks(dest, clip, frame, &matrices, &translation_stride);
    const uint32_t stride = ((PE_LoadU8(clip + 2u) >> 2) + 1u) << 2;
    unsigned row = 0;
    const unsigned stop_epoch = PE_Port_StopEpoch();
    (void)translation_stride;
    do {
        for (unsigned axis = 0; axis < 3u; ++axis) {
            const int constant = PE_LoadU8(cursor) != 0;
            const uint32_t offset = constant ? 0u : (uint32_t)(int32_t)frame;
            const uint16_t angle = (uint16_t)((uint16_t)PE_LoadU8(cursor + 1u + offset) << 4);
            cursor += constant ? 4u : stride;
            PE_StoreU16(ANIMATION_ANGLES + row * 8u + axis * 2u, angle);
        }
        (void)PE_RotMatrixYXZ_Guest(ANIMATION_ANGLES + row * 8u, matrices);
        if (PE_Port_StopEpoch() != stop_epoch) return;
        matrices += 32u;
        ++row;
        /* Retail reloads the byte after the matrix call; it is not a cached
         * clip joint count. Preserve aliases between clip and output RAM. */
    } while (row <= PE_LoadU8(clip + 1u));
}

void func_80039ED4(pe_addr_t dest, pe_addr_t clip, short frame)
{
    pe_addr_t matrices;
    uint32_t stride;
    pe_addr_t cursor = translation_tracks(dest, clip, frame, &matrices, &stride);
    const uint32_t offset = (uint32_t)(int32_t)frame << 1;
    unsigned row = 0;
    const unsigned stop_epoch = PE_Port_StopEpoch();
    do {
        for (unsigned axis = 0; axis < 3u; ++axis) {
            const int constant = PE_LoadU16(cursor) != 0;
            const uint16_t angle = PE_LoadU16(cursor + 2u + (constant ? 0u : offset));
            cursor += constant ? 4u : stride;
            PE_StoreU16(ANIMATION_ANGLES + row * 8u + axis * 2u, angle);
        }
        (void)PE_RotMatrixYXZ_Guest(ANIMATION_ANGLES + row * 8u, matrices);
        if (PE_Port_StopEpoch() != stop_epoch) return;
        matrices += 32u;
        ++row;
    } while (row <= PE_LoadU8(clip + 1u));
}

void func_80039B74(pe_addr_t dest, pe_addr_t clip, int frame, int unused)
{
    const unsigned stop_epoch = PE_Port_StopEpoch();
    (void)unused; /* existing native ABI; retail consumes only three args */
    if (!clip) return;
    PE_StoreU16(dest + 0x7Cu, PE_LoadU16(clip + 0xAu));
    PE_StoreU16(dest + 0x74u, PE_LoadU16(clip + 4u));
    PE_StoreU16(dest + 0x76u, PE_LoadU16(clip + 6u));
    PE_StoreU16(dest + 0x78u, PE_LoadU16(clip + 8u));
    if ((PE_LoadU8(clip) & 3u) == 2u) func_80039ED4(dest, clip, (short)frame);
    else func_80039D24(dest, clip, (short)frame);
    if (PE_Port_StopEpoch() != stop_epoch) return;
    for (unsigned i = 0; i < 2u; ++i) {
        const pe_addr_t entry = dest + 0xA0u + i * 8u;
        const uint32_t index = PE_LoadU8(entry + 6u);
        if (index == 0u) continue;
        const pe_addr_t angles = ANIMATION_ANGLES | (index << 3);
        for (unsigned axis = 0; axis < 3u; ++axis) {
            if (PE_LoadU8(entry + 7u) & (8u << axis)) {
                PE_StoreU16(angles + axis * 2u, PE_LoadU16(entry + axis * 2u));
            } else if (PE_LoadU8(entry + 7u) & (1u << axis)) {
                PE_StoreU16(angles + axis * 2u,
                           (uint16_t)(PE_LoadU16(angles + axis * 2u) + PE_LoadU16(entry + axis * 2u)));
            }
        }
        (void)PE_RotMatrixYXZ_Guest(angles, PE_LoadU32(dest + 0x58u) + index * 32u);
        if (PE_Port_StopEpoch() != stop_epoch) return;
    }
}

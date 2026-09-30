/*
 * Phase 6E-B41 — func_80053968: materialize an archive record in the
 * 128-entry runtime record table.
 *
 * Retail body: 120 instructions / 0x1E0 bytes at
 * 0x80053968..0x80053B47 (exclusive end 0x80053B48), executable file
 * offset 0x44168.  The live body is asm/disc1/43724.s:805-936, selected
 * by configs/USA/disc1.yaml:337.  tools/b41_oracle.py contains the full
 * executable-verified transcription and delay-slot-aware execution model.
 *
 * True ABI:
 *
 *     pe_addr_t func_80053968(int32_t resource_id)
 *
 * The function finds the first free 32-byte record at 0x800C0EAC and the
 * first zero halfword in the current authoritative resource-ID table.  If
 * either table is full/empty it returns zero without calling a dependency
 * or changing persistent state.  Otherwise it obtains archive record
 * resource_id-1 from the already translated func_8005DB44, copies exactly
 * 32 bytes in retail's two 16-byte load/store groups, installs the primary
 * resource-table state, writes 0x100+record_slot into the selected ID slot,
 * and returns the exact destination guest address.
 *
 * Classification: 1 — translated retail resource-record materialization.
 */
#include "psx_compat.h"

#define B41_RECORD_BASE       0x800C0EACu
#define B41_RECORD_END        0x800C1EACu
#define B41_PRIMARY_ID_BASE   0x800C0E48u
#define B41_PRIMARY_AUX       0x8009D05Cu

extern pe_addr_t func_8005DB44(unsigned int index);
extern unsigned int func_80052F70(void);

static int32_t B41_Sra1(uint32_t value)
{
    return (int32_t)((value >> 1u) |
                     ((value & 0x80000000u) ? 0x80000000u : 0u));
}

static int32_t B41_Sra5(uint32_t value)
{
    return (int32_t)((value >> 5u) |
                     ((value & 0x80000000u) ? 0xF8000000u : 0u));
}

pe_addr_t func_80053968(int32_t resource_id)
{
    pe_addr_t record_cursor = B41_RECORD_BASE;
    pe_addr_t id_cursor;
    pe_addr_t id_end;
    pe_addr_t source;
    pe_addr_t destination;
    int32_t record_slot = -1;
    int32_t id_slot = -1;
    uint32_t w0, w1, w2, w3;

    /* 0x80053974..0x800539DC: first record whose byte zero is zero. */
    if (record_cursor < B41_RECORD_END) {
        while (PE_LoadU8(record_cursor) != 0u) {
            record_cursor += 0x20u;
            if (record_cursor >= B41_RECORD_END)
                break;
        }
    }
    if (record_cursor < B41_RECORD_END) {
        record_slot = B41_Sra5(record_cursor - B41_RECORD_BASE);
    }

    /* 0x800539E0..0x80053A40: first zero signed halfword in the current
     * table.  The end expression and comparisons are exact unsigned
     * 32-bit operations; malformed state is neither clamped nor mirrored. */
    id_cursor = (pe_addr_t)D_8009D048;
    id_end = (pe_addr_t)(id_cursor + ((uint32_t)D_8009D050 << 1u));
    if (id_cursor < id_end) {
        while ((int16_t)PE_LoadU16(id_cursor) != 0) {
            id_cursor += 2u;
            if (id_cursor >= id_end)
                break;
        }

        id_end = (pe_addr_t)(D_8009D048 +
                             ((uint32_t)D_8009D050 << 1u));
        if (id_cursor < id_end) {
            id_slot = B41_Sra1(id_cursor - (pe_addr_t)D_8009D048);
        }
    }

    if (record_slot < 0 || id_slot < 0)
        return 0u;

    destination = (pe_addr_t)(B41_RECORD_BASE +
                              ((uint32_t)record_slot << 5u));
    source = func_8005DB44((uint32_t)resource_id - 1u);

    /* Retail issues four LWL/LWR pairs, four SWL/SWR pairs, then repeats
     * for the second 16 bytes.  Keeping the two groups separate preserves
     * its exact behavior even when source and destination overlap. */
    w0 = PE_LoadU32(source + 0x00u);
    w1 = PE_LoadU32(source + 0x04u);
    w2 = PE_LoadU32(source + 0x08u);
    w3 = PE_LoadU32(source + 0x0Cu);
    PE_StoreU32(destination + 0x00u, w0);
    PE_StoreU32(destination + 0x04u, w1);
    PE_StoreU32(destination + 0x08u, w2);
    PE_StoreU32(destination + 0x0Cu, w3);

    w0 = PE_LoadU32(source + 0x10u);
    w1 = PE_LoadU32(source + 0x14u);
    w2 = PE_LoadU32(source + 0x18u);
    w3 = PE_LoadU32(source + 0x1Cu);
    PE_StoreU32(destination + 0x10u, w0);
    PE_StoreU32(destination + 0x14u, w1);
    PE_StoreU32(destination + 0x18u, w2);
    PE_StoreU32(destination + 0x1Cu, w3);

    /* 0x80053AE8..0x80053B20: exact post-copy state commit order. */
    D_8009D048 = B41_PRIMARY_ID_BASE;
    D_8009D050 = func_80052F70();
    D_8009D058 = B41_PRIMARY_AUX;
    D_8009D064 = 2u;
    PE_StoreU16((pe_addr_t)(D_8009D048 + ((uint32_t)id_slot << 1u)),
                (uint32_t)(record_slot + 0x100));

    return destination;
}

/*
 * Phase 6E-B54K-Z — complete overlay-local func_80191FB8.
 *
 * Retail range [0x80191FB8,0x801922F4), 207 words, SHA-256
 * 1064769b74b7d2c5dedec9cc9abfd7a2fd0f8c4db4cf0f90a3b897f7a0bc1aeb.
 * The function consumes one or two pointer values synchronously; the Values
 * entry represents the caller's transient stack list without assigning a
 * native stack address a false guest identity.
 */
#include "psx_compat.h"
#include "game_port.h"

#include <string.h>

#define GA_DISP_SOURCE  0x800BCE80u
#define GA_DRAW_SOURCE  0x800BCDC8u
#define GA_DISP_COPY    0x801D1384u
#define GA_DRAW_COPY    0x801D13ACu

static void CopyGuestBytes(pe_addr_t destination, pe_addr_t source,
                           uint32_t size)
{
    memcpy(PE_Translate(destination, size),
           PE_TranslateConst(source, size), size);
}

int PE_func_80191FB8_Values(int count, const pe_addr_t *sources)
{
    uint8_t source_count = (uint8_t)count;
    uint32_t i;
    RECT rect;

    if ((uint8_t)(source_count - 1u) >= 2u || sources == NULL)
        return 0;
    for (i = 0u; i < source_count; i++) {
        if (sources[i] == 0u)
            return 0;
    }
    if (PE_LoadU8(0x800B0DBAu) != 0u)
        return 0;

    if (source_count == 1u) {
        pe_addr_t base = sources[0];
        PE_StoreU32(0x801D0DE8u, base);
        PE_StoreU32(0x801D0DECu, base + 0xFA00u);
        PE_StoreU32(0x801D0DFCu, base + 0x1F400u);
        PE_StoreU32(0x801D0DF8u, base + 0x3F400u);
        PE_StoreU32(0x801D0DF0u, base + 0x50400u);
        PE_StoreU32(0x801D0DF4u, base + 0x53100u);
    } else {
        pe_addr_t first = sources[0];
        pe_addr_t second = sources[1];
        PE_StoreU32(0x801D0DE8u, first);
        PE_StoreU32(0x801D0DECu, first + 0xFA00u);
        PE_StoreU32(0x801D0DFCu, first + 0x1F400u);
        PE_StoreU32(0x801D0DF8u, second);
        PE_StoreU32(0x801D0DF0u, second + 0x11000u);
        PE_StoreU32(0x801D0DF4u, second + 0x13D00u);
    }

    for (i = 0u; i < 2u; i++) {
        CopyGuestBytes(GA_DISP_COPY + i * 0x14u,
                       GA_DISP_SOURCE + i * 0x14u, 0x14u);
        CopyGuestBytes(GA_DRAW_COPY + i * 0x5Cu,
                       GA_DRAW_SOURCE + i * 0x5Cu, 0x5Cu);
    }
    PE_StoreU8(0x800B0DBAu, 1u);
    PE_StoreU8(0x800B0DBEu, 0x98u);
    PE_StoreU16(0x800B0DBCu, 0u);
    PE_StoreU16(0x801D11B0u, 0xFFFFu);

    rect.x = 320;
    rect.y = 0;
    rect.w = 192;
    rect.h = 256;
    (void)func_8007512C(&rect, 512, 0);
    if (PE_Port_ShouldStop())
        return 0;

    if ((PE_LoadU32(0x800B0CD8u) & 0x08000000u) == 0u) {
        rect.x = 0;
        rect.y = 448;
        rect.w = 320;
        rect.h = 64;
        (void)func_8007512C(&rect, 512, 256);
        if (PE_Port_ShouldStop())
            return 0;
    }
    return 1;
}

int func_80191FB8(int count, pe_addr_t source_list)
{
    pe_addr_t sources[2];
    uint8_t source_count = (uint8_t)count;
    uint32_t i;

    if ((uint8_t)(source_count - 1u) >= 2u ||
        !PE_RangeIsRam(source_list, (size_t)source_count * 4u))
        return 0;
    for (i = 0u; i < source_count; i++)
        sources[i] = PE_LoadU32(source_list + i * 4u);
    return PE_func_80191FB8_Values(count, sources);
}

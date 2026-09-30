/*
 * Phase 6E-B54K-AB — complete overlay display-pair initializer.
 * Retail [0x801918F8,0x80191B64), 155 words, SHA-256
 * 86c8721a16712363ff166438380acd4ca4b2f732b7c8aecb917a08298b050109.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

static void StoreDisplayFlags(pe_addr_t draw)
{
    PE_StoreU8(draw + 0x16u, 1u);
    PE_StoreU8(draw + 0x17u, 0u);
    PE_StoreU8(draw + 0x18u, 1u);
    PE_StoreU8(draw + 0x19u, 0u);
    PE_StoreU8(draw + 0x1Au, 0u);
    PE_StoreU8(draw + 0x1Bu, 0u);
}

void func_801918F8(int buffer, int wide)
{
    int8_t index = (int8_t)buffer;
    int x = (uint8_t)buffer == 0u ? 240 : 0;
    int y = (uint8_t)buffer == 0u ? 0 : 240;
    pe_addr_t disp = (pe_addr_t)(0x800BCE80u + (int32_t)index * 20);
    pe_addr_t draw = (pe_addr_t)(0x800BCDC8u + (int32_t)index * 92);

    if ((uint8_t)wide != 0u) {
        int16_t width;
        PE_StoreU8(0x801D0DBEu, 3u);
        (void)func_800749D8(disp, 0, x, 480, 240);
        PE_StoreU8(disp + 0x11u, 1u);
        width = (int16_t)PE_LoadU16(disp + 4u);
        PE_StoreU16(disp + 4u, (uint16_t)((width * 2) / 3));

        (void)func_80074924(draw, 0, y, 480, 240);
        StoreDisplayFlags(draw);
        width = (int16_t)PE_LoadU16(draw + 4u);
        PE_StoreU16(draw + 4u, (uint16_t)((width * 2) / 3));
    } else {
        PE_StoreU8(0x801D0DBEu, 2u);
        (void)func_800749D8(disp, 0, x, 320, 240);
        (void)func_80074924(draw, 0, y, 320, 240);
        StoreDisplayFlags(draw);
    }
}

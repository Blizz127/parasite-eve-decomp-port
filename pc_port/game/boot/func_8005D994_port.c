/*
 * Field-menu close page: the func_8004AE1C jump-table case-4/5 arm (selection
 * index 4 or 5).  It rebinds the status/inventory view to the carried slots,
 * refreshes the item row and stats, and commits the full heal.
 *
 * Original: [0x8005D994,0x8005DA8C), 0xF8 bytes / 62 words, asm/disc1/4E194.s.
 *
 *   *(uint8*)0x800C0E0C = 0x32;
 *   if (D_8009D2C8 == 0x800C0E48) D_8009D2D0 = func_80052F70();
 *   *(uint8*)0x800C0E0A = 0x62;
 *   D_800C0E00 = *(uint32*)(func_8005DBF8() + *(uint8*)0x800C0E0A * 4);
 *   v = *(uint16*)func_8005DBAC(0x62);
 *   D_800C0E24 = 0xFFFFF; *(uint16*)0x800C0E08 = v; *(uint16*)0x800C0E06 = v;
 *   for (i = 0; i < 7; i++)
 *       *(uint16*)(0x800C0E28 + i*2) = (index != 0 && (unsigned)(i-1) < 2) ? 0
 *                                                                          : 0x3E8;
 *   func_8005247C();                       ; full heal
 *
 * The 32-bit fixed-point store D_800C0E24 = 0xFFFFF is written as a word; the
 * three stat halfwords at 0x800C0E06/08 and the seven at 0x800C0E28 are halfword
 * stores (width matters, see the 0x8009D264 width test in the native suite).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8005D994(int32_t index)
{
    unsigned i;

    PE_StoreU8(0x800C0E0Cu, 0x32u);
    if (PE_LoadU32(0x8009D2C8u) == 0x800C0E48u)
        PE_StoreU32(0x8009D2D0u, func_80052F70());

    PE_StoreU8(0x800C0E0Au, 0x62u);
    PE_StoreU32(0x800C0E00u,
        PE_LoadU32(func_8005DBF8() + (uint32_t)PE_LoadU8(0x800C0E0Au) * 4u));

    {
        uint16_t value = PE_LoadU16(func_8005DBAC(0x62));
        PE_StoreU32(0x800C0E24u, 0xFFFFFu);
        PE_StoreU16(0x800C0E08u, value);
        PE_StoreU16(0x800C0E06u, value);
    }

    for (i = 0; i < 7u; i++)
        PE_StoreU16(0x800C0E28u + i * 2u,
            (index != 0 && (uint32_t)(i - 1u) < 2u) ? 0u : 0x3E8u);

    func_8005247C();
}

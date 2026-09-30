/*
 * Original menu icon/sprite packet builder: allocates a 0x28-byte primitive from
 * the menu packet pool, fills its texture page/UV rectangle from the icon spec
 * table, and links it into the ordering table.
 *
 * Original: [0x8005ED18,0x8005EEC8), 0x1B0 bytes / 108 words, asm/disc1/4F364.s.
 *
 *   spec = func_8005DADC(icon);              ; 8-byte-stride icon spec record
 *   if (D_8009D100 + 0x28 < D_8009D104 + 0x4000) {   ; pool has room
 *       packet = D_8009D100; D_8009D100 += 0x28;
 *   } else func_800527C0();                  ; pool exhausted (an empty stub)
 *   if (packet) {                            ; colour/prim header
 *       *(u32*)(packet+4) = D_8009D10C ? D_8009D114 : D_8009D110;
 *       *(u8*)(packet+3) = 9; *(u8*)(packet+7) = 0x2C;
 *   }
 *   x = *(u16*)D_8009D124; y = *(u16*)D_8009D128;
 *   *(u16*)(packet+0x18) = *(u16*)(packet+0x08) = x;
 *   *(u16*)(packet+0x12) = *(u16*)(packet+0x0A) = y;
 *   w = spec[4]; h = spec[5];
 *   *(u16*)(packet+0x20) = *(u16*)(packet+0x10) = x + w;
 *   *(u16*)(packet+0x22) = *(u16*)(packet+0x1A) = y + h;
 *   if (mode == 2) {                         ; interior UVs for the tiled body
 *       uv = spec[0] + w - 1; *(u8*)(packet+0x1C) = *(u8*)(packet+0x0C) = uv;
 *       uv = spec[1] + h - 1; *(u8*)(packet+0x15) = *(u8*)(packet+0x0D) = uv;
 *       uv = spec[0] - 1;     *(u8*)(packet+0x24) = *(u8*)(packet+0x14) = uv;
 *       uv = spec[1] - 1;     *(u8*)(packet+0x25) = *(u8*)(packet+0x1D) = uv;
 *   }
 *   *(u16*)(packet+0x0E) = spec[2] (texture page); *(u16*)(packet+0x16) = 7;
 *   *(u32*)packet = (*(u32*)packet & 0xFF000000) | (*(u32*)OT & 0xFFFFFF);
 *   *(u32*)OT     = (*(u32*)OT & 0xFF000000) | (packet & 0xFFFFFF);
 *
 * Alloc/free ordering-table state is the same guest quartet the already-native
 * func_8005EB64/PE_MenuPacketAlloc/PE_MenuPacketLink use (D_8009D100/D104/D11C,
 * $gp + 0x390/0x394/0x3AC).
 *
 * Address mirror: retail would write through a null packet pointer to low guest
 * addresses (0x08..0x25), which the PS1 maps onto the same RAM; the port's range
 * guard rejects sub-0x80000000 stores, so the packet/OT pointers go through the
 * hardware KUSEG mirror (address < 0x200000 -> |0x80000000).  Same physical RAM,
 * no abort, identical observable state.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_ED18_POOL   0x8009D100u   /* $gp + 0x390 */
#define GA_ED18_LIMIT  0x8009D104u   /* $gp + 0x394 */
#define GA_ED18_DIM    0x8009D10Cu   /* $gp + 0x39C */
#define GA_ED18_COLOR  0x8009D110u   /* $gp + 0x3A0 */
#define GA_ED18_HALF   0x8009D114u   /* $gp + 0x3A4 */
#define GA_ED18_OT     0x8009D11Cu   /* $gp + 0x3AC */
#define GA_ED18_X      0x8009D124u   /* $gp + 0x3B4 */
#define GA_ED18_Y      0x8009D128u   /* $gp + 0x3B8 */

static pe_addr_t ed18_ram(pe_addr_t address)
{ return address < 0x200000u ? address | 0x80000000u : address; }

void func_8005ED18(uint32_t icon, uint32_t mode)
{
    pe_addr_t spec = func_8005DADC(icon);
    pe_addr_t packet = 0u;
    pe_addr_t p;

    {
        pe_addr_t cursor = PE_LoadU32(GA_ED18_POOL);
        if (cursor + 0x28u < PE_LoadU32(GA_ED18_LIMIT) + 0x4000u) {
            PE_StoreU32(GA_ED18_POOL, cursor + 0x28u);
            packet = cursor;
        } else {
            func_800527C0();
        }
    }

    /* Retail uses the raw pointer for every store below (it never re-tests it);
     * the mirror keeps a null packet inside guest RAM instead of aborting. */
    p = ed18_ram(packet);

    if (packet != 0u) {
        PE_StoreU32(p + 4u,
            PE_LoadU32(PE_LoadU32(GA_ED18_DIM) ? GA_ED18_HALF : GA_ED18_COLOR));
        PE_StoreU8(p + 3u, 9u);
        PE_StoreU8(p + 7u, 0x2Cu);
    }

    PE_StoreU16(p + 0x08u, PE_LoadU16(GA_ED18_X));
    PE_StoreU16(p + 0x18u, PE_LoadU16(GA_ED18_X));
    PE_StoreU16(p + 0x0Au, PE_LoadU16(GA_ED18_Y));
    PE_StoreU16(p + 0x12u, PE_LoadU16(GA_ED18_Y));

    PE_StoreU16(p + 0x10u, (uint16_t)(PE_LoadU16(p + 0x08u) + PE_LoadU8(spec + 4u)));
    PE_StoreU16(p + 0x20u, (uint16_t)(PE_LoadU16(p + 0x08u) + PE_LoadU8(spec + 4u)));
    PE_StoreU16(p + 0x1Au, (uint16_t)(PE_LoadU16(p + 0x0Au) + PE_LoadU8(spec + 5u)));
    PE_StoreU16(p + 0x22u, (uint16_t)(PE_LoadU16(p + 0x0Au) + PE_LoadU8(spec + 5u)));

    if (mode == 2u) {
        uint8_t uv;

        uv = (uint8_t)(PE_LoadU8(spec) + PE_LoadU8(spec + 4u) - 1u);
        PE_StoreU8(p + 0x0Cu, uv);
        PE_StoreU8(p + 0x1Cu, uv);
        uv = (uint8_t)(PE_LoadU8(spec + 1u) + PE_LoadU8(spec + 5u) - 1u);
        PE_StoreU8(p + 0x0Du, uv);
        PE_StoreU8(p + 0x15u, uv);
        uv = (uint8_t)(PE_LoadU8(spec) - 1u);
        PE_StoreU8(p + 0x14u, uv);
        PE_StoreU8(p + 0x24u, uv);
        uv = (uint8_t)(PE_LoadU8(spec + 1u) - 1u);
        PE_StoreU8(p + 0x1Du, uv);
        PE_StoreU8(p + 0x25u, uv);
    }

    PE_StoreU16(p + 0x0Eu, PE_LoadU16(spec + 2u));
    PE_StoreU16(p + 0x16u, 7u);

    {
        pe_addr_t ot = ed18_ram(PE_LoadU32(GA_ED18_OT));
        PE_StoreU32(p, (PE_LoadU32(p) & 0xFF000000u) |
                       (PE_LoadU32(ot) & 0x00FFFFFFu));
        PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) |
                        (packet & 0x00FFFFFFu));
    }
}

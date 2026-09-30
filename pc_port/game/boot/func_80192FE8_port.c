/*
 * Title draw leaf func_80192FE8 (installed at title node +0xC).
 *
 * Retail overlay: [0x80192FE8, 0x80193084), 39 words / 0x9C.
 * y = +0x20 + acc(+0x24); alpha +0x1C = 256 - |acc<<4|; bumps +0x24
 * when nonzero; when +0x24 == +0x28 invokes +0x14; marks +0x30 when
 * acc >= 16.  (Retranslated from the overlay bytes: the earlier port had
 * the y and alpha formulas swapped.)
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_guestcode.h"

extern void func_8019319C(pe_addr_t node);

void func_80192FE8(pe_addr_t node)
{
    int32_t acc = (int32_t)PE_LoadU32(node + 0x24u);
    int32_t scaled = acc << 4;
    pe_addr_t callback;

    /* 0x80192FF8..3010: y = base(+0x20) + acc. */
    PE_StoreU16(node + 0x06u,
                (uint16_t)(PE_LoadU32(node + 0x20u) + (uint32_t)acc));
    /* 0x80193008..3020: alpha = 256 - |acc << 4|. */
    PE_StoreU32(node + 0x1Cu,
                (uint32_t)(scaled < 0 ? scaled + 256 : 256 - scaled));
    callback = PE_LoadU32(node + 0x14u);
    if (acc != 0)
        acc += 1;
    PE_StoreU32(node + 0x24u, (uint32_t)acc);
    if (callback != 0u && acc == (int32_t)PE_LoadU32(node + 0x28u)) {
        extern int PE_TitleNodeCall(const char *site, pe_addr_t fn,
                                    pe_addr_t node, pe_addr_t a1,
                                    int32_t a2, int32_t a3);
        (void)PE_TitleNodeCall("func_80192FE8_callback", callback, node,
                               0u, 0, 0);
    }
    if ((int32_t)PE_LoadU32(node + 0x24u) >= 16)
        PE_StoreU32(node + 0x30u, 1u);
}

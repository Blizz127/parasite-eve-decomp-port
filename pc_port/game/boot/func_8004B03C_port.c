/*
 * Field-menu Escape sub-page constructor: jtbl_80011034 case 1.
 *
 * Original: [0x8004B03C,0x8004B0A4), 0x68 bytes / 26 words, asm/disc1/37CD0.s.
 * Called by func_8004AE1C(list) when the Items page's selection index is 1.
 *
 *   window = func_80062D2C(0x23, owner, 0, 0)
 *   list   = func_8006322C(0x23, window, window)
 *   window+0x2C = 0x8004B0A4   ; this sub-page's input handler (native)
 *   list+0x30   = 0x8004FF80   ; this sub-page's list draw (draw adapter)
 *   func_80062CB8(list)
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8004B03C(pe_addr_t owner)
{
    pe_addr_t window = func_80062D2C(0x23u, owner, 0u, 0u);
    pe_addr_t list = func_8006322C(0x23u, window, window);

    PE_StoreU32(window + 44u, 0x8004B0A4u);
    PE_StoreU32(list + 48u, 0x8004FF80u);
    func_80062CB8(list);
}

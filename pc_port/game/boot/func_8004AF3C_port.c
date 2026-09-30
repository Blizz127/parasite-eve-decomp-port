/*
 * Field-menu Items sub-page constructor: jtbl_80011034 case 0.
 *
 * Original: [0x8004AF3C,0x8004AFA4), 0x68 bytes / 26 words, asm/disc1/37CD0.s.
 * Called by func_8004AE1C(list) when the Items page's selection index is 0.
 *
 *   window = func_80062D2C(0x21, owner, 0, 0)
 *   list   = func_8006322C(0x21, window, window)
 *   window+0x2C = 0x8004AFA4   ; this sub-page's input handler (native)
 *   list+0x30   = 0x8004FF58   ; this sub-page's list draw (draw adapter)
 *   func_80062CB8(list)
 *
 * Same layout as func_8004AD9C; the stored callback identities are guest code
 * addresses exactly as retail stores them.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8004AF3C(pe_addr_t owner)
{
    pe_addr_t window = func_80062D2C(0x21u, owner, 0u, 0u);
    pe_addr_t list = func_8006322C(0x21u, window, window);

    PE_StoreU32(window + 44u, 0x8004AFA4u);
    PE_StoreU32(list + 48u, 0x8004FF58u);
    func_80062CB8(list);
}

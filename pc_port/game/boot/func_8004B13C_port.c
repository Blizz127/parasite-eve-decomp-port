/*
 * Field-menu Equipment sub-page constructor: jtbl_80011034 case 2.
 *
 * Original: [0x8004B13C,0x8004B214), 0xD8 bytes / 54 words, asm/disc1/37CD0.s.
 * Called by func_8004AE1C(list) when the Items page's selection index is 2.
 * It is the only case that builds two sibling lists (ids 0x2E and 0x31) under
 * one window (id 0x2E) and cross-links them through +0x78/+0x7C:
 *
 *   window = func_80062D2C(0x2E, owner, 0, 0)
 *   list1  = func_8006322C(0x2E, window, window)
 *   window+0x30 = 0x8004B214   ; window input handler  (named boundary)
 *   window+0x2C = 0x8004B394   ; window input handler  (named boundary)
 *   window+0x4C = 0x800922D4   ; static sub-page table
 *   window+0x40 = 1
 *   list1+0x30  = 0x8004B534   ; list1 draw            (draw adapter)
 *   list1+0x28  = 1
 *   list2  = func_8006322C(0x31, window, window)
 *   list2+0x30  = 0x8004B55C   ; list2 draw            (draw adapter)
 *   list1+0x7C  = list2
 *   list2+0x78  = list1
 *   func_80062CB8(list1+0x44 < 0 ? list2 : list1)  ; publish the active list
 *   D_8009D260 = func_800614A0()
 *
 * The +0x44 sign test is the retail `bgez` on list1+0x44: list1 is published
 * unless it is marked negative, in which case list2 is.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern int func_800614A0(void);

void func_8004B13C(pe_addr_t owner)
{
    pe_addr_t window = func_80062D2C(0x2Eu, owner, 0u, 0u);
    pe_addr_t list1 = func_8006322C(0x2Eu, window, window);
    pe_addr_t list2;

    PE_StoreU32(window + 48u, 0x8004B214u);
    PE_StoreU32(window + 44u, 0x8004B394u);
    PE_StoreU32(window + 76u, 0x800922D4u);
    PE_StoreU32(window + 64u, 1u);
    PE_StoreU32(list1 + 48u, 0x8004B534u);
    PE_StoreU32(list1 + 40u, 1u);

    list2 = func_8006322C(0x31u, window, window);
    PE_StoreU32(list2 + 48u, 0x8004B55Cu);
    PE_StoreU32(list1 + 124u, list2);
    PE_StoreU32(list2 + 120u, list1);

    func_80062CB8((int32_t)PE_LoadU32(list1 + 68u) < 0 ? list2 : list1);
    PE_StoreU32(0x8009D260u, (uint32_t)func_800614A0());
}

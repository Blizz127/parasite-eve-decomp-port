/*
 * Field-menu modal sub-page constructor: jtbl_80011034 case 3.
 *
 * Original: [0x8004B584,0x8004B5DC), 0x58 bytes / 22 words, asm/disc1/3BD84.s.
 * Called by func_8004AE1C(list) when the Items page's selection index is 3.
 * Unlike the other cases it allocates a modal window only (no list) and
 * publishes the alarm-timer snapshot func_8005E884() into $gp+0x274
 * (0x8009D264, the word func_8004B6CC later reads):
 *
 *   window = func_80062D2C(0x38, owner, 0, 1)   ; modal = 1
 *   window+0x30 = 0x8004B5DC   ; draw callback (named boundary)
 *   window+0x2C = 0x8004B650   ; input handler (named boundary)
 *   func_80062CB8(window)
 *   D_8009D264 = func_8005E884()
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8004B584(pe_addr_t owner)
{
    pe_addr_t window = func_80062D2C(0x38u, owner, 0u, 1u);

    PE_StoreU32(window + 48u, 0x8004B5DCu);
    PE_StoreU32(window + 44u, 0x8004B650u);
    func_80062CB8(window);
    PE_StoreU32(0x8009D264u, (uint32_t)(int32_t)func_8005E884());
}

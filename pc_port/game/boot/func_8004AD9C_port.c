/*
 * Field menu page constructor (Items/Escape dispatch command 5).
 *
 * Original: [0x8004AD9C,0x8004AE1C), 32 words, asm/disc1/37CD0.s.
 * Call site: func_80043DA4's command-5 arm (asm/disc1/340EC.s:487, jal
 * 0x8004AD9C with a0 = the menu list) after the three shared
 * func_80062F3C(0x2D/0x18/0x12) teardown calls the caller already makes.
 *
 * Frame layout is transcribed exactly: window = 0x80062D2C(0x20, owner, 0,
 * 0); list = 0x8006322C(0x20, window, window); the window's input handler
 * slot +0x2C receives 0x8004AE1C and the list's callback slot +0x30 receives
 * 0x8004FF30 (the +0x30 store sits in the 0x80062CB8 jal delay slot, so it
 * lands before that call); then 0x80062CB8(list) and 0x800647D0(list, 4).
 *
 * The stored callback identities are guest function addresses, exactly as
 * retail stores them. The list callback 0x8004FF30 is translated
 * (pc_port/game/boot/func_8004FF30_port.c, tier-2 wrapper into
 * func_800638D8 with the per-cell draw 0x80050C50, wired in
 * func_800638D8_port.c's menu_draw_callback), so the plain disc-1 route no
 * longer stops at `PE_MenuDrawCallback` for this page. The input handler
 * 0x8004AE1C is still only a name in func_80063E0C_port.c's menu_callback
 * default arm (PE_MenuInputCallback): that is the recorded next residual
 * for this page, not a silent skip.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8004AD9C(pe_addr_t owner)
{
    pe_addr_t window = func_80062D2C(0x20u, owner, 0u, 0u);
    pe_addr_t list = func_8006322C(0x20u, window, window);

    PE_StoreU32(window + 44u, 0x8004AE1Cu);   /* input handler slot +0x2C */
    PE_StoreU32(list + 48u, 0x8004FF30u);    /* list callback slot +0x30 */
    func_80062CB8(list);
    func_800647D0(list, 4);
}

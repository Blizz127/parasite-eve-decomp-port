/*
 * Original menu close/commit helper: refresh the status line and commit the
 * pending HP (full-heal the active record).
 *
 * Original: [0x8005247C,0x800524D0), 0x54 bytes / 21 words, asm/disc1/42664.s.
 *
 *   func_8005218C();                       ; rebuild the menu/status view
 *   aya = D_8009D254;                      ; active actor
 *   if (aya) { record = *aya;
 *       if (record) { v = *(uint16*)(record+0x1C);
 *                     *(uint16*)(record+0x0E) = v;   ; gauge mirror
 *                     *(uint16*)(record+0x0C) = v; } } ; HP = max
 *
 * record+0x0C is current HP and record+0x1C is the max; this is the full-heal
 * the field-menu close page performs.  Caller: func_8005D994 (the 0x8004AE1C
 * jump-table case-4/5 close path).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8005247C(void)
{
    pe_addr_t aya;

    func_8005218C();
    aya = PE_LoadU32(0x8009D254u);
    if (aya) {
        pe_addr_t record = PE_LoadU32(aya);
        if (record) {
            uint16_t value = PE_LoadU16(record + 28u);
            PE_StoreU16(record + 14u, value);
            PE_StoreU16(record + 12u, value);
        }
    }
}

/*
 * Items list per-cell draw (installed via func_8004FF58 -> func_800638D8).
 *
 * Original: [0x80050C70,0x80050CB4), 0x44 bytes / 17 words, asm/disc1/41470.s.
 *
 *   func_80064C54(node + 0x2E);          ; text lookup keyed by the cell
 *   if (func_800527B4() == node)         ; focused-cell highlight
 *       func_80064C80();
 *
 * node+0x2E is passed as the guest string id exactly as retail does; the
 * comparison is a raw 32-bit register compare (bne $v0,$s0).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern int func_800527B4(void);

/* func_80050C70: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050C70_port.c (src/func_80050C70.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* Preserve 32-bit guest address wrap for the indexed string byte. */
void func_80050F10(int a0)
{
    func_8005E8C4();
    func_8005E8A4(2, 0);
    func_8005EED4(PE_LoadU8(PE_LoadU32(0x8009CF54u) + (uint32_t)a0));
    func_8005E914();
}

void func_80050AD8(int32_t index)
{
    uint32_t base = PE_LoadU32(0x8009CF20u);
    int value;
    if (index < (int)PE_LoadU8(base + 0x14u)) {
        value = PE_LoadU8(base + (uint32_t)index + 0x15u) & 0x1F;
        if (!value) return;
        value += PE_LoadU32(0x8009CF18u) ? 0x22 : 0x36;
    } else {
        value = 0x46;
    }
    func_8005EB64(value);
}

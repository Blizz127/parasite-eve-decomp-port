/*
 * Hand adapters — batch 18 (>= 0x80060000).  Bodies follow the matched
 * leaves src/func_XXXXXXXX.c; pointers are guest addresses; retail stack
 * locals handed to guest-address callees live at PE_HAND_HI_STACK.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern unsigned int func_80071A54(void);
extern int func_800C6B20(pe_addr_t quad);

/* src/func_800DB0D0.c: sparkle emitter.  Mode 0 seeds a1->+8 with rand,
 * takes the player position into a1 (func_800CE870 mode 0) and allocates
 * 0x10 x 0x10 particles with the per-particle callback func_800DAF8C
 * (retail text address).  Mode 1 spawns one per frame below 0x11 (+0xC =
 * (r & 0x3FF) - 0x200, +2 = seed, +4 = r, +0xE = (r & 0x7F) - 0x40; seed +=
 * 0x8AA + (r & 0x1F)) and ends at 0x49.  Mode 2 publishes a1's position
 * to D_800E221C..20 and D_800F3374 = 8. */
int func_800DB0D0(int a0, pe_addr_t a1)
{
    pe_addr_t p;

    switch (a0) {
    case 0: {
        int t = (int)func_80071A54();
        pe_addr_t q = PE_LoadU32(0x8009D254u);

        PE_StoreU32(a1 + 8u, (uint32_t)t);
        func_800CE870(q, 0, a1);
        return (int)func_800CE560(PE_LoadU32(PE_LoadU32(0x800F33E0u) + 8u), 0x10u, 0x10,
                                  0x800DAF8Cu);
    }
    case 1:
        if ((int)PE_LoadU32(0x800E27ECu) < 0x11) {
            p = func_800CE610(PE_LoadU32(PE_LoadU32(0x800F33E0u) + 8u));
            if (p != 0u) {
                PE_StoreU16(p + 12u, (uint16_t)((int)(func_80071A54() & 0x3FFu) - 0x200));
                PE_StoreU16(p + 2u, (uint16_t)PE_LoadU32(a1 + 8u));
                PE_StoreU16(p + 4u, (uint16_t)func_80071A54());
                PE_StoreU16(p + 14u, (uint16_t)((int)(func_80071A54() & 0x7Fu) - 0x40));
                PE_StoreU32(a1 + 8u, PE_LoadU32(a1 + 8u) + 0x8AAu + (func_80071A54() & 0x1Fu));
            }
        }
        if ((int)PE_LoadU32(0x800E27ECu) < 0x49)
            break;
        return 1;
    case 2:
        PE_StoreU16(0x800E221Cu, PE_LoadU16(a1 + 0u));
        PE_StoreU16(0x800E221Eu, PE_LoadU16(a1 + 2u));
        PE_StoreU16(0x800E2220u, PE_LoadU16(a1 + 4u));
        PE_StoreU16(0x800F3374u, 8u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DF6AC.c: ring burst.  Mode 0: a1 = {0, 0xA0}, allocate
 * 0x14 x 0x20 particles with callback func_800DEFFC.  Mode 1: every 4th
 * frame spawn ring a1[0] < 8 (+6 = D_800E2164[a1[0]], +0x10/+0x12 = 0);
 * from frame 0x20 fade a1[1] by 2; finished (a1[1] = 0) once it is <= 0.
 * Mode 2: blend/texture state for the fade (D_800F3368.. globals, the
 * D_800E2850 texpage for stage D_800E11E6, func_800CEDA8(1) — the extra
 * arguments retail passes are not read by the callee). */
int func_800DF6AC(int a0, pe_addr_t a1)
{
    switch (a0) {
    case 0:
        PE_StoreU16(a1 + 0u, 0u);
        PE_StoreU16(a1 + 2u, 0xA0u);
        return (int)func_800CE560(PE_LoadU32(PE_LoadU32(0x800F33E0u) + 8u), 0x14u, 0x20,
                                  0x800DEFFCu);
    case 1:
        if ((PE_LoadU32(0x800E27ECu) & 3u) == 0u && (int16_t)PE_LoadU16(a1) < 8) {
            pe_addr_t t = func_800CE610(PE_LoadU32(PE_LoadU32(0x800F33E0u) + 8u));

            if (t != 0u) {
                int v = PE_LoadU8(0x800E2164u + (uint32_t)(int16_t)PE_LoadU16(a1));

                PE_StoreU16(t + 0x10u, 0u);
                PE_StoreU16(t + 0x12u, 0u);
                PE_StoreU16(t + 6u, (uint16_t)v);
            }
            PE_StoreU16(a1, (uint16_t)((int16_t)PE_LoadU16(a1) + 1));
        }
        if ((int)PE_LoadU32(0x800E27ECu) >= 0x20)
            PE_StoreU16(a1 + 2u, (uint16_t)((int16_t)PE_LoadU16(a1 + 2u) - 2));
        if ((int16_t)PE_LoadU16(a1 + 2u) > 0)
            return 0;
        PE_StoreU16(a1 + 2u, 0u);
        return 1;
    case 2: {
        unsigned int u = PE_LoadU16(a1 + 2u);
        unsigned int ix = PE_LoadU16(0x800E11E6u);

        PE_StoreU16(0x800F3368u, 0x20u);
        PE_StoreU16(0x800F336Au, 2u);
        PE_StoreU16(0x800F3376u, 0x20u);
        PE_StoreU16(0x800F3378u, 0x20u);
        PE_StoreU16(0x800E2244u, (uint16_t)u);
        PE_StoreU16(0x800F336Cu, 1u);
        PE_StoreU16(0x800F3370u, PE_LoadU16(0x800E2850u + ix * 2u));
        func_800CEDA8(1);
        PE_StoreU16(0x800F336Eu, 0u);
        PE_StoreU16(0x800F3372u, 0u);
        PE_StoreU16(0x800F3374u, 0x18u);
        break;
    }
    }
    return 0;
}

/* src/func_800C5EB0.c: debris-ring tick over the 0x44-byte segment
 * records at *a0.  Marks segment a0->+0xE (when < count) active (2);
 * for each of count-1 segments: random frame byte (+3 = r % 4); an active
 * one tests its four corners (+0x14/+0x1C of this and +0x58/+0x60 of the
 * next record, vy forced 0) against the player (func_800C6B20 == 1 sets
 * *a2) and decays its +4 timer by 0xC (<= 0xC -> 0 and deactivate).
 * Returns whether the tick counter +0xE was 0x64, then increments it. */
int func_800C5EB0(pe_addr_t a0, int a1, pe_addr_t a2)
{
    const pe_addr_t v = PE_HAND_HI_STACK + 0x00u;
    pe_addr_t base = PE_LoadU32(a0);
    int e0 = (int16_t)PE_LoadU16(a0 + 0xEu);
    int n0 = (int16_t)PE_LoadU16(a0 + 4u);
    unsigned int i = 0;
    unsigned short e;

    (void)a1;
    PE_StoreU32(a2, 0u);
    if ((unsigned int)e0 < (unsigned int)n0)
        PE_StoreU8(base + (uint32_t)(e0 * 0x44), 2u);
    if (i < (unsigned int)((int16_t)PE_LoadU16(a0 + 4u) - 1)) {
        pe_addr_t p = base + 4u;

        do {
            int r = (int)func_80071A54();

            PE_StoreU8(p - 1u, (uint8_t)(r % 4));
            if (PE_LoadU8(base) == 2u) {
                static const uint32_t src[4] = {0x10u, 0x18u, 0x54u, 0x5Cu};
                unsigned int k;
                int res;
                unsigned short t;

                for (k = 0; k < 4u; k++) {
                    PE_StoreU32(v + k * 8u, PE_LoadU32(p + src[k]));
                    PE_StoreU32(v + k * 8u + 4u, PE_LoadU32(p + src[k] + 4u));
                }
                for (k = 0; k < 4u; k++)
                    PE_StoreU16(v + k * 8u + 2u, 0u);
                res = func_800C6B20(v);
                if (res == 1)
                    PE_StoreU32(a2, (uint32_t)res);
                t = PE_LoadU16(p);
                if (t >= 0xDu) {
                    PE_StoreU16(p, (uint16_t)(t - 0xCu));
                } else {
                    PE_StoreU16(p, 0u);
                    PE_StoreU8(base, 0u);
                }
            }
            i++;
            p += 0x44u;
            base += 0x44u;
        } while (i < (unsigned int)((int16_t)PE_LoadU16(a0 + 4u) - 1));
    }
    e = PE_LoadU16(a0 + 0xEu);
    PE_StoreU16(a0 + 0xEu, (uint16_t)(e + 1u));
    return (short)e == 0x64;
}

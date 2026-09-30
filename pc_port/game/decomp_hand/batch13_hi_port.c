/*
 * Hand adapters — batch 13 (>= 0x80060000).  Bodies follow the matched
 * leaves src/func_XXXXXXXX.c; pointers are guest addresses; retail stack
 * vectors passed to guest-address callees live at PE_HAND_HI_STACK.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern unsigned int func_80071A54(void);
extern int32_t func_80077CF4(int32_t angle);
extern int32_t func_80077DC4(int32_t angle);

/* src/func_800CAA38.c: two template SVECTORs (D_800C21C4 / D_800C21CC)
 * through the D_800E27A8 owner's matrix (+0x238 -> +0x260); endpoint 1 =
 * base + out into +8/+A/+C, endpoint 2 into +0x10/+0x12/+0x14; +4 = 0x7F,
 * +6 = 0. */
void func_800CAA38(int a0, int a1, pe_addr_t a2)
{
    const pe_addr_t v10 = PE_HAND_HI_STACK + 0x20u, v18 = PE_HAND_HI_STACK + 0x28u;
    const pe_addr_t out = PE_HAND_HI_STACK + 0x30u;
    unsigned short t;

    (void)a0; (void)a1;
    PE_StoreU32(v10, PE_LoadU32(0x800C21C4u)); PE_StoreU32(v10 + 4u, PE_LoadU32(0x800C21C8u));
    PE_StoreU32(v18, PE_LoadU32(0x800C21CCu)); PE_StoreU32(v18 + 4u, PE_LoadU32(0x800C21D0u));
    (void)func_80078C34(PE_LoadU32(PE_LoadU32(0x800E27A8u) + 0x238u) + 0x260u, v10, out);
    PE_StoreU16(a2 + 8u, (uint16_t)(PE_LoadU16(0x800E2360u) + PE_LoadU16(out + 0u)));
    PE_StoreU16(a2 + 0xAu, (uint16_t)(PE_LoadU16(0x800E2362u) + PE_LoadU16(out + 2u)));
    PE_StoreU16(a2 + 0xCu, (uint16_t)(PE_LoadU16(0x800E2364u) + PE_LoadU16(out + 4u)));
    (void)func_80078C34(PE_LoadU32(PE_LoadU32(0x800E27A8u) + 0x238u) + 0x260u, v18, out);
    PE_StoreU16(a2 + 0x10u, (uint16_t)(PE_LoadU16(0x800E2360u) + PE_LoadU16(out + 0u)));
    PE_StoreU16(a2 + 0x12u, (uint16_t)(PE_LoadU16(0x800E2362u) + PE_LoadU16(out + 2u)));
    t = (unsigned short)(PE_LoadU16(0x800E2364u) + PE_LoadU16(out + 4u));
    PE_StoreU16(a2 + 4u, 0x7Fu);
    PE_StoreU16(a2 + 6u, 0u);
    PE_StoreU16(a2 + 0x14u, t);
}

/* src/func_800CC0E0.c: copy the three colour words (slots 3/4/5 of the
 * func_800C2B10 table) to D_800E2280..82; +1 = 0x7F; ring size +2 = 8 when
 * the stage id ((*D_8009D254)->+0x68->+6) is 3, else 0x10; for each of
 * the (s8)+2 points: start at D_800E2290[0..2] (+0x26 + i*8), velocity
 * (cos, -0x1400, sin) of angle i << 9 (i << 8 for 16 points) at +0xA6,
 * life +6 + i*2 = 0x258. */
void func_800CC0E0(int a0, int a1, pe_addr_t a2)
{
    pe_addr_t stage;
    int i = 0;

    (void)a0; (void)a1;
    PE_StoreU8(0x800E2280u, (uint8_t)PE_LoadU32(func_800C2B10(3)));
    PE_StoreU8(0x800E2281u, (uint8_t)PE_LoadU32(func_800C2B10(4)));
    PE_StoreU8(0x800E2282u, (uint8_t)PE_LoadU32(func_800C2B10(5)));
    PE_StoreU8(a2 + 1u, 0x7Fu);
    stage = PE_LoadU32(PE_LoadU32(PE_LoadU32(0x8009D254u)) + 0x68u);
    PE_StoreU8(a2 + 2u, ((int16_t)PE_LoadU16(stage + 6u) == 3) ? 8u : 0x10u);
    while (i < (signed char)PE_LoadU8(a2 + 2u)) {
        pe_addr_t e = a2 + (uint32_t)i * 8u;
        int sh;

        PE_StoreU16(e + 0x26u, PE_LoadU16(0x800E2290u));
        PE_StoreU16(e + 0x28u, PE_LoadU16(0x800E2292u));
        PE_StoreU16(e + 0x2Au, PE_LoadU16(0x800E2294u));
        sh = i << 9;
        if ((signed char)PE_LoadU8(a2 + 2u) == 0x10)
            sh = i << 8;
        PE_StoreU16(e + 0xA6u, (uint16_t)func_80077CF4(sh));
        PE_StoreU16(e + 0xA8u, (uint16_t)-0x1400);
        PE_StoreU16(e + 0xAAu, (uint16_t)func_80077DC4(sh));
        PE_StoreU16(a2 + (uint32_t)i * 2u + 6u, 0x258u);
        i++;
    }
}

/* src/func_800DF9B0.c: three-phase effect callback.  Phase 0 seeds *a1
 * with rand, primes the mesh D_800F32D8 (func_800C6D5C) and allocates 0x10
 * x 0x18 particles with emitter func_800D9A8C (retail text address).
 * Phase 1 spawns a particle every 6 frames while D_800E27EC < 0x28
 * (+6 = 0, +0xC = 0, +0xA = *a1; *a1 -= 0x555 + (rand & 0xFF)) and ends
 * (returns 1) at frame 0x46.  Phase 2 func_800CE870(D_8009D254, 1,
 * &D_800E220C). */
int func_800DF9B0(int a0, pe_addr_t a1)
{
    switch (a0) {
    case 0: {
        int r = (int)func_80071A54();
        pe_addr_t d = PE_LoadU32(0x800F32D8u);

        PE_StoreU32(a1, (uint32_t)r);
        func_800C6D5C(d, 0u, 0u);
        return (int)func_800CE560(PE_LoadU32(PE_LoadU32(0x800F33E0u) + 8u), 0x10u, 0x18,
                                  0x800D9A8Cu);
    }
    case 1: {
        int x = (int)PE_LoadU32(0x800E27ECu);

        if (x < 0x28 && x % 6 == 0) {
            pe_addr_t t = func_800CE610(PE_LoadU32(PE_LoadU32(0x800F33E0u) + 8u));

            if (t != 0u) {
                int v = (int)PE_LoadU32(a1);
                int r;

                PE_StoreU16(t + 6u, 0u);
                PE_StoreU16(t + 0xCu, 0u);
                PE_StoreU16(t + 0xAu, (uint16_t)v);
                r = (int)func_80071A54();
                PE_StoreU32(a1, (uint32_t)(((int)PE_LoadU32(a1) - 0x555) - (r & 0xFF)));
            }
        }
        if ((int)PE_LoadU32(0x800E27ECu) >= 0x46)
            return 1;
        break;
    }
    case 2:
        func_800CE870(PE_LoadU32(0x8009D254u), 1, 0x800E220Cu);
        break;
    }
    return 0;
}

/* src/func_80071754.c: player facing (+0x3A, 12-bit angle).  After
 * func_8001A784(s, 0x11): analog mode ((D_800BE9A0 & 0xF000) == 0x7000)
 * takes atan2(stick) - 0x400 (wrapped) plus the D_800BD022 camera yaw;
 * digital mode picks one of the eight D-pad angles from D_8009D26C bits
 * (up 8 / down 0x20 with left 0x40 / right 0x10) plus D_800BD020.  When
 * D_8009D254 is live and D_8009D2E8 bit 4 is set, add 0x800 and the
 * ((*o)[19] >> 7) & 0xC00 quadrant; mask to 12 bits, and mirror
 * (0x1000 - angle) under the same bit. */
void func_80071754(pe_addr_t a0)
{
    int v3, v2;

    func_8001A784(a0, 0x11u);
    if ((PE_LoadU16(0x800BE9A0u) & 0xF000u) == 0x7000u) {
        int p0 = PE_LoadU8(0x800BE9A7u), p1 = PE_LoadU8(0x800BE9A6u);

        v3 = func_80079FB4(p0 - 0x80, p1 - 0x80) - 0x400;
        if (v3 < 0)
            v3 += 0x1000;
        v2 = PE_LoadU16(0x800BD022u);
    } else {
        uint32_t f = PE_LoadU32(0x8009D26Cu);

        if (f & 8u)
            PE_StoreU16(a0 + 0x3Au, (f & 0x40u) ? 0x600u : (f & 0x10u) ? 0xA00u : 0x800u);
        else if (f & 0x20u)
            PE_StoreU16(a0 + 0x3Au, (f & 0x40u) ? 0x200u : (f & 0x10u) ? 0xE00u : 0u);
        else if (f & 0x40u)
            PE_StoreU16(a0 + 0x3Au, 0x400u);
        else if (f & 0x10u)
            PE_StoreU16(a0 + 0x3Au, 0xC00u);
        v3 = (int16_t)PE_LoadU16(a0 + 0x3Au);
        v2 = PE_LoadU16(0x800BD020u);
    }
    v3 += v2;
    if (PE_LoadU32(0x8009D254u) != 0u && (PE_LoadU32(0x8009D2E8u) & 0x10u) != 0u) {
        v3 += 0x800;
        v3 += (int)((PE_LoadU32(PE_LoadU32(PE_LoadU32(0x8009D254u)) + 76u) >> 7) & 0xC00u);
    }
    PE_StoreU16(a0 + 0x3Au, (uint16_t)(v3 & 0xFFF));
    if ((PE_LoadU32(0x8009D2E8u) & 0x10u) != 0u)
        PE_StoreU16(a0 + 0x3Au, (uint16_t)((0x1000 - PE_LoadU16(a0 + 0x3Au)) & 0xFFF));
}

/*
 * Hand adapters — batch 17 (>= 0x80060000).  Bodies follow the matched
 * leaves src/func_XXXXXXXX.c; pointers are guest addresses; retail stack
 * locals handed to guest-address callees live at PE_HAND_HI_STACK.
 * Renderers with no pc_port implementation (func_800D27FC)
 * stay loud boundaries.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern unsigned int func_80071A54(void);
extern int32_t func_80077CF4(int32_t angle);
extern int32_t func_80077DC4(int32_t angle);
extern void func_80077C44(pe_addr_t p);

#define S7_A (PE_HAND_HI_STACK + 0x00u)
#define S7_B (PE_HAND_HI_STACK + 0x08u)
#define S7_C (PE_HAND_HI_STACK + 0x10u)

static int s7_frame(void) { return (int)PE_LoadU32(0x800E27ECu); }

static int s7_div(int num, int den)
{
    if (den == 0 || (den == -1 && num == (int)0x80000000u)) {
        PE_D_COMP_BOUNDARY2("fx_divide_trap", 0u, num, den);
        return 0;
    }
    return num / den;
}

/* src/func_800D5898.c: a1[0] = a1[1] * cos((f << 10) / a1[3]) / 0x1000,
 * finished at f >= a1[3]; draw via func_800D27FC(a1[0], a1[2], colour
 * D_800C22C0, sin / 64 + 0x40, 1) — boundary. */
int func_800D5898(int a0, pe_addr_t a1)
{
    int r;

    PE_StoreU32(S7_C, PE_LoadU32(0x800C22C0u));
    switch (a0) {
    case 1:
        r = func_80077DC4(s7_div(s7_frame() << 10, (int16_t)PE_LoadU16(a1 + 6u)));
        PE_StoreU16(a1 + 0u, (uint16_t)((int16_t)PE_LoadU16(a1 + 2u) * r / 0x1000));
        if (s7_frame() >= (int16_t)PE_LoadU16(a1 + 6u))
            return 1;
        break;
    case 2:
        r = func_80077DC4(s7_div(s7_frame() << 10, (int16_t)PE_LoadU16(a1 + 6u)));
        PE_D_COMP_BOUNDARY4("func_800D27FC", 0x800D27FCu,
                            (int16_t)PE_LoadU16(a1 + 0u), (int16_t)PE_LoadU16(a1 + 4u),
                            S7_C, r / 64 + 0x40);
        break;
    }
    return 0;
}

/* src/func_800D4EA4.c: 0x14-frame aura around the player.  Mode 2 sets
 * D_800F3374 = 0xC8, takes the player position (func_800CE870 mode 1),
 * lifts it by 0x12C, samples the colour curve D_800E1540 (D_800E1518 when
 * D_800E2368->+0x1E == 1) at f * 24 / 20 and scales by sin((f << 10) / 20)
 * / 2 + 0x800 into func_800D004C (pe_effect fan, func_800CEE20_port.c). */
int func_800D4EA4(int a0)
{
    pe_addr_t p;
    int w;

    switch (a0) {
    case 1:
        if (s7_frame() >= 0x14)
            return 1;
        break;
    case 2:
        PE_StoreU16(0x800F3374u, 0xC8u);
        func_800CE870(PE_LoadU32(0x8009D254u), 1, S7_A);
        PE_StoreU16(S7_A + 2u, (uint16_t)((int16_t)PE_LoadU16(S7_A + 2u) - 0x12C));
        p = (PE_LoadU16(PE_LoadU32(0x800E2368u) + 0x1Eu) == 1u) ? 0x800E1518u : 0x800E1540u;
        func_800CF3AC(p, S7_B, s7_frame() * 24 / 20);
        w = func_80077CF4((s7_frame() << 10) / 20) / 2 + 0x800;
        /* src/func_800D4EA4.c: func_800D004C(&v30, 0x190, 0x190, 8, 0, w, w,
         * &v38, 0, 0x80, 1) — v30/v38 are the S7_A/S7_B guest temps. */
        func_800D004C(S7_A, 0x190, 0x190, 8, 0u, w, w, S7_B, 0u, 0x80, 1);
        break;
    }
    return 0;
}

/* src/func_800CBFC4.c: colour bytes from func_800C2B10 slots 0/1/2 into
 * D_800F3450..52; +6 = D_800E2290 - 100 + r % 201; +8 = D_800E2292;
 * +4 = 0x20C; +3 = 0x7F; +0xA = D_800E2294 - 100 + r % 201. */
void func_800CBFC4(int a0, int a1, pe_addr_t a2)
{
    int r;

    (void)a0; (void)a1;
    PE_StoreU8(0x800F3450u, (uint8_t)PE_LoadU32(func_800C2B10(0)));
    PE_StoreU8(0x800F3451u, (uint8_t)PE_LoadU32(func_800C2B10(1)));
    PE_StoreU8(0x800F3452u, (uint8_t)PE_LoadU32(func_800C2B10(2)));
    r = (int)func_80071A54() % 201;
    PE_StoreU16(a2 + 6u, (uint16_t)(PE_LoadU16(0x800E2290u) - 0x64 + r));
    PE_StoreU16(a2 + 8u, PE_LoadU16(0x800E2292u));
    r = (int)func_80071A54() % 201;
    PE_StoreU16(a2 + 4u, 0x20Cu);
    PE_StoreU8(a2 + 3u, 0x7Fu);
    PE_StoreU16(a2 + 0xAu, (uint16_t)(PE_LoadU16(0x800E2294u) - 0x64 + r));
}

/* src/func_800D629C.c: orbiting flame.  Mode 1: y = a1[5]; x/z on a circle
 * of radius D_800E21D4 about (D_800E21D8, D_800E21DC) at angle a1[4];
 * a1[5] -= 7; a1[4] += 2; 0x18 frames.  Mode 2: scale and fade over the
 * life, jittered y from D_800E21DA, frame k = (a1[6] + f/2) & 7. */
int func_800D629C(int a0, pe_addr_t a1)
{
    int s1v, s2v, r, k;
    unsigned int idx, w;

    PE_StoreU32(S7_C, PE_LoadU32(0x800C22C8u));
    switch (a0) {
    case 1:
        PE_StoreU16(a1 + 2u, PE_LoadU16(a1 + 0xAu));
        PE_StoreU16(a1 + 0u, (uint16_t)(PE_LoadU16(0x800E21D8u) +
            (int)PE_LoadU16(0x800E21D4u) * func_80077DC4((int16_t)PE_LoadU16(a1 + 8u)) / 4096));
        PE_StoreU16(a1 + 4u, (uint16_t)(PE_LoadU16(0x800E21DCu) +
            (int)PE_LoadU16(0x800E21D4u) * func_80077CF4((int16_t)PE_LoadU16(a1 + 8u)) / 4096));
        PE_StoreU16(a1 + 0xAu, (uint16_t)((int16_t)PE_LoadU16(a1 + 0xAu) - 7));
        PE_StoreU16(a1 + 8u, (uint16_t)((int16_t)PE_LoadU16(a1 + 8u) + 2));
        if (s7_frame() < 0x18)
            break;
        return 1;
    case 2:
        s1v = (s7_frame() << 11) / 24 + 0x800;
        s2v = 0x80 - (s7_frame() << 7) / 24;
        r = (int)func_80071A54();
        PE_StoreU16(a1 + 2u, (uint16_t)(PE_LoadU16(0x800E21DAu) - (r & 0x1F)));
        s1v = s1v * ((((int16_t)PE_LoadU16(a1 + 0xCu) + s7_frame() / 2) & 7) * 128 + 0x1000) / 4096;
        idx = PE_LoadU16(0x800F336Cu);
        w = PE_LoadU16(0x800E1204u + idx * 2u);
        if (idx == 4u && PE_LoadU32(0x800F3428u) != 0u)
            w += 4u;
        k = ((int16_t)PE_LoadU16(a1 + 0xCu) + s7_frame() / 2) & 7;
        func_800CEE20(a1, 0u, s1v * 2, s1v,
                      (int16_t)PE_LoadU16(0x800F336Au) * k + 0x80,
                      func_80077AA4(0x40, w) & 0xFFFFu, 1, s2v, S7_C);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D1AE0.c: full-screen tint.  Allocate a 16-byte TILE from the
 * D_800B0E58[D_8009CDDC] arena, colour = a0[0..2] * a1 / 128; when
 * (unsigned)a3 < 0x1000 size it 320x240 at (0,0) and link it into OT slot
 * a3 of D_800B0E58[D_8009CDDC - 8], preceded by an 8-byte draw-mode packet
 * (texpage abr a2) unless a2 == 0xFF. */
void func_800D1AE0(pe_addr_t a0, int a1, int a2, int a3)
{
    uint32_t cdd = PE_LoadU32(0x8009CDDCu);
    pe_addr_t p = PE_LoadU32(0x800B0E58u + cdd * 4u) + PE_LoadU32(0x8009CDD8u);
    pe_addr_t ot, q;

    PE_StoreU32(0x8009CDD8u, PE_LoadU32(0x8009CDD8u) + 0x10u);
    func_80077C44(p);
    PE_StoreU8(p + 4u, (uint8_t)((int)PE_LoadU8(a0 + 0u) * a1 / 128));
    PE_StoreU8(p + 5u, (uint8_t)((int)PE_LoadU8(a0 + 1u) * a1 / 128));
    PE_StoreU8(p + 6u, (uint8_t)((int)PE_LoadU8(a0 + 2u) * a1 / 128));
    if ((unsigned int)a3 >= 0x1000u)
        return;
    PE_StoreU16(p + 0xCu, 0x140u);
    PE_StoreU16(p + 8u, 0u);
    PE_StoreU16(p + 0xAu, 0u);
    PE_StoreU16(p + 0xEu, 0xF0u);
    ot = (pe_addr_t)(a3 * 4) + PE_LoadU32(0x800B0E58u + (cdd - 8u) * 4u);
    if (a2 != 0xFF) {
        uint32_t base = PE_LoadU32(0x8009CDD8u);

        PE_StoreU32(0x8009CDD8u, base + 8u);
        q = PE_LoadU32(0x800B0E58u + cdd * 4u) + base;
        (void)func_80077C84(q, 0u, 1u, func_80077A64(0u, (uint32_t)a2, 0u, 0u) & 0xFFFFu);
        if (p != 0u) {
            PE_StoreU8(p + 7u, (uint8_t)(PE_LoadU8(p + 7u) | 2u));
            PE_StoreU32(p, (PE_LoadU32(p) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
            PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (p & 0xFFFFFFu));
        }
        PE_StoreU32(q, (PE_LoadU32(q) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
        PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (q & 0xFFFFFFu));
    } else if (p != 0u) {
        PE_StoreU32(p, (PE_LoadU32(p) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
        PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (p & 0xFFFFFFu));
    }
}

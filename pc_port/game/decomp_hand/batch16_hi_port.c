/*
 * Hand adapters — batch 16 (>= 0x80060000): per-particle effect callbacks
 * (mode 1 = step, returns 1 when finished; mode 2 = draw through the
 * billboard drawer func_800CEE20).  Bodies follow the matched leaves
 * src/func_XXXXXXXX.c.  Retail stack locals handed to guest-address
 * callees live at PE_HAND_HI_STACK: position buf +0x00, angle vector +0x08,
 * colour block +0x10.  Stack words a leaf leaves unset stay unset.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern unsigned int func_80071A54(void);
extern int32_t func_80077CF4(int32_t angle);

#define FX_BUF (PE_HAND_HI_STACK + 0x00u)
#define FX_VEC (PE_HAND_HI_STACK + 0x08u)
#define FX_BLK (PE_HAND_HI_STACK + 0x10u)

static int fx_frame(void) { return (int)PE_LoadU32(0x800E27ECu); }
static int16_t fx_s16(pe_addr_t a1, unsigned i) { return (int16_t)PE_LoadU16(a1 + i * 2u); }
static void fx_set(pe_addr_t a1, unsigned i, int v) { PE_StoreU16(a1 + i * 2u, (uint16_t)v); }

/* Signed division with the retail `break 7` / `break 6` guards as a loud
 * boundary instead of a host SIGFPE. */
static int fx_div(int num, int den)
{
    if (den == 0 || (den == -1 && num == (int)0x80000000u)) {
        PE_D_COMP_BOUNDARY2("fx_divide_trap", 0u, num, den);
        return 0;
    }
    return num / den;
}

/* The stage CLUT word every leaf shares: D_800E1204[D_800F336C], + 4 on
 * stage 4 when D_800F3428 is set, through func_80077AA4(x, ·). */
static unsigned int fx_clut(int x)
{
    unsigned int idx = PE_LoadU16(0x800F336Cu);
    unsigned int w = PE_LoadU16(0x800E1204u + idx * 2u);

    if (idx == 4u && PE_LoadU32(0x800F3428u) != 0u)
        w += 4u;
    return func_80077AA4(x, w) & 0xFFFFu;
}

static void fx_copy_pos(pe_addr_t a1)
{
    PE_StoreU16(FX_BUF + 0u, PE_LoadU16(a1 + 0u));
    PE_StoreU16(FX_BUF + 2u, PE_LoadU16(a1 + 2u));
    PE_StoreU16(FX_BUF + 4u, PE_LoadU16(a1 + 4u));
}

static void fx_vec(int x, int y, int z)
{
    PE_StoreU16(FX_VEC + 0u, (uint16_t)x);
    PE_StoreU16(FX_VEC + 2u, (uint16_t)y);
    PE_StoreU16(FX_VEC + 4u, (uint16_t)z);
}

/* src/func_800D7A1C.c: rising spark (colour D_800C22DC, curve D_800E18C0,
 * texture 0x8B, 0x18 frames); the angle vector's pad is not set. */
int func_800D7A1C(int a0, pe_addr_t a1)
{
    PE_StoreU32(FX_BLK, PE_LoadU32(0x800C22DCu));
    switch (a0) {
    case 1: {
        unsigned short t = (unsigned short)(PE_LoadU16(a1 + 6u) - 1u);

        fx_set(a1, 1, PE_LoadU16(a1 + 2u) - PE_LoadU16(a1 + 6u));
        fx_set(a1, 3, t);
        if (fx_frame() >= 0x18)
            return 1;
        break;
    }
    case 2:
        func_800CF3AC(0x800E18C0u, FX_BLK, fx_frame());
        fx_vec(PE_LoadU16(a1 + 0u), PE_LoadU16(a1 + 2u), PE_LoadU16(a1 + 4u));
        func_800CEE20(FX_VEC, 0u, 0x1000, 0x1000, 0x8B, fx_clut(0x20), 1, 0x80, FX_BLK);
        break;
    }
    return 0;
}

/* src/func_800D7E78.c: shrinking puff (6 frames, scale 0x1000 - f*0x1000/6). */
int func_800D7E78(int a0, pe_addr_t a1)
{
    int s;

    switch (a0) {
    case 1:
        fx_set(a1, 1, fx_s16(a1, 1) - 1);
        if (fx_frame() < 6)
            break;
        return 1;
    case 2:
        fx_copy_pos(a1);
        func_800CF3AC(0x800E1988u, FX_BLK, fx_frame());
        s = 0x1000 - fx_div(fx_frame() << 12, 6);
        func_800CEE20(FX_BUF, 0u, s, s, 0x8A, fx_clut(0x70), 1, 0x80, FX_BLK);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D8D14.c: spinning shrinking puff (12 frames, vz = f * 64). */
int func_800D8D14(int a0, pe_addr_t a1)
{
    int s;

    switch (a0) {
    case 1:
        fx_set(a1, 1, fx_s16(a1, 1) - 3);
        if (fx_frame() < 0xC)
            break;
        return 1;
    case 2:
        fx_copy_pos(a1);
        fx_vec(0, 0, fx_frame() * 64);
        PE_StoreU16(FX_VEC + 6u, 0u);
        func_800CF3AC(0x800E1AA0u, FX_BLK, fx_frame());
        s = 0x1000 - fx_div(fx_frame() << 12, 12);
        func_800CEE20(FX_BUF, FX_VEC, s, s, 0x8A, fx_clut(0x70), 1, 0x80, FX_BLK);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DC910.c / 800DBCD8.c: jittering smoke (0x20 frames). */
int func_800DC910(int a0, pe_addr_t a1)
{
    int r, s;

    switch (a0) {
    case 1:
        r = (int)func_80071A54();
        fx_set(a1, 0, (unsigned short)(PE_LoadU16(a1 + 0u) - 3u) + (r & 7));
        r = (int)func_80071A54();
        fx_set(a1, 2, (unsigned short)(PE_LoadU16(a1 + 4u) - 3u) + (r & 7));
        r = (int)func_80071A54();
        fx_set(a1, 1, (unsigned short)(PE_LoadU16(a1 + 2u) - 2u) - (r & 3));
        if (fx_frame() < 0x20)
            break;
        return 1;
    case 2:
        PE_StoreU16(FX_VEC + 0u, 0u);
        PE_StoreU16(FX_VEC + 2u, 0u);
        PE_StoreU16(FX_VEC + 6u, 0u);
        PE_StoreU16(FX_VEC + 4u, (uint16_t)(fx_frame() * 24));
        func_800CF3AC(0x800E1EE8u, FX_BLK, fx_frame());
        s = (fx_frame() << 7) + 0x2000;
        func_800CEE20(a1, FX_VEC, s, s, 0x82, fx_clut(0x30), 1, 0x80, FX_BLK);
        break;
    default:
        return 0;
    }
    return 0;
}
int func_800DBCD8(int a0, pe_addr_t a1)
{
    int r, s;
    unsigned int t;

    switch (a0) {
    case 1:
        r = (int)func_80071A54();
        fx_set(a1, 0, (unsigned short)(PE_LoadU16(a1 + 0u) - 3u) + (r & 7));
        r = (int)func_80071A54();
        fx_set(a1, 2, (unsigned short)(PE_LoadU16(a1 + 4u) - 3u) + (r & 7));
        r = (int)func_80071A54();
        fx_set(a1, 1, fx_s16(a1, 1) + (r & 3));
        if (fx_frame() < 0x20)
            break;
        return 1;
    case 2:
        PE_StoreU16(FX_VEC + 0u, 0u);
        PE_StoreU16(FX_VEC + 2u, 0u);
        PE_StoreU16(FX_VEC + 6u, 0u);
        PE_StoreU16(FX_VEC + 4u, (uint16_t)(fx_frame() * 12));
        func_800CF3AC(0x800E1DA4u, FX_BLK, fx_frame());
        s = (fx_frame() << 7) + 0x1800;
        t = fx_clut(0xC0);
        func_800CEE20(a1, FX_VEC, s, s,
                      (int16_t)PE_LoadU16(0x800F336Au) * (fx_frame() / 8 + 2) + 0xC0,
                      t, 1, 0x80, FX_BLK);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D9E5C.c / 800DA780.c: arcing flare (0x18 frames; velocity
 * a1[3] decays by 2 / by 1 below frame 0x13), brightness from the sine
 * leaf, no colour block. */
int func_800D9E5C(int a0, pe_addr_t a1)
{
    int b;

    switch (a0) {
    case 1:
        fx_set(a1, 1, fx_s16(a1, 1) + fx_s16(a1, 3));
        if (fx_frame() < 0x13)
            fx_set(a1, 3, fx_s16(a1, 3) - 2);
        if (fx_frame() < 0x18)
            break;
        return 1;
    case 2:
        fx_copy_pos(a1);
        fx_vec(0, fx_frame() * 128, 0);
        PE_StoreU16(FX_VEC + 6u, 1u);
        b = fx_div(func_80077CF4(fx_div(fx_frame() << 11, 24)), 32);
        func_800CEE20(FX_BUF, FX_VEC, 0x1000, 0x1000, 0x88, fx_clut(0x60), 1, b, 0u);
        break;
    default:
        return 0;
    }
    return 0;
}
int func_800DA780(int a0, pe_addr_t a1)
{
    int b;

    switch (a0) {
    case 1:
        fx_set(a1, 1, fx_s16(a1, 1) + fx_s16(a1, 3));
        if (fx_frame() < 0x13)
            fx_set(a1, 3, fx_s16(a1, 3) - 1);
        if (fx_frame() < 0x18)
            break;
        return 1;
    case 2:
        fx_copy_pos(a1);
        fx_vec(0, 0, fx_s16(a1, 1) + fx_s16(a1, 0) + fx_frame() * 32);
        PE_StoreU16(FX_VEC + 6u, 0u);
        b = fx_div(func_80077CF4(fx_div(fx_frame() << 11, 24)), 128);
        func_800CEE20(FX_BUF, FX_VEC, 0x1000, 0x1000,
                      (int16_t)PE_LoadU16(0x800F336Au) * fx_div(fx_frame(), 6) + 0x60,
                      fx_clut(0x60), 1, b, 0u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D5CE4.c: drifting ember (0x18 frames; +3..+5 velocity,
 * gravity 2 + random bit on vy); the angle vector's pad is not set. */
int func_800D5CE4(int a0, pe_addr_t a1)
{
    int r1, s;

    switch (a0) {
    case 1:
        fx_set(a1, 0, PE_LoadU16(a1 + 0u) + PE_LoadU16(a1 + 6u));
        fx_set(a1, 1, PE_LoadU16(a1 + 2u) + PE_LoadU16(a1 + 8u));
        fx_set(a1, 2, PE_LoadU16(a1 + 4u) + PE_LoadU16(a1 + 0xAu));
        r1 = (int)(func_80071A54() & 1u);
        fx_set(a1, 4, PE_LoadU16(a1 + 8u) + 2 + r1);
        if (fx_frame() < 0x18)
            break;
        return 1;
    case 2:
        func_800CF3AC(0x800E1694u, FX_BLK, fx_frame());
        fx_copy_pos(a1);
        fx_vec(0, 0, PE_LoadU16(a1 + 0xCu) + fx_frame() * 32);
        s = func_80077CF4(fx_div(fx_frame() << 10, 24)) + 0x1000;
        func_800CEE20(FX_BUF, FX_VEC, s, s,
                      (int16_t)PE_LoadU16(0x800F336Au) * fx_div(fx_frame(), 3) + 0x80,
                      fx_clut(0x20), 1, 0x80, FX_BLK);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D6C58.c: bouncing debris — integrate +3..+5, damp vx/vz by
 * 31/32, bounce vy at the floor, gravity +3, life a1[6]; draw with colour
 * D_800C22D0 and scale (a1[7] & 0x7FF) + 0x400. */
int func_800D6C58(int a0, pe_addr_t a1)
{
    int s;

    PE_StoreU32(FX_BLK, PE_LoadU32(0x800C22D0u));
    switch (a0) {
    case 1:
        fx_set(a1, 0, fx_s16(a1, 0) + fx_s16(a1, 3));
        fx_set(a1, 1, fx_s16(a1, 1) + fx_s16(a1, 4));
        fx_set(a1, 2, fx_s16(a1, 2) + fx_s16(a1, 5));
        fx_set(a1, 3, fx_s16(a1, 3) * 31 / 32);
        fx_set(a1, 5, fx_s16(a1, 5) * 31 / 32);
        if (fx_s16(a1, 1) > 0)
            fx_set(a1, 4, -fx_s16(a1, 4));
        fx_set(a1, 4, fx_s16(a1, 4) + 3);
        if (fx_frame() < fx_s16(a1, 6))
            break;
        return 1;
    case 2:
        fx_copy_pos(a1);
        fx_vec(0, 0, (int)fx_s16(a1, 7) * 256 + fx_frame() * 128);
        s = (fx_s16(a1, 7) & 0x7FF) + 0x400;
        func_800CEE20(FX_BUF, FX_VEC, s, s, 0xBC, fx_clut(0x80), 0xFF, 0x80, FX_BLK);
        break;
    default:
        return 0;
    }
    return 0;
}

/*
 * Hand adapters — batch 21 (>= 0x80060000).  Bodies follow the matched
 * leaves src/func_XXXXXXXX.c; pointers are guest addresses.  Renderers with
 * no pc_port implementation stay loud boundaries (arguments past the
 * fourth are not representable in the boundary log and are noted).
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"
void func_800D27FC(int32_t a_x, uint32_t a_ang, uint32_t a_col, int32_t a_bright);   /* generated (gen_decomp_ports --e21) */
void func_800DB25C(uint32_t a_position, int32_t a_x, int32_t a_y, int32_t a_angle, int32_t a_radius,
                  int32_t a_depth, int32_t a_intensity, uint32_t a_color);   /* generated (gen_decomp_ports --e21) */

extern int32_t func_80077CF4(int32_t angle);
extern int32_t func_80077DC4(int32_t angle);

#define S21_BUF (PE_HAND_HI_STACK + 0x00u)

static int s21_div(int num, int den)
{
    if (den == 0 || (den == -1 && num == (int)0x80000000u)) {
        PE_D_COMP_BOUNDARY2("fx_divide_trap", 0u, num, den);
        return 0;
    }
    return num / den;
}

/* src/func_800679C4.c: camera pan with clamping.  From the camera block at
 * D_800B1624: nx = +0x28 + dx clamped to [+0x30, +0x32] (on the s16 value),
 * ny = +0x2A + dy clamped to [+0x34, +0x36], na = +0x24 + dz; results into
 * +0x2C / +0x2E / +0x26 (halfword stores); returns 0. */
/* func_800679C4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800679C4_port.c (src/func_800679C4.c); hand port retired (port3 switch-over C). */

/* src/func_800DAB98.c: swaying wisp.  Mode 1: x = a1[1] * cos((f << 10) /
 * a1[3]) / 4096, z += 0x10, life a1[3].  Mode 2: colour curve D_800E1C04
 * at f, drawn by func_800D27FC(x, z, colour, 0x80, 1); the fifth argument is
 * unread by the generated func_800D27FC (four parameters). */
int func_800DAB98(int a0, pe_addr_t a1)
{
    int f = (int)PE_LoadU32(0x800E27ECu);
    int v;

    switch (a0) {
    case 1:
        v = func_80077DC4(s21_div(f << 10, (int16_t)PE_LoadU16(a1 + 6u)));
        PE_StoreU16(a1 + 0u, (uint16_t)((int16_t)PE_LoadU16(a1 + 2u) * v / 4096));
        PE_StoreU16(a1 + 4u, (uint16_t)((int16_t)PE_LoadU16(a1 + 4u) + 0x10));
        if (f < (int16_t)PE_LoadU16(a1 + 6u))
            break;
        return 1;
    case 2:
        func_800CF3AC(0x800E1C04u, S21_BUF, f);
        func_800D27FC((int16_t)PE_LoadU16(a1 + 0u),
                      (uint32_t)(int32_t)(int16_t)PE_LoadU16(a1 + 4u), S21_BUF, 0x80);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DAF8C.c: spiral flare.  Mode 1: y += a1[7], z += 8,
 * a1[0] = A - A * f / 64 (A = a1[6]), a1[4] = sin(f * 32) / 64,
 * a1[5] = 0x640 - f * 1000 / 64; 0x40 frames.  Mode 2: colour curve
 * D_800E1C2C, drawn by func_800D0E88(D_800E221C, a1, a1[5], a1[4], buf,
 * 0, 0, 0x80, 1) (native port, func_800D0E88_port.c). */
int func_800DAF8C(int a0, pe_addr_t a1)
{
    int f = (int)PE_LoadU32(0x800E27ECu);
    int A;

    switch (a0) {
    case 1:
        A = (int16_t)PE_LoadU16(a1 + 0xCu);
        PE_StoreU16(a1 + 2u, (uint16_t)((int16_t)PE_LoadU16(a1 + 2u) + (int16_t)PE_LoadU16(a1 + 0xEu)));
        PE_StoreU16(a1 + 4u, (uint16_t)((int16_t)PE_LoadU16(a1 + 4u) + 8));
        PE_StoreU16(a1 + 0u, (uint16_t)(A - A * f / 64));
        PE_StoreU16(a1 + 8u, (uint16_t)(func_80077CF4(f * 32) / 64));
        PE_StoreU16(a1 + 0xAu, (uint16_t)(0x640 - (int)PE_LoadU32(0x800E27ECu) * 1000 / 64));
        if ((int)PE_LoadU32(0x800E27ECu) < 0x40)
            break;
        return 1;
    case 2:
        func_800CF3AC(0x800E1C2Cu, S21_BUF, f);
        func_800D0E88(0x800E221Cu, a1, (int16_t)PE_LoadU16(a1 + 0xAu),
                      (int16_t)PE_LoadU16(a1 + 8u), S21_BUF, 0, 0, 0x80, 1);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DB5F4.c: short burst.  Mode 1 ends at frame 8.  Mode 2:
 * colour curve D_800E1CC8 at a1[5], drawn by func_800DB25C(D_800E2224,
 * a1[0..3], 1, 0x80 - (f << 4), colour), generated from its matched C. */
int func_800DB5F4(int a0, pe_addr_t a1)
{
    int f = (int)PE_LoadU32(0x800E27ECu);

    switch (a0) {
    case 1:
        if (f < 8)
            break;
        return 1;
    case 2: {
        /* src/func_800DB5F4.c: v = 0x80 - (D_800E27EC << 4);
         * func_800CF3AC(&D_800E1CC8, buf, a1[5]);
         * func_800DB25C(&D_800E2224, a1[0], a1[1], a1[2], a1[3], 1, v, buf); */
        int32_t v = 0x80 - (int32_t)((uint32_t)f << 4);
        func_800CF3AC(0x800E1CC8u, S21_BUF, (int16_t)PE_LoadU16(a1 + 0xAu));
        func_800DB25C(0x800E2224u, (int16_t)PE_LoadU16(a1 + 0u), (int16_t)PE_LoadU16(a1 + 2u),
                      (int16_t)PE_LoadU16(a1 + 4u), (int16_t)PE_LoadU16(a1 + 6u), 1, v, S21_BUF);
        break;
    }
    default:
        return 0;
    }
    return 0;
}

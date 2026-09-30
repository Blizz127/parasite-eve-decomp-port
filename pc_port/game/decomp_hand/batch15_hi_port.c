/*
 * Hand adapters — batch 15 (>= 0x80060000).  Bodies follow the matched
 * leaves src/func_XXXXXXXX.c; pointers are guest addresses; retail stack
 * locals passed to guest-address callees live at PE_HAND_HI_STACK.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern int func_800C653C(int arg0, int arg1);

/* src/func_800CF4B4.c: CLUT row upload.  RECT (x = (a0 & 0xF) << 4,
 * y = D_800E1204[D_800F336C] + a0 / 16 (signed), 16 x 1).  a1 == -1 hands
 * a2 straight to func_800750CC; otherwise entry 0 is kept and entries
 * 1..15 are rotated (dest (a1 % 15) + 1, a1 counting up) into D_800E21A8,
 * which func_8007506C uploads. */
void func_800CF4B4(int a0, int a1, pe_addr_t a2)
{
    RECT r;
    unsigned short i;
    const pe_addr_t buf = 0x800E21A8u;

    r.x = (int16_t)((a0 & 0xF) << 4);
    r.y = (int16_t)(PE_LoadU16(0x800E1204u + PE_LoadU16(0x800F336Cu) * 2u) + a0 / 16);
    r.w = 16;
    r.h = 1;
    if (a1 == -1) {
        (void)func_800750CC(&r, a2);
        return;
    }
    PE_StoreU16(buf, PE_LoadU16(a2));
    for (i = 1; i < 16; i++) {
        PE_StoreU16(buf + (uint32_t)(a1 % 15 + 1) * 2u, PE_LoadU16(a2 + i * 2u));
        a1++;
    }
    (void)func_8007506C(&r, buf);
}

/* src/func_800C61A8.c: the four corners at D_800F3310 (8-byte SVECTORs)
 * through the a1 matrix; vx += a1->+0x14, vy = 0, vz += a1->+0x1C; then
 * func_800C653C(a0, corners) != 0. */
int func_800C61A8(pe_addr_t a0, pe_addr_t a1)
{
    const pe_addr_t v = PE_HAND_HI_STACK + 0x00u;   /* SV4 v[4] */
    unsigned int i;

    for (i = 0; i < 4u; i++)
        (void)func_80078C34(a1, 0x800F3310u + i * 8u, v + i * 8u);
    for (i = 0; i < 4u; i++)
        PE_StoreU16(v + i * 8u + 0u, (uint16_t)(PE_LoadU16(v + i * 8u + 0u) + PE_LoadU32(a1 + 0x14u)));
    for (i = 0; i < 4u; i++)
        PE_StoreU16(v + i * 8u + 2u, 0u);
    for (i = 0; i < 4u; i++)
        PE_StoreU16(v + i * 8u + 4u, (uint16_t)(PE_LoadU16(v + i * 8u + 4u) + PE_LoadU32(a1 + 0x1Cu)));
    return func_800C653C((int)a0, (int)v) != 0;
}

/* src/func_800CF6F8.c: ordering-table insert.  a2 != 0xFF allocates an
 * 8-byte draw-mode packet from the D_800B0E58[D_8009CDDC] + D_8009CDD8
 * arena (texpage abr a2), links a1 (flag bit 1 of byte 7) then the packet
 * in front of *a0; a2 == 0xFF links only a1. */
void func_800CF6F8(pe_addr_t a0, pe_addr_t a1, int a2)
{
    if (a2 != 0xFF) {
        uint32_t base = PE_LoadU32(0x8009CDD8u);
        pe_addr_t p = PE_LoadU32(0x800B0E58u + PE_LoadU32(0x8009CDDCu) * 4u) + base;

        PE_StoreU32(0x8009CDD8u, base + 8u);
        (void)func_80077C84(p, 0u, 1u,
                            func_80077A64(0u, (uint32_t)a2, 0u, 0u) & 0xFFFFu);
        if (a1 != 0u) {
            PE_StoreU8(a1 + 7u, (uint8_t)(PE_LoadU8(a1 + 7u) | 2u));
            PE_StoreU32(a1, (PE_LoadU32(a1) & 0xFF000000u) | (PE_LoadU32(a0) & 0xFFFFFFu));
            PE_StoreU32(a0, (PE_LoadU32(a0) & 0xFF000000u) | (a1 & 0xFFFFFFu));
        }
        PE_StoreU32(p, (PE_LoadU32(p) & 0xFF000000u) | (PE_LoadU32(a0) & 0xFFFFFFu));
        PE_StoreU32(a0, (PE_LoadU32(a0) & 0xFF000000u) | (p & 0xFFFFFFu));
    } else if (a1 != 0u) {
        PE_StoreU32(a1, (PE_LoadU32(a1) & 0xFF000000u) | (PE_LoadU32(a0) & 0xFFFFFFu));
        PE_StoreU32(a0, (PE_LoadU32(a0) & 0xFF000000u) | (a1 & 0xFFFFFFu));
    }
}

/* Draw-parameter block { u8 r, g, b, pad; u8 f4, f5, f6, pad; s16 f8, fA }
 * (f6 untouched here, unlike the func_800C7BA0 family). */
static void param_block5(pe_addr_t s, uint8_t f4, uint8_t f5, int16_t f8,
                         int16_t fA, uint8_t r, uint8_t g, uint8_t b)
{
    PE_StoreU8(s + 4u, f4);
    PE_StoreU8(s + 5u, f5);
    PE_StoreU16(s + 8u, (uint16_t)f8);
    PE_StoreU16(s + 0xAu, (uint16_t)fA);
    PE_StoreU8(s + 0u, r);
    PE_StoreU8(s + 1u, g);
    PE_StoreU8(s + 2u, b);
}

/* Identity rotation + zero translation in a global MATRIX, then scale it
 * by a copy (stack) of the 16-byte template VECTOR. */
static void scaled_identity(pe_addr_t mat, pe_addr_t tmpl, pe_addr_t stack)
{
    unsigned int i;

    for (i = 0; i < 16u; i += 4u)
        PE_StoreU32(stack + i, PE_LoadU32(tmpl + i));
    for (i = 0; i < 9u; i++)
        PE_StoreU16(mat + i * 2u, (i % 4u == 0u) ? 0x1000u : 0u);
    for (i = 0; i < 3u; i++)
        PE_StoreU32(mat + 0x14u + i * 4u, 0u);
    (void)func_80078CC4(mat, stack);
}

/* src/func_800CCBA8.c: func_800C22F8 slot reset with the caller's live $a0
 * (0x800CCBA8: `addiu sp / sw ra / jal 0x800c22f8`), script D_800E0EB8,
 * three scaled identity matrices (D_800F3478 / D_800F33C0 / D_800F32B0 by
 * the D_800C220C / 21C / 22C templates) and four parameter blocks. */
int func_800CCBA8(pe_addr_t a0)
{
    PE_StoreU32(func_800C22F8(a0), 0x800E0EB8u);
    scaled_identity(0x800F3478u, 0x800C220Cu, PE_HAND_HI_STACK + 0x00u);
    scaled_identity(0x800F33C0u, 0x800C221Cu, PE_HAND_HI_STACK + 0x10u);
    scaled_identity(0x800F32B0u, 0x800C222Cu, PE_HAND_HI_STACK + 0x20u);
    param_block5(0x800E2298u, 0x42, 0x20, 0x32, 0x7F, 0x80, 0x80, 0x80);
    param_block5(0x800E2250u, 0x80, 0x4, -0x32, 0x7F, 0x80, 0x80, 0x80);
    param_block5(0x800E27E0u, 0x68, 0x0, -0x63, 0x7F, 0x40, 0x80, 0x40);
    param_block5(0x800F3460u, 0x6E, 0x3, -0x64, 0x7F, 0x80, 0x80, 0x80);
    return 0;
}

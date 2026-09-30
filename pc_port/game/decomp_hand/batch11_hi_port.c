/*
 * Hand adapters — batch 11 (>= 0x80060000): overlay sprite emitters built
 * from RotMatrix / ScaleMatrix (func_800794C4 / func_80078CC4) and the
 * sprite drawer func_800C42A4.  Bodies follow the matched leaves
 * src/func_XXXXXXXX.c; retail stack MATRIX / SVECTOR / VECTOR locals live at
 * PE_HAND_HI_STACK.  Each leaf zeroes its scale VECTOR with BIOS A(2Bh)
 * (func_80071A44 — a `jr t2` trampoline with no matched C: loud boundary)
 * before setting vx/vy/vz; the pad word is never read by func_80078CC4.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern unsigned int func_80071A54(void);

#define HB_MAT (PE_HAND_HI_STACK + 0x40u)   /* MATRIX (0x20 bytes)        */
#define HB_ROT (PE_HAND_HI_STACK + 0x20u)   /* SVECTOR (8 bytes)          */
#define HB_VEC (PE_HAND_HI_STACK + 0x60u)   /* VECTOR (0x10 bytes)        */

/* The shared tail: translation from a2 +8/+A/+C, scale s on all axes,
 * then func_800C42A4(style, &m, billboard). */
static void scaled_emit(pe_addr_t a2, int s, pe_addr_t style, unsigned billboard)
{
    PE_StoreU32(HB_MAT + 0x14u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(a2 + 8u));
    PE_StoreU32(HB_MAT + 0x18u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(a2 + 0xAu));
    PE_StoreU32(HB_MAT + 0x1Cu, (uint32_t)(int32_t)(int16_t)PE_LoadU16(a2 + 0xCu));
    func_80071A44(HB_VEC, 0, 0x10);
    PE_StoreU32(HB_VEC + 0u, (uint32_t)s);
    PE_StoreU32(HB_VEC + 4u, (uint32_t)s);
    PE_StoreU32(HB_VEC + 8u, (uint32_t)s);
    (void)func_80078CC4(HB_MAT, HB_VEC);
    func_800C42A4(style, HB_MAT, billboard);
}

/* src/func_800C8F94.c: the D_800E27A0 twin of func_800C7E50. */
void func_800C8F94(int a0, int a1, pe_addr_t a2)
{
    const pe_addr_t v = HB_ROT;

    (void)a0; (void)a1;
    PE_StoreU16(v + 0u, (uint16_t)(-((int)func_80071A54() % 3 + 9)));
    PE_StoreU16(v + 2u, (uint16_t)(-((int)func_80071A54() % 3 + 9)));
    PE_StoreU16(v + 4u, (uint16_t)((int)func_80071A54() % 5 - 2));
    (void)func_80078C34(PE_LoadU32(PE_LoadU32(0x800E27A0u) + 0x238u), v, a2 + 0x10u);
    PE_StoreU16(a2 + 8u, PE_LoadU16(0x800E2350u));
    PE_StoreU16(a2 + 0xAu, PE_LoadU16(0x800E2352u));
    PE_StoreU16(a2 + 0xCu, PE_LoadU16(0x800E2354u));
    PE_StoreU8(a2 + 2u, 0x14u);
    PE_StoreU8(a2 + 1u, 0u);
}

/* src/func_800C8970.c (+ twins 800CB8E0 / 800CD2EC): rotation from the
 * 8-byte SVECTOR template `rot_tmpl`; draw state (page 3, 0x100 colours,
 * 32x32, blend 2); *g = a2->+4 where g is the style's +0xA halfword; scale
 * (short)a2->+6; emit with the style at g - 0xA, billboard 0. */
static void c8970_emit(pe_addr_t a2, pe_addr_t rot_tmpl, pe_addr_t g)
{
    PE_StoreU32(HB_ROT + 0u, PE_LoadU32(rot_tmpl + 0u));
    PE_StoreU32(HB_ROT + 4u, PE_LoadU32(rot_tmpl + 4u));
    func_800C2EAC(3);
    func_800C3098(0x100);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    PE_StoreU16(g, PE_LoadU16(a2 + 4u));
    func_800794C4(HB_ROT, HB_MAT);
    scaled_emit(a2, (int16_t)PE_LoadU16(a2 + 6u), g - 0xAu, 0);
}
void func_800C8970(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8970_emit(a2, 0x800C2174u, 0x800F34E2u); }
void func_800CB8E0(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8970_emit(a2, 0x800C2204u, 0x800F34F2u); }
void func_800CD2EC(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8970_emit(a2, 0x800C223Cu, 0x800E22A2u); }

/* src/func_800C8A88.c (+ twin 800CB9F8): as func_800C8970 but the
 * rotation part of the MATRIX is copied from a2 + 0x10 (whole struct copy;
 * the translation is then overwritten). */
static void c8a88_emit(pe_addr_t a2, pe_addr_t g)
{
    unsigned int i;

    func_800C2EAC(3);
    func_800C3098(0x100);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    PE_StoreU16(g, PE_LoadU16(a2 + 4u));
    for (i = 0; i < 0x20u; i += 4u)
        PE_StoreU32(HB_MAT + i, PE_LoadU32(a2 + 0x10u + i));
    scaled_emit(a2, (int16_t)PE_LoadU16(a2 + 6u), g - 0xAu, 0);
}
void func_800C8A88(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8a88_emit(a2, 0x800F34E2u); }
void func_800CB9F8(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8a88_emit(a2, 0x800F34F2u); }

/* src/func_800C8270.c (+ twins 800C9268 / 800CACDC): rotation (0, 0,
 * (s8)a2[1] << 6); draw state (page 3, 0x10, 16x16, blend 0); scale from
 * the short table entry n - 1, n = (*D_8009D254)->+0x68->+6; emit with the
 * fixed style, billboard 1. */
static void c8270_emit(pe_addr_t a2, pe_addr_t scale_tab, pe_addr_t style)
{
    pe_addr_t q = PE_LoadU32(PE_LoadU32(PE_LoadU32(0x8009D254u)) + 0x68u);
    short n = (short)PE_LoadU16(q + 6u);

    PE_StoreU16(HB_ROT + 0u, 0u);
    PE_StoreU16(HB_ROT + 2u, 0u);
    n = (short)(n - 1);
    PE_StoreU16(HB_ROT + 4u, (uint16_t)((signed char)PE_LoadU8(a2 + 1u) << 6));
    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x10, 0x10);
    func_800C3238(0);
    func_800794C4(HB_ROT, HB_MAT);
    scaled_emit(a2, (int16_t)PE_LoadU16(scale_tab + (uint32_t)((int)n * 2)), style, 1);
}
void func_800C8270(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8270_emit(a2, 0x800E0888u, 0x800E22D8u); }
void func_800C9268(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8270_emit(a2, 0x800E09F0u, 0x800E22E8u); }
void func_800CACDC(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8270_emit(a2, 0x800E0BE8u, 0x800E2308u); }

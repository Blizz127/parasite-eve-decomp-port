/*
 * Hand adapters — batch 10 (>= 0x80060000): overlay effect emitters that
 * transform template vectors through the owner's matrix (func_80078C34)
 * and publish the sprite record.  Bodies follow the matched leaves
 * src/func_XXXXXXXX.c; retail stack buffers passed to guest-address
 * callees live at PE_HAND_HI_STACK.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern unsigned int func_80071A54(void);

#define HB_BUF (PE_HAND_HI_STACK + 0x20u)   /* 3-halfword vector (+pad)   */
#define HB_MAT (PE_HAND_HI_STACK + 0x40u)   /* MATRIX: 3x3 s16 + pad + 3 s32 */
#define HB_VEC (PE_HAND_HI_STACK + 0x60u)   /* VECTOR: 4 x s32            */

/* src/func_800C7E50.c: a random local offset vector
 * (-(r%3+9), -(r%3+9), r%5-2) — signed int remainders — transformed by the
 * matrix at D_800E279C->+0x238 into a2 + 0x10; base position from
 * D_800E2348/4A/4C into +8/+A/+C; +2 = 0x14 frames, +1 = 0. */
void func_800C7E50(int a0, int a1, pe_addr_t a2)
{
    (void)a0; (void)a1;
    PE_StoreU16(HB_BUF + 0u, (uint16_t)(-((int)func_80071A54() % 3 + 9)));
    PE_StoreU16(HB_BUF + 2u, (uint16_t)(-((int)func_80071A54() % 3 + 9)));
    PE_StoreU16(HB_BUF + 4u, (uint16_t)((int)func_80071A54() % 5 - 2));
    (void)func_80078C34(PE_LoadU32(PE_LoadU32(0x800E279Cu) + 0x238u), HB_BUF, a2 + 0x10u);
    PE_StoreU16(a2 + 8u, PE_LoadU16(0x800E2348u));
    PE_StoreU16(a2 + 0xAu, PE_LoadU16(0x800E234Au));
    PE_StoreU16(a2 + 0xCu, PE_LoadU16(0x800E234Cu));
    PE_StoreU8(a2 + 2u, 0x14u);
    PE_StoreU8(a2 + 1u, 0u);
}

/* src/func_800C7F60.c (+ twins 800C90A4 / 800CA934, and 800C815C /
 * 800CABC8 which also set +6 = 0x224): vertex n - 1 of the template table
 * (n = (*D_8009D254)->+0x68->+6, 8-byte entries) transformed by the matrix
 * at owner->+0x238 + 0x260; +8/+A/+C = base + result; the 8 words at
 * owner->+0x238 + 0x260 copied to +0x10; +4 = 0x7F. */
static void c7f60_emit(pe_addr_t a2, pe_addr_t table, pe_addr_t owner_slot,
                       pe_addr_t gx, pe_addr_t gy, pe_addr_t gz, int set_f6)
{
    pe_addr_t p = PE_LoadU32(PE_LoadU32(0x8009D254u));
    pe_addr_t q = PE_LoadU32(p + 0x68u);
    unsigned short n = PE_LoadU16(q + 6u);
    pe_addr_t inner;
    unsigned int i;

    (void)func_80078C34(PE_LoadU32(PE_LoadU32(owner_slot) + 0x238u) + 0x260u,
                        table + (uint32_t)((int)(short)(n - 1) * 8), HB_BUF);
    PE_StoreU16(a2 + 8u, (uint16_t)(PE_LoadU16(gx) + PE_LoadU16(HB_BUF + 0u)));
    PE_StoreU16(a2 + 0xAu, (uint16_t)(PE_LoadU16(gy) + PE_LoadU16(HB_BUF + 2u)));
    PE_StoreU16(a2 + 0xCu, (uint16_t)(PE_LoadU16(gz) + PE_LoadU16(HB_BUF + 4u)));
    inner = PE_LoadU32(PE_LoadU32(owner_slot) + 0x238u);
    for (i = 0; i < 8u; i++)
        PE_StoreU32(a2 + 0x10u + i * 4u, PE_LoadU32(inner + 0x260u + i * 4u));
    PE_StoreU16(a2 + 4u, 0x7Fu);
    if (set_f6)
        PE_StoreU16(a2 + 6u, 0x224u);
}
void func_800C7F60(int a0, int a1, pe_addr_t a2)
{
    (void)a0; (void)a1;
    c7f60_emit(a2, 0x800E08A8u, 0x800E279Cu, 0x800E2348u, 0x800E234Au, 0x800E234Cu, 0);
}
void func_800C90A4(int a0, int a1, pe_addr_t a2)
{
    (void)a0; (void)a1;
    c7f60_emit(a2, 0x800E0A10u, 0x800E27A0u, 0x800E2350u, 0x800E2352u, 0x800E2354u, 0);
}
void func_800CA934(int a0, int a1, pe_addr_t a2)
{
    (void)a0; (void)a1;
    c7f60_emit(a2, 0x800E0C08u, 0x800E27A8u, 0x800E2360u, 0x800E2362u, 0x800E2364u, 0);
}
void func_800C815C(int a0, int a1, pe_addr_t a2)
{
    (void)a0; (void)a1;
    c7f60_emit(a2, 0x800E08E8u, 0x800E279Cu, 0x800E2348u, 0x800E234Au, 0x800E234Cu, 1);
}
void func_800CABC8(int a0, int a1, pe_addr_t a2)
{
    (void)a0; (void)a1;
    c7f60_emit(a2, 0x800E0C48u, 0x800E27A8u, 0x800E2360u, 0x800E2362u, 0x800E2364u, 1);
}

/* src/func_800C8D34.c: reset the effect slot via func_800C22F8 — called
 * bare, retail forwards the caller's live $a0 (0x800C8D34: `jal
 * 0x800c22f8` with only sp/ra touched) — store the D_800E0A50 script in
 * its first word, then seed the three draw-parameter blocks. */
int func_800C8D34(pe_addr_t a0)
{
    PE_StoreU32(func_800C22F8(a0), 0x800E0A50u);
    PE_StoreU8(0x800E22ECu, 0xBDu);
    PE_StoreU8(0x800E22EDu, 9u);
    PE_StoreU16(0x800E22F0u, 0u);
    PE_StoreU16(0x800E22F2u, 0x80u);
    PE_StoreU8(0x800E22E8u, 0x80u);
    PE_StoreU8(0x800E22E9u, 0x80u);
    PE_StoreU8(0x800E22EAu, 0x80u);
    PE_StoreU8(0x800F34ACu, 0xAEu);
    PE_StoreU8(0x800F34ADu, 7u);
    PE_StoreU16(0x800F34B0u, (uint16_t)-0x32);
    PE_StoreU8(0x800E22EEu, 0u);
    PE_StoreU16(0x800F34B2u, 0x80u);
    PE_StoreU8(0x800F34A8u, 0x50u);
    PE_StoreU8(0x800F34A9u, 0x50u);
    PE_StoreU8(0x800F34AAu, 0x50u);
    PE_StoreU8(0x800F34AEu, 0u);
    PE_StoreU8(0x800E232Cu, 0x68u);
    PE_StoreU8(0x800E232Du, 0u);
    PE_StoreU16(0x800E2330u, 0u);
    PE_StoreU16(0x800E2332u, 0x80u);
    PE_StoreU8(0x800E2328u, 0x50u);
    PE_StoreU8(0x800E2329u, 0x50u);
    PE_StoreU8(0x800E232Au, 0x50u);
    PE_StoreU8(0x800E232Eu, 0u);
    return 0;
}

/* src/func_800C8870.c (+ twin 800C9868 with D_800E2328): draw state (page
 * 3, 0x10, 32x32, blend 2); identity rotation with translation a2 +8/+A/+C
 * (s16); scale s = (short)a2->+6 + 0x170 on all axes via func_80078CC4;
 * style [5] = a2->+4; emit func_800C42A4(style, &m, 1).  The leaf zeroes
 * the scale VECTOR with BIOS A(2Bh) (func_80071A44, a `jr t2` trampoline
 * with no matched C: loud boundary) and then sets vx/vy/vz; its pad word
 * is not read by func_80078CC4. */
static void c8870_emit(pe_addr_t a2, pe_addr_t style)
{
    int s;
    unsigned short u;
    unsigned int i;

    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    s = (int16_t)PE_LoadU16(a2 + 6u) + 0x170;
    u = PE_LoadU16(a2 + 4u);
    for (i = 0; i < 9u; i++)
        PE_StoreU16(HB_MAT + i * 2u, (i % 4u == 0u) ? 0x1000u : 0u);
    PE_StoreU16(style + 10u, u);
    PE_StoreU32(HB_MAT + 0x14u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(a2 + 8u));
    PE_StoreU32(HB_MAT + 0x18u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(a2 + 0xAu));
    PE_StoreU32(HB_MAT + 0x1Cu, (uint32_t)(int32_t)(int16_t)PE_LoadU16(a2 + 0xCu));
    func_80071A44(HB_VEC, 0, 0x10);
    PE_StoreU32(HB_VEC + 0u, (uint32_t)s);
    PE_StoreU32(HB_VEC + 4u, (uint32_t)s);
    PE_StoreU32(HB_VEC + 8u, (uint32_t)s);
    (void)func_80078CC4(HB_MAT, HB_VEC);
    func_800C42A4(style, HB_MAT, 1);
}
void func_800C8870(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8870_emit(a2, 0x800E2318u); }
void func_800C9868(int a0, int a1, pe_addr_t a2) { (void)a0; (void)a1; c8870_emit(a2, 0x800E2328u); }

/* Draw-parameter block of src/func_800C7BA0.c / 800CA574.c: struct
 * { u8 r, g, b, pad; u8 f4, f5, f6, pad; s16 f8, fA; } — f6 cleared. */
static void param_block(pe_addr_t s, uint8_t f4, uint8_t f5, int16_t f8,
                        int16_t fA, uint8_t rgb)
{
    PE_StoreU8(s + 4u, f4);
    PE_StoreU8(s + 5u, f5);
    PE_StoreU16(s + 8u, (uint16_t)f8);
    PE_StoreU16(s + 0xAu, (uint16_t)fA);
    PE_StoreU8(s + 0u, rgb);
    PE_StoreU8(s + 1u, rgb);
    PE_StoreU8(s + 2u, rgb);
    PE_StoreU8(s + 6u, 0u);
}

/* src/func_800C7BA0.c / 800CA574.c: func_800C22F8 slot reset with the
 * caller's live $a0 (both begin `addiu sp / sw ra / jal 0x800c22f8`),
 * script pointer, then four parameter blocks. */
static int c7ba0_init(pe_addr_t a0, pe_addr_t script, pe_addr_t b0,
                      pe_addr_t b1, pe_addr_t b2, pe_addr_t b3)
{
    PE_StoreU32(func_800C22F8(a0), script);
    param_block(b0, 0xBD, 0x9, 0, 0x80, 0x80);
    param_block(b1, 0xAE, 0x7, -0x32, 0x80, 0x50);
    param_block(b2, 0x68, 0x0, -0x3C, 0x80, 0x50);
    param_block(b3, 0x42, 0x20, 0x32, 0x80, 0x80);
    return 0;
}
int func_800C7BA0(pe_addr_t a0)
{
    return c7ba0_init(a0, 0x800E0928u, 0x800E22D8u, 0x800F3498u, 0x800E2318u, 0x800F34D8u);
}
int func_800CA574(pe_addr_t a0)
{
    return c7ba0_init(a0, 0x800E0C88u, 0x800E2308u, 0x800F34C8u, 0x800E2338u, 0x800F34E8u);
}

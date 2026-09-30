/*
 * Hand adapters — batch 14 (>= 0x80060000).  Bodies follow the matched
 * leaves src/func_XXXXXXXX.c; pointers are guest addresses; retail stack
 * MATRIX / VECTOR locals live at PE_HAND_HI_STACK.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern unsigned int func_80071A54(void);
extern void func_8008F0D0(pe_addr_t pe_a0, pe_addr_t pe_a1, int a2);

/* Shared tail of src/func_8008E4E8.c / 8008E664.c: skip when the voice's
 * current program (+0x5A) already equals entry c (or c + adj for c >=
 * 0x20); otherwise select it — program p[0] (+adj), instrument record
 * D_800B2900 + prog * 0x40 into func_8008F0D0 with its first word, and the
 * +0x10E/+0x114/+0x104/+0x116 parameters from p[3..6], flag 0x4000. */
static int prog_is_current(pe_addr_t a0, unsigned int c, unsigned int adj)
{
    unsigned int cur = PE_LoadU16(a0 + 0x5Au);

    return (c < 0x20u) ? cur == c : cur == c + adj;
}
static void prog_select(pe_addr_t a0, pe_addr_t p, unsigned int adj)
{
    unsigned int e = PE_LoadU8(p);
    unsigned int v = e & 0xFFu;
    unsigned int t6;

    if (e >= 0x20u)
        v += adj;
    PE_StoreU16(a0 + 0x5Au, (uint16_t)v);
    v *= 0x40u;
    func_8008F0D0(a0, 0x800B2900u + v, (int)PE_LoadU32(0x800B2900u + v));
    PE_StoreU16(a0 + 0x10Eu, PE_LoadU8(p + 3u));
    PE_StoreU16(a0 + 0x114u, PE_LoadU8(p + 4u));
    PE_StoreU32(a0 + 0x104u, PE_LoadU8(p + 5u));
    t6 = PE_LoadU8(p + 6u);
    PE_StoreU32(a0 + 0xF4u, PE_LoadU32(a0 + 0xF4u) | 0x4000u);
    PE_StoreU16(a0 + 0x116u, (uint16_t)t6);
}

/* src/func_8008E4E8.c: key split by upper bound — count the 8-byte zone
 * entries at a0->+0x18 (terminated by a first byte >= 0x80), pick the
 * first whose p[2] >= a1 (else the one past the last), and skip when it
 * or its successor is already current. */
void func_8008E4E8(pe_addr_t a0, unsigned int a1)
{
    pe_addr_t p = PE_LoadU32(a0 + 0x18u);
    unsigned int cnt = 1, i = 0;
    unsigned int adj = (PE_LoadU32(PE_LoadU32(0x8009D2C8u)) & 0x100u) ? 0x30u : 0u;

    if (PE_LoadU8(p + 8u) < 0x80u) {
        do {
            p += 8u;
            cnt++;
        } while (PE_LoadU8(p + 8u) < 0x80u);
        p = PE_LoadU32(a0 + 0x18u);
    }
    if (cnt != 0) {
        while (PE_LoadU8(p + 2u) < a1) {
            i++;
            p += 8u;
            if (i >= cnt)
                break;
        }
    }
    if (prog_is_current(a0, PE_LoadU8(p), adj))
        return;
    if (prog_is_current(a0, PE_LoadU8(p + 8u), adj))
        return;
    prog_select(a0, p, adj);
}

/* src/func_8008E664.c: velocity split by lower bound — count entries
 * (terminator on the entry's own first byte), scan backward from the last
 * for the first with a1 >= p[1], and skip when it or (if any remain) its
 * predecessor is already current. */
void func_8008E664(pe_addr_t a0, unsigned int a1)
{
    pe_addr_t p = PE_LoadU32(a0 + 0x18u);
    int cnt = 1, k;
    unsigned int adj = (PE_LoadU32(PE_LoadU32(0x8009D2C8u)) & 0x100u) ? 0x30u : 0u;

    if (PE_LoadU8(p) < 0x80u) {
        do {
            p += 8u;
            cnt++;
        } while (PE_LoadU8(p) < 0x80u);
        p = PE_LoadU32(a0 + 0x18u);
    }
    k = cnt;
    p += (uint32_t)((k - 1) * 8);
    if (k != 0) {
        while (a1 < PE_LoadU8(p + 1u)) {
            k--;
            p -= 8u;
            if (k == 0)
                break;
        }
    }
    if (prog_is_current(a0, PE_LoadU8(p), adj))
        return;
    if (k != 0 && prog_is_current(a0, PE_LoadU8(p - 8u), adj))
        return;
    prog_select(a0, p, adj);
}

/* src/func_800CC2C4.c: +3 = 0x7F; count +4 = 4 on stage 3 else 0x10; each
 * point starts at D_800E2290[0..2] (columns +8/+0x28/+0x48, 2 bytes per
 * point) with velocity ((r%11-5) << 8, -(r%16+0x14) << 8, (r%11-5) << 8)
 * in columns +0x68/+0x88/+0xA8 (signed int remainders). */
void func_800CC2C4(int a0, int a1, pe_addr_t a2)
{
    pe_addr_t stage = PE_LoadU32(PE_LoadU32(PE_LoadU32(0x8009D254u)) + 0x68u);
    int i = 0;

    (void)a0; (void)a1;
    PE_StoreU8(a2 + 3u, 0x7Fu);
    PE_StoreU16(a2 + 4u, ((int16_t)PE_LoadU16(stage + 6u) == 3) ? 4u : 0x10u);
    while (i < (int16_t)PE_LoadU16(a2 + 4u)) {
        uint32_t k = (uint32_t)i * 2u;
        int r;

        PE_StoreU16(a2 + k + 0x8u, PE_LoadU16(0x800E2290u));
        PE_StoreU16(a2 + k + 0x28u, PE_LoadU16(0x800E2292u));
        PE_StoreU16(a2 + k + 0x48u, PE_LoadU16(0x800E2294u));
        PE_StoreU16(a2 + k + 0x68u, (uint16_t)(((int)func_80071A54() % 11 - 5) << 8));
        r = (int)func_80071A54();
        PE_StoreU16(a2 + k + 0x88u, (uint16_t)((-(r % 16 + 0x14)) << 8));
        PE_StoreU16(a2 + k + 0xA8u, (uint16_t)(((int)func_80071A54() % 11 - 5) << 8));
        i++;
    }
}

/* src/func_800CB750.c: draw state (page 3, 0x10, 32x32, blend 2); two
 * sprites with style D_800E2338 (+0xA = a2->+4), identity rotation, scale
 * (short)a2->+6 + 0x42C, translations a2 +8/+A/+C and +0x10/+0x12/+0x14.
 * The scale VECTOR is zeroed by BIOS A(2Bh) (func_80071A44: loud boundary)
 * before vx/vy/vz are set; its pad word is not read. */
void func_800CB750(int a0, int a1, pe_addr_t a2)
{
    const pe_addr_t m = PE_HAND_HI_STACK + 0x40u, v = PE_HAND_HI_STACK + 0x60u;
    int d, pass;
    unsigned short e;

    (void)a0; (void)a1;
    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    d = (int16_t)PE_LoadU16(a2 + 6u) + 0x42C;
    e = PE_LoadU16(a2 + 4u);
    for (pass = 0; pass < 2; pass++) {
        pe_addr_t t = a2 + (pass ? 0x10u : 0x8u);
        unsigned int i;

        for (i = 0; i < 9u; i++)
            PE_StoreU16(m + i * 2u, (i % 4u == 0u) ? 0x1000u : 0u);
        if (pass == 0)
            PE_StoreU16(0x800E2338u + 0xAu, e);
        PE_StoreU32(m + 0x14u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(t + 0u));
        PE_StoreU32(m + 0x18u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(t + 2u));
        PE_StoreU32(m + 0x1Cu, (uint32_t)(int32_t)(int16_t)PE_LoadU16(t + 4u));
        func_80071A44(v, 0, 0x10);
        PE_StoreU32(v + 0u, (uint32_t)d);
        PE_StoreU32(v + 4u, (uint32_t)d);
        PE_StoreU32(v + 8u, (uint32_t)d);
        (void)func_80078CC4(m, v);
        func_800C42A4(0x800E2338u, m, 1);
    }
}

/*
 * Hand adapters — batch 12 (>= 0x80060000).  Bodies follow the matched
 * leaves src/func_XXXXXXXX.c; pointers are guest addresses.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern unsigned int func_80071A54(void);

/* Signed division with retail's ASPSX trap semantics: a divisor of 0
 * (`break 7`) or INT_MIN / -1 (`break 6`) is a loud boundary, never a host
 * SIGFPE. */
static int div_trapped(int num, int den, const char *site, unsigned vma)
{
    if (den == 0 || (den == -1 && num == (int)0x80000000u)) {
        PE_D_COMP_BOUNDARY2(site, vma, num, den);
        return 0;
    }
    return num / den;
}

/* src/func_80088E64.c: split the a0 voice's L/R volumes (+0x118/+0x11A)
 * by the pan byte (+0xD8 >> 8): a0 keeps (l * (0x7F - pan)) >> 8 (unsigned
 * shift of the int product), the paired D_800B8AC0 voice a1 gets
 * (l * (short)+0xD8) >> 16; copy +0x10C and OR +0xF4 into the pair, then
 * func_800878F0 on both (absent_hi2_port.c). */
void func_80088E64(pe_addr_t a0, int a1)
{
    pe_addr_t q = 0x800B8AC0u + (uint32_t)(a1 * 0x11C);
    int d8 = (int16_t)PE_LoadU16(a0 + 0xD8u);
    int r = 0x7F - (d8 >> 8);
    int l;

    l = (int16_t)PE_LoadU16(a0 + 0x118u);
    PE_StoreU16(a0 + 0x118u, (uint16_t)((unsigned int)(l * r) >> 8));
    PE_StoreU16(q + 0x118u, (uint16_t)((l * d8) >> 16));
    l = (int16_t)PE_LoadU16(a0 + 0x11Au);
    PE_StoreU16(a0 + 0x11Au, (uint16_t)((unsigned int)(l * r) >> 8));
    PE_StoreU16(q + 0x11Au, (uint16_t)((l * d8) >> 16));
    PE_StoreU16(q + 0x10Cu, PE_LoadU16(a0 + 0x10Cu));
    PE_StoreU32(q + 0xF4u, PE_LoadU32(q + 0xF4u) | PE_LoadU32(a0 + 0xF4u));
    /* The matched call passes a third argument (+0x38); func_800878F0's own
     * matched C takes two, so the host call drops it. */
    func_800878F0((int)PE_LoadU32(a0 + 0xF0u), a0 + 0xF0u);
    func_800878F0(a1, q + 0xF0u);
}

/* src/func_800CD404.c: draw state (page 3, 0x10, 32x32, blend 2); core
 * sprite at translation a2 +8/+A/+C through D_800F33C0 with style
 * D_800E2250 (byte +4 = a2[3] * 2 - 0x80); then blend 3 and two satellite
 * sprites at a2 + 0x10 + i*8 through D_800F32B0 with style D_800F3460. */
/* func_800CD404: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CD404_port.c (src/func_800CD404.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_800CD0BC.c: seed the burst record — position from D_800E27F8
 * [0..2], +4 = 0x7F, +6 = 0x3B4, +3 = 0; two satellites at +0x10 + i*8
 * start at the same position with velocity (+0x20 + i*8) of
 * (r % 50 - 25, r % 50 - 25, 0) (signed int remainders). */
/* func_800CD0BC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CD0BC_port.c (src/func_800CD0BC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_800CA824.c: the D_800E27A8 twin of func_800C7E50 (base z read
 * before the +2/+1 stores, stored last). */
void func_800CA824(int a0, int a1, pe_addr_t a2)
{
    const pe_addr_t v = PE_HAND_HI_STACK + 0x20u;
    unsigned short t;

    (void)a0; (void)a1;
    PE_StoreU16(v + 0u, (uint16_t)(-((int)func_80071A54() % 3 + 9)));
    PE_StoreU16(v + 2u, (uint16_t)(-((int)func_80071A54() % 3 + 9)));
    PE_StoreU16(v + 4u, (uint16_t)((int)func_80071A54() % 5 - 2));
    (void)func_80078C34(PE_LoadU32(PE_LoadU32(0x800E27A8u) + 0x238u), v, a2 + 0x10u);
    PE_StoreU16(a2 + 8u, PE_LoadU16(0x800E2360u));
    PE_StoreU16(a2 + 0xAu, PE_LoadU16(0x800E2362u));
    t = PE_LoadU16(0x800E2364u);
    PE_StoreU8(a2 + 2u, 0x14u);
    PE_StoreU8(a2 + 1u, 0u);
    PE_StoreU16(a2 + 0xCu, t);
}

/* src/func_8008B2CC.c: volume fade — n = a0->+4 (0 -> 1) steps toward
 * (a0->+8 & 0x7F) << 16 from the bank's current +0x48 (bank 1: +0xB0):
 * step = (target - current) / n into +0x4C (+0xB4), n into +0x50 (+0xB8),
 * mark the bank.  Bank selection by a0->+0x10 as in func_8008B1FC. */
void func_8008B2CC(pe_addr_t a0)
{
    int t = (int)PE_LoadU32(a0 + 4u);
    int n = (t != 0) ? t : 1;
    int val = (int)((PE_LoadU32(a0 + 8u) & 0x7Fu) << 16);
    int v = (int)PE_LoadU32(a0 + 0x10u);
    pe_addr_t s = PE_LoadU32(0x8009D2C8u);

    if (v == 0 || (unsigned int)v == PE_LoadU16(s + 0x54u)) {
        val = div_trapped(val - (int)PE_LoadU32(s + 0x48u), n, "func_8008B2CC_divide_trap", 0x8008B2CCu);
        PE_StoreU16(s + 0x50u, (uint16_t)n);
        PE_StoreU32(s + 0x4Cu, (uint32_t)val);
        func_8008AB9C(0x800B8AC0u);
    } else if (a0 != 0u && (unsigned int)v == PE_LoadU16(s + 0xBCu)) {
        val = div_trapped(val - (int)PE_LoadU32(s + 0xB0u), n, "func_8008B2CC_divide_trap", 0x8008B2CCu);
        PE_StoreU16(s + 0xB8u, (uint16_t)n);
        PE_StoreU32(0x8009D2C8u, s + 0x68u);
        PE_StoreU32(s + 0xB4u, (uint32_t)val);
        func_8008AB9C(0x800BA560u);
        PE_StoreU32(0x8009D2C8u, PE_LoadU32(0x8009D2C8u) - 0x68u);
    }
}

/* src/func_8008B410.c: volume ramp — base = (a0->+8 & 0x7F) << 16 into
 * +0x48 (+0xB0), step = (((a0->+0xC & 0x7F) << 16) - base) / n into +0x4C
 * (+0xB4), n into +0x50 (+0xB8), mark the bank. */
void func_8008B410(pe_addr_t a0)
{
    int t = (int)PE_LoadU32(a0 + 4u);
    int n = (t != 0) ? t : 1;
    int v = (int)PE_LoadU32(a0 + 0x10u);
    pe_addr_t s = PE_LoadU32(0x8009D2C8u);
    int base, d;

    if (v == 0 || (unsigned int)v == PE_LoadU16(s + 0x54u)) {
        base = (int)((PE_LoadU32(a0 + 8u) & 0x7Fu) << 16);
        PE_StoreU32(s + 0x48u, (uint32_t)base);
        d = (int)((PE_LoadU32(a0 + 0xCu) & 0x7Fu) << 16) - base;
        d = div_trapped(d, n, "func_8008B410_divide_trap", 0x8008B410u);
        PE_StoreU16(s + 0x50u, (uint16_t)n);
        PE_StoreU32(s + 0x4Cu, (uint32_t)d);
        func_8008AB9C(0x800B8AC0u);
    } else if (v != 0 && (unsigned int)v == PE_LoadU16(s + 0xBCu)) {
        base = (int)((PE_LoadU32(a0 + 8u) & 0x7Fu) << 16);
        PE_StoreU32(s + 0xB0u, (uint32_t)base);
        d = (int)((PE_LoadU32(a0 + 0xCu) & 0x7Fu) << 16) - base;
        d = div_trapped(d, n, "func_8008B410_divide_trap", 0x8008B410u);
        PE_StoreU16(s + 0xB8u, (uint16_t)n);
        PE_StoreU32(0x8009D2C8u, s + 0x68u);
        PE_StoreU32(s + 0xB4u, (uint32_t)d);
        func_8008AB9C(0x800BA560u);
        PE_StoreU32(0x8009D2C8u, PE_LoadU32(0x8009D2C8u) - 0x68u);
    }
}

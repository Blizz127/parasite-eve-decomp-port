/*
 * Hand adapters — field-VM state handlers, menu/task constructors and small
 * list walkers below 0x80060000 (port_absent lane, batch 2).
 *
 * Conventions as in absent_lo_small_port.c: guest pointers are `pe_addr_t`
 * loaded with PE_LoadU32; data symbols are accessed at their retail address
 * with the leaf's declared width; function-address values are the retail
 * VMA; a pointer global the leaf reads more than once is re-read at each C
 * access (it is a plain global, and intervening calls may change it).
 */
#include "pe_guest_decomp.h"

/* Decomp-derived callee (pc_port/game/decomp/, no header prototype). */
void func_800622B0(int a0);

/* src/func_80017D3C.c: state->+0x224 (short) = (*arg0)[1]. */
int func_80017D3C(pe_addr_t arg0)
{
    pe_addr_t state = PE_LoadU32(0x8009D2F0u);
    int value = (short)PE_LoadU16(PE_LoadU32(arg0) + 2u);

    PE_StoreU16(state + 0x224u, (unsigned short)value);
    return 1;
}

/* src/func_80018598.c: velocity from speed and heading.
 *   t = (state == D_8009D254) ? func_8003708C(0x50000, s[8]) : s[8];
 *   t = func_8003708C(t, (u16)s16[0x13] << 4);
 *   s[0x1A] = func_8003708C(-t, func_80077CF4(s16[0x1D]) << 4);
 *   s[0x1C] = func_8003708C(-t, func_80077DC4(s16[0x1D]) << 4);          */
int func_80018598(void)
{
    int t;

    if (PE_LoadU32(0x8009D2F0u) == PE_LoadU32(0x8009D254u)) {
        t = (int)func_8003708C(0x50000u,
                               PE_LoadU32(PE_LoadU32(0x8009D2F0u) + 8u * 4u));
    } else {
        t = (int)PE_LoadU32(PE_LoadU32(0x8009D2F0u) + 8u * 4u);
    }
    t = (int)func_8003708C((uint32_t)t,
        (uint32_t)PE_LoadU16(PE_LoadU32(0x8009D2F0u) + 0x13u * 2u) << 4);
    {
        int c = func_80077CF4((short)PE_LoadU16(PE_LoadU32(0x8009D2F0u) + 0x1Du * 2u));
        uint32_t r = func_8003708C((uint32_t)-t, (uint32_t)c << 4);
        PE_StoreU32(PE_LoadU32(0x8009D2F0u) + 0x1Au * 4u, r);
    }
    {
        int s = func_80077DC4((short)PE_LoadU16(PE_LoadU32(0x8009D2F0u) + 0x1Du * 2u));
        uint32_t r = func_8003708C((uint32_t)-t, (uint32_t)s << 4);
        PE_StoreU32(PE_LoadU32(0x8009D2F0u) + 0x1Cu * 4u, r);
    }
    return 1;
}

/* src/func_80018660.c: timer seed from four operands.
 *   D_800A76C8 = (*a0[0] + *a0[1]) * 225 * 960 + *a0[2] * 60;   (32-bit wrap)
 *   if (*a0[3] == 1) { D_800A76CC = 0; D_800A76C4 |= 2; } else D_800A76C8 = 0;
 *   D_800A76C4 = (D_800A76C4 | 1) & ~4;                                    */
int func_80018660(pe_addr_t a0)
{
    uint32_t v = ((PE_LoadU32(PE_LoadU32(a0 + 0u)) +
                   PE_LoadU32(PE_LoadU32(a0 + 4u))) * 225u * 960u) +
                 PE_LoadU32(PE_LoadU32(a0 + 8u)) * 60u;

    PE_StoreU32(0x800A76C8u, v);
    if ((int)PE_LoadU32(PE_LoadU32(a0 + 12u)) == 1) {
        PE_StoreU32(0x800A76CCu, 0u);
        PE_StoreU32(0x800A76C4u, PE_LoadU32(0x800A76C4u) | 2u);
    } else {
        PE_StoreU32(0x800A76C8u, 0u);
    }
    PE_StoreU32(0x800A76C4u, (PE_LoadU32(0x800A76C4u) | 1u) & ~4u);
    return 1;
}

/* src/func_80018894.c: func_8006F820(*first, 1, second) — the second operand
 * is passed as the guest pointer itself (mode 1 writes through it). */
int func_80018894(pe_addr_t arg0)
{
    pe_addr_t first = PE_LoadU32(arg0);
    pe_addr_t second = PE_LoadU32(arg0 + 4u);

    func_8006F820(PE_LoadU32(first), 1u, second);
    return 1;
}

/* src/func_800199CC.c: v = **a0; state+0x250 |= 0x10; state+0x24E = (short)v. */
int func_800199CC(pe_addr_t a0)
{
    pe_addr_t p = PE_LoadU32(a0);
    pe_addr_t s = PE_LoadU32(0x8009D2F0u);
    int v = (int)PE_LoadU32(p);

    PE_StoreU16(s + 0x250u, (unsigned short)(PE_LoadU16(s + 0x250u) | 0x10u));
    PE_StoreU16(s + 0x24Eu, (unsigned short)(short)v);
    return 1;
}

/* src/func_8001A064.c: clear bit 20 of state+0x98, then for every actor on
 * the D_8009D20C list (next at +4) whose +0x18C target is this state, clear
 * the target and bits 21-22 of its +0x98 flags. */
int func_8001A064(void)
{
    pe_addr_t s = PE_LoadU32(0x8009D2F0u);
    pe_addr_t p;

    PE_StoreU32(s + 0x26u * 4u, PE_LoadU32(s + 0x26u * 4u) & 0xFFEFFFFFu);
    p = PE_LoadU32(0x8009D20Cu);
    while (p != 0u) {
        if (PE_LoadU32(p + 0x63u * 4u) == s) {
            PE_StoreU32(p + 0x63u * 4u, 0u);
            PE_StoreU32(p + 0x26u * 4u, PE_LoadU32(p + 0x26u * 4u) & 0xFF9FFFFFu);
        }
        p = PE_LoadU32(p + 4u);
    }
    return 1;
}

/* src/func_8001A9F8.c: if (D_8009D2E8 & 4) func_8001D170(D_8009D254); then
 * func_8001AE40 on every D_8009D20C actor whose +0x98 bit 7 is clear.
 * func_8001D170 has no pc_port implementation yet: loud boundary. */
void func_8001A9F8(void)
{
    pe_addr_t p;

    if (PE_LoadU32(0x8009D2E8u) & 4u)
        (void)PE_D_COMP_BOUNDARY1("func_8001D170", 0x8001D170u,
                                  PE_LoadU32(0x8009D254u));
    p = PE_LoadU32(0x8009D20Cu);
    while (p != 0u) {
        if ((PE_LoadU32(p + 0x26u * 4u) & 0x80u) == 0u)
            func_8001AE40(p);
        p = PE_LoadU32(p + 4u);
    }
}

/* src/func_80020C74.c: D_8009D2E8 |= 1; player+0x98 &= ~0x100;
 * (*D_8009D278)+0x4C |= 0x10000; D_8009D298 = 0; func_8001A680(player, 0x12);
 * func_80021D4C(). */
void func_80020C74(void)
{
    pe_addr_t s;
    pe_addr_t t;

    PE_StoreU32(0x8009D2E8u, PE_LoadU32(0x8009D2E8u) | 1u);
    s = PE_LoadU32(0x8009D254u);
    PE_StoreU32(s + 0x26u * 4u, PE_LoadU32(s + 0x26u * 4u) & 0xFFFFFEFFu);
    t = PE_LoadU32(0x8009D278u);
    PE_StoreU32(t + 0x4Cu, PE_LoadU32(t + 0x4Cu) | 0x10000u);
    PE_StoreU16(0x8009D298u, 0u);
    func_8001A680(s, 0x12u);
    func_80021D4C();
}

/* src/func_80033430.c: i = D_8009CDDC; D_8009EC38[i].f12 -= 2 (0x1C-byte
 * entries); func_80077AC4(D_800B0E38[i] + 0x1C, &D_8009EC38[i]);
 * D_8009D235--. */
void func_80033430(void)
{
    int i = (int)PE_LoadU32(0x8009CDDCu);
    pe_addr_t e = 0x8009EC38u + (uint32_t)i * 0x1Cu;

    PE_StoreU16(e + 0x12u, (unsigned short)(PE_LoadU16(e + 0x12u) - 2u));
    func_80077AC4(PE_LoadU32(0x800B0E38u + (uint32_t)i * 4u) + 0x1Cu, e);
    PE_StoreU8(0x8009D235u, (unsigned char)(PE_LoadU8(0x8009D235u) - 1u));
}

/* src/func_80039970.c: reset the D_80091A1C..28 cursor block; returns 0. */
int func_80039970(void)
{
    PE_StoreU8(0x80091A1Fu, 0xFFu);
    PE_StoreU8(0x80091A1Cu, 0u);
    PE_StoreU8(0x80091A1Eu, 0u);
    PE_StoreU32(0x80091A28u, 0u);
    return 0;
}

/* src/func_8003E0D0.c: +0x24 = 0; +0x28 = ((*arg0)[2] == 2) ? 2 : 0. */
void func_8003E0D0(pe_addr_t arg0)
{
    pe_addr_t data = PE_LoadU32(arg0);

    PE_StoreU32(arg0 + 0x24u, 0u);
    if (PE_LoadU8(data + 2u) == 2u)
        PE_StoreU16(arg0 + 0x28u, 2u);
    else
        PE_StoreU16(arg0 + 0x28u, 0u);
}

/* src/func_8003E0FC.c: search the 12-byte records at *(a0+0x80) (count =
 * (*a0)[3]) for rec[3] == key; on a hit copy rec[0..2] (sign-extended
 * shorts) to out[0..2] and return 1, else 0. */
int func_8003E0FC(pe_addr_t a0, short key, pe_addr_t out)
{
    pe_addr_t rec = PE_LoadU32(a0 + 0x80u);
    int i = 0;

    while (i < (int)PE_LoadU8(PE_LoadU32(a0) + 3u)) {
        if ((short)PE_LoadU16(rec + 6u) == key) {
            PE_StoreU32(out + 0u, (uint32_t)(int)(short)PE_LoadU16(rec + 0u));
            PE_StoreU32(out + 4u, (uint32_t)(int)(short)PE_LoadU16(rec + 2u));
            PE_StoreU32(out + 8u, (uint32_t)(int)(short)PE_LoadU16(rec + 4u));
            return 1;
        }
        i++;
        rec += 12u;
    }
    return 0;
}

/* src/func_80042BC8.c: return D_800A1870 (a callback pointer) != 0. */
/* func_80042BC8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042BC8_port.c (src/func_80042BC8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_80047BEC.c: task 0xA (draw 0x80047C50, update 0x80047D74) and a
 * 0x1C child (draw 0x8004FFD0, +0x44 = -1). */
void func_80047BEC(int a0)
{
    pe_addr_t p;
    pe_addr_t q;

    p = func_80062D2C(0xAu, (pe_addr_t)a0, 0u, 0u);
    PE_StoreU32(p + 0x30u, 0x80047C50u);
    PE_StoreU32(p + 0x2Cu, 0x80047D74u);
    q = func_8006322C(0x1Cu, p, p);
    PE_StoreU32(q + 0x30u, 0x8004FFD0u);
    PE_StoreU32(q + 0x44u, 0xFFFFFFFFu);
}

/* src/func_8004D978.c: task 0x27 under the current focus (modal), draw
 * 0x8004DA04 / update 0x8004DA9C, activate it, then D_8009D000 = a0. */
void func_8004D978(int a0)
{
    pe_addr_t p;

    p = func_80062D2C(0x27u, func_80062CC4(), 0u, 1u);
    PE_StoreU32(p + 0x30u, 0x8004DA04u);
    PE_StoreU32(p + 0x2Cu, 0x8004DA9Cu);
    func_80062CB8(p);
    PE_StoreU32(0x8009D000u, (uint32_t)a0);
}

/* src/func_8004EBCC.c: D_8009CEF4 = a0; list draw 0x80050618; dim on; then
 * *(a0 + 0x38) times: icon 0x68, move (0, 0x10). */
void func_8004EBCC(int a0)
{
    int n;

    PE_StoreU32(0x8009CEF4u, (uint32_t)a0);
    func_800638D8((pe_addr_t)a0, 0x80050618u);
    func_8005EB58(1u);
    n = (int)PE_LoadU32((pe_addr_t)a0 + 0x38u);
    while (n != 0) {
        func_8005EB64(0x68u);
        func_8005E8A4(0, 0x10);
        n--;
    }
}

/* src/func_80050544.c: when enabled: func_80062F3C(0x1F); func_8004D978(0x45);
 * func_80042B50(func_800504F4). */
/* func_80050544: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050544_port.c (src/func_80050544.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_80050690.c declares `func_80058E08(void)` and calls it with no
 * arguments.  Retail keeps the caller's $a0 live into that call
 * (tools/analysis/pe_dis.sh 0x80050690: the first instruction after the
 * prologue is `jal 0x80058e08` with $a0 untouched), so the leaf's own first
 * argument register is the index; it is spelled as the explicit parameter
 * the menu draw-callback dispatch already passes (`value`). */
void func_80050690(int a0)
{
    int x = func_80058E08(a0);

    func_8005EB58((uint32_t)(func_80055FE0(x) == 0));
    func_800536B8(x);
    if (func_80054240(x) != 0)
        func_80064C80();
}

/* src/func_80052EC0.c: seed the D_8009D048..64 config block. */
void func_80052EC0(void)
{
    PE_StoreU32(0x8009D04Cu, 0u);
    PE_StoreU32(0x8009D054u, 0u);
    PE_StoreU32(0x8009D048u, 0x800C0E48u);
    PE_StoreU32(0x8009D050u, (uint32_t)func_80052F70());
    PE_StoreU32(0x8009D058u, 0x8009D05Cu);
    PE_StoreU32(0x8009D064u, 2u);
}

/* src/func_800577E0.c: record = func_8005DB44(a0 - 1); if present, clear bit
 * 6 of +5 and set it again when a1 == 0 (two stores, as retail). */
void func_800577E0(int a0, int a1)
{
    pe_addr_t p = func_8005DB44((unsigned int)(a0 - 1));
    int v;

    if (p != 0u) {
        v = PE_LoadU8(p + 5u) & 0xBF;
        PE_StoreU8(p + 5u, (unsigned char)v);
        if (a1 == 0)
            v |= 0x40;
        PE_StoreU8(p + 5u, (unsigned char)v);
    }
}

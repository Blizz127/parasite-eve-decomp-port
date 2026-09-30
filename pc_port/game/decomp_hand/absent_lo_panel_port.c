/*
 * Hand adapters — item/menu panels and the 0x80034DE0 banner tick
 * (port_absent lane, batch 7).  Conventions as in absent_lo_window_port.c;
 * a retail stack local whose address is passed on uses PE_HAND_LO_STACK_TEMP.
 */
#include "pe_guest_decomp.h"

/* Decomp-derived callee (pc_port/game/decomp/, no header prototype). */
void func_8005E894(int a0, int a1);

/* src/func_80034DE0.c: banner state machine on D_8009D1CE.
 * 1: reset, place the banner (x 0x14/0x61 by func_8005BCB0, y 0xF/0xC3 by
 *    D_8009CE80 < 2), register list mode 2 with a -1 short, arm record 0
 *    (b0 = 2, w4 = D_8009D1F8, flags |= 0x2000000) for 0x4B ticks;
 * 2: when the counter hits 0, reset and go idle.
 * Every tick: counter--, position (0, 0xB/0xBF), draw a 0x140x0x14 box. */
void func_80034DE0(void)
{
    const pe_addr_t v = PE_HAND_LO_STACK_TEMP;

    switch (PE_LoadU8(0x8009D1CEu)) {
    case 1:
        PE_StoreU16(v, 0xFFFFu);
        func_800374E8();
        {
            int x = func_8005BCB0() ? 0x14 : 0x61;
            int y = PE_LoadU8(0x8009CE80u) < 2u ? 0xF : 0xC3;
            func_80037454(x, y, 0, 0);
        }
        func_800375E0(0, 2u, v);
        PE_StoreU8(0x800BCEA8u + 0u, 2u);
        PE_StoreU8(0x8009CE88u, 0x4Bu);
        PE_StoreU32(0x800BCEA8u + 4u, PE_LoadU32(0x8009D1F8u));
        PE_StoreU32(0x800BCEA8u + 0xCu, PE_LoadU32(0x800BCEA8u + 0xCu) | 0x2000000u);
        PE_StoreU8(0x8009D1CEu, (unsigned char)(PE_LoadU8(0x8009D1CEu) + 1u));
        break;
    case 2:
        if (PE_LoadU8(0x8009CE88u) == 0u) {
            func_800374E8();
            PE_StoreU8(0x8009D1CEu, 0u);
        }
        break;
    }
    PE_StoreU8(0x8009CE88u, (unsigned char)(PE_LoadU8(0x8009CE88u) - 1u));
    func_8005E894(0, PE_LoadU8(0x8009CE80u) < 2u ? 0xB : 0xBF);
    func_80061C34(0x140u, 0x14u, 0u, 0u);
}

/* src/func_800480AC.c: equip-slot picker task 0x28 (draw 0x800481FC /
 * update 0x8004D030, list draw 0x8004FFA8), fixed geometry, then
 * D_8009CFC0 = 0x37 + (first 0xA0-class entry's low 5 bits - 1) of the
 * current party slot's record (the scan stops at the record's +0x14 count and
 * then reads that entry, exactly as retail). */
void func_800480AC(void)
{
    pe_addr_t p;
    pe_addr_t q;
    pe_addr_t r;
    int s;
    int i;

    p = func_80062D2C(0x28u, func_80062CC4(), 0u, 1u);
    q = func_8006322C(0x28u, p, p);
    PE_StoreU32(p + 0x30u, 0x800481FCu);
    PE_StoreU32(p + 0x2Cu, 0x8004D030u);
    PE_StoreU32(q + 0x30u, 0x8004FFA8u);
    func_80062CB8(q);
    func_8004D024(0u);
    PE_StoreU32(p + 0x34u, 0xDCu);
    PE_StoreU32(p + 0x18u, 0x32u);
    PE_StoreU32(p + 0x38u, PE_LoadU32(p + 0x38u) + 10u);
    PE_StoreU32(q + 0x18u, PE_LoadU32(p + 0x34u) - 0x44u);
    PE_StoreU32(q + 0x1Cu, PE_LoadU32(q + 0x1Cu) + 8u);
    PE_StoreU32(0x8009CFC0u, 0x37u);
    s = (signed char)PE_LoadU8(0x800C0E22u);
    if (s >= 0) {
        func_80052E30(0u);
        r = func_8005332C((signed char)PE_LoadU8(0x800C0E22u));
        for (i = 0; i < (int)PE_LoadU8(r + 0x14u); i++) {
            if ((PE_LoadU8(r + (uint32_t)i + 0x15u) & 0xE0u) == 0xA0u)
                break;
        }
        PE_StoreU32(0x8009CFC0u, PE_LoadU32(0x8009CFC0u) +
                    (uint32_t)((int)(PE_LoadU8(r + (uint32_t)i + 0x15u) & 0x1Fu) - 1));
    }
}

/* src/func_8004790C.c: equip panel draw — slot icon (or the 0x66 caption),
 * the 0x64/0x65 header, the party-slot name unless D_8009CF2C bit 0, and the
 * 0x68 footer. */
void func_8004790C(void)
{
    int h;
    int x;
    pe_addr_t p;
    int t;

    h = func_80059F08(PE_LoadU32(0x8009CF24u));
    func_8005E8A4(4, 4);
    func_8005E8C4();
    if ((int)PE_LoadU32(0x8009CF28u) >= 0) {
        p = func_8005332C(h);
        t = PE_LoadU8(p + PE_LoadU32(0x8009CF28u) + 0x15u) & 0x1F;
        if (PE_LoadU32(0x8009CF18u) != 0u)
            func_8005EB64((uint32_t)(t + 0x22));
        else
            func_8005EB64((uint32_t)(t + 0x36));
        x = 0x24;
    } else {
        func_8005F5B8(0x66u);
        x = (int)func_8005F1A0(func_8005DC4C(0x66u));
    }
    func_8005E8A4(x, 0);
    func_8005F5B8((PE_LoadU32(0x8009CF2Cu) & 1u) | 0x64u);
    func_8005E914();
    if ((PE_LoadU32(0x8009CF2Cu) & 1u) == 0u) {
        func_8005E8A4(0, 0xE);
        func_800536B8(h);
        func_8005E8A4(0, 0xE);
        func_8005F5B8(0x67u);
    }
    func_8005E8A4(0, 0x14);
    func_8005F5B8(0x68u);
}

/* src/func_80058AA8.c: item list cell a0 — dim when the D_8009D058 owned bit
 * is clear, resolve the item record by id range (as func_80059C44), pick its
 * name (D_800C20A4 [+0x10 for type 9] for bit-4 items, else
 * func_8005DC9C(record[4] - 1)) and draw both.  A 0 id draws nothing. */
void func_80058AA8(int a0)
{
    int v;
    pe_addr_t p;
    pe_addr_t q;

    v = (short)PE_LoadU16(PE_LoadU32(0x8009D07Cu) + (uint32_t)a0 * 2u);
    if (v == 0)
        return;
    func_8005EB58((uint32_t)((PE_LoadU32(PE_LoadU32(0x8009D058u) +
                   (uint32_t)(a0 >> 5) * 4u) & (1u << (a0 & 0x1F))) == 0u));
    if ((unsigned int)(v - 0x100) < 0x80u)
        p = ((uint32_t)v << 5) + 0x800BEEACu;
    else if ((unsigned int)(v - 1) < 0xFFu)
        p = func_8005DB44((unsigned int)(v - 1));
    else if ((unsigned int)(v - 0x200) < 9u)
        p = ((uint32_t)v << 5) + 0x8009DE64u;
    else
        p = 0u;
    if (PE_LoadU8(p + 5u) & 0x10u) {
        q = 0x800C20A4u;
        if (PE_LoadU8(p + 6u) == 9u)
            q += 0x10u;
    } else {
        q = func_8005DC9C((uint32_t)((int)PE_LoadU8(p + 4u) - 1));
    }
    func_800534E4(p, q);
}

/* src/func_800506E8.c declares `func_80058AA8(void)` and calls it with no
 * arguments.  Retail keeps the caller's $a0 live into that call
 * (tools/analysis/pe_dis.sh 0x800506E8: `jal 0x80058aa8` right after the
 * prologue, $a0 untouched), and func_80058AA8 reads $a0 as the cell index.
 * func_800506E8 is the per-cell draw func_8004EC78 hands to func_800638D8,
 * whose dispatch passes the cell `value` in $a0 — spelled out here. */
void func_800506E8(int a0)
{
    func_80058AA8(a0);
}

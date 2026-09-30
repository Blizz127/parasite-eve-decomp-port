/*
 * Hand adapters — field-menu window builders and draw/update callbacks
 * below 0x80060000 (port_absent lane, batch 5).
 *
 * Conventions as in absent_lo_small_port.c: window/task nodes are guest
 * addresses (`pe_addr_t`) returned by the canonical func_80062A34 /
 * func_80062D2C / func_8006322C host ports; a function address stored into a
 * node is the retail VMA; `register ... asm("$N")` pins in the leaves are
 * dropped (codegen hints only).  An unconditional dereference of a window the
 * leaf does not null-check is kept, so a missing window aborts loudly in
 * PE_Translate rather than touching the PS1 kernel area.
 */
#include "pe_guest_decomp.h"

/* src/func_80047560.c: open the item/status screen windows. */
void func_80047560(void)
{
    pe_addr_t p;
    pe_addr_t q;

    func_80062F3C(0x35u);
    PE_StoreU32(func_80062A34(1u, 5u) + 0x34u, 0x80u);
    PE_StoreU32(func_80062A34(1u, 6u) + 0x34u, 0x80u);
    PE_StoreU32(func_80062A34(2u, 6u) + 0x3Cu, 0x3Eu);
    func_800631AC(func_80062A34(1u, 7u));
    p = func_80062A34(2u, 0u);
    if (p == 0u)
        p = func_80062A34(2u, 0x32u);
    func_80047E94((int)p);
    func_80047BEC((int)p);
    p = func_80062A20(func_80062A34(1u, 5u), 1u);
    if (p != 0u) {
        func_80062CB8(p);
        PE_StoreU32(p + 0x44u, 0u);
    }
    p = func_80062A34(2u, 0x1Bu);
    q = func_80062A34(2u, 0x1Cu);
    PE_StoreU32(p + 0x7Cu, q);
    PE_StoreU32(q + 0x78u, p);
    p = func_80062A34(2u, 5u);
    if (p != 0u)
        PE_StoreU32(p + 0x44u, 0xFFFFFFFFu);
}

/* src/func_80047C50.c: party-slot status panel draw (three stat rows with
 * their icons).  Every draw primitive is the canonical host port. */
void func_80047C50(void)
{
    int h = func_80059F08(1u);
    pe_addr_t p = func_8005332C(h);

    func_8005E8A4(4, 4);
    func_800536B8(h);
    func_8005E8A4(-4, -4);
    func_8004551C(p);
    if (p != 0u) {
        func_8005E8A4(0x2A, -0xC);
        func_8005FDF0((int32_t)PE_LoadU8(p + 9u));
        func_8005E8A4(5, 0);
        func_8005FF28((short)PE_LoadU16(p + 0x12u));
        func_8005E8A4(-0x2D, -0xE);
        func_8005FDF0((int32_t)PE_LoadU8(p + 8u));
        func_8005E8A4(5, 0);
        func_8005FF28((short)PE_LoadU16(p + 0x10u));
        func_8005E8A4(-0x2D, -0xE);
        func_8005FDF0((int32_t)PE_LoadU8(p + 7u));
        func_8005E8A4(5, 0);
        func_8005FF28((short)PE_LoadU16(p + 0xEu));
        func_8005E8A4(-0x2D, -0xA);
        func_8005EB64(0x87u);
        func_8005E8A4(0x19, 0);
        func_8005EB64(0x88u);
    }
}

/* src/func_80048F24.c: shift windows 1/0xF, 1/0xB and 1/0x2F horizontally to
 * column 0xB0 (D_8009CF30 set), 0xA2 (sub-window +0x80 set) or 0x9C, relative
 * to window 1/0xF's current +0x18. */
void func_80048F24(void)
{
    pe_addr_t p;
    pe_addr_t q;
    int v;
    int off;
    int t;

    p = func_80062A34(1u, 0xFu);
    q = func_80062A20(func_80062A34(1u, 0xDu), 0u);
    v = (int)PE_LoadU32(0x8009CF30u);
    t = (int)PE_LoadU32(p + 0x18u);
    if (v == 0) {
        v = (int)PE_LoadU32(q + 0x80u);
        if (v == 0)
            v = 0x9C;
        else
            v = 0xA2;
    } else {
        v = 0xB0;
    }
    off = v - t;
    func_80063158(p, off, 0);
    func_80063198(p);
    p = func_80062A34(1u, 0xBu);
    func_80063158(p, off, 0);
    func_80063198(p);
    p = func_80062A34(1u, 0x2Fu);
    func_80063158(p, off, 0);
    func_80063198(p);
}

/* src/func_8004A570.c: modal task 9 (draw 0x8004A6CC / update 0x8004A9A0).
 * Modes < 3 snapshot the current resource block (32 bytes) into D_800A1A00
 * and raise the window by 0x14; others select D_800A18D8[a1]. */
void func_8004A570(int a0, int a1)
{
    pe_addr_t p;
    unsigned k;

    p = func_80062D2C(9u, (pe_addr_t)a0, 0u, 1u);
    PE_StoreU32(p + 0x30u, 0x8004A6CCu);
    PE_StoreU32(p + 0x2Cu, 0x8004A9A0u);
    PE_StoreU32(p + 0x28u, 1u);
    func_80062CB8(p);
    PE_StoreU32(0x8009CFD0u, (uint32_t)a1);
    PE_StoreU32(0x8009CFD8u, PE_LoadU32(0x8009CF68u));
    if (a1 < 3) {
        pe_addr_t src = func_8005332C(func_80059F08(0u));
        for (k = 0; k < 0x20u; k += 4u)
            PE_StoreU32(0x800A1A00u + k, PE_LoadU32(src + k));
        PE_StoreU32(p + 0x34u, PE_LoadU32(p + 0x34u) - 0x14u);
        func_80063158(p, 0xA, 0);
    } else {
        PE_StoreU32(0x8009CFDCu, PE_LoadU32(0x800A18D8u + (uint32_t)a1 * 4u));
    }
    PE_StoreU32(0x8009CFACu, 0u);
}

/* src/func_8004D4C4.c: card-slot list a0 under window 2/0x24 (update
 * 0x8004D6D4; list draw 0x8004FEEC; cell predicate 0x8004FE58) with a0's
 * row count a1; show it unless another modal is open; add the 0x3F footer
 * (draw 0x8004D690).  Returns the list's current cell. */
int func_8004D4C4(int a0, int a1)
{
    pe_addr_t p;
    pe_addr_t q;
    pe_addr_t r;
    int h;

    p = func_80062D2C((uint32_t)(a0 + 0x25), func_80062A34(2u, 0x24u), 0u, 0u);
    q = func_8006322C((uint32_t)(a0 + 0x25), p, p);
    PE_StoreU32(p + 0x2Cu, 0x8004D6D4u);
    PE_StoreU32(q + 0x30u, 0x8004FEECu);
    PE_StoreU32(q + 0x8Cu, 0x8004FE58u);
    func_800647D0(q, a1);
    h = func_80063428(q);
    if (func_800631DC() == 0u) {
        PE_StoreU32(q + 0x44u, 0u);
        func_80062CB8(q);
    } else {
        PE_StoreU32(q + 0x44u, 0xFFFFFFFFu);
    }
    r = func_80062D2C(0x3Fu, q, 0u, 0u);
    PE_StoreU32(r + 0x30u, 0x8004D690u);
    func_8005DE88();
    PE_StoreU32(0x8009CF44u, (uint32_t)a0);
    return h;
}

/* src/func_8004F30C.c: equip-menu update.  Confirm (bit 16): latch the chosen
 * entry into D_8009CF9C/98, apply it, close the menu windows and hand the
 * party cell to func_8004E704 (no pc_port implementation yet: loud
 * boundary).  Cancel (bit 6): close and hide the three menu windows. */
int func_8004F30C(int a0, int a1)
{
    pe_addr_t p;
    int v;

    if ((a1 & 0x10000) != 0) {
        p = func_80062A34(2u, 1u);
        PE_StoreU32(0x8009CF9Cu, (uint32_t)(int)(short)PE_LoadU16(
            0x800C0E48u + (uint32_t)func_80063428(p) * 2u));
        v = func_80063428(p) | ((int)PE_LoadU32(p + 0x5Cu) << 8);
        PE_StoreU32(0x8009CF98u, (uint32_t)(v + 1));
        (void)func_80057D30(v & 0xFF);
        func_80062F1C((pe_addr_t)a0);
        func_80062F3C(0x1Bu);
        func_80062F3C(1u);
        func_80062F3C(0u);
        func_8004E704(func_80063428(func_80062A20((pe_addr_t)a0, 0u)));   /* absent_lo2_port.c */
        func_800525EC();
    } else if ((a1 & 0x40) != 0) {
        func_80062F1C((pe_addr_t)a0);
        func_80063198(func_80062A34(1u, 0u));
        func_80063198(func_80062A34(1u, 1u));
        func_80063198(func_80062A34(1u, 0x1Bu));
        func_80052634();
    }
    return 1;
}

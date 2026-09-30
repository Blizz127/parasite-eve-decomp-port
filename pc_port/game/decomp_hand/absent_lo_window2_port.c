/*
 * Hand adapters — more field-menu builders plus the D_8009D0E0 event-queue
 * consumer (port_absent lane, batch 6).  Conventions as in
 * absent_lo_window_port.c.
 */
#include "pe_guest_decomp.h"

/* Decomp-derived callees (pc_port/game/decomp/, no header prototype). */
void func_8004C5DC(void);
short func_800534CC(int index);
/* Hand ports with no header prototype (pc_port/game/boot/). */
void func_8005C1EC(int enabled);
void func_80042538(void);

/* src/func_8004D084.c: card menu task 0x24 under the focus (update
 * 0x8004D2DC; list draw 0x8004FDE8; cell predicate 0x8004FDA4), reset a
 * mode-2 list to 0, show it, ensure the 0x13 caption window (draw
 * 0x8004C608) exists and points at 0x24, then record the save/load mode. */
void func_8004D084(int a0)
{
    pe_addr_t p;
    pe_addr_t q;
    pe_addr_t r;

    p = func_80062D2C(0x24u, func_80062CC4(), 0u, 0u);
    q = func_8006322C(0x24u, p, p);
    PE_StoreU32(p + 0x2Cu, 0x8004D2DCu);
    PE_StoreU32(q + 0x30u, 0x8004FDE8u);
    PE_StoreU32(q + 0x8Cu, 0x8004FDA4u);
    if ((int)PE_LoadU32(q + 0x48u) == 2)
        PE_StoreU32(q + 0x48u, 0u);
    func_80062CB8(q);
    if (func_80062A34(1u, 0x13u) == 0u) {
        r = func_80062D2C(0x13u, 0u, 0u, 0u);
        PE_StoreU32(r + 0x30u, 0x8004C608u);
    }
    PE_StoreU32(func_80062A34(1u, 0x13u) + 0x38u, 0x24u);
    PE_StoreU32(0x8009CF50u, (uint32_t)a0);
    if (a0 != 0) {
        func_8005C1EC(1);
        func_80042538();
    }
    func_8005DE88();
}

/* src/func_80047D74.c: item-menu update.  Bit 14: hide this list, open
 * window 2/0xB; bit 16: func_80048254; bit 6: func_80059FD0, close the item
 * windows, return focus to 2/0 (or 2/0x32) and post message 0x33E for the
 * other party slot.  func_80048254: port7_port.c (round 7). */
int func_80047D74(int a0, int a1)
{
    pe_addr_t p;
    pe_addr_t q;

    p = func_80062A20((pe_addr_t)a0, 0u);
    if ((a1 & 0x4000) != 0) {
        PE_StoreU32(p + 0x44u, 0xFFFFFFFFu);
        p = func_80062A34(2u, 0xBu);
        PE_StoreU32(p + 0x44u, 0u);
        PE_StoreU32(p + 0x48u, 0u);
        func_80062CB8(p);
        func_8005267C();
    } else if ((a1 & 0x10000) != 0) {
        func_80048254();
    } else if ((a1 & 0x40) != 0) {
        func_80059FD0();
        func_80062F3C(0x2Cu);
        func_80062F3C(0xBu);
        func_80062F3C(0xAu);
        func_80062F3C(5u);
        func_80062F3C(6u);
        func_80062F3C(7u);
        q = func_80062A34(2u, 0u);
        if (q == 0u)
            q = func_80062A34(2u, 0x32u);
        func_80062CB8(q);
        func_80048918(0x33E, -1,
                (int)func_80059F08((uint32_t)(PE_LoadU32(0x8009CF24u) == 0u)));
        func_80052634();
    }
    return 1;
}

/* src/func_8005033C.c: item-use confirm (a1 != 0): close the item windows,
 * refocus, bump the used item's +0x14 count and hand its id to
 * func_8004F490. */
void func_8005033C(int a0, int a1)
{
    int h;
    pe_addr_t p;

    (void)a0;
    if (a1 != 0) {
        h = func_800556E8(func_80063428(func_80062A34(2u, 0xDu)));
        func_80062F3C(0xFu);
        func_80062F3C(0xBu);
        func_80062F3C(0xDu);
        func_80062F3C(0x18u);
        func_80062F3C(0x30u);
        if (PE_LoadU32(0x8009CF0Cu) != 0u) {
            func_80062CB8(func_80062A34(2u, 0x32u));
        } else {
            if (func_80062CC4() != 0u)
                func_80063198(PE_LoadU32(func_80062CC4() + 4u));
            func_80063198(func_80062A34(1u, 0x1Bu));
        }
        PE_StoreU32(0x8009CF34u, 0u);
        func_8004C5DC();
        p = func_8005332C(h);
        PE_StoreU8(p + 0x14u, (unsigned char)(PE_LoadU8(p + 0x14u) + 1u));
        func_8004F490((uint32_t)(int)func_800534CC(h));
    }
}

/* src/func_8005DF6C.c: take the first queued D_8009D0E0 node whose f4 has a
 * `mask` bit, unlink it (fixing the D_8009D0E4 tail), return it to the
 * D_8009D0DC free list, and copy the 12-byte node (its `next` is now the old
 * free head, as retail) to *out; with no match, out->f4 = out->f8 = 0. */
void func_8005DF6C(int mask, pe_addr_t out)
{
    pe_addr_t p;
    pe_addr_t prev;

    if (out == 0u)
        return;
    if (PE_LoadU32(0x8009D0E0u) != 0u) {
        p = PE_LoadU32(0x8009D0E0u);
        prev = 0u;
        while (!((int)PE_LoadU32(p + 4u) & mask)) {
            prev = p;
            p = PE_LoadU32(p);
            if (p == 0u)
                break;
        }
        if (p != 0u) {
            if (prev != 0u)
                PE_StoreU32(prev, PE_LoadU32(p));
            else
                PE_StoreU32(0x8009D0E0u, PE_LoadU32(p));
            if (p == PE_LoadU32(0x8009D0E4u))
                PE_StoreU32(0x8009D0E4u, prev);
            PE_StoreU32(p, PE_LoadU32(0x8009D0DCu));
            PE_StoreU32(0x8009D0DCu, p);
            PE_StoreU32(out + 0u, PE_LoadU32(p + 0u));
            PE_StoreU32(out + 4u, PE_LoadU32(p + 4u));
            PE_StoreU32(out + 8u, PE_LoadU32(p + 8u));
            return;
        }
    }
    PE_StoreU32(out + 4u, 0u);
    PE_StoreU32(out + 8u, 0u);
}

/* src/func_80047A30.c: item-use confirm list update.  Confirm (bit 16) on
 * cell 0: close, apply the item (func_8005A318 — no pc_port implementation
 * yet: loud boundary with its four arguments), decrement the D_800A1888
 * stock counter when it is below 999 (the leaf's `*p -= (*p < 0x3E7)`),
 * close the item windows, refocus, post message 0x33E, and for each of the
 * two party slots that holds the equipped item (D_800C0E20[0] / [2]) outside
 * battle send equip command 2 / 3; cell 1 or cancel (bit 6): close and back
 * out.  func_80048918: port6_port.c. */
int func_80047A30(int a0, int a1)
{
    int k;
    pe_addr_t p;
    int i;
    int h;
    pe_addr_t q;

    if ((a1 & 0x10000) != 0) {
        k = func_80063428(func_80062A20((pe_addr_t)a0, 0u));
        switch (k) {
        case 0:
            func_80062F1C((pe_addr_t)a0);
            (void)PE_D_COMP_BOUNDARY4("func_8005A318", 0x8005A318u,
                PE_LoadU32(0x8009CF24u), PE_LoadU32(0x8009CF28u),
                PE_LoadU32(0x8009CF2Cu),
                PE_LoadU32(0x800A1888u + PE_LoadU32(0x8009CF2Cu) * 4u));
            p = 0x800A1888u + PE_LoadU32(0x8009CF2Cu) * 4u;
            PE_StoreU32(p, PE_LoadU32(p) -
                        (uint32_t)((int)PE_LoadU32(p) < 0x3E7));
            func_80062F3C(0x2Cu);
            func_80062F3C(0xBu);
            func_80062F3C(0xAu);
            func_80062F3C(5u);
            func_80062F3C(6u);
            func_80062F3C(7u);
            q = func_80062A34(2u, 0u);
            if (q == 0u)
                q = func_80062A34(2u, 0x32u);
            func_80062CB8(q);
            i = 0;
            func_80048918(0x33E, -1,
                (int)func_80059F08((uint32_t)(PE_LoadU32(0x8009CF24u) == 0u)));
            do {
                h = func_80059F08((uint32_t)i);
                if (func_80052F0C() == 0u) {
                    if (h == (signed char)PE_LoadU8(0x800C0E20u + 0u))
                        func_800512AC(2, 0u);
                    else if (h == (signed char)PE_LoadU8(0x800C0E20u + 2u))
                        func_800512AC(3, 0u);
                }
                i++;
            } while (i < 2);
            func_800525EC();
            break;
        case 1:
            func_80062F1C((pe_addr_t)a0);
            func_80052634();
            break;
        }
    } else if ((a1 & 0x40) != 0) {
        func_80062F1C((pe_addr_t)a0);
        func_80052634();
    }
    return 1;
}

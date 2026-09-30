/*
 * Hand adapters — menu/task constructors, record getters and VM handlers that
 * use retail stack temporaries (port_absent lane, batch 3).
 *
 * Conventions as in absent_lo_small_port.c.  A retail stack local whose
 * address is handed to a callee taking a guest address lives in the lane's
 * guest temp PE_HAND_LO_STACK_TEMP (storage substitution only; see
 * hand_lo_protos.h).  A pointer-returning leaf returns the guest address as
 * `pe_addr_t`.
 */
#include "pe_guest_decomp.h"

/* Decomp-derived callees (pc_port/game/decomp/, no header prototype). */
int func_80047FE0(int a0, int a1, int a2);
int func_8003FFBC(void);

/* src/func_80017BB4.c: decode **a0 into an 8-byte name, latch the token in
 * D_8009D280 with D_8009D1A0 |= 0x2000; token 0xA9400048 either clears a
 * 0x7D0 D_800A7918 counter or calls func_8006A25C.  Returns 0. */
int func_80017BB4(pe_addr_t a0)
{
    const pe_addr_t buf = PE_HAND_LO_STACK_TEMP;
    unsigned int x;

    func_8006E2D0(buf, PE_LoadU32(PE_LoadU32(a0)));
    x = PE_LoadU32(PE_LoadU32(a0));
    PE_StoreU32(0x8009D1A0u, PE_LoadU32(0x8009D1A0u) | 0x2000u);
    PE_StoreU32(0x8009D280u, x);
    if (PE_LoadU32(PE_LoadU32(a0)) == 0xA9400048u) {
        if ((int)PE_LoadU32(0x800A7918u) == 0x7D0)
            PE_StoreU32(0x800A7918u, 0u);
        else
            func_8006A25C();
    }
    return 0;
}

/* src/func_800183E8.c: for each of the three lists at state+0xA0..+0xA8,
 * walk nodes (next at +0x24) and on the first node whose +0xA halfword
 * equals **a0 set bit 6 of its +8 halfword; always returns 1. */
int func_800183E8(pe_addr_t a0)
{
    pe_addr_t base = PE_LoadU32(0x8009D2F0u);
    pe_addr_t p;
    unsigned int i = 0;

    do {
        p = PE_LoadU32(base + 0x28u * 4u);
        while (p != 0u) {
            if ((int)PE_LoadU16(p + 5u * 2u) == (int)PE_LoadU32(PE_LoadU32(a0))) {
                PE_StoreU16(p + 4u * 2u, (unsigned short)(PE_LoadU16(p + 8u) | 0x40u));
                return 1;
            }
            p = PE_LoadU32(p + 0x24u);
        }
        i++;
        base += 4u;
    } while (i < 3u);
    return 1;
}

/* src/func_800188C4.c: gather eight operands *a0[0..7] into an int[8] stack
 * buffer and store func_8001CAB0(state[0xA], state[0xC], buf, 4) at *a0[8]. */
int func_800188C4(pe_addr_t a0)
{
    const pe_addr_t buf = PE_HAND_LO_STACK_TEMP;
    pe_addr_t d = buf;
    pe_addr_t s = a0;
    unsigned int i = 0;
    int r;

    do {
        PE_StoreU32(d + 0u, PE_LoadU32(PE_LoadU32(s + 0u)));
        i++;
        PE_StoreU32(d + 4u, PE_LoadU32(PE_LoadU32(s + 4u)));
        s += 8u;
        d += 8u;
    } while (i < 4u);
    r = func_8001CAB0((int)PE_LoadU32(PE_LoadU32(0x8009D2F0u) + 0xAu * 4u),
                      (int)PE_LoadU32(PE_LoadU32(0x8009D2F0u) + 0xCu * 4u),
                      buf, 4u);
    PE_StoreU32(PE_LoadU32(a0 + 8u * 4u), (uint32_t)r);
    return 1;
}

/* src/func_800424B4.c: card slot record getter.  For slot a0 < 2 and
 * 0 <= a1 < D_800A0ED6[a0*1048], returns &D_800A0EF0[a0*1048 + a1*68] when
 * that record's +1 byte (D_800A0EF1) is set; otherwise 0. */
pe_addr_t func_800424B4(int a0, int a1)
{
    int rec;
    int off;

    if ((unsigned int)a0 < 2u) {
        if (a1 >= 0) {
            rec = a0 * 1048;
            if (a1 < (int)PE_LoadU8(0x800A0ED6u + (uint32_t)rec)) {
                off = a1 * 68;
                if (PE_LoadU8(0x800A0EF1u + (uint32_t)(off + rec)) != 0u)
                    return 0x800A0EF0u + (uint32_t)rec + (uint32_t)off;
            }
        }
    }
    return 0u;
}

/* src/func_8004784C.c: modal task 4 (draw 0x8004790C / update 0x80047A30)
 * with a child (draw 0x8004F950); D_8009CF14 = 5; activate the child; when
 * D_8009CF2C bit 0 is set, raise both by 0x1C. */
void func_8004784C(void)
{
    pe_addr_t p;
    pe_addr_t q;

    p = func_80062D2C(4u, func_80062CC4(), 0u, 1u);
    q = func_8006322C(4u, p, p);
    PE_StoreU32(p + 0x30u, 0x8004790Cu);
    PE_StoreU32(p + 0x2Cu, 0x80047A30u);
    PE_StoreU32(q + 0x30u, 0x8004F950u);
    PE_StoreU32(0x8009CF14u, 5u);
    func_80062CB8(q);
    if (PE_LoadU32(0x8009CF2Cu) & 1u) {
        PE_StoreU32(p + 0x38u, PE_LoadU32(p + 0x38u) - 0x1Cu);
        PE_StoreU32(q + 0x1Cu, PE_LoadU32(q + 0x1Cu) - 0x1Cu);
    }
}

/* src/func_80047F48.c: node = func_80062A20(a0, 0); func_80047FE0(node,
 * window(2, 6), a1); with a1 bit 12 also mark node[0x11] = -1 and open
 * window(2, 0x1C) at its last entry. */
int func_80047F48(int a0, int a1)
{
    pe_addr_t p;

    p = func_80062A20((pe_addr_t)a0, 0u);
    (void)func_80047FE0((int)p, (int)func_80062A34(2u, 6u), a1);
    if ((a1 & 0x1000) != 0) {
        PE_StoreU32(p + 0x11u * 4u, 0xFFFFFFFFu);
        p = func_80062A34(2u, 0x1Cu);
        PE_StoreU32(p + 0x11u * 4u, 0u);
        PE_StoreU32(p + 0x12u * 4u, PE_LoadU32(p + 0x16u * 4u) - 1u);
        func_80062CB8(p);
        func_8005267C();
    }
    return 1;
}

/* src/func_8004F23C.c: task 0x40 under the focus (update 0x8004F30C) with a
 * child (list draw 0x8004F2E4); activate it and close windows 1/0, 1/1,
 * 1/0x1B. */
void func_8004F23C(void)
{
    pe_addr_t p;
    pe_addr_t q;

    p = func_80062D2C(0x40u, func_80062CC4(), 0u, 0u);
    q = func_8006322C(0x40u, p, p);
    PE_StoreU32(p + 0x2Cu, 0x8004F30Cu);
    PE_StoreU32(q + 0x30u, 0x8004F2E4u);
    func_80062CB8(q);
    func_800631AC(func_80062A34(1u, 0u));
    func_800631AC(func_80062A34(1u, 1u));
    func_800631AC(func_80062A34(1u, 0x1Bu));
}

/* src/func_8004FE58.c: list-cell predicate over func_800424B4(slot, a0) with
 * slot = D_8009CEF4[9] - 0x25.  With D_8009CF50 set: record type != 3;
 * otherwise type == 1 and record+0x29 == func_8003FFBC().  A null record
 * is dereferenced exactly as in retail, which PE_Translate reports loudly. */
int func_8004FE58(int a0)
{
    pe_addr_t p;
    int r;
    int slot = (int)PE_LoadU32(PE_LoadU32(0x8009CEF4u) + 9u * 4u) - 0x25;

    if (PE_LoadU32(0x8009CF50u) != 0u) {
        p = func_800424B4(slot, a0);
        r = (PE_LoadU8(p) != 3u);
    } else {
        p = func_800424B4(slot, a0);
        if (PE_LoadU8(p) != 1u) {
            r = 0;
        } else {
            r = PE_LoadU8(p + 0x29u);
            r = ((r ^ func_8003FFBC()) == 0);
        }
    }
    return r;
}

/* src/func_80050E70.c: draw item row a0 — icon a0 + 0x91, then the value
 * from table a0 + 5 keyed by D_800A18EC[a0] (index + 1), then the label. */
void func_80050E70(int a0)
{
    const pe_addr_t tmp = PE_HAND_LO_STACK_TEMP;
    int v;
    int i;

    v = (int)PE_LoadU32(0x800A18ECu + (uint32_t)a0 * 4u);
    func_8005E8A4(2, 1);
    func_8005EB64((uint32_t)(a0 + 0x91));
    func_8005E8A4(0x42, -1);
    i = a0 + 5;
    func_8005B91C(i, v, tmp, 0u);
    func_800605F8((int32_t)PE_LoadU32(tmp) + 1);
    func_8005E8A4(2, 0);
    func_800437B4((uint32_t)i);
    func_8005E8A4(-0x44, 0xE);
}

/* src/func_80053F90.c: count the non-zero D_800C1EB8[0..99] ids whose record
 * (func_8005DB44(id - 1)) byte +6 equals a0. */
int func_80053F90(int a0)
{
    pe_addr_t p = 0x800C1EB8u;
    int i = 0;
    int count = 0;
    int v;

    do {
        v = (short)PE_LoadU16(p);
        if (v != 0) {
            if ((int)PE_LoadU8(func_8005DB44((unsigned int)(v - 1)) + 6u) == a0)
                count++;
        }
        i++;
        p += 2u;
    } while (i < 0x64);
    return count;
}

/* src/func_8005DD8C.c: message-table entry address for id a0.
 * idx = D_800A8028[a0 + D_800A804C]; 0 -> 0.  rec = D_800A8028 + D_800A802C;
 * p = rec + *(int *)(rec + 0x10); idx += 0x7F; idx >= *(u16 *)p -> 0;
 * else p + p[idx + 1] (signed short offset). */
int func_8005DD8C(int a0)
{
    const pe_addr_t base = 0x800A8028u;
    pe_addr_t rec;
    pe_addr_t p;
    int idx;

    idx = PE_LoadU8(base + (uint32_t)(a0 + (int)PE_LoadU32(0x800A804Cu)));
    if (idx == 0)
        return 0;
    rec = base + PE_LoadU32(0x800A802Cu);
    p = rec + PE_LoadU32(rec + 0x10u);
    idx += 0x7F;
    if ((unsigned int)idx >= (unsigned int)PE_LoadU16(p))
        return 0;
    return (int)(p + (uint32_t)(int)(short)PE_LoadU16(p + (uint32_t)(idx + 1) * 2u));
}

/* src/func_8005DE08.c: skip a0-th string.  base = &D_800A8054 - 11 words
 * (0x800A8028); p = base + D_800A8054; count = a0 >= 0 ?
 * base[a0 + D_800A804C] : 0xF; count == 0 -> 0; else advance p past `count`
 * NUL terminators and return it. */
/* func_8005DE08: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005DE08_port.c (src/func_8005DE08.c); hand port retired (port3 switch-over E). */

/* Decomp-derived callee (pc_port/game/decomp/, no header prototype). */
void func_800622B0(int a0);

/* src/func_800434C0.c: memory-card slot list per-cell draw for cell a.
 * Record = func_800424B4(D_8009CEF4[9] - 0x25, a); none -> nothing.
 * Type 1 (save): icon (animated when a is the focused cell, else a shaded
 * frame), title (+0x14 or +4 by func_8005BCB0), optional file number
 * (+0x2A), "File" + a + 1, level (+0x28 + 1), HP (+0x24 / +0x26), play time
 * (+0xC), then either the 0x60 caption (+0x29 set) or the day (+0x2C) and
 * the location name (message id +0x2E).  Type 2 / 3: the 0x41 / 0x76
 * "empty" / "other game" caption. */
void func_800434C0(int a)
{
    pe_addr_t p;
    pe_addr_t q;
    pe_addr_t x;

    p = func_800424B4((int)PE_LoadU32(PE_LoadU32(0x8009CEF4u) + 0x24u) - 0x25, a);
    if (p == 0u)
        return;
    switch (PE_LoadU8(p)) {
    case 1:
        if (a == func_80063428(PE_LoadU32(0x8009CEF4u))) {
            func_800622B0((int)PE_LoadU32(p + 0x20u));
        } else {
            func_8005E8A4(-2, -2);
            (void)func_800614AC((int)PE_LoadU32(p + 0x20u));
            q = PE_LoadU32(0x8009CEF4u);
            func_80061A3C(PE_LoadU32(q + 0x3Cu), PE_LoadU32(q + 0x40u), 1u);
            (void)func_800614AC((int)PE_LoadU32(0x800C0E44u));
            func_8005E8A4(2, 2);
        }
        func_8005E8A4(2, 2);
        if (func_8005BCB0() != 0)
            x = p + 0x14u;
        else
            x = p + 4u;
        func_8005F27C(x);
        func_8005E8A4(0x58, 0);
        if (PE_LoadU8(p + 0x2Au) != 0u) {
            func_8005EB64(0x95u);
            func_8005E8A4(0x1A, 2);
            func_8005EB64(0x96u);
            func_8005E8A4(0x22, 1);
            func_8005FA3C((int32_t)PE_LoadU8(p + 0x2Au) + 1);
            func_8005E8A4(4, -3);
        } else {
            func_8005E8A4(0x4A, 0);
        }
        func_8005F5B8(0x5Au);
        func_8005E8A4(0x19, 0);
        func_800605F8(a + 1);
        func_8005E8A4(-0xC4, 0xF);
        func_8005EB64(0x53u);
        func_8005E8A4(0x1E, 1);
        func_8005FB74((int32_t)PE_LoadU8(p + 0x28u) + 1);
        func_8005E8A4(8, -1);
        func_8005EB64(0x54u);
        func_8005E8A4(0x10, 1);
        func_8005FDF0((short)PE_LoadU16(p + 0x24u));
        func_8005EB64(0x4Cu);
        func_8005E8A4(5, 0);
        func_8005FDF0((short)PE_LoadU16(p + 0x26u));
        func_8005E8A4(8, -1);
        func_8005EB64(0x55u);
        func_8005E8A4(0x1A, 1);
        func_8006006C((int32_t)PE_LoadU32(p + 0xCu), 0u);
        func_8005E8A4(-0xC0, 0xC);
        if (PE_LoadU8(p + 0x29u) != 0u) {
            func_8005F5B8(0x60u);
        } else {
            func_8005F5B8(0xBu);
            func_8005E8A4(0x1E, 0);
            func_800605F8((short)PE_LoadU16(p + 0x2Cu));
            func_8005E8A4(0x14, 0);
            func_8005F27C((pe_addr_t)func_8005DD8C((short)PE_LoadU16(p + 0x2Eu)));
        }
        break;
    case 2:
        func_8005E8A4(0, 0x12);
        func_80064C54(0x41u);
        func_8005E8A4(0x5C, -0x10);
        break;
    case 3:
        func_8005E8A4(0, 0x12);
        func_80064C54(0x76u);
        func_8005E8A4(0x5C, -0x10);
        break;
    }
}

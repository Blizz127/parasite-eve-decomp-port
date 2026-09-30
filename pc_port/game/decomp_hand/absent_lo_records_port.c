/*
 * Hand adapters — record copiers, queue/packet helpers and menu builders
 * below 0x80060000 (port_absent lane, batch 4).
 *
 * Conventions as in absent_lo_small_port.c.  Where the leaf's C re-reads a
 * field through a pointer on every statement (no restrict, stores in
 * between), the adapter re-reads it too, so aliasing behaves as retail.
 * A dereference of a null guest pointer that the leaf performs
 * unconditionally is kept: PE_Translate reports it loudly instead of
 * reading the PS1 kernel area at 0x00000000.
 */
#include "pe_guest_decomp.h"

/* src/func_8003DBE4.c: copy the idx-th 32-byte Src record of
 * a1->table (a1 + 0x84) into Dst: nine halfwords to +0x34.., three words to
 * +0x48.. (Dst.w follows the halfwords, aligned to 4). */
void func_8003DBE4(pe_addr_t a0, pe_addr_t a1, short idx)
{
    unsigned k;

    for (k = 0; k < 9u; k++) {
        pe_addr_t e = PE_LoadU32(a1 + 0x84u) + (uint32_t)(int)idx * 32u;
        PE_StoreU16(a0 + 0x34u + k * 2u, PE_LoadU16(e + k * 2u));
    }
    for (k = 0; k < 3u; k++) {
        pe_addr_t e = PE_LoadU32(a1 + 0x84u) + (uint32_t)(int)idx * 32u;
        PE_StoreU32(a0 + 0x48u + k * 4u, PE_LoadU32(e + 20u + k * 4u));
    }
}

/* src/func_8003DD08.c: copy the a2-th 32-byte record of *(a1 + 0x84) twice
 * into *(a0 + 0x84): halfwords +0..+0x10 and words +0x14..+0x1C to the same
 * offsets, then again to +0x20.. (halfwords) and +0x34.. (words).  Both
 * bases are re-read per statement exactly as the leaf does. */
void func_8003DD08(pe_addr_t a0, pe_addr_t a1, short a2)
{
    uint32_t off = (uint32_t)(a2 * 32);
    unsigned k;

    for (k = 0; k < 9u; k++)
        PE_StoreU16(PE_LoadU32(a0 + 0x84u) + k * 2u,
                    PE_LoadU16(off + PE_LoadU32(a1 + 0x84u) + k * 2u));
    for (k = 0; k < 3u; k++)
        PE_StoreU32(PE_LoadU32(a0 + 0x84u) + 0x14u + k * 4u,
                    PE_LoadU32(off + PE_LoadU32(a1 + 0x84u) + 0x14u + k * 4u));
    for (k = 0; k < 9u; k++)
        PE_StoreU16(PE_LoadU32(a0 + 0x84u) + 0x20u + k * 2u,
                    PE_LoadU16(off + PE_LoadU32(a1 + 0x84u) + k * 2u));
    for (k = 0; k < 3u; k++)
        PE_StoreU32(PE_LoadU32(a0 + 0x84u) + 0x34u + k * 4u,
                    PE_LoadU32(off + PE_LoadU32(a1 + 0x84u) + 0x14u + k * 4u));
}

/* src/func_8003DF50.c: bind dst to source's index-th 16-byte record:
 * +0x32 = index, +0 = 0, +0x24 = source, +0x70 = rec.06, +0x2C/+0x2E =
 * rec.00/.02, +0x30 = rec.04 + dst->+0x70. */
void func_8003DF50(pe_addr_t dst, pe_addr_t source, short index)
{
    pe_addr_t rec;

    PE_StoreU16(dst + 0x32u, (unsigned short)index);
    PE_StoreU32(dst + 0x00u, 0u);
    PE_StoreU32(dst + 0x24u, source);
    rec = PE_LoadU32(source + 0x18u) + (uint32_t)(int)index * 16u;
    PE_StoreU16(dst + 0x70u, PE_LoadU16(rec + 6u));
    rec = PE_LoadU32(source + 0x18u) + (uint32_t)(int)index * 16u;
    PE_StoreU16(dst + 0x2Cu, PE_LoadU16(rec + 0u));
    rec = PE_LoadU32(source + 0x18u) + (uint32_t)(int)index * 16u;
    PE_StoreU16(dst + 0x2Eu, PE_LoadU16(rec + 2u));
    rec = PE_LoadU32(source + 0x18u) + (uint32_t)(int)index * 16u;
    PE_StoreU16(dst + 0x30u,
                (unsigned short)(PE_LoadU16(rec + 4u) + PE_LoadU16(dst + 0x70u)));
}

/* src/func_80018460.c: for every node in the three state lists whose +0xA
 * halfword equals **a0: clear bit 6 of +8; if its +4 word is set, move it to
 * +0, set +0x10 = 1 and clear bit 5 of +8.  Always returns 1. */
int func_80018460(pe_addr_t a0)
{
    pe_addr_t base = PE_LoadU32(0x8009D2F0u);
    pe_addr_t q;
    unsigned int i = 0;
    unsigned int one = 1u;

    do {
        q = PE_LoadU32(base + 0x28u * 4u);
        while (q != 0u) {
            if ((int)PE_LoadU16(q + 10u) == (int)PE_LoadU32(PE_LoadU32(a0))) {
                unsigned short f = PE_LoadU16(q + 8u);
                unsigned int v = PE_LoadU32(q + 4u);
                PE_StoreU16(q + 8u, (unsigned short)(f & 0xFFBFu));
                if (v != 0u) {
                    PE_StoreU32(q + 0u, v);
                    PE_StoreU32(q + 16u, one);
                    PE_StoreU16(q + 8u, (unsigned short)(PE_LoadU16(q + 8u) & 0xFFDFu));
                }
            }
            q = PE_LoadU32(q + 9u * 4u);
        }
        i++;
        base += 4u;
    } while (i < 3u);
    return 1;
}

/* src/func_80042170.c: memory-card LOAD entry for card slot `card`: when
 * its state byte is 1, re-arm the record for a 0x2000-byte read into
 * D_8009EED0 and point D_800A1854/58 at it. */
int func_80042170(int card, int slot)
{
    pe_addr_t s0 = 0x800A0ED4u + (uint32_t)card * 0x418u;
    int s1 = slot;

    if (PE_LoadU8(s0) != 1u)
        return 0;
    func_80042798();
    PE_StoreU32(s0 + 0x18u, 0x8009EED0u);
    PE_StoreU16(s0 + 0x14u, 0x2000u);
    PE_StoreU16(s0 + 0x16u, 0x0Au);
    PE_StoreU8(s0 + 7u, 2u);
    PE_StoreU8(s0 + 1u, 1u);
    PE_StoreU8(s0 + 0xBu, 5u);
    PE_StoreU8(s0 + 3u, (unsigned char)s1);
    PE_StoreU32(0x800A1854u, s0);
    PE_StoreU32(0x800A1858u, 0x2000u);
    (void)func_80071A24(0x8009EED0u, 0x2000u);
    return 0;
}

/* src/func_80047E94.c: task 0xB (update 0x80047F48) with a list child
 * (draw 0x8004FAF8) sized from the selected resource's +0x14 byte; link the
 * child with window (2, 6) when present, then refresh the resource block. */
void func_80047E94(int a0)
{
    pe_addr_t p;
    pe_addr_t q;
    pe_addr_t r;

    p = func_80062D2C(0xBu, (pe_addr_t)a0, 0u, 0u);
    q = func_8006322C(0xBu, p, p);
    PE_StoreU32(p + 0x2Cu, 0x80047F48u);
    PE_StoreU32(q + 0x30u, 0x8004FAF8u);
    func_80052E30(0u);
    func_800647D0(q, (int32_t)PE_LoadU8(func_8005332C(func_80059F08(1u)) + 0x14u));
    func_80064C20(q);
    r = func_80062A34(2u, 6u);
    if (r != 0u) {
        PE_StoreU32(q + 0x78u, r);
        PE_StoreU32(r + 0x7Cu, q);
    }
    func_80059C44();
}

/* src/func_80059C44.c: reset the D_8009D084..8C cursor, then for the two
 * selected party slots (func_80059F08(0/1)) resolve the item record and copy
 * its 32 bytes to D_800A204C / D_800A206C.  Id ranges: 0x100..0x17F ->
 * D_800BEEAC[id], 1..0xFF -> func_8005DB44(id - 1), 0x200..0x208 ->
 * D_8009DE64[id]; anything else (or an out-of-range slot) is the null
 * record, which retail copies from address 0. */
static pe_addr_t item_record_at(int i)
{
    int v;

    if (i >= 0 && i < (int)PE_LoadU32(0x8009D050u)) {
        v = (short)PE_LoadU16(PE_LoadU32(0x8009D048u) + (uint32_t)i * 2u);
        if ((unsigned int)(v - 0x100) < 0x80u)
            return ((uint32_t)v << 5) + 0x800BEEACu;
        if ((unsigned int)(v - 1) < 0xFFu)
            return func_8005DB44((unsigned int)(v - 1));
        if ((unsigned int)(v - 0x200) < 9u)
            return ((uint32_t)v << 5) + 0x8009DE64u;
        return 0u;
    }
    return 0u;
}

void func_80059C44(void)
{
    pe_addr_t p;
    unsigned k;

    PE_StoreU32(0x8009D084u, 0x800A1FE8u);
    PE_StoreU32(0x8009D08Cu, 0u);
    PE_StoreU32(0x8009D088u, 0u);
    p = item_record_at(func_80059F08(0u));
    for (k = 0; k < 32u; k += 4u)
        PE_StoreU32(0x800A204Cu + k, PE_LoadU32(p + k));
    p = item_record_at(func_80059F08(1u));
    for (k = 0; k < 32u; k += 4u)
        PE_StoreU32(0x800A206Cu + k, PE_LoadU32(p + k));
}

/* src/func_80059FD0.c: the inverse of func_80059C44 — for the two saved
 * party slots (D_8009D090 / D_8009D094) select the item list (the D_8009D04C
 * override list with D_800A1F84 flags and 4 columns when D_8009D098 /
 * D_8009D09C and D_8009D04C are set, else the D_800C0E48 inventory with
 * func_80052F70() entries, D_8009D05C flags and 2 columns), resolve the
 * item record and copy the 32-byte snapshot D_800A204C / D_800A206C back
 * into it (a null record is written through, as retail). */
static void item_list_select_59FD0(pe_addr_t use_override)
{
    if (PE_LoadU32(use_override) != 0u && PE_LoadU32(0x8009D04Cu) != 0u) {
        PE_StoreU32(0x8009D048u, PE_LoadU32(0x8009D04Cu));
        PE_StoreU32(0x8009D058u, 0x800A1F84u);
        PE_StoreU32(0x8009D064u, 4u);
        PE_StoreU32(0x8009D050u, PE_LoadU32(0x8009D054u));
    } else {
        PE_StoreU32(0x8009D048u, 0x800C0E48u);
        PE_StoreU32(0x8009D050u, (uint32_t)func_80052F70());
        PE_StoreU32(0x8009D058u, 0x8009D05Cu);
        PE_StoreU32(0x8009D064u, 2u);
    }
}

void func_80059FD0(void)
{
    pe_addr_t p;
    unsigned k;

    item_list_select_59FD0(0x8009D098u);
    p = item_record_at((int)PE_LoadU32(0x8009D090u));
    for (k = 0; k < 32u; k += 4u)
        PE_StoreU32(p + k, PE_LoadU32(0x800A204Cu + k));
    item_list_select_59FD0(0x8009D09Cu);
    p = item_record_at((int)PE_LoadU32(0x8009D094u));
    for (k = 0; k < 32u; k += 4u)
        PE_StoreU32(p + k, PE_LoadU32(0x800A206Cu + k));
}

/* src/func_8005BBE4.c: binary search (start 0x40, step 0x20) of the
 * ascending int table func_8005DB8C(a0) for the bracket containing key;
 * returns key - tbl[r - 1] for r = min(idx - 1, 0x62) when positive, else 0. */
int func_8005BBE4(int a0, int key)
{
    pe_addr_t tbl = func_8005DB8C(a0);
    int idx = 0x40;
    int step = 0x20;
    int r;

    do {
        pe_addr_t p = ((uint32_t)idx << 2) + tbl;
        if (key >= (int)PE_LoadU32(p)) {
            idx += step;
        } else if (key < (int)PE_LoadU32(p - 4u)) {
            idx -= step;
        } else {
            step = 0;
        }
        step >>= 1;
    } while (step != 0);
    idx--;
    r = 0x62;
    if (idx < 0x63)
        r = idx;
    if (r <= 0)
        return 0;
    return key - (int)PE_LoadU32(((uint32_t)r << 2) + tbl - 4u);
}

/* src/func_8005DEE4.c: pop a node from the D_8009D0DC free list and append it
 * to the D_8009D0E0/E4 queue with payload (a0, a1).  The leaf calls the empty
 * stub func_800527C0 with 0x1F when the queue head is set but the tail is
 * not; that callee's matched C is `void func_800527C0(void) {}`, so the
 * argument is unobservable and the host call takes none. */
void func_8005DEE4(int a0, int a1)
{
    pe_addr_t node = PE_LoadU32(0x8009D0DCu);
    pe_addr_t tail;
    pe_addr_t next;

    if (node != 0u) {
        next = PE_LoadU32(node);
        tail = PE_LoadU32(0x8009D0E4u);
        PE_StoreU32(node, 0u);
        PE_StoreU32(0x8009D0DCu, next);
        if (tail != 0u) {
            PE_StoreU32(tail, node);
        } else {
            if (PE_LoadU32(0x8009D0E0u) != 0u)
                func_800527C0();
            PE_StoreU32(0x8009D0E0u, node);
        }
        PE_StoreU32(0x8009D0E4u, node);
        PE_StoreU32(node + 4u, (uint32_t)a0);
        PE_StoreU32(node + 8u, (uint32_t)a1);
    }
}

/* Shared tail of func_8005E9C8 / func_8005EA8C: reserve a 3-word packet from
 * the D_8009D100 arena (limit D_8009D104 + 0x4000), fill it with
 * func_80075B84(p, rect) and splice it after the D_8009D11C OT entry.  On
 * arena exhaustion p stays 0 (after the empty func_800527C0(1) stub) and the
 * leaf still splices through it, exactly as retail. */
static void draw_area_packet_5E9C8(pe_addr_t rect)
{
    pe_addr_t p = 0u;
    pe_addr_t q;
    pe_addr_t d100 = PE_LoadU32(0x8009D100u);

    if (d100 + 12u < PE_LoadU32(0x8009D104u) + 0x4000u) {
        PE_StoreU32(0x8009D100u, d100 + 12u);
        p = PE_LoadU32(0x8009D100u) - 12u;
    } else {
        func_800527C0();
    }
    if (p != 0u)
        func_80075B84(p, rect);
    q = PE_LoadU32(0x8009D11Cu);
    PE_StoreU32(p, (PE_LoadU32(p) & 0xFF000000u) | (PE_LoadU32(q) & 0xFFFFFFu));
    PE_StoreU32(q, (PE_LoadU32(q) & 0xFF000000u) | (p & 0xFFFFFFu));
}

/* src/func_8005E9C8.c: draw-area packet for (a0, a1 [+0xE0 on the odd
 * buffer], a2, a3); the short[4] rect lives in the lane's guest temp. */
void func_8005E9C8(int a0, int a1, int a2, int a3)
{
    const pe_addr_t buf = PE_HAND_LO_STACK_TEMP;

    if ((int)PE_LoadU32(0x8009D108u) != 0)
        a1 += 0xE0;
    PE_StoreU16(buf + 0u, (unsigned short)a0);
    PE_StoreU16(buf + 2u, (unsigned short)a1);
    PE_StoreU16(buf + 4u, (unsigned short)a2);
    PE_StoreU16(buf + 6u, (unsigned short)a3);
    draw_area_packet_5E9C8(buf);
}

/* src/func_8005EA8C.c: full-screen (0, 0|0xE0, 0x140, 0xE0) draw-area packet. */
void func_8005EA8C(void)
{
    const pe_addr_t buf = PE_HAND_LO_STACK_TEMP;

    PE_StoreU16(buf + 4u, 0x140u);
    PE_StoreU16(buf + 0u, 0u);
    PE_StoreU16(buf + 2u, 0u);
    PE_StoreU16(buf + 6u, 0xE0u);
    if ((int)PE_LoadU32(0x8009D108u) != 0)
        PE_StoreU16(buf + 2u, 0xE0u);
    draw_area_packet_5E9C8(buf);
}

/* src/func_8003E474.c: scroll a mesh's textured primitives — for each of the
 * double-buffered POLY_GT4 pairs (count hdr+8) then POLY_GT3 pairs (count
 * hdr+0xA) at m+0x54, add dv/du to every vertex's v/u byte and dc to the
 * CLUT halfword.  Quirk kept from the leaf: POLY_GT4 vertex 3 gets only v3,
 * not u3.  Layouts are the retail GPU packets (Mesh is guest layout: hdr at
 * +0, prims at +0x54): GT4 0x34 bytes (u0/v0 +0xC, clut +0xE, u1/v1 +0x18,
 * u2/v2 +0x24, u3/v3 +0x30), GT3 0x28 bytes. */
void func_8003E474(pe_addr_t m, int du, int dv, signed char dc)
{
    static const uint32_t uv4[4] = {0x0Cu, 0x18u, 0x24u, 0x30u};
    pe_addr_t p = PE_LoadU32(m + 0x54u);
    int i;
    int j;
    unsigned k;

#define ADD8(a, d) PE_StoreU8((a), (unsigned char)(PE_LoadU8(a) + (d)))
    for (i = 0; i < (int)PE_LoadU16(PE_LoadU32(m) + 8u); i++) {
        for (j = 0; j < 2; j++, p += 0x34u) {
            for (k = 0; k < 3u; k++) {
                ADD8(p + uv4[k] + 1u, dv);
                ADD8(p + uv4[k], du);
            }
            ADD8(p + uv4[3] + 1u, dv);
            PE_StoreU16(p + 0x0Eu, (unsigned short)(PE_LoadU16(p + 0x0Eu) + dc));
        }
    }
    for (i = 0; i < (int)PE_LoadU16(PE_LoadU32(m) + 0xAu); i++) {
        for (j = 0; j < 2; j++, p += 0x28u) {
            for (k = 0; k < 3u; k++) {
                ADD8(p + uv4[k] + 1u, dv);
                ADD8(p + uv4[k], du);
            }
            PE_StoreU16(p + 0x0Eu, (unsigned short)(PE_LoadU16(p + 0x0Eu) + dc));
        }
    }
#undef ADD8
}

/* src/func_8005BF44.c: equip an owned item by its record id.  Find the
 * 32-byte D_800C0EAC entry whose +4 byte is `id` (0x1000-byte table); if
 * found, select the inventory list (D_800C0E48, func_80052F70() entries,
 * D_8009D05C flags, 2 columns), find its 0x100 + index tag in the list, and
 * by the resolved record's +6 type byte store the list slot into
 * D_800C0E20[0] (types 1..8) or D_800C0E22[0] (type 9); a missing tag,
 * record or other type yields -1.  Returns the sign bit of the final id —
 * note an unmatched `id` is returned unchanged through that sign test, as
 * the leaf does. */
int func_8005BF44(int id)
{
    const pe_addr_t base = 0x800C0EACu;
    pe_addr_t p = base;
    pe_addr_t q;
    pe_addr_t lst;
    pe_addr_t res;
    int tag;
    int t;
    int k;

    while (p < base + 0x1000u && (int)PE_LoadU8(p + 4u) != id)
        p += 0x20u;
    if (p < base + 0x1000u) {
        PE_StoreU32(0x8009D048u, base - 0x64u);
        PE_StoreU32(0x8009D050u, (uint32_t)func_80052F70());
        PE_StoreU32(0x8009D058u, 0x8009D05Cu);
        PE_StoreU32(0x8009D064u, 2u);
        tag = (int)((p - base) >> 5) + 0x100;
        lst = PE_LoadU32(0x8009D048u);
        q = lst;
        while (q < lst + PE_LoadU32(0x8009D050u) * 2u &&
               (short)PE_LoadU16(q) != tag)
            q += 2u;
        if (q < lst + PE_LoadU32(0x8009D050u) * 2u)
            t = (int)((q - lst) >> 1);
        else
            t = -1;
        id = t;
        if (id >= 0) {
            res = item_record_at(id);
            k = 0;
            if (res != 0u)
                k = PE_LoadU8(res + 6u);
            if (k == 0)
                id = -1;
            else if (k < 9)
                PE_StoreU8(0x800C0E20u, (unsigned char)id);
            else if (k == 9)
                PE_StoreU8(0x800C0E22u, (unsigned char)id);
            else
                id = -1;
        }
    }
    return (int)((unsigned int)id >> 31);
}

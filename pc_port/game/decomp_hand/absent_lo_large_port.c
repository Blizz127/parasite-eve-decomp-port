/*
 * Hand adapters — the large remaining leaves below 0x80060000 (port_absent
 * lane, batch 9).  Conventions as in absent_lo_small_port.c; register pins
 * and asm fences in the leaves are dropped (codegen hints only).
 */
#include "pe_guest_decomp.h"

/* Decomp-derived callees (pc_port/game/decomp/, no header prototype). */
void func_8003FFAC(int value);
int func_80042964(int a0);

/* Item-record resolver shared with absent_lo_records_port.c's leaves:
 * the id at list[i] -> 0x100.. D_800BEEAC[id], 1..0xFF func_8005DB44(id-1),
 * 0x200..0x208 D_8009DE64[id], else 0.  (No bounds check: callers check.) */
static pe_addr_t item_record_for_id(int v)
{
    if ((unsigned int)(v - 0x100) < 0x80u)
        return ((uint32_t)v << 5) + 0x800BEEACu;
    if ((unsigned int)(v - 1) < 0xFFu)
        return func_8005DB44((unsigned int)(v - 1));
    if ((unsigned int)(v - 0x200) < 9u)
        return ((uint32_t)v << 5) + 0x8009DE64u;
    return 0u;
}

/* src/func_800588EC.c: select the item-list view.
 * a0 == 0: the 100-entry D_800C1EB8 list.
 * a0 != 0: the 0x52-entry D_800C1F80 list; if it does not already hold
 * key-item 0x204 and the inventory (D_8009D048, D_8009D050 entries) holds a
 * type-6 item, 0x204 is written into the first free slot (the scan stops at
 * index 0x51); the view length is 0x50 + (0x204 present).
 * Both: D_8009D080 = length, then D_8009D04C/54 mirror D_8009D07C/80. */
void func_800588EC(int a0)
{
    int i;
    int found = 0;
    int n;
    int cnt;
    pe_addr_t t;
    pe_addr_t base;

    if (a0 == 0) {
        PE_StoreU32(0x8009D07Cu, 0x800C1EB8u);
        n = 0x64;
    } else {
        PE_StoreU32(0x8009D07Cu, 0x800C1F80u);
        i = 0;
        t = 0x800C1F80u;
        while (i < 0x52 && (short)PE_LoadU16(t) != 0x204) {
            i++;
            t += 2u;
        }
        found = i < 0x52;
        if (!found) {
            i = 0;
            cnt = (int)PE_LoadU32(0x8009D050u);
            if (cnt > 0) {
                do {
                    pe_addr_t p = 0u;
                    if (i >= 0 && i < cnt)
                        p = item_record_for_id((short)PE_LoadU16(
                            PE_LoadU32(0x8009D048u) + (uint32_t)i * 2u));
                    if (p != 0u && PE_LoadU8(p + 6u) == 6u)
                        break;
                    cnt = (int)PE_LoadU32(0x8009D050u);
                    i++;
                } while (i < cnt);
                if (i < (int)PE_LoadU32(0x8009D050u)) {
                    i = 0;
                    base = PE_LoadU32(0x8009D07Cu);
                    t = base;
                    while (i < 0x51 && PE_LoadU16(t) != 0u) {
                        i++;
                        t += 2u;
                    }
                    if (i < 0x52) {
                        PE_StoreU16(base + (uint32_t)i * 2u, 0x204u);
                        found = 1;
                    }
                }
            }
        }
        n = found + 0x50;
    }
    PE_StoreU32(0x8009D080u, (uint32_t)n);
    PE_StoreU32(0x8009D04Cu, PE_LoadU32(0x8009D07Cu));
    PE_StoreU32(0x8009D054u, PE_LoadU32(0x8009D080u));
}

/* src/func_8004D6D4.c: memory-card slot list update (window a).
 * Cancel (bit 6): close 0x3F and this window, release the card, back out.
 * Confirm (bit 16) on cell r = func_8006346C(list) (D_8009CF48):
 *  - no record / load mode with an empty slot -> func_800526C4 (buzz);
 *  - save mode (D_8009CF50) over an existing save: build the 0x29 overwrite
 *    confirm (draw 0x80044E14, update 0x80044E98, list draw 0x8004F950,
 *    D_8009CF14 = 0x6C) with message 0x44, width max(0x78, text) + 0x14,
 *    D_8009CFA8 = func_80050544;
 *  - save mode over an empty slot: with fewer than 15 free blocks open the
 *    0x45 notice (callback func_800504F4), else the slot-a + 0x47 message
 *    unless window 1/0x28 is up;
 *  - load mode: the 0x46 notice with callback func_8005051C;
 *  then func_800525EC. */
int func_8004D6D4(int a, int flags)
{
    pe_addr_t obj;
    pe_addr_t nw;
    pe_addr_t p;
    int r;
    int m;
    int h;
    int s;
    int t;

    obj = func_80062A20((pe_addr_t)a, 0u);
    if (flags & 0x40) {
        func_80062F3C(0x3Fu);
        func_80062F1C((pe_addr_t)a);
        func_80042A10();
        func_80052634();
        return 1;
    }
    if (flags & 0x10000) {
        r = func_8006346C(obj);
        PE_StoreU32(0x8009CF48u, (uint32_t)r);
        if (r < 0)
            goto fail;
        p = func_800424B4((int)PE_LoadU32(0x8009CF44u), r);
        if (p == 0u)
            goto fail;
        m = (int)PE_LoadU32(0x8009CF50u);
        if (m == 0 && PE_LoadU8(p) == 2u)
            goto fail;
        PE_StoreU32(0x8009CF4Cu, PE_LoadU8(p));
        if (m != 0) {
            func_8003FFAC((int)PE_LoadU32(0x8009CFF8u));
            if ((int)PE_LoadU32(0x8009CF4Cu) != 2) {
                nw = func_80062D2C(0x29u, obj, 0u, 1u);
                obj = func_8006322C(0x29u, nw, nw);
                PE_StoreU32(nw + 0x30u, 0x80044E14u);
                PE_StoreU32(nw + 0x2Cu, 0x80044E98u);
                PE_StoreU32(obj + 0x30u, 0x8004F950u);
                PE_StoreU32(0x8009CF14u, 0x6Cu);
                func_80062CB8(obj);
                t = (int)PE_LoadU32(0x8009CF10u);
                PE_StoreU32(obj + 0x44u, 1u);
                func_80052E30((uint32_t)t);
                PE_StoreU8(0x800A1980u, 0xFFu);
                func_80052C08(0x800A1980u, func_8005DC4C(0x44u));
                PE_StoreU32(0x8009CFA0u, 0u);
                if ((int)func_8005F1A0(0x800A1980u) < 0x78)
                    h = 0x78;
                else
                    h = (int)func_8005F1A0(0x800A1980u);
                PE_StoreU32(nw + 0x34u, (uint32_t)(h + 0x14));
                PE_StoreU32(nw + 0x38u, 0x32u);
                PE_StoreU32(nw + 0x18u, (uint32_t)((0x12C - h) >> 1));
                PE_StoreU32(obj + 0x18u,
                            (uint32_t)(((int)PE_LoadU32(nw + 0x34u) - 0x80) >> 1));
                PE_StoreU32(0x8009CFA8u, 0x80050544u);
                PE_StoreU32(obj + 0x1Cu, PE_LoadU32(nw + 0x38u) - 0x14u);
            } else if (func_80042964((int)PE_LoadU32(0x8009CF44u)) < 0xF) {
                func_80062F3C(0x1Fu);
                func_8004D978(0x45);
                func_80042B50(0x800504F4u);
            } else {
                s = (int)PE_LoadU32(0x8009CF44u);
                if (func_80062A34(1u, 0x28u) == 0u)
                    func_8004CE28((uint32_t)(s + 0x47),
                                  PE_LoadU32(0x8009CF50u) + 0x42u);
            }
        } else {
            func_80062F3C(0x1Fu);
            func_8004D978(0x46);
            func_80042B50(0x8005051Cu);
        }
        func_800525EC();
        return 1;
    fail:
        func_800526C4();
    }
    return 1;
}

/* src/func_8005D020.c: consume the one-shot item.  Select the inventory
 * list; find the first 0x100-range entry whose record byte +4
 * (D_800BEEB0[id * 0x20]) is 0x61; remove it (clearing its D_800C0EAC
 * ownership byte) and, when it was the equipped D_800C0E22 slot, unequip it
 * through func_80059A40 / func_80054E4C / func_80054CF8 / command 3.  Then
 * message 0x93, and re-point D_800C0E20 at the first type-7 item (or -1),
 * clear D_8009D028 and send equip command 2.  The unequip scratch int[4] is
 * the lane's guest temp.  A null record in the type scan is dereferenced
 * as retail does (loud in PE_Translate). */
void func_8005D020(void)
{
    const pe_addr_t local = PE_HAND_LO_STACK_TEMP;
    int i = 0;
    int n;
    int v;
    int idx;
    unsigned int u;
    pe_addr_t p;
    pe_addr_t q;
    pe_addr_t res;

    PE_StoreU32(0x8009D048u, 0x800C0E48u);
    n = (int)func_80052F70();
    PE_StoreU32(0x8009D058u, 0x8009D05Cu);
    PE_StoreU32(0x8009D050u, (uint32_t)n);
    PE_StoreU32(0x8009D064u, 2u);
    if (n > 0) {
        p = PE_LoadU32(0x8009D048u);
        for (;;) {
            u = PE_LoadU16(p);
            if ((u - 0x100u) < 0x80u &&
                PE_LoadU8(0x800BEEB0u + (uint32_t)((int)(short)u * 0x20)) == 0x61u)
                break;
            i++;
            p += 2u;
            if (i >= n)
                break;
        }
        if (i < (int)PE_LoadU32(0x8009D050u)) {
            pe_addr_t list = PE_LoadU32(0x8009D048u);
            if (list == 0x800C0E48u && (signed char)PE_LoadU8(0x800C0E22u) == i)
                (void)func_80059A40(local);
            v = (short)PE_LoadU16(PE_LoadU32(0x8009D048u) + (uint32_t)i * 2u);
            PE_StoreU16(PE_LoadU32(0x8009D048u) + (uint32_t)i * 2u, 0u);
            if (v >= 0x100)
                PE_StoreU8(0x800C0EACu + ((uint32_t)(v - 0x100) << 5), 0u);
            if (PE_LoadU32(0x8009D048u) == 0x800C0E48u &&
                (signed char)PE_LoadU8(0x800C0E22u) == i) {
                PE_StoreU8(0x800C0E22u, 0xFFu);
                (void)func_80054E4C((int32_t)PE_LoadU32(local));
                func_80054CF8();
                func_800512AC(3, 0u);
            }
        }
    }
    (void)func_80053D2C(0x93);
    q = PE_LoadU32(0x8009D048u);
    idx = -1;
    if (q < q + PE_LoadU32(0x8009D050u) * 2u) {
        do {
            res = item_record_for_id((short)PE_LoadU16(q));
            if (PE_LoadU8(res + 6u) == 7u)
                break;
            q += 2u;
        } while (q < PE_LoadU32(0x8009D048u) + PE_LoadU32(0x8009D050u) * 2u);
        if (q < PE_LoadU32(0x8009D048u) + PE_LoadU32(0x8009D050u) * 2u)
            idx = (int)((q - PE_LoadU32(0x8009D048u)) >> 1);
    }
    PE_StoreU8(0x800C0E20u, (unsigned char)idx);
    PE_StoreU32(0x8009D028u, 0u);
    func_800512AC(2, 0u);
}

/* src/func_8002F300.c: sewer-exit scene setup.  Refresh the player state
 * (func_800293F4(0)), rewrite the 64 gradient colour bytes of the
 * D_800B00EC.. / D_800B692C.. packets, set D_8009D28C = 2 and
 * D_8009D2E8 bit 0, zero the player's +0x68/+0x6C/+0x70 velocity, clear
 * +0x98 bit 8, set D_800B0CD8 bit 15, and unless D_8009D1A0 & 0x1800 bind
 * handler 0x14 and spawn effect 0x45B at the player's position. */
/* func_8002F300: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8002F300_port.c (src/func_8002F300.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

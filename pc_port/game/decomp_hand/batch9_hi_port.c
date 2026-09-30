/*
 * Hand adapters — batch 9 (>= 0x80060000).  Bodies follow the matched leaves
 * src/func_XXXXXXXX.c; pointers are guest addresses.
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "hand_hi_protos.h"

extern unsigned int func_80071A54(void);
extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);
extern void func_8008F1B0(pe_addr_t pe_a0, unsigned int a1);
extern void func_80089960(void);
extern void func_80089B28(void);
extern void func_80089CF0(void);

/* src/func_800DFB78.c: effect-trigger gate over obj = a0->+8 and the
 * sub-record at a0 + 0xC.  Returns 1 when: a0[0x19] bit 2 and obj[0xE] < 2;
 * sub[0xD] bit 1 consumes obj->+0x98 bit 19; sub[0xD] bit 0 consumes bit
 * 18; or obj's record (*obj) has bits 0x1800 or ((bits >> 1) & 7) > 0. */
int func_800DFB78(pe_addr_t a0)
{
    pe_addr_t obj = PE_LoadU32(a0 + 8u);
    pe_addr_t sub = a0 + 0xCu;
    uint32_t word, bits;
    pe_addr_t rec;

    if ((PE_LoadU8(a0 + 0x19u) & 4u) != 0u && PE_LoadU8(obj + 0xEu) < 2u)
        return 1;
    if ((PE_LoadU8(sub + 0xDu) & 2u) != 0u) {
        word = PE_LoadU32(obj + 0x98u);
        if (word & 0x80000u) {
            PE_StoreU32(obj + 0x98u, word & 0xFFF7FFFFu);
            return 1;
        }
    }
    if ((PE_LoadU8(sub + 0xDu) & 1u) != 0u) {
        word = PE_LoadU32(obj + 0x98u);
        if (word & 0x40000u) {
            PE_StoreU32(obj + 0x98u, word & 0xFFFBFFFFu);
            return 1;
        }
    }
    rec = PE_LoadU32(obj);
    if (rec == 0u)
        return 0;
    bits = PE_LoadU32(rec);
    if (bits & 0x1800u)
        return 1;
    if ((((int)(bits >> 1)) & 7) > 0)
        return 1;
    return 0;
}

/* src/func_8008B1FC.c: route a volume command (rec->+4 & 0x7F, << 16) to
 * the bank whose id matches rec->+0x10 (0 = bank 0): bank 0 writes
 * D_8009D2C8->+0x48 / +0x50 and marks D_800B8AC0; bank 1 (id == +0xBC)
 * writes +0xB0 / +0xB8 with the record advanced by 0x68 and marks
 * D_800BA560. */
void func_8008B1FC(pe_addr_t a0)
{
    int id = (int)PE_LoadU32(a0 + 0x10u);
    pe_addr_t s = PE_LoadU32(0x8009D2C8u);
    int v;

    if (id == 0 || (unsigned int)id == PE_LoadU16(s + 0x54u)) {
        PE_StoreU32(s + 0x48u, (PE_LoadU32(a0 + 4u) & 0x7Fu) << 16);
        PE_StoreU16(s + 0x50u, 0u);
        func_8008AB9C(0x800B8AC0u);
        return;
    }
    if ((unsigned int)id != PE_LoadU16(s + 0xBCu))
        return;
    v = (int)PE_LoadU32(a0 + 4u);
    PE_StoreU32(0x8009D2C8u, s + 0x68u);
    PE_StoreU16(s + 0xB8u, 0u);
    PE_StoreU32(s + 0xB0u, (uint32_t)((v & 0x7F) << 16));
    func_8008AB9C(0x800BA560u);
    PE_StoreU32(0x8009D2C8u, PE_LoadU32(0x8009D2C8u) - 0x68u);
}

/* src/func_800CC480.c: draw state (page 3, 0x10, 32x32, blend 2); scale
 * [0x14] = max(0, (short)(0x80 - a1->+2 * 2)); point = a2 +6/+8/+A; words
 * +0x10/+0x14/+0x18 = r, 2r, r with r = (short)a2->+4; emit D_800F3430. */
/* func_800CC480: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CC480_port.c (src/func_800CC480.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_800CD1FC.c: jittered position around D_800E27F8/FA — x +
 * rand % 201 - 100, y + rand % 101 - 50 (signed int remainders, as the leaf
 * declares func_80071A54 returning int), z = D_800E27FC; a2[2] = 0x7F,
 * a2[3] = 0x544. */
void func_800CD1FC(int a0, int a1, pe_addr_t a2)
{
    int u;

    (void)a0; (void)a1;
    PE_StoreU16(a2 + 8u, (uint16_t)(PE_LoadU16(0x800E27F8u) + (int)func_80071A54() % 201 - 0x64));
    PE_StoreU16(a2 + 10u, (uint16_t)(PE_LoadU16(0x800E27FAu) + (int)func_80071A54() % 101 - 0x32));
    u = PE_LoadU16(0x800E27FCu);
    PE_StoreU16(a2 + 4u, 0x7Fu);
    PE_StoreU16(a2 + 6u, 0x544u);
    PE_StoreU16(a2 + 12u, (uint16_t)u);
}

/* src/func_800625B8.c: pop a node from the D_8009D158 free list (an empty
 * list calls func_800527C0(0xA), whose body is empty, and then reads NULL
 * exactly as retail), push it on D_8009D154, set +4 = a0, clear the child
 * slots [2..5] and words [6..12], and when a1 is given store the node in
 * a1's first free child slot (none free -> func_800527C0(0xB)). */
pe_addr_t func_800625B8(int a0, pe_addr_t a1)
{
    pe_addr_t s = PE_LoadU32(0x8009D158u);
    pe_addr_t nx, pv;
    int i;

    if (s == 0u)
        func_800527C0();
    nx = PE_LoadU32(s);
    pv = PE_LoadU32(0x8009D154u);
    PE_StoreU32(0x8009D154u, s);
    PE_StoreU32(s + 4u, (uint32_t)a0);
    PE_StoreU32(s + 44u, 0u);
    PE_StoreU32(s + 48u, 0u);
    PE_StoreU32(0x8009D158u, nx);
    PE_StoreU32(s, pv);
    for (i = 3; i >= 0; i--)
        PE_StoreU32(s + 8u + (uint32_t)i * 4u, 0u);
    PE_StoreU32(s + 28u, 0u);
    PE_StoreU32(s + 24u, 0u);
    PE_StoreU32(s + 36u, 0u);
    PE_StoreU32(s + 32u, 0u);
    PE_StoreU32(s + 40u, 0u);
    if (a1 != 0u) {
        for (i = 0; i < 4; i++)
            if (PE_LoadU32(a1 + 8u + (uint32_t)i * 4u) == 0u)
                break;
        if (i < 4)
            PE_StoreU32(a1 + 8u + (uint32_t)i * 4u, s);
        else
            func_800527C0();
    }
    return s;
}

/* src/func_8006BD68.c: when D_800B0CE0 != D_800B0CE1, DrawSync-like
 * func_80074DC0(0), then (re)issue the CD read of file entry
 * (s8)D_800B0CE0 + 0x2B from the D_800930D8 sector table into the buffer at
 * D_800B0CD8 + 0x194 until the issue succeeds and the poll
 * (func_8006E7E8) returns 0 — a -1 poll restarts the issue; then
 * func_800718D0(buffer) and D_800B0CE1 = D_800B0CE0. */
int func_8006BD68(void)
{
    const pe_addr_t g = 0x800B0CD8u;
    int r;

    if ((signed char)PE_LoadU8(g + 8u) == (signed char)PE_LoadU8(g + 9u))
        return 0;
    (void)func_80074DC0(0);
    for (;;) {
        do {
            int idx = ((signed char)PE_LoadU8(g + 8u) + 0x2B) * 2;
            int lo = PE_LoadU16(0x800930D8u + (uint32_t)idx);
            int hi = PE_LoadU16(0x800930D8u + 2u + (uint32_t)idx);

            r = func_8006E6A8((int)PE_LoadU32(g + 0x100u) + lo,
                              PE_LoadU32(g + 0x194u), hi - lo);
        } while (r == -1);
        do {
            r = func_8006E7E8();
        } while (r != 0 && r != -1);
        if (r == 0)
            break;
    }
    (void)func_800718D0(PE_LoadU32(g + 0x194u));
    PE_StoreU8(g + 9u, PE_LoadU8(g + 8u));
    return 0;
}

/* src/func_8008C46C.c: for the 12 records (stride 0x11C) of D_800BC000
 * whose D_800BCD50 bit (from 0x1000) is set and whose +0x2C word lacks bit
 * 0x2000000: set the bit in D_800BCD5C, func_8008F1B0(record, bit), clear
 * +0x38.  Then D_8009D2C4 |= 0x10 and the three voice-update passes. */
/* func_8008C46C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8008C46C_port.c (src/func_8008C46C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

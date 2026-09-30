/*
 * PE-BTL49 — func_80013C34: opcode 0x4B turn-toward
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 148 words 0x80013C34..0x80013E84, SHA-256 0c508334…5de2.
 * D_800910A0[0x4B]. jal 79FB4. v0=1.
 *
 * *arg0==0 uses D254; else walks D20C for type/id with
 * +(0x98)&0x10 clear. First visit stores the actor at
 * D300+0x18, step at +0x14, and sets task+8 bit 0x20.
 * desired = (0x1400 - ratan2(dZ,dX)) & 0xFFF. Steps
 * D2F0+0x3A by at most the step. If not yet facing,
 * CE00 -= 0x14 and D300+0x10 = 1, return 0 (retry next visit).
 *
 * Live type-2 fork 0x6B0: (0,0,0x400) toward type 0.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009CE00 0x8009CE00u

static pe_addr_t pe_4b_find(pe_addr_t args)
{
    uint32_t type;
    uint32_t idb;
    pe_addr_t actor;

    type = PE_LoadU32(PE_LoadU32(args));
    if (type == 0u)
        return PE_LoadU32(GA_D_8009D254);

    idb = PE_LoadU32(PE_LoadU32(args + 4u));
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        if (PE_LoadU8(actor + 0x0Cu) == (uint8_t)type
            && PE_LoadU8(actor + 0x0Du) == (uint8_t)idb
            && (PE_LoadU32(actor + 0x98u) & 0x10u) == 0u)
            return actor;
        actor = PE_LoadU32(actor + 4u);
    }
    return 0u;
}

int func_80013C34(pe_addr_t args)
{
    pe_addr_t task;
    pe_addr_t self;
    pe_addr_t target;
    uint16_t flags;
    int32_t step;
    int32_t desired;
    int32_t current;
    int32_t heading;
    int32_t delta;
    int32_t dx;
    int32_t dz;

    task = PE_LoadU32(GA_D_8009D300);
    flags = PE_LoadU16(task + 8u);
    if ((flags & 0x20u) == 0u) {
        target = pe_4b_find(args);
        if (target == 0u)
            return 1;
        step = (int32_t)PE_LoadU32(PE_LoadU32(args + 8u));
        PE_StoreU32(task + 0x18u, target);
        PE_StoreU16(task + 8u, (uint16_t)(flags | 0x20u));
        PE_StoreU32(task + 0x14u, (uint32_t)step);
    } else {
        target = PE_LoadU32(task + 0x18u);
        if ((PE_LoadU32(target + 0x98u) & 0x10u) != 0u) {
            PE_StoreU16(task + 8u, (uint16_t)(flags & 0xFFDFu));
            return 1;
        }
        step = (int32_t)PE_LoadU32(task + 0x14u);
    }

    self = PE_LoadU32(GA_D_8009D2F0);
    dx = (int32_t)PE_LoadU32(self + 0x28u) - (int32_t)PE_LoadU32(target + 0x28u);
    dz = (int32_t)PE_LoadU32(self + 0x30u) - (int32_t)PE_LoadU32(target + 0x30u);
    desired = (0x1400 - func_80079FB4(dz >> 16, dx >> 16)) & 0xFFF;
    current = (int32_t)(int16_t)PE_LoadU16(self + 0x3Au);
    heading = desired;
    if (desired != current) {
        if (current < desired) {
            delta = desired - current;
            if (delta < 0x800) {
                if (step < delta)
                    heading = current + step;
            } else if (step < delta) {
                heading = current - step;
                if (heading < 0) {
                    if ((current + 0x1000 - desired) < step)
                        heading = desired;
                }
            }
        } else {
            delta = current - desired;
            if (delta < 0x800) {
                if (step < delta)
                    heading = current - step;
            } else if (step < delta) {
                heading = current + step;
                if (heading >= 0x1001) {
                    if ((desired + 0x1000 - current) < step)
                        heading = desired;
                }
            }
        }
    }
    heading &= 0xFFF;
    PE_StoreU16(self + 0x3Au, (uint16_t)heading);
    if (heading != desired) {
        PE_StoreU32(GA_D_8009CE00, PE_LoadU32(GA_D_8009CE00) - 0x14u);
        PE_StoreU32(task + 0x10u, 1u);
        /* Retail yields here (src/func_80013C34.c: `return 0`). Returning 1
         * re-ran the rewound opcode in the same tick, so every turn-toward
         * finished in one frame and the script's anim-01 walk window collapsed
         * (date actor 02/00 in M0001I: anim 01->00 in one tick, no EA 406 steps). */
        return 0;
    }
    PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) & 0xFFDFu));
    return 1;
}

/*
 * PE-BTL65 — opcode 0xB8 turn-toward-point 13514.
 *
 * 107 words 0x80013514..0x800136C0, SHA-256 573a82c6…4ee7.
 * D_800910A0[0xB8]. jal 79FB4. Zero TEXT callers (jalr only).
 *
 * First visit (task+8 bit 0x20 clear): latch *arg0/*arg1/*arg2
 * into task+0x14/+0x18/+0x1C and set the bit. Later visits
 * reuse the latch. dx/dz are (actor+0x28/+0x30) minus those
 * points, then >>16. If both high halves are 0, return 1 and
 * leave the bit set (no facing write). Else the 0x4B angle
 * step: desired=(0x1400-ratan2(dZ,dX))&0xFFF. Unfinished
 * facing rewinds CE00 by 0x14, sets delay 1, and returns 0.
 * Finished facing clears bit 0x20 and returns 1.
 *
 * Live type-0 persist==0x27 +0x478: (0x40D0000, 0xFF670000, 0xB4).
 */
/* func_80013514: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80013514_port.c (src/func_80013514.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800136C0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800136C0_port.c (src/func_800136C0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

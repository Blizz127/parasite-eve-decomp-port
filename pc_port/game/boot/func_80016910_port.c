/*
 * PE-BTL20 — 16910 opcode 0xED live key-2900 cut.
 * Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80016910 — 314 words 0x80016910..0x80016DF8, SHA-256
 * 273744b7…8c37. D_800910A0[0xED]. Switch on *arg0.
 * Live type-1 +0x048 is key 0xB54 = 2900 → 0x80016D00:
 *   D_800B0CD8 |= 0x00400000
 *   v0=1
 * Live type-0/2 +0x014 is key 0xA29 → 0x80016C78:
 *   sb *arg1 → *(D2F0)+0x27D
 *   v0=1
 * DAY2-109 restores key3100: full6914C(1), with VM rewind and retry
 * when it returns1. DAY2-114 restores effect color/allocation keys2400/1/2.
 * DAY2-115 restores the effect update/draw graph in 3F3C4_port.c.
 * Actor-spawn keys remain unfinished.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800B0CD8 0x800B0CD8u
#define GA_D_8009D2F0 0x8009D2F0u

/* Original E00CC..E01BC: backward slot search with head replacement when
 * the signed scan count exceeds E21A4. The stored count saturates at20. */
static void pe_e00cc(pe_addr_t position, unsigned mode, unsigned parameter,
                     unsigned ticks, unsigned red, unsigned green, unsigned blue)
{
    pe_addr_t head=PE_LoadU32(0x800E2800u),slot=head;
    uint32_t scanned=0;
    int count=(int16_t)PE_LoadU16(0x800E21A4u);
    if (PE_LoadU8(slot)!=0u) {
        for (;;) {
            int exceeded=(int16_t)scanned>count;
            slot-=20u;
            if (exceeded) {slot=head;break;}
            unsigned active=PE_LoadU8(slot);
            scanned++;
            if (!active) {
                if ((int16_t)scanned>count)slot=head;
                break;
            }
        }
    } else if (0>count)slot=head;
    PE_StoreU8(slot+10u,(uint8_t)mode);
    PE_StoreU16(slot+4u,PE_LoadU16(position));
    PE_StoreU16(slot+6u,PE_LoadU16(position+2u));
    uint16_t z=PE_LoadU16(position+4u);
    PE_StoreU8(slot+1u,(uint8_t)ticks);PE_StoreU8(slot+2u,(uint8_t)ticks);
    PE_StoreU8(slot+3u,0);PE_StoreU8(slot,1);
    PE_StoreU8(slot+16u,(uint8_t)red);PE_StoreU8(slot+17u,(uint8_t)green);PE_StoreU8(slot+18u,(uint8_t)blue);
    count=(int16_t)PE_LoadU16(0x800E21A4u);
    PE_StoreU16(slot+14u,(uint16_t)parameter);PE_StoreU16(slot+12u,0);PE_StoreU16(slot+8u,z);
    if(count<20)PE_StoreU16(0x800E21A4u,(uint16_t)(count+1));
}

int func_80016910_key2900_cut(pe_addr_t args)
{
    uint32_t key;
    pe_addr_t actor;

    key = PE_LoadU32(PE_LoadU32(args));
    if (key == 2401u) {
        for (unsigned i=0;i<3;i++)
            PE_StoreU8(0x8009CDF8u+i,(uint8_t)PE_LoadU32(PE_LoadU32(args+4u+i*4u)));
        return 1;
    }
    if (key == 2400u || key == 2402u) {
        /* Guest scratch substitutes for the original local three-halfword vector. */
        const pe_addr_t position=0x80122380u;
        for (unsigned i=0;i<3;i++)
            PE_StoreU16(position+i*2u,PE_LoadU16(PE_LoadU32(args+4u+i*4u)+2u));
        pe_e00cc(position,key==2402u,PE_LoadU16(PE_LoadU32(args+16u)),
                  PE_LoadU8(PE_LoadU32(args+20u)),
                  key==2402u?PE_LoadU8(0x8009CDF8u):0,
                  key==2402u?PE_LoadU8(0x8009CDF9u):0,
                  key==2402u?PE_LoadU8(0x8009CDFAu):0);
        return 1;
    }
    if (key == 3100u) {
        if (func_8006914C(1) == 1) {
            PE_StoreU32(0x8009CE00u, PE_LoadU32(0x8009CE00u) - 40u);
            PE_StoreU32(PE_LoadU32(0x8009D300u) + 16u, 1u);
            return 0;
        }
        return 1;
    }
    if (key == 2300u) {
        /* Original16ABC ->375D0: background enable is boolean, not a byte cast. */
        PE_StoreU32(0x8009CED4u, PE_LoadU32(PE_LoadU32(args + 4u)) != 0u);
        return 1;
    }
    if (key == 2101u) {
        PE_StoreU32(GA_D_800B0CD8, PE_LoadU32(GA_D_800B0CD8) | 0x800u);
        return 1;
    }
    if (key == 2900u)
        PE_StoreU32(GA_D_800B0CD8, PE_LoadU32(GA_D_800B0CD8) | 0x00400000u);
    else if (key == 0xA29u) {
        actor = PE_LoadU32(GA_D_8009D2F0);
        if (actor != 0u)
            PE_StoreU8(actor + 0x27Du,
                       (uint8_t)PE_LoadU32(PE_LoadU32(args + 4u)));
    }
    return 1;
}

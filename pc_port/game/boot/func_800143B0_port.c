/*
 * PE-BTL49 — func_800143B0: opcode 0x54 pose-distance
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 83 words 0x800143B0..0x800144FC, SHA-256 53cb1131…a629.
 * D_800910A0[0x54]. Zero jal. v0=1.
 *
 * *arg0==0 uses D254; else walks D20C for type/id with
 * +(0x98)&0x10 clear. Miss stores -1 to *arg2. Hit stores
 * abs(dX)+abs(dY)+abs(dZ) of D2F0 vs target +0x28/+0x2C/+0x30.
 *
 * Live type-2 fork 0x6FE: (0,0,local[0x11]) vs type 0.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D20C 0x8009D20Cu

static pe_addr_t pe_54_find(pe_addr_t args)
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

int func_800143B0(pe_addr_t args)
{
    pe_addr_t self;
    pe_addr_t target;
    pe_addr_t dest;
    int32_t dx;
    int32_t dy;
    int32_t dz;
    int32_t sum;

    dest = PE_LoadU32(args + 8u);
    target = pe_54_find(args);
    if (target == 0u) {
        PE_StoreU32(dest, 0xFFFFFFFFu);
        return 1;
    }

    self = PE_LoadU32(GA_D_8009D2F0);
    dx = (int32_t)PE_LoadU32(self + 0x28u) - (int32_t)PE_LoadU32(target + 0x28u);
    if (dx < 0)
        dx = -dx;
    PE_StoreU32(dest, (uint32_t)dx);
    dy = (int32_t)PE_LoadU32(self + 0x2Cu) - (int32_t)PE_LoadU32(target + 0x2Cu);
    sum = (int32_t)PE_LoadU32(dest);
    if (dy < 0)
        dy = -dy;
    PE_StoreU32(dest, (uint32_t)(sum + dy));
    dz = (int32_t)PE_LoadU32(self + 0x30u) - (int32_t)PE_LoadU32(target + 0x30u);
    sum = (int32_t)PE_LoadU32(dest);
    if (dz < 0)
        dz = -dz;
    PE_StoreU32(dest, (uint32_t)(sum + dz));
    return 1;
}

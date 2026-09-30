/*
 * PE-BTL52 — func_80014228: opcode 0x0E actor-field read
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 98 words 0x80014228..0x800143B0, SHA-256 e26b9d26…bfab.
 * D_800910A0[0x0E]. Zero jal. v0=1.
 *
 * *arg1==0 uses D254 (no +0x98 bit check). *arg1==self
 * type and *arg2==self id uses D2F0 (no bit check).
 * Else walks D20C for type/id with +0x98 bit 0x10 clear.
 * Miss: *arg3 = -1. Hit *arg0: 0=+0x0E, 1=+0x98,
 * 2=lh +0x16, 3=+0x0F.
 *
 * Live fork after 0x4B: (0, 2, 0, local[0xD]) reads
 * self +0x0E.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D20C 0x8009D20Cu

static pe_addr_t pe_0e_find(pe_addr_t args)
{
    uint32_t type;
    uint32_t idb;
    pe_addr_t self;
    pe_addr_t actor;

    type = PE_LoadU32(PE_LoadU32(args + 4u));
    if (type == 0u)
        return PE_LoadU32(GA_D_8009D254);

    idb = PE_LoadU32(PE_LoadU32(args + 8u));
    self = PE_LoadU32(GA_D_8009D2F0);
    if (self != 0u
        && PE_LoadU8(self + 0x0Cu) == (uint8_t)type
        && PE_LoadU8(self + 0x0Du) == (uint8_t)idb)
        return self;

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

int func_80014228(pe_addr_t args)
{
    pe_addr_t target;
    pe_addr_t dest;
    uint32_t code;

    dest = PE_LoadU32(args + 12u);
    target = pe_0e_find(args);
    if (target == 0u) {
        PE_StoreU32(dest, 0xFFFFFFFFu);
        return 1;
    }

    code = PE_LoadU32(PE_LoadU32(args));
    if (code == 0u)
        PE_StoreU32(dest, PE_LoadU8(target + 0x0Eu));
    else if (code == 1u)
        PE_StoreU32(dest, PE_LoadU32(target + 0x98u));
    else if (code == 2u)
        PE_StoreU32(dest, (uint32_t)(int32_t)(int16_t)PE_LoadU16(target + 0x16u));
    else if (code == 3u)
        PE_StoreU32(dest, PE_LoadU8(target + 0x0Fu));
    return 1;
}

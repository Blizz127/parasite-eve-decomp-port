/*
 * PE-BTL14 — func_800181CC opcode 0xCE tagged setter (translated
 * retail, not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 53 words 0x800181CC..0x800182A0, SHA-256 79f58896…0b07.
 * D_800910A0[0xCE] = this leaf. TEXT jals only 2FF78 and 30220
 * (already ported). Always returns 1 (17018 re-fetches from
 * gp+0x90).
 *
 * a0 = 17018 arg frame.
 *   *arg0 == 0: 2FF78(lbu(*arg2), lw(*arg3))  # Aya / D254 object
 *   *arg0 != 0: walk D_8009D20C for type==*arg0 and idB==*arg1
 *               with actor+0x98 bit 4 clear, then
 *               30220(actor, lbu(*arg2), lw(*arg3))
 *
 * Live m0005i type-6 first word is 0x000080CE, kinds=0, imms
 * 0 / 0 / 0xFF / 0xFFFFFBA9 → 2FF78(255, 0xFFFFFBA9) →
 * sh 0xFBA9 → D_800942EC. Not BattleInput. Not M2.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D20C 0x8009D20Cu

int func_800181CC(pe_addr_t args)
{
    uint32_t filter;
    pe_addr_t actor;
    uint32_t want_idb;
    uint8_t tag;
    uint32_t value;

    filter = PE_LoadU32(PE_LoadU32(args));
    if (filter == 0u) {
        tag = PE_LoadU8(PE_LoadU32(args + 8u));
        value = PE_LoadU32(PE_LoadU32(args + 12u));
        func_8002FF78(tag, value);
        return 1;
    }

    actor = PE_LoadU32(GA_D_8009D20C);
    if (actor == 0u)
        return 1;

    want_idb = PE_LoadU32(PE_LoadU32(args + 4u));
    while (actor != 0u) {
        if (PE_LoadU8(actor + 0x0Cu) == (uint8_t)filter &&
            (uint32_t)PE_LoadU8(actor + 0x0Du) == want_idb &&
            (PE_LoadU32(actor + 0x98u) & 0x10u) == 0u) {
            tag = PE_LoadU8(PE_LoadU32(args + 8u));
            value = PE_LoadU32(PE_LoadU32(args + 12u));
            func_80030220(actor, tag, value);
            return 1;
        }
        actor = PE_LoadU32(actor + 4u);
    }
    return 1;
}

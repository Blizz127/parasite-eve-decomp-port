/*
 * PE-BTL46 — func_8002FE78: opcode 0x59 Aya tagged reader
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 64 words 0x8002FE78..0x8002FF78, SHA-256 10946440…c3aa.
 * Inverse of 2FF78's setter, but the load base is *D254
 * then one extra deref: a1 = *(*D254). sltiu tag 0x17;
 * jr D_80010AC8[tag]. Out of range and JT nops return
 * -1000 (0xFFFFFC18). Jal sites: 18038 (0x59) and
 * 180B4 (0x8B). Live type-2 is type!=0 so this leaf
 * is not the executed path.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D254 0x8009D254u
#define SENTINEL      ((int)-1000)

int func_8002FE78(unsigned int tag)
{
    pe_addr_t obj;
    pe_addr_t base;
    uint8_t t;
    uint32_t w;

    t = (uint8_t)(tag & 0xFFu);
    obj = PE_LoadU32(GA_D_8009D254);
    base = PE_LoadU32(obj);
    if (t >= 0x17u)
        return SENTINEL;

    switch (t) {
        case 0:  return (int)PE_LoadU32(base);
        case 1:  return (int)(int16_t)PE_LoadU16(base + 0x04u);
        case 2:  return (int)PE_LoadU16(base + 0x06u);
        case 3:  return (int)PE_LoadU32(base + 0x08u);
        case 4:  return (int)(int16_t)PE_LoadU16(base + 0x0Cu);
        case 6:  return (int)PE_LoadU16(base + 0x10u);
        case 7:  return (int)PE_LoadU8(base + 0x12u);
        case 10: return (int)(int16_t)PE_LoadU16(base + 0x1Cu);
        case 11: return (int)PE_LoadU16(base + 0x1Eu);
        case 12: return (int)PE_LoadU16(base + 0x20u);
        case 13: return (int)PE_LoadU32(base + 0x28u);
        case 14: return (int)PE_LoadU16(base + 0x22u);
        case 20:
            w = PE_LoadU32(base + 0x4Cu);
            return (int)((w >> 9) & 1u);
        case 21:
            w = PE_LoadU32(base + 0x4Cu);
            return (int)((w >> 6) & 3u);
        case 22:
            w = PE_LoadU32(base + 0x4Cu);
            return (int)((w >> 29) & 1u);
        default:
            return SENTINEL;
    }
}

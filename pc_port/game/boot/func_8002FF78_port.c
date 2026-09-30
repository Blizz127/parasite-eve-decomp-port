/*
 * PE-CH1 — func_8002FF78: opcode 0x5A Aya tagged setter (translated retail).
 *
 * Complete retail body (101 words / 0x194, exe 0x8002FF78–0x80030108,
 * file offset 0x20778). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml places this in [0x20210, asm]. This worktree has no era/asm
 * split, so the leaf cannot yet be a matching src/ unit.
 *
 * Opcode 0x5A jump table D_800910A0[0x5A] @ 0x80091208 = wrapper
 * func_80018164. If lbu(*(D_8009D2F0)+0x0C)==0, jal this leaf with
 * a0=tag (lbu), a1=value (lw). Else jal func_80030220 (slot tags
 * 40+). BTL1 m0005i 40/41/42/50–52 therefore do NOT hit this leaf.
 * Jal sites of 2FF78: 0x80018194 (0x5A) and 0x80018200 (0xCE).
 *
 * ROM: dest = *(*D_8009D254). Binary-search switch on tag&0xFF.
 * The lw a2,0(v0) at8002FF88 follows Aya's actor-to-stats pointer:
 *
 *   0  sw +0x00     1  sh +0x04     2  sh +0x06     3  sw +0x08
 *   4  sh +0x0C     5  sh +0x0E     6  sh +0x10    10  sh +0x1C
 *  11  sh +0x1E    12  sh +0x20    14  sh +0x22    18  sh +0x26
 *  30  sh +0x50    31  sb +0x56    32  sb +0x57    33  sh +0x58
 *  34  sb +0x5E   255  sh D_800942EC
 *  else no store
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D254 0x8009D254u
#define GA_D_800942EC 0x800942ECu

void func_8002FF78(unsigned int tag, unsigned int value)
{
    pe_addr_t obj;
    uint8_t t;

    t = (uint8_t)(tag & 0xFFu);
    obj = PE_LoadU32(GA_D_8009D254);
    /* Preserve physical RAM aliases, including the unconditional read at0
     * when Aya is absent and the command only changes the global tag255. */
    if (obj < 0x200000u) obj |= 0x80000000u;
    obj = PE_LoadU32(obj);
    if (obj < 0x200000u) obj |= 0x80000000u;
    switch (t) {
        case 0:  PE_StoreU32(obj + 0x00u, value); break;
        case 1:  PE_StoreU16(obj + 0x04u, (uint16_t)value); break;
        case 2:  PE_StoreU16(obj + 0x06u, (uint16_t)value); break;
        case 3:  PE_StoreU32(obj + 0x08u, value); break;
        case 4:  PE_StoreU16(obj + 0x0Cu, (uint16_t)value); break;
        case 5:  PE_StoreU16(obj + 0x0Eu, (uint16_t)value); break;
        case 6:  PE_StoreU16(obj + 0x10u, (uint16_t)value); break;
        case 10: PE_StoreU16(obj + 0x1Cu, (uint16_t)value); break;
        case 11: PE_StoreU16(obj + 0x1Eu, (uint16_t)value); break;
        case 12: PE_StoreU16(obj + 0x20u, (uint16_t)value); break;
        case 14: PE_StoreU16(obj + 0x22u, (uint16_t)value); break;
        case 18: PE_StoreU16(obj + 0x26u, (uint16_t)value); break;
        case 30: PE_StoreU16(obj + 0x50u, (uint16_t)value); break;
        case 31: PE_StoreU8(obj + 0x56u, (uint8_t)value); break;
        case 32: PE_StoreU8(obj + 0x57u, (uint8_t)value); break;
        case 33: PE_StoreU16(obj + 0x58u, (uint16_t)value); break;
        case 34: PE_StoreU8(obj + 0x5Eu, (uint8_t)value); break;
        case 255:
            PE_StoreU16(GA_D_800942EC, (uint16_t)value);
            break;
        default:
            break;
    }
}

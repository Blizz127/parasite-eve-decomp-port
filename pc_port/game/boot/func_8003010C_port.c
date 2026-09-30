/*
 * PE-BTL46 — func_8003010C: opcode 0x59 slot tagged reader
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 69 words 0x8003010C..0x80030220, SHA-256 10bca057…8787.
 * Inverse of 30220. slot=*actor; idx=(tag&0xFF)-41;
 * if idx>=90 return -1000; jr D_80010B28[idx].
 * Unhandled in-range tags also return -1000.
 * Jal sites: 18054 (0x59) and 18138 (0x8B).
 *
 * Live type-2 tag 44: lw slot+0x10, clamp <0 to 0.
 * Tag 130 is a destructive GET (clears slot+0xCC bit
 * 0x01000000 when the *slot 0x000C0000 mask matches).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define TAG_BASE  41u
#define TAG_COUNT 90u
#define SENTINEL  ((int)-1000)

int func_8003010C(pe_addr_t actor, unsigned int tag)
{
    pe_addr_t slot;
    unsigned int idx;
    int value;
    uint32_t w;
    uint32_t cc;

    slot = PE_LoadU32(actor);
    idx = (tag & 0xFFu) - TAG_BASE;
    if (idx >= TAG_COUNT)
        return SENTINEL;

    switch (idx + TAG_BASE) {
        case 41:
            return (int)(int8_t)PE_LoadU8(slot + 0x04u);
        case 43:
            return (int)PE_LoadU16(slot + 0x0Cu);
        case 44:
            value = (int)PE_LoadU32(slot + 0x10u);
            return (value < 0) ? 0 : value;
        case 48:
            w = PE_LoadU32(slot);
            return (int)PE_LoadU8(slot + ((w >> 17) & 0x70u) + 0x1Cu);
        case 60:
            value = (int)PE_LoadU32(slot + 0x88u);
            return (value < 0) ? 0 : value;
        case 61:
            return (int)PE_LoadU16(slot + 0x8Cu);
        case 77:
            return (int)((PE_LoadU32(slot) >> 13) & 3u);
        case 82:
            return (int)(PE_LoadU8(slot + 0x03u) & 0x3Fu);
        case 130:
            cc = PE_LoadU32(slot + 0xCCu);
            if ((cc & 0x01000000u) == 0u)
                return 0;
            if ((PE_LoadU32(slot) & 0x000C0000u) != 0x000C0000u)
                return 0;
            PE_StoreU32(slot + 0xCCu, cc & 0xFEFFFFFFu);
            return 1;
        default:
            return SENTINEL;
    }
}

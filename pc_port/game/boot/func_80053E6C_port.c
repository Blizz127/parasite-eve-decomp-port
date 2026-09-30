/*
 * PE-BTL100 — func_80053E6C category/ID occupancy query.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. 45 words
 * 0x80053E6C..0x80053F20 exclusive, SHA-256
 * computed by pe_btl100_mode3_oracle.py. No matching src/ C.
 *
 *   if D_8009D03C <= a0 < D_8009D03C+3:
 *       return lhu(0x800A1E6E + (a0-D_8009D03C)*32)
 *   else scan D_8009D048[0 .. D_8009D050) halfwords for a0
 *
 * Mode-3 2A7F8 calls this with a0=18. Reset fixture (all three
 * host words 0) returns 0, so 2A7F8 takes 2B29C.
 */
#include "psx_compat.h"

#define GA_CAT_HALF 0x800A1E6Eu
#define GA_MAP_BASE 0x800BEEB0u /* 0x800C0000 + lbu -4432 */

int func_80053E6C(int id)
{
    unsigned int base;
    unsigned int tbl;
    unsigned int end;
    int count;

    base = D_8009D03C;
    if (id >= (int)base && id < (int)base + 3)
        return (int)PE_LoadU16(GA_CAT_HALF +
                               (pe_addr_t)(id - (int)base) * 32u);

    count = 0;
    tbl = D_8009D048;
    end = tbl + (D_8009D050 << 1);
    if (tbl >= end)
        return 0;

    while (tbl < end) {
        uint16_t word;
        uint32_t v;

        word = PE_LoadU16(tbl);
        v = (uint32_t)word << 16;
        if ((unsigned)((int)word - 256) < 128u) {
            v = (uint32_t)((int32_t)v >> 11);
            v = (uint32_t)PE_LoadU8(GA_MAP_BASE + v);
            v ^= (uint32_t)id;
        } else {
            v = (uint32_t)((int32_t)v >> 16);
            v ^= (uint32_t)id;
        }
        if (v == 0u)
            count++;
        tbl += 2u;
    }
    return count;
}

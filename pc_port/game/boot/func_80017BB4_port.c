/*
 * PE-CH1 — func_80017BB4_btl1_cut: opcode 0x31 normal destination path.
 *
 * Retail handler: 0x80017BB4..0x80017C54 (40 words, file 0x83B4).
 * Any token except 0xA9400048 follows the normal path:
 *
 *   token = **args;
 *   D_8009D1A0 |= 0x2000;
 *   D_8009D280 = token;
 *   return 0;
 *
 * Live type-0 Watch arm uses 0xA80663C8. BTL1 used 0xA80002C8.
 * func_8006E2D0 decodes the packed token into a seven-byte stack local;
 * that local does not escape. The token-specific A9400048 branch at
 * 0x80017C14 (D_800A7918 / 6A25C) is not this cut.
 *
 * Words and branch path are independently checked by
 * pc_port/tools/pe_ch1_17bb4_oracle.py against the SHA-1-exact EXE.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define SPECIAL_TOKEN   0xA9400048u

int func_80017BB4_btl1_cut(pe_addr_t args)
{
    unsigned int token;

    token = PE_LoadU32(PE_LoadU32(args));
    D_8009D1A0 |= 0x2000u;
    D_8009D280 = token;
    /* A9400048 → D_800A7918 / 6A25C is not this cut. */
    (void)SPECIAL_TOKEN;
    return 0;
}

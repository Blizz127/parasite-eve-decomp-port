/*
 * PE-BTL6 — func_8006698C live leaf (117 words, zero callees).
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d050_remainder_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * The 215-word window 0x8006698C..0x80066CE8 is four jr leaves.
 * EE=13 jals only the first: 0x8006698C..0x80066B60 exclusive.
 * Fills D_800BEA40 / D_800BEA60 from dest+0x88/89/8A and
 * D_800BD025/26/27. Trailing five ctc2 words load BEA40+0x20 into
 * LCM (C2CTRL 16-20). Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define GA_BEA40  0x800BEA40u
#define GA_BD025  0x800BD025u

static int pe_6698c_clamp_shift4(int sum)
{
    if (sum < 0)
        sum = 0;
    else if (sum >= 256)
        sum = 255;
    return sum << 4;
}

static int pe_6698c_sra8(int prod)
{
    if (prod < 0)
        prod += 255;
    return prod >> 8;
}

void func_8006698C(pe_addr_t dest)
{
    int scale_r;
    int scale_g;
    int scale_b;
    int v;

    if (dest == 0u)
        return;

    PE_StoreU16(GA_BEA40 + 0x00u, 0u);
    PE_StoreU16(GA_BEA40 + 0x02u, 4096u);
    PE_StoreU16(GA_BEA40 + 0x04u, 0u);

    scale_r = (int)PE_LoadU8(GA_BD025 + 0u);
    scale_g = (int)PE_LoadU8(GA_BD025 + 1u);
    scale_b = (int)PE_LoadU8(GA_BD025 + 2u);

    v = pe_6698c_clamp_shift4((int)PE_LoadU8(dest + 0x88u)
                              + (int)PE_LoadU8(dest + 0x8Au));
    PE_StoreU16(GA_BEA40 + 0x20u, (uint16_t)pe_6698c_sra8(v * scale_r));
    PE_StoreU16(GA_BEA40 + 0x26u, (uint16_t)pe_6698c_sra8(v * scale_g));
    PE_StoreU16(GA_BEA40 + 0x2Cu, (uint16_t)pe_6698c_sra8(v * scale_b));

    PE_StoreU16(GA_BEA40 + 0x06u, 0u);
    PE_StoreU16(GA_BEA40 + 0x08u, (uint16_t)-4096);
    PE_StoreU16(GA_BEA40 + 0x0Au, 0u);

    v = pe_6698c_clamp_shift4((int)PE_LoadU8(dest + 0x88u)
                              + (int)PE_LoadU8(dest + 0x89u));
    PE_StoreU16(GA_BEA40 + 0x22u, (uint16_t)pe_6698c_sra8(v * scale_r));
    PE_StoreU16(GA_BEA40 + 0x28u, (uint16_t)pe_6698c_sra8(v * scale_g));
    PE_StoreU16(GA_BEA40 + 0x2Eu, (uint16_t)pe_6698c_sra8(v * scale_b));

    PE_StoreU16(GA_BEA40 + 0x0Cu, 0u);
    PE_StoreU16(GA_BEA40 + 0x0Eu, 0u);
    PE_StoreU16(GA_BEA40 + 0x10u, 0u);
    PE_StoreU16(GA_BEA40 + 0x24u, 0u);
    PE_StoreU16(GA_BEA40 + 0x2Au, 0u);
    PE_StoreU16(GA_BEA40 + 0x30u, 0u);
    PE_GTE_LoadLCM(GA_BEA40 + 0x20u);
}

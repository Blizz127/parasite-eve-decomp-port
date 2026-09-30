/*
 * PE-BTL6 — func_8003B97C empty early-out + lighting cut at 0x8003BA24.
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3b97c_lighting_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b and PE.IMG [428,434).
 *
 * Retail 217 words 0x8003B97C..0x8003BCE0, zero callees. Live 3D834
 * is jal 3B97C(dest, D_800BEA40). Returns when dest+0==0, dest+0xBA==0,
 * or lbu(obj+2) is blez.
 *
 * Lighting at 0x8003BA24: rec.byte4==1 does three RTIR 0x049E012
 * (bea40 RT × dest+0x84 columns) into a local 0x1F800004-shaped
 * buffer, then NCCT 0x118043F (psx-spx, integer). Not NCLIP.
 * Tables 0x800B1638 / 0x800A6360. Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define CMD_RTIR   0x049E012u
#define GA_B1638   0x800B1638u
#define GA_A6360   0x800A6360u
#define GA_91A58   0x80091A58u
#define GA_9CDA0   0x8009CDA0u

void func_8003B97C_empty_cut(pe_addr_t dest, pe_addr_t bea40)
{
    pe_addr_t obj;

    (void)bea40;
    if (dest == 0u)
        return;
    obj = PE_LoadU32(dest + 0x00u);
    if (obj == 0u)
        return;
    if ((int16_t)PE_LoadU16(dest + 0xBAu) == 0)
        return;
    if (PE_LoadU8(obj + 2u) == 0u)
        return;
}

static void pe_3b97c_load_v(void (*setv)(int16_t, int16_t, int16_t),
                            pe_addr_t rec)
{
    uint32_t xy = PE_LoadU32(rec);
    uint32_t z = PE_LoadU32(rec + 4u);

    setv((int16_t)(xy & 0xFFFFu), (int16_t)(xy >> 16),
         (int16_t)(z & 0xFFFFu));
}

void func_8003B97C_lighting_cut(pe_addr_t dest, pe_addr_t bea40)
{
    pe_addr_t obj;
    pe_addr_t mats;
    int16_t count;
    int i;
    int bk;

    if (dest == 0u)
        return;
    obj = PE_LoadU32(dest + 0x00u);
    if (obj == 0u)
        return;
    if ((int16_t)PE_LoadU16(dest + 0xBAu) == 0)
        return;
    count = (int16_t)PE_LoadU8(obj + 2u);
    if (count <= 0)
        return;

    bk = (int)PE_LoadU8(dest + 0x88u) << 4;
    PE_GTE_SetBK(bk, bk, bk);
    PE_GTE_SetRGBC(PE_LoadU32(GA_9CDA0));
    mats = PE_LoadU32(dest + 0x84u);

    for (i = 0; i < count; i++) {
        pe_addr_t rec = PE_LoadU32(dest + 0x04u) + (pe_addr_t)i * 12u;

        if (PE_LoadU8(rec + 4u) == 1u) {
            int16_t scratch[9];
            unsigned col;
            int16_t ntri;
            int t2;

            PE_GTE_LoadRT33(bea40);
            for (col = 0; col < 3u; col++) {
                unsigned off = col * 2u;

                PE_GTE_SetIR((int16_t)PE_LoadU16(mats + off),
                             (int16_t)PE_LoadU16(mats + 6u + off),
                             (int16_t)PE_LoadU16(mats + 12u + off));
                PE_GTE_MVMVA(CMD_RTIR);
                scratch[col] = (int16_t)g_pe_gte.ir[0];
                scratch[3u + col] = (int16_t)g_pe_gte.ir[1];
                scratch[6u + col] = (int16_t)g_pe_gte.ir[2];
            }
            PE_GTE_LoadLLM_halfs(scratch);

            ntri = (int16_t)PE_LoadU16(rec + 2u);
            if (ntri > 0) {
                unsigned idx = PE_LoadU16(rec);
                pe_addr_t a3 = PE_LoadU32(dest + 0x08u)
                    + (pe_addr_t)(idx << 3) + 22u;
                pe_addr_t t1 = GA_B1638 + idx * 4u;
                pe_addr_t t0 = GA_A6360 + idx * 4u;
                pe_addr_t dest_c = PE_LoadU32(dest + 0x0Cu);

                t2 = 0;
                do {
                    int16_t i0 = (int16_t)PE_LoadU16(a3 - 16u);
                    int16_t i1 = (int16_t)PE_LoadU16(a3 - 8u);
                    int16_t i2 = (int16_t)PE_LoadU16(a3);

                    pe_3b97c_load_v(PE_GTE_SetV0, GA_91A58 + (pe_addr_t)i0 * 8u);
                    pe_3b97c_load_v(PE_GTE_SetV1, GA_91A58 + (pe_addr_t)i1 * 8u);
                    pe_3b97c_load_v(PE_GTE_SetV2, GA_91A58 + (pe_addr_t)i2 * 8u);
                    PE_GTE_SetRGBC(PE_LoadU32(GA_9CDA0));
                    PE_GTE_NCCT();
                    PE_StoreU32(t1 + 0u, g_pe_gte.rgb_fifo[0]);
                    PE_StoreU32(t1 + 4u, g_pe_gte.rgb_fifo[1]);
                    PE_StoreU32(t1 + 8u, g_pe_gte.rgb_fifo[2]);

                    {
                        pe_addr_t color = dest_c + (pe_addr_t)(idx + (unsigned)t2) * 4u;
                        int colored = 1;

                        if (PE_LoadU8(color + 3u) != 0u)
                            PE_GTE_SetRGBC(PE_LoadU32(color));
                        else if (PE_LoadU8(color + 7u) != 0u)
                            PE_GTE_SetRGBC(PE_LoadU32(color + 4u));
                        else if (PE_LoadU8(color + 11u) != 0u)
                            PE_GTE_SetRGBC(PE_LoadU32(color + 8u));
                        else
                            colored = 0;
                        /* Retail 3BC4C skips the second NCCT and all three
                         * A6360 stores when the triangle has no color tag. */
                        if (colored) {
                            PE_GTE_NCCT();
                            PE_StoreU32(t0 + 0u, g_pe_gte.rgb_fifo[0]);
                            PE_StoreU32(t0 + 4u, g_pe_gte.rgb_fifo[1]);
                            PE_StoreU32(t0 + 8u, g_pe_gte.rgb_fifo[2]);
                        }
                    }

                    t2 += 3;
                    t1 += 12u;
                    t0 += 12u;
                    a3 += 24u;
                } while (t2 < ntri);
            }
        }
        mats += 32u;
    }
}

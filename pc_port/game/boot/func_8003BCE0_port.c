/*
 * PE-BTL6 — func_8003BCE0 (245 words, zero callees).
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d834_callees_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Exclusive 0x8003BCE0..0x8003C0B4. Live 3D834 jals
 * 3BCE0(dest, 1, D_8009CDDC) then xor CDDC and jal again.
 * Four directory walks from dest+0x10 using obj+8/+A/+C/+E counts
 * write table words into dest+0x54 packets. Tables 0x800B1638
 * (first two loops) and 0x800A6360 (last two). Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define TBL_B1638 0x800B1638u
#define TBL_A6360 0x800A6360u
#define MASK24    0x00FFFFFFu

static void pe_3bce0_store4(pe_addr_t pkt, pe_addr_t rec, pe_addr_t table,
                            int force, int rec0, int rec1, int rec2, int rec3,
                            unsigned dst0, unsigned dst1, unsigned dst2,
                            unsigned dst3)
{
    unsigned word;
    uint8_t keep;

    word = PE_LoadU32(pkt) & MASK24;
    if (word == 0u && force == 0)
        return;
    keep = PE_LoadU8(pkt + 7u);
    PE_StoreU32(pkt + dst0, PE_LoadU32(table + (pe_addr_t)PE_LoadU16((pe_addr_t)((int)rec + rec0)) * 4u));
    PE_StoreU32(pkt + dst1, PE_LoadU32(table + (pe_addr_t)PE_LoadU16((pe_addr_t)((int)rec + rec1)) * 4u));
    PE_StoreU32(pkt + dst2, PE_LoadU32(table + (pe_addr_t)PE_LoadU16((pe_addr_t)((int)rec + rec2)) * 4u));
    PE_StoreU8(pkt + 7u, keep);
    PE_StoreU32(pkt + dst3, PE_LoadU32(table + (pe_addr_t)PE_LoadU16((pe_addr_t)((int)rec + rec3)) * 4u));
}

void func_8003BCE0(pe_addr_t dest, int a1, int a2)
{
    pe_addr_t obj;
    pe_addr_t stream;
    pe_addr_t dir;
    int16_t sel;
    int16_t force;
    unsigned n;
    unsigned i;

    if (dest == 0u)
        return;
    obj = PE_LoadU32(dest + 0x00u);
    if (obj == 0u)
        return;
    if ((int16_t)PE_LoadU16(dest + 0xBAu) == 0)
        return;

    sel = (int16_t)a2;
    force = (int16_t)a1;
    stream = PE_LoadU32(dest + 0x54u);
    dir = PE_LoadU32(dest + 0x10u);

    n = PE_LoadU16(obj + 0x08u);
    if ((int16_t)n > 0) {
        pe_addr_t pkt = stream + (pe_addr_t)sel * 52u;
        pe_addr_t rec = dir + 10u;
        for (i = 0; i < n; i++) {
            pe_3bce0_store4(pkt, rec, TBL_B1638, force,
                            -6, -4, -2, 0, 0x04u, 0x10u, 0x1Cu, 0x28u);
            pkt += 104u;
            rec += 12u;
            dir += 12u;
        }
        stream += 104u * n;
    }

    n = PE_LoadU16(obj + 0x0Au);
    if ((int16_t)n > 0) {
        pe_addr_t pkt = stream + (pe_addr_t)sel * 40u;
        pe_addr_t rec = dir + 8u;
        for (i = 0; i < n; i++) {
            unsigned word = PE_LoadU32(pkt) & MASK24;
            if (word != 0u || force != 0) {
                uint8_t keep = PE_LoadU8(pkt + 7u);
                PE_StoreU32(pkt + 0x04u, PE_LoadU32(TBL_B1638 + (pe_addr_t)PE_LoadU16(rec - 4u) * 4u));
                PE_StoreU32(pkt + 0x10u, PE_LoadU32(TBL_B1638 + (pe_addr_t)PE_LoadU16(rec - 2u) * 4u));
                PE_StoreU8(pkt + 7u, keep);
                PE_StoreU32(pkt + 0x1Cu, PE_LoadU32(TBL_B1638 + (pe_addr_t)PE_LoadU16(rec) * 4u));
            }
            pkt += 80u;
            rec += 12u;
            dir += 12u;
        }
        stream += 80u * n;
    }

    n = PE_LoadU16(obj + 0x0Cu);
    if ((int16_t)n > 0) {
        pe_addr_t pkt = stream + (pe_addr_t)sel * 36u;
        pe_addr_t rec = dir + 10u;
        for (i = 0; i < n; i++) {
            pe_3bce0_store4(pkt, rec, TBL_A6360, force,
                            -6, -4, -2, 0, 0x04u, 0x0Cu, 0x14u, 0x1Cu);
            pkt += 72u;
            rec += 12u;
            dir += 12u;
        }
        stream += 72u * n;
    }

    n = PE_LoadU16(obj + 0x0Eu);
    if ((int16_t)n > 0) {
        pe_addr_t pkt = stream + (pe_addr_t)sel * 28u;
        pe_addr_t rec = dir + 8u;
        for (i = 0; i < n; i++) {
            unsigned word = PE_LoadU32(pkt) & MASK24;
            if (word != 0u || force != 0) {
                uint8_t keep = PE_LoadU8(pkt + 7u);
                PE_StoreU32(pkt + 0x04u, PE_LoadU32(TBL_A6360 + (pe_addr_t)PE_LoadU16(rec - 4u) * 4u));
                PE_StoreU32(pkt + 0x0Cu, PE_LoadU32(TBL_A6360 + (pe_addr_t)PE_LoadU16(rec - 2u) * 4u));
                PE_StoreU8(pkt + 7u, keep);
                PE_StoreU32(pkt + 0x14u, PE_LoadU32(TBL_A6360 + (pe_addr_t)PE_LoadU16(rec) * 4u));
            }
            pkt += 56u;
            rec += 12u;
        }
    }
}

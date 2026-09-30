/*
 * Phase 6E-B22 — func_8005DE88: resource-list and state initializer.
 *
 * Raw body: 23 words / 0x5C, executable 0x8005DE88..0x8005DEE3,
 * file offset 0x4E688, live split asm/disc1/4CC98.s:1972–1998.
 * The function has no callees and returns void.
 *
 * Retail first links the 20 twelve-byte records beginning at D_800A2090;
 * each record's first word points to the following record, with the final
 * successor temporarily equal to D_800A2180.  The following store then
 * null-terminates D_800A2174 and initializes
 * the five adjacent $gp-relative state words at D_8009D0DC..D_8009D0F0.
 */
#include "psx_compat.h"

#define GA_LIST_BASE 0x800A2090u
#define GA_LIST_END  0x800A2180u
#define GA_STATE      0x800A2174u
#define GA_GP_36C     (0x8009CD70u + 0x36Cu)
#define GA_GP_370     (0x8009CD70u + 0x370u)
#define GA_GP_374     (0x8009CD70u + 0x374u)
#define GA_GP_378     (0x8009CD70u + 0x378u)
#define GA_GP_37C     (0x8009CD70u + 0x37Cu)
#define GA_GP_380     (0x8009CD70u + 0x380u)

void func_8005DE88(void)
{
    pe_addr_t node;
    for (node = GA_LIST_BASE; node < GA_LIST_END; node += 0xCu)
        PE_StoreU32(node, node + 0xCu);

    PE_StoreU32(GA_STATE, 0u);
    PE_StoreU32(GA_GP_36C, GA_LIST_BASE);
    PE_StoreU32(GA_GP_374, 0u);
    PE_StoreU32(GA_GP_370, 0u);
    PE_StoreU32(GA_GP_378, 0u);
    PE_StoreU32(GA_GP_37C, 0u);
    PE_StoreU32(GA_GP_380, 0u);
}

/* Retail target list construction and distance ordering, 2156C..218BC.
 * Authority: 11718.s; matching src/30534, 5186C and 21850.
 * Targets retain retail visibility/health guards and quicksort tie order.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8005186C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005186C_port.c (src/func_8005186C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80030534: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80030534_port.c (src/func_80030534.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80021850: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80021850_port.c (src/func_80021850.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_800216E4(pe_addr_t records, int8_t first, int8_t last)
{
    int partition=first, i;
    if (first>=last) return;
    func_80021850(records,first,(int8_t)((first+last)/2));
    for (i=first;i<=last;i++) {
        if ((int32_t)PE_LoadU32(0x8009E004u+(uint32_t)i*12u)<
            (int32_t)PE_LoadU32(0x8009E004u+(uint32_t)(int32_t)first*12u)) {
            ++partition;
            func_80021850(records,(int8_t)partition,(int8_t)i);
        }
    }
    func_80021850(records,first,(int8_t)partition);
    func_800216E4(records,first,(int8_t)(partition-1));
    func_800216E4(records,(int8_t)(partition+1),last);
}

int32_t func_8002156C(void)
{
    pe_addr_t actor=PE_LoadU32(0x8009D20Cu), aya=PE_LoadU32(0x8009D254u);
    PE_StoreU8(0x8009CE44u,0u);
    while (actor) {
        pe_addr_t body=PE_LoadU32(actor);
        uint32_t flags=PE_LoadU32(actor+0x98u);
        if (actor!=aya && body && (flags&0x2040u)!=0x40u && !(flags&0x4000u) &&
            (int32_t)PE_LoadU32(body+0x10u)>0) {
            int32_t index=(int8_t)PE_LoadU8(0x8009CE44u);
            pe_addr_t out=0x8009E000u+(uint32_t)index*12u;
            int32_t dx=(int16_t)PE_LoadU16(actor+0x268u)-(int16_t)PE_LoadU16(aya+0x2Au);
            int32_t dz=(int16_t)PE_LoadU16(actor+0x26Cu)-(int16_t)PE_LoadU16(aya+0x32u);
            PE_StoreU32(out,actor);
            PE_StoreU32(out+4u,(uint32_t)func_80030534(actor,aya));
            PE_StoreU16(out+8u,(uint16_t)func_80079FB4(dx,dz));
            PE_StoreU8(0x8009CE44u,(uint8_t)(index+1));
        }
        actor=PE_LoadU32(actor+4u);
    }
    {
        int8_t count=(int8_t)PE_LoadU8(0x8009CE44u);
        if (count>=2) func_800216E4(0x8009E000u,0,(int8_t)(count-1));
        PE_StoreU32(0x8009E000u+(uint32_t)(int32_t)count*12u,0u);
        return count;
    }
}

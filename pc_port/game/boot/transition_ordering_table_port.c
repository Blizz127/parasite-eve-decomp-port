/* Original transition frame loop 801925A0..8019262C.
 * SHA256 e3bbb8893991be52590560e3222bffa28dcdec3c44eaf5ff1be22fa387a1464a.
 * Skip runs of empty ordering-table buckets while retaining packet chains.
 * The descriptor is captured once; its OT pointer is reloaded each iteration.
 * The outer loop retains the returned empty-run start in s0 across frames. */
#include "pe_port_compat.h"
uint32_t PE_TransitionCompactOT(uint32_t empty_start)
{
    pe_addr_t descriptor=PE_LoadU32(0x8019C9C0u);
    uint32_t occupied=1;
    for(int32_t index=4095;index>=0;index--) {
        pe_addr_t ot=PE_LoadU32(descriptor+4u),bucket=ot+(uint32_t)index*4u;
        uint32_t previous=ot+(uint32_t)index*4u-4u;
        uint32_t link=PE_LoadU32(bucket)|0x80000000u;
        if(occupied) {
            if(link==previous){empty_start=(uint32_t)index;occupied=0;}
        } else if(link!=previous) {
            PE_StoreU32(ot+empty_start*4u,bucket&0xFFFFFFu);
            occupied=1;
        }
    }
    return empty_start;
}

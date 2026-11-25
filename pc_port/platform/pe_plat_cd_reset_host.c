/* Independently authored host reset contract from frozen caller traces.
 * Retail reset has a distinct, test-only implementation. */
#include "pe_sdk.h"
int pe_plat_cd_reset_host(void)
{
    PE_StoreU32(0x8009B554u,0);
    if(PE_LoadU32(0x8009B574u)==2u){
        uint32_t sub=PE_LoadU32(0x8009B578u);
        if(sub==0xBu||sub==0x10u||sub==0x11u){
            PE_StoreU32(0x8009B574u,1);PE_StoreU32(0x8009B578u,0xB);
        }
    }
    for(uint32_t i=0;i<3;i++)PE_StoreU32(0x800B8AB0u+4*i,0);
    for(uint32_t i=0;i<3;i++)PE_StoreU32(0x800A3530u-16*i,0);
    for(uint32_t i=0;i<3;i++)PE_StoreU8(0x800A3534u-16*i,0);
    for(uint32_t j=5;j<=12;j++)for(uint32_t i=0;i<3;i++)PE_StoreU8(0x800A3510u+16*i+j,0);
    for(uint32_t i=0;i<8;i++){
        uint32_t q=0x800A3540u+24*i;PE_StoreU32(q,0);
        for(uint32_t j=4;j<=8;j++)PE_StoreU8(q+j,0);
        for(uint32_t j=12;j<=20;j+=4)PE_StoreU32(q+j,0);
    }
    PE_StoreU32(0x800A3604u,0);PE_StoreU32(0x800A3600u,0);PE_StoreU32(0x800A3608u,0);
    for(uint32_t i=0;i<9;i++)PE_StoreU32(0x800A3610u+16*i,0);
    pe_plat_cd_store_setloc_hint(0);
    PE_StoreU32(0x8009B70Cu,0);PE_StoreU32(0x8009B708u,0);
    PE_StoreU32(0x8009B554u,1);PE_StoreU32(0x8009B574u,1);return 1;
}

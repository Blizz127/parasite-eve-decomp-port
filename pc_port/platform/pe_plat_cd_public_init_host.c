/* Independent public-init host contract from frozen caller observations.
 * Test-only retail caller is separate. */
#include "pe_sdk.h"
#include "pe_cdreg.h"
#include "game_port.h"
int pe_plat_cd_public_init_host(void)
{
    if(PE_LoadU32(0x8009B554u))return 0;
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
    if(PE_CdReg_DeviceEnabled()){
        (void)func_8007F994();
        if(PE_Port_ShouldStop())return 0;
    }else{
        PE_StoreU32(0x800A36A8u,0);PE_StoreU32(0x800A36A4u,0);PE_StoreU32(0x800A36A0u,0);
        PE_StoreU32(0x8009AFB4u,0x80080164u);PE_StoreU32(0x8009AFB8u,0x80080778u);
        PE_StoreU32(0x8009AFD8u,1);PE_StoreU32(0x8009B554u,1);
    }
    PE_StoreU32(0x800A36A4u,0x8007E964u);PE_StoreU32(0x800A36A8u,0x8007F88Cu);
    PE_StoreU32(0x800A36ACu,0x8007F960u);PE_StoreU32(0x800A36A0u,0x8007F7E8u);
    PE_StoreU32(0x8009B70Cu,0);PE_StoreU32(0x8009B708u,0);return 1;
}

/* Independent host queue contract from original and frozen-caller observations.
 * Retail core is separate and test-only. */
#include "pe_sdk.h"
#include "game_port.h"
static uint32_t enqueue(uint32_t cmd,uint32_t param,uint32_t extra,uint32_t callback)
{
    if((int32_t)PE_LoadU32(0x800A3608u)>=8)return 0;
    uint32_t serial=PE_LoadU32(0x8009B53Cu)+1u;
    PE_StoreU32(0x8009B53Cu,serial);
    if(!serial){serial=1;PE_StoreU32(0x8009B53Cu,serial);}
    uint32_t rec=func_8007E6B0();
    PE_StoreU32(rec,serial);PE_StoreU8(rec+4,(uint8_t)cmd);
    if(param)func_80080950(rec+5,param);
    PE_StoreU32(rec+12,param);PE_StoreU32(rec+16,extra);PE_StoreU32(rec+20,callback);
    PE_StoreU32(0x800A3608u,PE_LoadU32(0x800A3608u)+1u);
    if(func_8007FBF0(0)==1){
        uint32_t index=PE_LoadU32(0x800A3604u);
        if(PE_LoadU32(0x800A3540u+24u*index)==serial)(void)func_8007E8F4();
    }
    return PE_Port_ShouldStop()?0:serial;
}
uint32_t pe_plat_cd_queue_command_host(uint32_t command,uint32_t param,uint32_t extra,uint32_t callback)
{
    command&=255u;
    if(PE_LoadU32(0x8009B4BCu+4u*command)&&param&&!enqueue(2,param,0,0))return 0;
    return enqueue(command,param,extra,callback);
}

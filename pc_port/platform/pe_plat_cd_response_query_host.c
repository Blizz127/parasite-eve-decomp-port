/* Independent host caller reconstructed from frozen production traces. */
#include "pe_sdk.h"
#include "game_port.h"
#include "host_framebuffer.h"
int pe_plat_cd_response_query_host(uint32_t sequence,uint32_t response)
{
    PE_Port_EscapePoint();
    if(sequence)(void)func_8007FC64(0);else HostFB_DevicePoll();
    if(PE_Port_ShouldStop())return 0;
    uint32_t index,record;
    if(sequence){
        index=PE_LoadU32(0x800A3690u);
        unsigned n;
        for(n=0;n<8;n++){
            if(PE_LoadU32(0x800A3610u+16u*index)==sequence)break;
            index=(index+1u)&7u;
        }
        if(n==8){
            index=PE_LoadU32(0x800A3690u);
            if((int32_t)PE_LoadU32(0x800A3610u+16u*index)<(int32_t)sequence)return 0;
        }
        index=(PE_LoadU32(0x800A3690u)-1u)&7u;
        for(n=0;n<8;n++){
            if(PE_LoadU32(0x800A3610u+16u*index)==sequence)break;
            index=(index-1u)&7u;
        }
        if(n==8)return 6;
    }else{
        index=(PE_LoadU32(0x800A3690u)-1u)&7u;
        if(!PE_LoadU32(0x800A3610u+16u*index))return 6;
    }
    record=0x800A3610u+16u*index;
    uint32_t a=PE_LoadU32(record),b=PE_LoadU32(record+4),c=PE_LoadU32(record+8);
    PE_StoreU32(0x800A3500u,a);PE_StoreU32(0x800A3504u,b);PE_StoreU32(0x800A3508u,c);
    PE_StoreU32(0x800A350Cu,PE_LoadU32(record+12));
    func_80080998(response,0x800A3505u);
    return PE_LoadU8(0x800A3504u);
}

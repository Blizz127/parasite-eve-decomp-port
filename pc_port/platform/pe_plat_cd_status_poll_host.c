#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "pe_gpu.h"
#include "pe_sdk.h"
#include "pe_plat/state.h"
#include "host_framebuffer.h"
#include "game_port.h"
extern void PE_Cd_TimeoutRecover(void);
int pe_plat_cd_status_poll_host(uint32_t mode,uint32_t dst)
{
    if(PE_Port_ShouldStop())return 0;
    (void)HostFB_VSync(-1);uint32_t now=PE_GPU_VSyncQuery();
    PE_StoreU32(0x800a3478u,now+0x3c0u);PE_StoreU32(0x800a347cu,0);PE_StoreU32(0x800a3480u,0x80011ba0u);
    for(;;){
        if(PE_Port_ShouldStop())return 0;
        (void)HostFB_VSync(-1);now=PE_GPU_VSyncQuery();
        uint32_t deadline=PE_LoadU32(0x800a3478u);int timed=(int32_t)deadline<(int32_t)now;
        if(!timed){uint32_t prev=PE_LoadU32(0x800a347cu);PE_StoreU32(0x800a347cu,prev+1u);timed=(int32_t)prev>0x3c0000;}
        if(timed){PE_Cd_TimeoutRecover();return -1;}
        if(func_80073DE8()){
            uint32_t p=PE_LoadU32(0x8009b27cu);uint8_t saved=PE_CdLoadU8(p)&3u;
            for(;;){
                uint32_t bits=(uint32_t)pe_plat_cd_acknowledge_host();if(PE_Port_ShouldStop())return 0;
                if(!bits)break;
                if(bits&4u){uint32_t cb=PE_LoadU32(0x8009afb8u);if(cb){uint8_t status=PE_LoadU8(0x8009b295u);if(!PE_Cd_DispatchDataCallback(cb,status,0x800a3468u))return 0;if(PE_Port_ShouldStop())return 0;}}
                if(bits&2u){uint32_t cb=PE_LoadU32(0x8009afb4u);if(cb){uint8_t status=PE_LoadU8(0x8009b294u);if(!PE_Cd_DispatchDataCallback(cb,status,0x800a3460u))return 0;if(PE_Port_ShouldStop())return 0;}}
            }
            p=PE_LoadU32(0x8009b27cu);PE_CdStoreU8(p,saved);
        }
        uint32_t state=PE_LoadU8(0x8009b294u);
        if(state==2u||state==5u){PE_StoreU8(0x8009b294u,2);if(dst)for(unsigned i=0;i<8u;i++)PE_StoreU8(dst+i,PE_LoadU8(0x800a3460u+i));return (int)state;}
        if(mode)return 0;
    }
}

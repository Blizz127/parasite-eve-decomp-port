#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "pe_gpu.h"
#include "pe_sdk.h"
#include "pe_plat/state.h"
#include "host_framebuffer.h"
#include "game_port.h"
#include "pe_bios_string.h"
extern void PE_Cd_TimeoutRecover(void);
int pe_plat_cd_command_issue_host(uint32_t cmd,uint32_t input,uint32_t dst,uint32_t mode)
{
 if(PE_Port_ShouldStop())return -1;
 uint32_t id=cmd&255u;int32_t debug=(int32_t)PE_LoadU32(0x8009afc0u);
 if(debug>=2){PE_Printf_Guest(0x80011bb4u,PE_LoadU32(0x8009afdcu+4u*id),0,0,0,0,0,0);if(PE_Port_ShouldStop())return -1;}
 if(PE_LoadU32(0x8009b1fcu+4u*id)&&!input){
  debug=(int32_t)PE_LoadU32(0x8009afc0u);
  if(debug>0)PE_Printf_Guest(0x80011bbcu,PE_LoadU32(0x8009afdcu+4u*id),0,0,0,0,0,0);
  return -2;
 }
 (void)func_8007B010(0,0);if(PE_Port_ShouldStop())return 0;
 if(id==2u)for(unsigned i=0;i<4;i++)PE_StoreU8(0x8009afd0u+i,PE_LoadU8(input+i));
 if(id==14u)PE_StoreU8(0x8009afd4u,PE_LoadU8(input));
 PE_StoreU8(0x8009b294u,0);if(PE_LoadU32(0x8009b0fcu+4u*id))PE_StoreU8(0x8009b295u,0);
 uint32_t p=PE_LoadU32(0x8009b27cu);PE_CdStoreU8(p,0);
 uint32_t count=PE_LoadU32(0x8009b1fcu+4u*id),i=0;
 if((int32_t)count>0)do{p=PE_LoadU32(0x8009b284u);uint8_t v=PE_LoadU8(input+i);PE_CdStoreU8(p,v);i++;count=PE_LoadU32(0x8009b1fcu+4u*id);}while((int32_t)i<(int32_t)count);
 p=PE_LoadU32(0x8009b280u);PE_StoreU8(0x8009afd5u,(uint8_t)id);PE_CdStoreU8(p,(uint8_t)id);if(mode)return 0;
 (void)HostFB_VSync(-1);uint32_t now=PE_GPU_VSyncQuery();PE_StoreU32(0x800a3478u,now+960u);PE_StoreU32(0x800a347cu,0);uint32_t state=PE_LoadU8(0x8009b294u);PE_StoreU32(0x800a3480u,0x80011bccu);
 while(!state){
  if(PE_Port_ShouldStop())return 0;
  (void)HostFB_VSync(-1);now=PE_GPU_VSyncQuery();uint32_t deadline=PE_LoadU32(0x800a3478u);int timed=(int32_t)deadline<(int32_t)now;
  if(!timed){uint32_t prev=PE_LoadU32(0x800a347cu);PE_StoreU32(0x800a347cu,prev+1u);timed=(int32_t)prev>0x3c0000;}
  if(timed){PE_Cd_TimeoutRecover();return -1;}
  if(func_80073DE8()){
   p=PE_LoadU32(0x8009b27cu);uint32_t saved=PE_CdLoadU8(p)&3u;
   for(;;){uint32_t bits=(uint32_t)pe_plat_cd_acknowledge_host();if(PE_Port_ShouldStop())return 0;if(!bits)break;
    if(bits&4u){uint32_t cb=PE_LoadU32(0x8009afb8u);if(cb){uint8_t status=PE_LoadU8(0x8009b295u);if(!PE_Cd_DispatchDataCallback(cb,status,0x800a3468u))return 0;if(PE_Port_ShouldStop())return 0;}}
    if(bits&2u){uint32_t cb=PE_LoadU32(0x8009afb4u);if(cb){uint8_t status=PE_LoadU8(0x8009b294u);if(!PE_Cd_DispatchDataCallback(cb,status,0x800a3460u))return 0;if(PE_Port_ShouldStop())return 0;}}
   }
   p=PE_LoadU32(0x8009b27cu);PE_CdStoreU8(p,(uint8_t)saved);
  }
  state=PE_LoadU8(0x8009b294u);
 }
 if(dst)for(unsigned j=0;j<8;j++)PE_StoreU8(dst+j,PE_LoadU8(0x800a3460u+j));
 return PE_LoadU8(0x8009b294u)==5u?-1:0;
}

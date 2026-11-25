/* Independent guest8007FCFC from original-code vectors and frozen host traces.
 * Preserves null data, stop-command translation and host stop ordering. */
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "game_port.h"
#include "pe_plat/cd.h"
int pe_plat_cd_issue_wrapper(uint32_t command,uint32_t data)
{
 pe_plat_cd_flush();
 if(PE_Port_ShouldStop())return 0;
 PE_StoreU8(0x8009B558u,(uint8_t)command);
 if(data)pe_plat_cd_copy4(0x8009B559u,data);
 PE_StoreU32(0x8009B560u,data?0x8009B559u:0u);
 PE_StoreU32(0x8009B598u,PE_LoadU32(0x8009B5A4u+4u*PE_LoadU8(0x8009B558u))?960u:30u);
 PE_StoreU32(0x8009B59Cu,0u);
 if((PE_LoadU8(0x8009B558u)==8u && PE_LoadU8(0x8009B58Bu)!=1u) ||
    (PE_LoadU8(0x8009B558u)==7u && PE_LoadU8(0x8009B58Bu)==1u)){
  PE_StoreU8(0x8009B558u,1u);PE_StoreU32(0x8009B560u,0u);
 }
 int result=func_8007B558(PE_LoadU8(0x8009B558u),PE_LoadU32(0x8009B560u),0u,1u);
 if(PE_Port_ShouldStop())return 0;
 if(result!=0){
  PE_StoreU32(0x8009B59Cu,0u);PE_StoreU32(0x8009B598u,0u);return 0;
 }
 PE_StoreU8(0x8009B580u,PE_LoadU8(0x8009B558u));
 return 1;
}

/* Independent retail draft from original-code traces; scratch only. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
int pe_plat_cd_read_stream(uint32_t location,uint32_t mode)
{
 uint32_t old_data=0;int old_ready=0;
 if(mode&0x100u){
  PE_StoreU32(0x800A8020u,(mode&0x20u)?0u:1u);
  old_data=func_800824F0(0x8007C214u);
  old_ready=func_800824C8(0x800813E8u);
 }
 int result=func_8007F0C8((uint8_t)mode,location,0x1B,0u,0xFFFFFFFFu);
 if(!result && (mode&0x100u)){
  func_800824F0(old_data);func_800824C8(old_ready);
 }
 return result;
}

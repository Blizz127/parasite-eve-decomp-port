/* Independent host caller from frozen-object traces; bound for 80080DC4. */
#include "pe_sdk.h"
#include "pe_cdreg.h"
#include "pe_str_feed.h"
#include "pe_bootstrap.h"
#include "game_port.h"
int pe_plat_cd_issue_step_host(int command,uint32_t param,uint32_t response)
{
 PE_Port_EscapePoint();
 if(!PE_CdReg_DeviceEnabled() && ((uint8_t)command==8 || (uint8_t)command==9)){
  PE_StrFeed_Close();return 1;
 }
 uint32_t sequence=func_8007EE84((uint8_t)command,param,0u,0u);
 if(!sequence || PE_Port_ShouldStop())return 0;
 for(;;){
  int status=func_8007F418(sequence,response);
  if(PE_Port_ShouldStop())return 0;
  if(status)return status==2;
  if(!PE_CdReg_DeviceEnabled()){
   Bootstrap_ReturnVoid1("CD_command_wait","func_80080DC4",sequence);
   PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
  }
 }
}

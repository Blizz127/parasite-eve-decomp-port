/* Independent draft from original-code command/poll vectors. */
#include "pe_sdk.h"
int pe_plat_cd_command_wait(int command, uint32_t param, uint32_t response)
{
 uint32_t sequence=func_8007EE84((uint8_t)command,param,0u,0u);
 if(!sequence)return 0;
 uint8_t status;
 do {status=(uint8_t)func_8007F418(sequence,response);} while(!status);
 return status==2u;
}

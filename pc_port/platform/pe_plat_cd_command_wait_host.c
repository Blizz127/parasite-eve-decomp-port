/* Independent host adapter from caller traces; retail core is separate. */
#include "pe_sdk.h"
#include "pe_disc.h"
#include "pe_cdreg.h"
#include "pe_guest_ram.h"
#include "pe_plat/cd.h"
int pe_plat_cd_command_wait_host(int command,uint32_t param,uint32_t response)
{
 if(PE_CdReg_DeviceEnabled())return func_80080DC4(command,param,response);
 if((uint8_t)command!=2u || response)return 0;
 if(!PE_RangeIsRam(param,4u))return 0;
 if(func_8007F72C()!=1 || pe_plat_cd_queue_len()!=0)return 0;
 PE_Disc *disc=PE_Disc_GetActive();if(!disc)return 0;
 uint8_t m=PE_LoadU8(param),s=PE_LoadU8(param+1u),f=PE_LoadU8(param+2u);
 if((m&15u)>9u || (m>>4)>9u || (s&15u)>9u || (s>>4)>5u || (f&15u)>9u || (f>>4)>7u)return 0;
 int32_t sector=pe_plat_cd_pos_to_int(param);
 if(sector<0 || (uint32_t)sector>=PE_Disc_UserSectorCount(disc))return 0;
 pe_plat_cd_store_setloc_hint(PE_LoadU32(param));return 1;
}

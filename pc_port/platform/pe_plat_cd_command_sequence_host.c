/* Host contract independently reconstructed from frozen caller traces. */
#include "pe_sdk.h"
int pe_plat_cd_command_sequence_host(uint32_t mode,uint32_t location,int command,uint32_t extra,uint32_t callback)
{
    uint8_t bytes[4];
    if(func_80080C48(location)<0)return 0;
    for(unsigned i=0;i<4;i++){
        bytes[i]=PE_LoadU8(location+i);
        PE_StoreU8(0x801FFE80u+i,bytes[i]);
    }
    uint8_t cmd=(uint8_t)command;
    if(cmd!=3 && cmd!=6 && cmd!=0x15 && cmd!=0x16 && cmd!=0x1B)return 0;
    if((PE_LoadU32(0x800A3608u)+4u)>8u)return 0;
    uint32_t serial=PE_LoadU32(0x8009B53Cu)+1u;
    if(!serial)serial=1;
    PE_StoreU32(0x8009B53Cu,serial);
    const uint8_t commands[4]={9,14,2,cmd};
    for(unsigned n=0;n<4;n++){
        uint32_t rec=func_8007E6B0();
        PE_StoreU32(rec,serial);
        PE_StoreU8(rec+4,commands[n]);
        if(n==1 || n==2){
            for(unsigned i=0;i<4;i++)PE_StoreU8(rec+5+i,n==2?bytes[i]:(i==0?(uint8_t)mode:0));
        }
        PE_StoreU32(rec+12,(n==1 || n==2)?rec+5:0);
        PE_StoreU32(rec+16,n==3?extra:0);
        PE_StoreU32(rec+20,callback);
        PE_StoreU32(0x800A3608u,PE_LoadU32(0x800A3608u)+1u);
    }
    if(func_8007FBF0(0)==1){
        uint32_t index=PE_LoadU32(0x800A3604u);
        if(PE_LoadU32(0x800A3540u+24u*index)==serial)(void)func_8007E8F4();
    }
    return (int32_t)serial;
}

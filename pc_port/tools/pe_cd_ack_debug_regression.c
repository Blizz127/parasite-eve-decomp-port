#include <stdint.h>
#include <stdio.h>
#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "pe_gpu.h"
#include "pe_sdk.h"
#include "game_port.h"
void Trace_Direct(const char *s){(void)s;}
static unsigned first,second;
int32_t __wrap_PE_Printf_Guest(uint32_t fmt,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f,uint32_t g)
{
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;
    if(fmt==0x80011b44u){first++;PE_StoreU32(0x8009afc0u,0);}
    if(fmt==0x80011b50u)second++;
    return 0;
}
extern int func_8007AAB4(void);
int main(void)
{
    const uint32_t targets[]={0x8007ae5cu,0x8007ae10u,0x8007ad10u,0x8007aedcu,0x8007af5cu};
    PE_RamReset();PE_Port_RunControlReset();PE_GPU_Init();PE_CdReg_Reset();
    PE_StoreU32(0x8009b27cu,0x1f801800u);PE_StoreU32(0x8009b280u,0x1f801801u);
    PE_StoreU32(0x8009b284u,0x1f801802u);PE_StoreU32(0x8009b288u,0x1f801803u);
    PE_StoreU8(0x8009afd5u,2);PE_StoreU32(0x8009afc0u,1);
    for(unsigned i=0;i<5;i++)PE_StoreU32(0x80011b8cu+4u*i,targets[i]);
    const uint8_t data[]={0x1d,0x42,0x43};
    if(!PE_CdReg_PushResponse(5,data,sizeof(data)))return 2;
    int result=func_8007AAB4();
    printf("result=%d first=%u second=%u stop=%d\n",result,first,second,PE_Port_ShouldStop());
    return !(result==6 && first==1u && second==0u && !PE_Port_ShouldStop());
}

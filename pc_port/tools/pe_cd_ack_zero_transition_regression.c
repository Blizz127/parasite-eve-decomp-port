/* Focused native bridge regression: interrupt status 1 -> 0 while stabilizing.
 * Link the runtime with --wrap=PE_CdLoadU8,--wrap=PE_CdStoreU8,
 * --wrap=PE_GuestCall,--wrap=PE_Printf_Guest. No game data is embedded. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "game_port.h"
void Trace_Direct(const char *s){(void)s;}
static unsigned reads,puts_calls,printf_calls;
uint8_t __wrap_PE_CdLoadU8(uint32_t a)
{
    if(a==0x80140090u)return reads++==0u?1u:0u;
    return 0;
}
void __wrap_PE_CdStoreU8(uint32_t a,uint8_t v){PE_StoreU8(a,v);}
int __wrap_PE_GuestCall(const char *site,uint32_t fn,unsigned arity,uintptr_t a,uintptr_t b,uintptr_t c,uintptr_t d)
{
    if(!strcmp(site,"CD unknown interrupt message") && fn==0x80073c5cu && arity==1u && a==0x80011b6cu && !b && !c && !d)puts_calls++;
    return 0;
}
int32_t __wrap_PE_Printf_Guest(uint32_t fmt,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f,uint32_t g)
{
    if(fmt==0x80011b80u && !a && !b && !c && !d && !e && !f && !g)printf_calls++;
    return 0;
}
int main(void)
{
    PE_RamReset();PE_Port_RunControlReset();
    PE_StoreU32(0x8009b27cu,0x80140080u);PE_StoreU32(0x8009b280u,0x801400b0u);
    PE_StoreU32(0x8009b284u,0x801400a0u);PE_StoreU32(0x8009b288u,0x80140090u);
    int result=func_8007AAB4();
    printf("result=%d reads=%u puts=%u printf=%u stop=%d\n",result,reads,puts_calls,printf_calls,PE_Port_ShouldStop());
    return !(result==0 && reads==4u && puts_calls==1u && printf_calls==1u && !PE_Port_ShouldStop());
}

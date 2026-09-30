#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include "retail_transition_exit_cases.h"
static unsigned current,calls;
static uint64_t trace;
static const uint32_t ranges[][2]={{0x80091A24u,4},{0x8009D280u,4},{0x800A77F4u,4},{0x800B0CD8u,4},{0x800B0DB0u,8},{0x800BCD80u,20}};
static uint64_t byte(uint64_t h,uint8_t b){return (h^b)*UINT64_C(1099511628211);}
static uint64_t state(void)
{
    uint64_t h=UINT64_C(14695981039346656037);
    for(unsigned i=0;i<6;i++)for(unsigned j=0;j<ranges[i][1];j++)h=byte(h,PE_LoadU8(ranges[i][0]+j));
    return h;
}
static void call(uint32_t fn,uint32_t a,uint32_t b,uint32_t c,uint32_t r0,uint32_t r1)
{
    uint64_t h=state();uint32_t words[]={fn,a,b,c,r0,r1,(uint32_t)h,(uint32_t)(h>>32)};
    for(unsigned i=0;i<8;i++)for(unsigned j=0;j<4;j++)trace=byte(trace,(uint8_t)(words[i]>>(j*8)));
    calls++;
    if((DAY1_exit_cases[current].seed&2) && calls==8)PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)^0xA5A54040u);
    if(calls==DAY1_exit_cases[current].stop)PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
void __wrap_HostFB_SetDispMask(int a){call(0x80074D28u,(uint32_t)a,0,0,0,0);}
void __wrap_HostFB_VSync(int a){call(0x80073A44u,(uint32_t)a,0,0,0,0);}
int __wrap_func_80074A44(int a){call(0x80074A44u,(uint32_t)a,0,0,0,0);return 0;}
int __wrap_func_80074DC0(int a){call(0x80074DC0u,(uint32_t)a,0,0,0,0);return 0;}
void __wrap_func_800755F0(pe_addr_t a){call(0x800755F0u,a,0,0,0,0);}
int __wrap_func_80074F44(RECT *r,uint8_t a,uint8_t b,uint8_t c)
{call(0x80074F44u,a,b,c,(uint16_t)r->x|((uint32_t)(uint16_t)r->y<<16),(uint16_t)r->w|((uint32_t)(uint16_t)r->h<<16));return 0;}
int __wrap_func_80086C5C(int a,uint32_t b,uint32_t c){call(0x80086C5Cu,(uint32_t)a,b,c,0,0);return 0;}
int __wrap_func_8008CBA8(void){call(0x8008CBA8u,0,0,0,0,0);return 0;}
void __wrap_func_80086FF8(void){call(0x80086FF8u,0,0,0,0,0);}
void __wrap_func_80087024(void){call(0x80087024u,0,0,0,0,0);}
static void fixture(void)
{
    PE_RamReset();PE_Port_RunControlReset();calls=0;trace=UINT64_C(14695981039346656037);
    unsigned seed=DAY1_exit_cases[current].seed;
    for(unsigned i=0;i<6;i++)for(unsigned j=0;j<ranges[i][1];j++)PE_StoreU8(ranges[i][0]+j,(uint8_t)(seed*71+j*29));
    static const uint32_t latches[]={0,1,0x80000000u,0xFFFFFFFFu};
    static const uint8_t handles[]={0,127,128,255};
    PE_StoreU32(0x80091A24u,latches[seed]);PE_StoreU8(0x800B0DB5u,handles[seed]);
    PE_StoreU32(0x800A7918u,DAY1_exit_cases[current].story);PE_StoreU32(0x8019CA68u,DAY1_exit_cases[current].selector);PE_StoreU32(0x800A77FCu,DAY1_exit_cases[current].flag);
}
int main(void)
{
    PE_RamInit();
    for(current=0;current<sizeof(DAY1_exit_cases)/sizeof(DAY1_exit_cases[0]);current++){
        fixture();func_80192030();
        if(calls!=DAY1_exit_cases[current].calls || trace!=DAY1_exit_cases[current].trace || state()!=DAY1_exit_cases[current].state || !!PE_Port_ShouldStop()!=!!DAY1_exit_cases[current].stop){
            fprintf(stderr,"exit case%u calls%u/%u trace%016llX/%016llX state%016llX/%016llX stop%d\n",current,calls,DAY1_exit_cases[current].calls,(unsigned long long)trace,(unsigned long long)DAY1_exit_cases[current].trace,(unsigned long long)state(),(unsigned long long)DAY1_exit_cases[current].state,PE_Port_ShouldStop());return 1;
        }
    }
    printf("PASS: %u original exit routing and stop-prefix cases\n",current);
    PE_RamDestroy();return 0;
}

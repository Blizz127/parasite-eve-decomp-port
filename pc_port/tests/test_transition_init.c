#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
#include "retail_transition_init_cases.h"
static unsigned current,calls,stop_at;
static uint64_t trace;
static const uint32_t ranges[][2]={{0x8019BFF0u,0x20},{0x8019C1F0u,4},{0x8019CC50u,4}};
static uint64_t byte(uint64_t h,uint8_t b){return (h^b)*UINT64_C(1099511628211);}
static uint64_t state(void)
{
    uint64_t h=UINT64_C(14695981039346656037);
    for(unsigned i=0;i<3;i++)for(unsigned j=0;j<ranges[i][1];j++)h=byte(h,PE_LoadU8(ranges[i][0]+j));
    return h;
}
static void call(uint32_t fn,uint32_t arg)
{
    static const uint32_t order[]={0x8019BD78u,0x8005BCB0u,0x800371A4u,0x80191C94u};
    if(calls>=4 || order[calls]!=fn){fprintf(stderr,"initializer provider order\n");exit(1);}
    uint64_t h=state();uint32_t words[]={fn,arg,(uint32_t)h,(uint32_t)(h>>32)};
    for(unsigned i=0;i<4;i++)for(unsigned j=0;j<4;j++)trace=byte(trace,(uint8_t)(words[i]>>(j*8)));
    if(++calls==stop_at)PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
void __wrap_func_8019BD78(void){call(0x8019BD78u,0);}
int __wrap_func_8005BCB0(void){call(0x8005BCB0u,0);return DAY1_init_cases[current].profile&4?-1:0;}
void __wrap_func_800371A4(int arg){call(0x800371A4u,(uint32_t)arg);}
void __wrap_func_80191C94(void){call(0x80191C94u,0);}
static void fixture(void)
{
    PE_RamReset();PE_Port_RunControlReset();calls=0;trace=UINT64_C(14695981039346656037);
    uint32_t scene=DAY1_init_cases[current].scene,encounter=DAY1_init_cases[current].encounter,profile=DAY1_init_cases[current].profile;
    for(unsigned i=0;i<3;i++)for(unsigned j=0;j<ranges[i][1];j++)PE_StoreU8(ranges[i][0]+j,(uint8_t)(scene+encounter+profile+j*17));
    PE_StoreU32(0x800A77F4u,scene);PE_StoreU32(0x800A7918u,encounter);
    PE_StoreU32(0x800B0CD8u,profile&1?0x40000000u:0);
    PE_StoreU32(0x800A77FCu,profile&2?0x2000u:0);
}
int main(void)
{
    PE_RamInit();
    for(current=0;current<sizeof(DAY1_init_cases)/sizeof(DAY1_init_cases[0]);current++){
        fixture();func_80191854();
        if(calls!=4 || trace!=DAY1_init_cases[current].trace || state()!=DAY1_init_cases[current].state){
            fprintf(stderr,"initializer case%u calls%u trace%016llX/%016llX state%016llX/%016llX\n",current,calls,(unsigned long long)trace,(unsigned long long)DAY1_init_cases[current].trace,(unsigned long long)state(),(unsigned long long)DAY1_init_cases[current].state);return 1;
        }
    }
    printf("PASS: %u original initializer selection cases\n",current);
    current=7;
    for(stop_at=1;stop_at<=4;stop_at++){
        fixture();uint64_t before=state();func_80191854();
        if(calls!=stop_at || !PE_Port_ShouldStop() || (stop_at<=2 && state()!=before)){
            fprintf(stderr,"initializer stop prefix%u differs\n",stop_at);return 1;
        }
    }
    puts("PASS: four initializer provider stop prefixes");PE_RamDestroy();return 0;
}

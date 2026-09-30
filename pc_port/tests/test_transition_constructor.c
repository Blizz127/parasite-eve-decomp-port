#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include "test_retail_fixups.h"
#include "retail_transition_constructor_cases.h"
#include "retail_transition_constructor_faults.h"
#include "retail_transition_path_cases.h"
static unsigned current,calls,tim,stop_at;
static uint64_t trace;
static const uint32_t ranges[][2]={{0x80091648u,32},{0x8019BFCCu,0xE44},{0x801E4A80u,0x5DAC}};
static const int counts[3][9]={{-1,0,3,4,5,6,7,8,0},{0,0,0,0,0,0,0,0,0},{8,8,8,8,8,8,8,8,8}};
static uint64_t byte(uint64_t h,uint8_t b){return (h^b)*UINT64_C(1099511628211);}
static uint64_t state(void)
{
    uint64_t h=UINT64_C(14695981039346656037);
    for(unsigned i=0;i<3;i++)for(unsigned j=0;j<ranges[i][1];j++)h=byte(h,PE_LoadU8(ranges[i][0]+j));
    return h;
}
static void call(uint32_t fn,uint32_t a,uint32_t b,uint32_t c)
{
    uint32_t words[]={fn,a,b,c};calls++;
    for(unsigned i=0;i<4;i++)for(unsigned j=0;j<4;j++)trace=byte(trace,(uint8_t)(words[i]>>(j*8)));
    if(calls==stop_at)PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
void __wrap_func_80191854(void){call(0x80191854u,0,0,0);}
void __wrap_func_800371B0(pe_addr_t p){call(0x800371B0u,p,0,0);if(++tim==1 && DAY1_constructor_cases[current].mutate)PE_StoreU32(0x8019C008u,1);}
void __wrap_func_80078E34(pe_addr_t p){call(0x80078E34u,p,0,0);}
void __wrap_func_80078E64(pe_addr_t p){call(0x80078E64u,p,0,0);}
void __wrap_func_80078FC4(uint32_t a,uint32_t b,uint32_t c){call(0x80078FC4u,a,b,c);}
void __wrap_func_80077E64(uint32_t a,uint32_t b,int32_t c){call(0x80077E64u,a,b,(uint32_t)c);}
void __wrap_func_80078FE4(uint32_t a,uint32_t b,uint32_t c){call(0x80078FE4u,a,b,c);}
static void fixture(void)
{
    PE_RamReset();PE_Port_RunControlReset();calls=tim=0;trace=UINT64_C(14695981039346656037);
    unsigned seed=DAY1_constructor_cases[current].seed,profile=DAY1_constructor_cases[current].profile;
    for(unsigned i=0;i<1025;i++)PE_StoreU16(0x8009A6ECu+i*2u,DAY1_path_atan[i]);
    for(unsigned i=0;i<3;i++)for(unsigned j=0;j<ranges[i][1];j++)PE_StoreU8(ranges[i][0]+j,(uint8_t)(seed*31+j*29+(j>>8)));
    PE_StoreU32(0x8019BFF4u,DAY1_constructor_cases[current].mode);
    PE_StoreU32(0x8019BFF8u,DAY1_constructor_cases[current].resume);PE_StoreU32(0x8019BFFCu,0x12340005u);
    PE_StoreU32(0x8019C008u,seed&1);PE_StoreU32(0x8019C020u,0);PE_StoreU32(0x800A77FCu,DAY1_constructor_cases[current].flags);
    PE_StoreU16(0x8019CC52u,(uint16_t)(seed%10));PE_StoreU8(0x8019C017u,(uint8_t)(30+seed%8));
    for(unsigned i=0;i<2;i++)PE_StoreU32(0x8019CE10u+4*i,0x80150000u+i*0x4000u-0x8019CE10u);
    for(unsigned i=0;i<3;i++)PE_StoreU32(0x801D0260u+4*i,(i==2?0x80140000u:0x80158000u+i*0x100u)-0x801D0260u);
    for(unsigned i=0;i<128;i++){
        unsigned off=0x400+i*64;PE_StoreU32(0x80150000u+i*4,off);PE_StoreU32(0x80150000u+off+0x30,0xABCD0000u+seed*131+i*17);
        pe_addr_t rec=0x80141000u+i*128;PE_StoreU32(0x80140000u+i*4,rec-0x80140000u);
        int count=i==0?10:i<=9?counts[profile][i-1]:(int)(3+(i+seed)%8);
        PE_StoreU16(rec+6u,(uint16_t)count);
        for(unsigned j=0;j<12;j++)for(unsigned k=0;k<3;k++)PE_StoreU16(rec+8u+j*8+k*2,(uint16_t)(seed*4711+i*7919+j*12347+k*29009));
    }
    for(unsigned j=0;j<6;j++)PE_StoreU32(0x1F800260u+j*4,0xC0FFEE00u+j);
}
int main(void)
{
    char why[320];
    PE_RamInit();
    /* The libgte ratan table seed comes from the user's disc (retail bytes
     * are never committed); exit 77 = ctest SKIP when no disc is configured. */
    if(PE_TestRetail_Apply(RETAILFIX_transition_path,sizeof(RETAILFIX_transition_path)/sizeof(RETAILFIX_transition_path[0]),why,sizeof(why))!=0){
        printf("SKIP: %s\n",why);PE_RamDestroy();return 77;
    }
    for(current=0;current<sizeof(DAY1_constructor_cases)/sizeof(DAY1_constructor_cases[0]);current++){
        fixture();func_80196498();
        if(calls!=DAY1_constructor_cases[current].calls || trace!=DAY1_constructor_cases[current].trace || state()!=DAY1_constructor_cases[current].state || PE_Port_ShouldStop()){
            fprintf(stderr,"constructor case%u calls%u trace%016llX/%016llX state%016llX/%016llX stop%d\n",current,calls,(unsigned long long)trace,(unsigned long long)DAY1_constructor_cases[current].trace,(unsigned long long)state(),(unsigned long long)DAY1_constructor_cases[current].state,PE_Port_ShouldStop());return 1;
        }
        for(unsigned j=0;j<6;j++)if(PE_LoadU32(0x1F800260u+j*4)!=0xC0FFEE00u+j){fputs("constructor scratch escaped\n",stderr);return 1;}
    }
    printf("PASS: %u whole original constructor cases (55/121/199 objects)\n",current);
    current=1; /* seed0, mode0, populated lists, mutate first window call */
    for(unsigned k=0;k<sizeof(DAY1_constructor_faults)/sizeof(DAY1_constructor_faults[0]);k++){
        fixture();stop_at=DAY1_constructor_faults[k].stop;
        unsigned id=DAY1_constructor_faults[k].bad_id;
        if(id)PE_StoreU16(0x80140000u+PE_LoadU32(0x80140000u+id*4u)+6u,2);
        func_80196498();
        if(calls!=DAY1_constructor_faults[k].calls || trace!=DAY1_constructor_faults[k].trace || state()!=DAY1_constructor_faults[k].state || !PE_Port_ShouldStop()){
            fprintf(stderr,"constructor fault%u calls%u trace%016llX/%016llX state%016llX/%016llX stop%d\n",k,calls,(unsigned long long)trace,(unsigned long long)DAY1_constructor_faults[k].trace,(unsigned long long)state(),(unsigned long long)DAY1_constructor_faults[k].state,PE_Port_ShouldStop());return 1;
        }
        for(unsigned j=0;j<6;j++)if(PE_LoadU32(0x1F800260u+j*4)!=0xC0FFEE00u+j){fputs("constructor fault lost scratch\n",stderr);return 1;}
    }
    puts("PASS: six original constructor stop prefixes");
    PE_RamDestroy();return 0;
}

#include "psx_compat.h"
#include "pe_port_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include "retail_transition_loader_cases.h"

static unsigned current,calls,phase,tims,issues[3],polls[3];
static uint64_t trace;
static uint64_t byte(uint64_t h,uint8_t b){return (h^b)*UINT64_C(1099511628211);}
static void call(uint32_t fn,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f)
{
    uint32_t words[]={fn,a,b,c,d,e,f,PE_LoadU32(0x800B0CD8u)};
    for(unsigned i=0;i<8;i++) for(unsigned j=0;j<4;j++) trace=byte(trace,(uint8_t)(words[i]>>(j*8)));
    if(++calls>2000){fprintf(stderr,"transition call overflow\n");exit(1);}
}
void __wrap_HostFB_VSync(int mode){call(0x80073A44,mode,0,0,0,0,0);}
void __wrap_HostFB_SetDispMask(int mask){call(0x80074D28,mask,0,0,0,0,0);}
int __wrap_func_8006CDA4(int a,int b,int c,pe_addr_t d,int e,int f)
{call(0x8006CDA4,a,b,c,d,e,f);return DAY1_transition_cases[current].xa;}
int __wrap_func_8006E6D4(int a,int b,pe_addr_t c,int d)
{
    call(0x8006E6D4,a,b,c,d,0,0);
    if(c==0x80140000u)phase=0;
    else if(c==0x80141000u)phase=1;
    else if(c==0x80150000u)phase=2;
    else{fprintf(stderr,"unexpected transition destination %08X\n",c);exit(1);}
    int result=(DAY1_transition_cases[current].profile&1)&&issues[phase]<2?-1:1;
    issues[phase]++;
    PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)|0x1004000u);
    return result;
}
int __wrap_func_800811E4(pe_addr_t local)
{
    (void)local;call(0x800811E4,0,0,0,0,0,0);
    static const int simple[]={2,1,0},retry[]={2,1,-1,2,1,0};
    unsigned i=polls[phase]++;
    if(i>=((DAY1_transition_cases[current].profile&2)?6u:3u)){
        fprintf(stderr,"unexpected extra transition poll\n");exit(1);
    }
    return (DAY1_transition_cases[current].profile&2)?retry[i]:simple[i];
}
pe_addr_t __wrap_func_800718D0(pe_addr_t tim)
{
    call(0x800718D0,tim,0,0,0,0,0);
    if(++tims==1 && DAY1_transition_cases[current].mutate)PE_StoreU32(0x800B0DD8u,2013);
    return 0;
}
int __wrap_func_80074DC0(int mode){call(0x80074DC0,mode,0,0,0,0,0);return 0;}
void __wrap_func_80072714(void){call(0x80072714,0,0,0,0,0,0);}
void __wrap_func_800726C4(void){call(0x800726C4,0,0,0,0,0,0);}
void __wrap_func_80072724(void){call(0x80072724,0,0,0,0,0,0);}

static void fixture(unsigned flag)
{
    PE_RamReset();
    static const uint32_t values[][2]={{0x800B0DD8u,1013},{0x800B0E34u,0x80140000u},
        {0x800B0E44u,0x80141000u},{0x80011614u,0x80150000u},{0x800B0E6Cu,0x80160000u},
        {0x800B0CD8u,0xA5A54042u}};
    for(unsigned i=0;i<6;i++)PE_StoreU32(values[i][0],values[i][1]);
    PE_StoreU32(0x800A77FCu,flag);
    static const uint16_t table[]={1276,1302,1547,1792,1975};
    for(unsigned i=0;i<5;i++)PE_StoreU16(0x80093168u+i*2,table[i]);
    for(unsigned i=0;i<3;i++)PE_StoreU32(0x80140000u+i*4,0x800+i*16);
    for(unsigned i=0;i<262;i++)PE_StoreU32(0x80141000u+i*4,0x800+i*16);
}
#include "test_transition_packages.h"

int main(void)
{
    PE_RamInit();
    for(current=0;current<sizeof(DAY1_transition_cases)/sizeof(DAY1_transition_cases[0]);current++) {
        fixture(DAY1_transition_cases[current].flag);
        calls=phase=tims=0;trace=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<3;i++)issues[i]=polls[i]=0;
        int result=func_8006ECEC();
        uint64_t state=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<0x198;i++)state=byte(state,PE_LoadU8(0x800B0CD8u+i));
        if(result || tims!=265 || calls!=DAY1_transition_cases[current].calls ||
           trace!=DAY1_transition_cases[current].trace || state!=DAY1_transition_cases[current].state) {
            fprintf(stderr,"transition case%u ret%d tims%u calls%u/%u trace%016llX/%016llX state%016llX/%016llX\n",
                current,result,tims,calls,DAY1_transition_cases[current].calls,
                (unsigned long long)trace,(unsigned long long)DAY1_transition_cases[current].trace,
                (unsigned long long)state,(unsigned long long)DAY1_transition_cases[current].state);return 1;
        }
    }
    printf("PASS: %u original transition-loader control-flow cases\n",current);
    if(test_transition_packages())return 1;
    PE_RamDestroy();return 0;
}

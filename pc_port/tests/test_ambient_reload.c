#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
#include "retail_ambient_reload_cases.h"
static unsigned current,called;
void Trace_Direct(const char *s) {(void)s;}
static void call(uint32_t fn,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f)
{
    uint32_t row[]={fn,a,b,c,d,e,f};unsigned i=ambient_cases[current].first+called++;
    if (i>=ambient_cases[current].end) {fprintf(stderr,"ambient%u unexpected call%X\n",current,fn);exit(1);}
    for(unsigned j=0;j<7;j++) if(row[j]!=ambient_calls[i][j]) {
        fprintf(stderr,"ambient%u call%u arg%u %X/%X\n",current,called-1,j,row[j],ambient_calls[i][j]);exit(1);
    }
}
void __wrap_func_80086FF8(void) {call(0x80086FF8,0,0,0,0,0,0);}
int __wrap_func_8006CDA4(int a,int b,int c,pe_addr_t d,int e,int f)
{call(0x8006CDA4,a,b,c,d,e,f);return ambient_cases[current].busy==1;}
int __wrap_func_8006D2B8(int a,int b,int c,pe_addr_t d,int e)
{call(0x8006D2B8,a,b,c,0,e,0);PE_StoreU32(d,1);return ambient_cases[current].busy==2;}
int __wrap_func_80086464(pe_addr_t a)
{call(0x80086464,a,0,0,0,0,0);return ambient_cases[current].handle;}
int __wrap_func_80086C5C(int a,uint32_t b,uint32_t c)
{call(0x80086C5C,a,b,c,0,0,0);return 0;}
int __wrap_func_8006DB48(uint32_t a,uint32_t b,uint32_t c,uint32_t d)
{call(0x8006DB48,a,b,c,d,0,0);return 0;}
int main(void)
{
    PE_RamInit();
    for(current=0;current<sizeof(ambient_cases)/sizeof(ambient_cases[0]);current++) {
        PE_RamReset();called=0;
        PE_StoreU32(0x800B0CD8u,ambient_cases[current].flags|0x12340000u);
        PE_StoreU8(0x800B0DCAu,ambient_cases[current].state);
        PE_StoreU8(0x800B0DB2u,0x83);PE_StoreU8(0x800B0DB4u,0xF9);PE_StoreU8(0x800B0DD6u,101);
        PE_StoreU32(0x800B0E00u,0x80142000u);PE_StoreU32(0x800B0E6Cu,0x80140000u);
        PE_StoreU16(0x800B0DC0u,0xFFFF);PE_StoreU32(0x8009D190u,ambient_cases[current].timer);
        PE_StoreU32(0x8009D18Cu,100);PE_StoreU32(0x8009CDA4u,100+ambient_cases[current].elapsed);
        uint32_t result=func_8006D60C(0);
        static const uint32_t ranges[][2]={{0x8009D188u,12},{0x800B0CD8u,0x198}};
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<2;i++) for(unsigned j=0;j<ranges[i][1];j++)
            hash=(hash^PE_LoadU8(ranges[i][0]+j))*UINT64_C(1099511628211);
        if(result!=ambient_cases[current].result || hash!=ambient_cases[current].hash ||
           called!=ambient_cases[current].end-ambient_cases[current].first || PE_Port_ShouldStop()) {
            fprintf(stderr,"ambient%u result%X/%X hash%016llX/%016llX calls%u\n",current,result,
                ambient_cases[current].result,(unsigned long long)hash,(unsigned long long)ambient_cases[current].hash,called);
            return 1;
        }
    }
    printf("PASS: %u original ambient reload control-flow cases\n",current);PE_RamDestroy();return 0;
}

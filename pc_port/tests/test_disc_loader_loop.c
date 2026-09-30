#include "psx_compat.h"
#include "pe_port_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include "retail_disc_loader_loop_cases.h"

/* Linker wrappers isolate hardware providers only. The production6CDA4
 * translation is linked unchanged from pe_field_runtime. */
static unsigned current_case,call_index;
static int provider(uint32_t fn,uint32_t a,uint32_t b,uint32_t c,uint32_t d)
{
    uint32_t actual[5]={fn,a,b,c,d};
    unsigned first=DAY1_loader_cases[current_case].first;
    unsigned index=first+call_index++;
    if (index>=DAY1_loader_cases[current_case].end) {
        fprintf(stderr,"loader case%u unexpected provider%08X\n",current_case,fn);
        exit(1);
    }
    for(unsigned i=0;i<5;i++) if(actual[i]!=DAY1_loader_calls[index][i]) {
        fprintf(stderr,"loader case%u call%u word%u: %08X != %08X\n",
                current_case,index-first,i,actual[i],DAY1_loader_calls[index][i]);
        exit(1);
    }
    return (int)(call_index-1)==DAY1_loader_cases[current_case].fault?
            DAY1_loader_cases[current_case].ret:0;
}
int __wrap_func_80087198(void){return provider(0x80087198u,0,0,0,0);}
int __wrap_func_80087414(void){return provider(0x80087414u,0,0,0,0);}
int __wrap_func_8006E6D4(int a,int b,pe_addr_t c,int d){return provider(0x8006E6D4u,a,b,c,d);}
int __wrap_func_8006E7E8(void){return provider(0x8006E7E8u,0,0,0,0);}
int __wrap_func_800871AC(pe_addr_t a,uint32_t b){return provider(0x800871ACu,a,b,0,0);}
int __wrap_func_80087090(pe_addr_t a,int b){return provider(0x80087090u,a,b,0,0);}
int __wrap_func_800875FC(unsigned a,pe_addr_t b){return provider(0x800875FCu,a,b,0,0);}
int __wrap_func_80087428(unsigned a,pe_addr_t b,uint32_t c){return provider(0x80087428u,a,b,c,0);}
int __wrap_func_800870E0(void){return provider(0x800870E0u,0,0,0,0);}

int main(void)
{
    PE_RamInit();
    for(current_case=0;current_case<sizeof(DAY1_loader_cases)/sizeof(DAY1_loader_cases[0]);current_case++) {
        uint64_t hash=UINT64_C(14695981039346656037);
        static const uint32_t ranges[][2]={{0x8009D170u,16},{0x800B0CD8u,0x120}};
        PE_RamReset();call_index=0;
        PE_StoreU32(0x8009317Cu,100);
        PE_StoreU16(0x80093182u,5);
        PE_StoreU16(0x80093184u,DAY1_loader_cases[current_case].empty?5:9);
        PE_StoreU32(0x8009D170u,1005);PE_StoreU32(0x8009D174u,4);
        PE_StoreU32(0x8009D178u,DAY1_loader_cases[current_case].empty?0:4);
        PE_StoreU32(0x8009D17Cu,2);PE_StoreU32(0x800B0DD8u,900);
        PE_StoreU8(0x800B0DC8u,(uint8_t)DAY1_loader_cases[current_case].state);
        int result=func_8006CDA4(DAY1_loader_cases[current_case].mode,1,3,
                0x80150000u,2,DAY1_loader_cases[current_case].blocking);
        for(unsigned i=0;i<2;i++) for(unsigned j=0;j<ranges[i][1];j++)
            hash=(hash^PE_LoadU8(ranges[i][0]+j))*UINT64_C(1099511628211);
        if((uint32_t)result!=DAY1_loader_cases[current_case].result ||
           hash!=DAY1_loader_cases[current_case].hash ||
           call_index!=DAY1_loader_cases[current_case].end-DAY1_loader_cases[current_case].first) {
            fprintf(stderr,"loader case%u: return%d/%u hash%016llX/%016llX calls%u\n",current_case,
                    result,DAY1_loader_cases[current_case].result,(unsigned long long)hash,
                    (unsigned long long)DAY1_loader_cases[current_case].hash,call_index);
            return 1;
        }
    }
    printf("PASS: %u original disc-loader control-flow cases\n",current_case);
    PE_RamDestroy();
    return 0;
}

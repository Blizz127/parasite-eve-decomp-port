#include "psx_compat.h"
#include "pe_port_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include "retail_music_bank_cases.h"

static unsigned current_case,call_index,load_count;
static void provider(uint32_t fn,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f)
{
    uint32_t actual[7]={fn,a,b,c,d,e,f};
    unsigned first=DAY1_bank_cases[current_case].first;
    unsigned index=first+call_index++;
    if(index>=DAY1_bank_cases[current_case].end) {
        fprintf(stderr,"bank case%u unexpected provider%08X\n",current_case,fn);exit(1);
    }
    for(unsigned i=0;i<7;i++) if(actual[i]!=DAY1_bank_calls[index][i]) {
        fprintf(stderr,"bank case%u call%u word%u: %08X != %08X\n",current_case,
                index-first,i,actual[i],DAY1_bank_calls[index][i]);exit(1);
    }
}
void __wrap_func_80086FF8(void){provider(0x80086FF8u,0,0,0,0,0,0);}
int __wrap_func_8006CDA4(int a,int b,int c,pe_addr_t d,int e,int f)
{
    provider(0x8006CDA4u,a,b,c,d,e,f);
    int busy=DAY1_bank_cases[current_case].busy;
    return (int)load_count++<busy?1:(busy==-1?-1:0);
}
static void fixture(unsigned kind)
{
    PE_RamReset();
    PE_StoreU32(0x800B0E64u,0x80130000u);PE_StoreU32(0x80130004u,0x40);
    PE_StoreU32(0x80130070u,(2u<<22)|0x100u);
    PE_StoreU32(0x80130100u,0xAB00001Du);PE_StoreU32(0x80130104u,0xCD000200u);
    PE_StoreU16(0x80130108u,5);PE_StoreU16(0x8013010Au,7);
    PE_StoreU32(0x8013010Cu,0xAB000011u);PE_StoreU32(0x80130110u,0xCD000220u);
    PE_StoreU16(0x80130114u,9);PE_StoreU16(0x80130116u,200);
    for(unsigned i=0;i<64;i++) PE_StoreU8(0x80130200u+i,(uint8_t)(i+1));
    PE_StoreU32(0x800B0E00u,0x80140000u);PE_StoreU32(0x800B0E04u,0x80141000u);
    PE_StoreU32(0x800B0E6Cu,0x80150000u);PE_StoreU32(0x80142000u,0xDEADBEEFu);
    PE_StoreU32(0x800B0CD8u,0xA5A50100u);
    static const uint8_t channels[]={0xFF,0xFF,0xFF,0xA1,0xFF,0xB2};
    for(unsigned i=0;i<6;i++) PE_StoreU8(0x800B0DB2u+i,channels[i]);
    if(kind>=1 && kind<=5) {
        unsigned slot=(kind==3 || kind==4)?1:0;
        PE_StoreU8(0x800B0DB4u+slot*2,7);PE_StoreU8(0x800B0DB2u+slot,5);
        if(kind==2 || kind==4) PE_StoreU32(0x800B0CD8u,0xA5A50100u|(0x40u<<slot));
        if(kind==5) {
            PE_StoreU8(0x800B0DB6u,7);PE_StoreU8(0x800B0DB3u,6);
            PE_StoreU32(0x800B0CD8u,0xA5A501C0u);
        }
    }
    if(kind==6 || kind==7) {
        PE_StoreU32(0x80130070u,0x100);
        if(kind==7){PE_StoreU8(0x800B0DB4u,7);PE_StoreU8(0x800B0DB2u,0xFE);}
    }
    if(kind==8 || kind==9) {
        PE_StoreU8(0x800B0DC9u,kind==8?7:9);
        PE_StoreU32(0x8009D180u,0x80130100u);PE_StoreU16(0x8009D184u,5);
    }
    if(kind==11) {
        PE_StoreU32(0x80130070u,0x100);PE_StoreU8(0x800B0DB4u,0x80);
        PE_StoreU8(0x800B0DB2u,0x80);
    }
}
int main(void)
{
    PE_RamInit();
    for(current_case=0;current_case<sizeof(DAY1_bank_cases)/sizeof(DAY1_bank_cases[0]);current_case++) {
        static const uint32_t ranges[][2]={{0x8009D180u,8},{0x800B0CD8u,0x198},
            {0x80140000u,0x40},{0x80141000u,0x40},{0x80142000u,4}};
        fixture(DAY1_bank_cases[current_case].kind);call_index=load_count=0;
        int result=func_8006D2B8(DAY1_bank_cases[current_case].id,DAY1_bank_cases[current_case].load,
            DAY1_bank_cases[current_case].second,0x80142000u,DAY1_bank_cases[current_case].blocking);
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<5;i++) for(unsigned j=0;j<ranges[i][1];j++)
            hash=(hash^PE_LoadU8(ranges[i][0]+j))*UINT64_C(1099511628211);
        if((uint32_t)result!=DAY1_bank_cases[current_case].result || hash!=DAY1_bank_cases[current_case].hash ||
           call_index!=DAY1_bank_cases[current_case].end-DAY1_bank_cases[current_case].first) {
            fprintf(stderr,"bank case%u: return%d/%u hash%016llX/%016llX calls%u\n",current_case,result,
                DAY1_bank_cases[current_case].result,(unsigned long long)hash,
                (unsigned long long)DAY1_bank_cases[current_case].hash,call_index);return 1;
        }
    }
    printf("PASS: %u original music-bank control-flow cases\n",current_case);
    PE_RamDestroy();return 0;
}

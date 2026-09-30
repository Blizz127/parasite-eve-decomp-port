#include "retail_transition_path_cases.h"
static void test_DAY1_transition_path(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_path"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_path);
    const uint32_t ranges[][2]={{0x140000u,512},{0x150000u,32},{0x19BFCCu,4}};
    for(unsigned k=0;k<sizeof(DAY1_path_cases)/sizeof(DAY1_path_cases[0]);k++){
        ResetTestState();
        uint32_t seed=DAY1_path_cases[k].seed;
        for(unsigned i=0;i<1025;i++)PE_StoreU16(0x8009A6ECu+i*2,DAY1_path_atan[i]);
        for(unsigned i=0;i<3;i++)for(unsigned j=0;j<ranges[i][1];j++)PE_StoreU8(0x80000000u+ranges[i][0]+j,(uint8_t)(seed+j*17));
        pe_addr_t rec=0x80140041u+seed%4u;
        PE_StoreU32(0x8014000Cu,rec-0x80140000u);PE_StoreU16(rec+6u,(uint16_t)DAY1_path_cases[k].count);
        uint32_t random=seed+1u;
        for(unsigned i=0;i<16;i++)for(unsigned j=0;j<3;j++){
            random=random*1664525u+1013904223u;PE_StoreU16(rec+8u+i*8+j*2,(uint16_t)(random>>16));
        }
        pe_addr_t pos=DAY1_path_cases[k].alias?rec+8u:0x80150000u;
        pe_addr_t rot=DAY1_path_cases[k].alias?pos+2u:0x80150010u;
        uint32_t result=(uint32_t)func_8018F55C(DAY1_path_cases[k].time,3,0x80140000u,pos,rot);
        uint64_t hash=hit_camera_hash(ranges,3);
        if(result!=DAY1_path_cases[k].result || hash!=DAY1_path_cases[k].hash)
            fprintf(stderr,"path case%u ret%08X/%08X hash%016llX/%016llX\n",k,result,DAY1_path_cases[k].result,(unsigned long long)hash,(unsigned long long)DAY1_path_cases[k].hash);
        ASSERT(result==DAY1_path_cases[k].result && hash==DAY1_path_cases[k].hash,"path differs from original instructions");
        ASSERT(!PE_Port_ShouldStop(),"valid path crossed a boundary");
    }
    PASS();
}
static void test_DAY1_transition_path_zero_period(void)
{
    TEST("DAY1_transition_path_zero_period");
    for(unsigned time=0;time<=256;time+=256){
        ResetTestState();PE_StoreU32(0x80140000u,16);PE_StoreU16(0x80140016u,2);
        PE_StoreU32(0x80150000u,0xDEADBEEFu);PE_StoreU32(0x80150010u,0xCAFEBABEu);
        func_8018F55C(time,0,0x80140000u,0x80150000u,0x80150010u);
        ASSERT(PE_Port_ShouldStop() && PE_LoadU8(0x8019BFCCu)==(time?1:0),"zero period must preserve pre-BREAK flag write");
        ASSERT(PE_LoadU32(0x80150000u)==0xDEADBEEFu && PE_LoadU32(0x80150010u)==0xCAFEBABEu,"BREAK boundary modified outputs");
    }
    PASS();
}

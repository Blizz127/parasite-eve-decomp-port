#include "retail_transition_bounds_cases.h"
static void test_DAY1_transition_bounds(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_bounds"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_bounds);
    static const uint32_t ranges[][2]={{0x963DC,0x290},{0x140000,128},{0x19BFD0,32},{0x19CA90,0x190},{0x19CDF0,32}};
    for(unsigned k=0;k<sizeof(DAY1_bounds_cases)/sizeof(DAY1_bounds_cases[0]);k++) {
        ResetTestState();memset(&g_pe_gte,0,sizeof(g_pe_gte));
        for(unsigned i=0;i<sizeof(DAY1_bounds_tables);i++)PE_StoreU8(0x80095D00u+i,DAY1_bounds_tables[i]);
        for(unsigned j=0;j<5;j++)for(unsigned i=0;i<ranges[j][1];i++)PE_StoreU8(0x80000000u+ranges[j][0]+i,(uint8_t)(DAY1_bounds_cases[k].seed+i*17));
        PE_StoreU32(0x800963E8u,DAY1_bounds_cases[k].depth);
        for(unsigned i=0;i<8;i++) {
            PE_StoreU32(0x8019CDF0u+i*4,DAY1_bounds_cases[k].matrix[i]);
            PE_StoreU32(0x80140000u+i*4,DAY1_bounds_cases[k].input[i]);
            PE_StoreU32(0x8019BFD0u+i*4,DAY1_bounds_cases[k].input[i]);
        }
        PE_GTE_LoadRT(0x8019CDF0u);
        for(unsigned i=0;i<112;i++)PE_StoreU8(0x1F800300u+i,(uint8_t)(i*13+7));
        unsigned kind=DAY1_bounds_cases[k].kind,offset=DAY1_bounds_cases[k].offset;
        uint32_t ret=0,state[18]={0};
        if(!kind)func_8018F92C(k%3==0?0x80140000u:k%3==1?0x8019CBB0u:0x8019CB50u);
        else if(kind==1)ret=func_800792D4(0x80140000u,0x80140000u+offset,0x80140060u);
        else func_800791D0(0x80140000u,0x80140010u,0x80140000u+offset);
        uint64_t h=hit_camera_hash(ranges,5);
        if(h!=DAY1_bounds_cases[k].hash)fprintf(stderr,"bounds%u kind%u RAM %016llX/%016llX\n",k,kind,(unsigned long long)h,(unsigned long long)DAY1_bounds_cases[k].hash);
        ASSERT(h==DAY1_bounds_cases[k].hash,"bounds persistent RAM differs from original");
        ASSERT(!PE_Port_ShouldStop(),"bounds graph stopped");
        if(kind==1)ASSERT(ret==DAY1_bounds_cases[k].ret,"SDK vector flags differ");
        for(unsigned i=0;i<112;i++)ASSERT(PE_LoadU8(0x1F800300u+i)==(uint8_t)(i*13+7),"bounds scratch leaked");
        for(unsigned i=0;i<9;i++)state[i/2]|=(uint32_t)(uint16_t)g_pe_gte.rt[i/3][i%3]<<((i%2)*16);
        for(unsigned i=0;i<3;i++)state[5+i]=(uint32_t)g_pe_gte.tr[i];
        state[8]=(uint16_t)g_pe_gte.ir0;
        for(unsigned i=0;i<3;i++) {state[9+i]=(uint16_t)g_pe_gte.ir[i];state[12+i]=g_pe_gte.rgb_fifo[i];state[15+i]=(uint32_t)g_pe_gte.mac[i];}
        for(unsigned i=0;i<18;i++) {
            if(state[i]!=DAY1_bounds_cases[k].state[i])fprintf(stderr,"bounds%u GTE%u %08X/%08X\n",k,i,state[i],DAY1_bounds_cases[k].state[i]);
            ASSERT(state[i]==DAY1_bounds_cases[k].state[i],"bounds terminal GTE differs");
        }
        ASSERT(!g_stub_order_count,"bounds graph used stub");
    }
    PASS();
}

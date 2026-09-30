#include "retail_transition_update_cases.h"
static void test_DAY1_transition_update(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_update"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_path);
    const uint16_t subtitle_tail[]={0x1234,0xCDEF,0x89AB,0xEFA8};
    for(unsigned k=0;k<sizeof(DAY1_update_cases)/sizeof(DAY1_update_cases[0]);k++) {
        DAY1_camera_fixture(DAY1_update_cases[k].seed,0);
        for(unsigned j=0;j<sizeof(DAY1_update_ranges)/sizeof(DAY1_update_ranges[0]);j++)
            for(unsigned i=0;i<DAY1_update_ranges[j][1];i++)PE_StoreU8(0x80000000u+DAY1_update_ranges[j][0]+i,0);
        for(unsigned i=DAY1_update_cases[k].first;i<DAY1_update_cases[k].end;i++)PE_StoreU32(0x80000000u+DAY1_update_patches[i][0],DAY1_update_patches[i][1]);
        for(unsigned i=0;i<160;i++)PE_StoreU8(0x1F800300u+i,(uint8_t)(i*13+7));
        for(unsigned i=0;i<10;i++)PE_StoreU8(0x1F8003D0u+i,(uint8_t)(i*7+5));
        unsigned fault=DAY1_update_cases[k].fault;
        if(fault>=3){pe_addr_t rec=0x80140000u+PE_LoadU32(0x80140000u+(75u+fault)*4u);PE_StoreU16(rec+6u,2);}
        if(DAY1_update_cases[k].outer)func_801942FC();else PE_TransitionUpdate(fault==1?NULL:DAY1_update_cases[k].menu,fault==2?NULL:subtitle_tail);
        uint64_t h=hit_camera_hash(DAY1_update_ranges,sizeof(DAY1_update_ranges)/sizeof(DAY1_update_ranges[0]));
        if(h!=DAY1_update_cases[k].hash)fprintf(stderr,"update%u RAM %016llX/%016llX\n",k,(unsigned long long)h,(unsigned long long)DAY1_update_cases[k].hash);
        ASSERT(h==DAY1_update_cases[k].hash,"whole update persistent RAM differs");
        ASSERT(!!PE_Port_ShouldStop()==!!fault,"whole update stop prefix differs");
        for(unsigned i=0;i<160;i++)ASSERT(PE_LoadU8(0x1F800300u+i)==(uint8_t)(i*13+7),"update scratch escaped");
        for(unsigned i=0;i<10;i++)ASSERT(PE_LoadU8(0x1F8003D0u+i)==(uint8_t)(i*7+5),"subtitle scratch escaped");
        ASSERT(!g_stub_order_count,"update used stub");
    }
    PASS();
}

#include "retail_dialogue_background_cases.h"
static void test_DAY2_dialogue_background(void)
{
    TEST("DAY2_dialogue_background");
    for (unsigned k=0;k<sizeof(DBG_cases)/sizeof(DBG_cases[0]);k++) {
        uint64_t flag,draw;
        ResetTestState();
        for (unsigned i=0;i<sizeof(DBG_common)/sizeof(DBG_common[0]);i++)
            PE_StoreU32(0x80000000u+DBG_common[i][0],DBG_common[i][1]);
        for (unsigned i=DBG_cases[k].first;i<DBG_cases[k].end;i++)
            PE_StoreU32(0x80000000u+DBG_patches[i][0],DBG_patches[i][1]);
        ASSERT(func_80016910_key2900_cut(0x80156000u)==1,"scene flag command completes");
        flag=hit_camera_hash(DBG_ranges,sizeof(DBG_ranges)/sizeof(DBG_ranges[0]));
        ASSERT(flag==DBG_cases[k].flag,"scene flag differs from original");
        func_80037870();
        draw=hit_camera_hash(DBG_ranges,sizeof(DBG_ranges)/sizeof(DBG_ranges[0]));
        if(draw!=DBG_cases[k].draw)fprintf(stderr,"dialogue background%u %016llX/%016llX\n",k,(unsigned long long)draw,(unsigned long long)DBG_cases[k].draw);
        ASSERT(draw==DBG_cases[k].draw,"background packet update differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"background graph completed natively");
    }
    PASS();
}

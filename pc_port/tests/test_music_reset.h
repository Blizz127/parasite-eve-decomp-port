#include "retail_music_reset_cases.h"
static void test_DAY2_music_reset(void)
{
    TEST("DAY2_music_reset");
    for (unsigned k=0;k<sizeof(MRS_cases)/sizeof(MRS_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(MRS_common)/sizeof(MRS_common[0]);i++)
            PE_StoreU32(0x80000000u+MRS_common[i][0],MRS_common[i][1]);
        for (unsigned i=MRS_cases[k].first;i<MRS_cases[k].end;i++)
            PE_StoreU32(0x80000000u+MRS_patches[i][0],MRS_patches[i][1]);
        ASSERT(func_80015DAC_default_cut(0x80140200u)==1,"EA217 reset completes");
        hash=hit_camera_hash(MRS_ranges,sizeof(MRS_ranges)/sizeof(MRS_ranges[0]));
        ASSERT(hash==MRS_cases[k].hash,"EA217 channel reset and audio FIFO differ from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"music reset completes natively");
    }
    PASS();
}

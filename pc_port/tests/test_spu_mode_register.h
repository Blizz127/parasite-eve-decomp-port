#include "retail_spu_mode_register_cases.h"
static void test_DAY2_spu_mode_register(void)
{
    TEST("DAY2_spu_mode_register");
    for(unsigned k=0;k<sizeof(SMR_cases)/sizeof(SMR_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(SMR_common)/sizeof(SMR_common[0]);i++)PE_StoreU32(0x80000000u|SMR_common[i][0],SMR_common[i][1]);
        for(unsigned i=SMR_cases[k].first;i<SMR_cases[k].end;i++)PE_StoreU32(0x80000000u|SMR_patches[i][0],SMR_patches[i][1]);
        func_8008D140(0x80150000u);
        ASSERT(hit_camera_hash(SMR_ranges,sizeof(SMR_ranges)/sizeof(SMR_ranges[0]))==SMR_cases[k].hash,"SPU mode-register stores match original");
        ASSERT(!PE_Port_ShouldStop(),"mode register fixture completes");
    }
    PASS();
}

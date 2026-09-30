#include "retail_actor_retirement_cases.h"

static void test_DAY1_actor_retirement(void)
{
    TEST("DAY1_actor_retirement");
    for (unsigned k=0;k<sizeof(retirement_cases)/sizeof(retirement_cases[0]);k++) {
        ResetTestState();
        for (unsigned i=0;i<sizeof(retirement_common)/sizeof(retirement_common[0]);i++)
            PE_StoreU32(0x80000000u+retirement_common[i][0],retirement_common[i][1]);
        for (unsigned i=retirement_cases[k].first;i<retirement_cases[k].end;i++)
            PE_StoreU32(0x80000000u+retirement_patches[i][0],retirement_patches[i][1]);
        if(retirement_cases[k].kind==2u)func_8003601C();
        else func_800360B4();
        uint64_t hash=hit_camera_hash(retirement_ranges,sizeof(retirement_ranges)/sizeof(retirement_ranges[0]));
        if (hash!=retirement_cases[k].hash)
            fprintf(stderr,"actor retirement case %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)retirement_cases[k].hash);
        ASSERT(hash==retirement_cases[k].hash,"actor retirement RAM differs from original MIPS");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"complete native actor retirement graph");
    }
    PASS();
}

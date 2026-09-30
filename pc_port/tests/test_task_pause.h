#include "retail_task_pause_cases.h"

static void test_DAY1_task_pause(void)
{
    TEST_RETAIL_DISC1("DAY1_task_pause"); TEST_RETAIL_FIXUPS(RETAILFIX_task_pause);
    for (unsigned k=0;k<sizeof(task_pause_cases)/sizeof(task_pause_cases[0]);k++) {
        ResetTestState();
        for (unsigned i=0;i<sizeof(task_pause_common)/sizeof(task_pause_common[0]);i++)
            PE_StoreU32(0x80000000u+task_pause_common[i][0],task_pause_common[i][1]);
        for (unsigned i=task_pause_cases[k].first;i<task_pause_cases[k].end;i++)
            PE_StoreU32(0x80000000u+task_pause_patches[i][0],task_pause_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        unsigned result=0;
        if (task_pause_cases[k].entry==0) result=func_80018300(0);
        else if (task_pause_cases[k].entry==1) result=func_80018364(0);
        else func_80017018();
        uint64_t hash=hit_camera_hash(task_pause_ranges,sizeof(task_pause_ranges)/sizeof(task_pause_ranges[0]));
        if (hash!=task_pause_cases[k].hash || result!=task_pause_cases[k].result)
            fprintf(stderr,"task pause case%u hash%016llX/%016llX result%u/%u\n",k,
                    (unsigned long long)hash,(unsigned long long)task_pause_cases[k].hash,
                    result,task_pause_cases[k].result);
        ASSERT(hash==task_pause_cases[k].hash,"task flags, saved PCs and delays match original");
        ASSERT(result==task_pause_cases[k].result,"task handler return matches original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"task pause/resume executes natively");
    }
    ResetTestState();PASS();
}

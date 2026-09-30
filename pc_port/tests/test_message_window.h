#include "retail_message_window_cases.h"

static void test_DAY1_message_window(void)
{
    TEST("DAY1_message_window");
    for (unsigned k=0;k<sizeof(message_window_cases)/sizeof(message_window_cases[0]);k++) {
        ResetTestState();
        for (unsigned i=0;i<sizeof(message_window_common)/sizeof(message_window_common[0]);i++)
            PE_StoreU32(0x80000000u+message_window_common[i][0],message_window_common[i][1]);
        for (unsigned i=message_window_cases[k].first;i<message_window_cases[k].end;i++)
            PE_StoreU32(0x80000000u+message_window_patches[i][0],message_window_patches[i][1]);
        int result=func_80019D84(0x80140000u);
        uint64_t hash=hit_camera_hash(message_window_ranges,sizeof(message_window_ranges)/sizeof(message_window_ranges[0]));
        if (hash!=message_window_cases[k].hash)
            fprintf(stderr,"message window case%u hash%016llX/%016llX\n",k,
                    (unsigned long long)hash,(unsigned long long)message_window_cases[k].hash);
        ASSERT(hash==message_window_cases[k].hash,"message slot, flags and geometry match original");
        ASSERT(result==1,"message window handler returns original result");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"message window executes natively");
    }
    ResetTestState();PASS();
}

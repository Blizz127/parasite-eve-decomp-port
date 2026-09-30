#include "retail_linked_animation_cases.h"
static void test_DAY1_linked_animation(void)
{
    TEST("DAY1_linked_animation");
    for (unsigned k=0;k<sizeof(LAN_cases)/sizeof(LAN_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(LAN_common)/sizeof(LAN_common[0]);i++)
            PE_StoreU32(0x80000000u+LAN_common[i][0],LAN_common[i][1]);
        for (unsigned i=LAN_cases[k].first;i<LAN_cases[k].end;i++)
            PE_StoreU32(0x80000000u+LAN_patches[i][0],LAN_patches[i][1]);
        if (LAN_cases[k].entry == 0x8001A680u)
            func_8001A680_command_cut(LAN_cases[k].actor,LAN_cases[k].command);
        else if (LAN_cases[k].entry == 0x8001A784u)
            func_8001A784(LAN_cases[k].actor,LAN_cases[k].command);
        else func_8001A4AC(LAN_cases[k].actor);
        hash=hit_camera_hash(LAN_ranges,sizeof(LAN_ranges)/sizeof(LAN_ranges[0]));
        if (hash!=LAN_cases[k].hash)
            fprintf(stderr,"linked animation %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)LAN_cases[k].hash);
        ASSERT(hash==LAN_cases[k].hash,"linked actor state differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"linked animation graph executes natively");
    }
    PASS();
}

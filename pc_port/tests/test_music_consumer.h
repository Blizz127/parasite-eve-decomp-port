#include "retail_music_consumer_cases.h"
static void test_DAY2_music_consumer(void)
{
    TEST_RETAIL_DISC1("DAY2_music_consumer"); TEST_RETAIL_FIXUPS(RETAILFIX_music_consumer);
    for(unsigned k=0;k<sizeof(MUSICCON_cases)/sizeof(MUSICCON_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(MUSICCON_common)/sizeof(MUSICCON_common[0]);i++)PE_StoreU32(0x80000000u|MUSICCON_common[i][0],MUSICCON_common[i][1]);
        for(unsigned i=MUSICCON_cases[k].first;i<MUSICCON_cases[k].end;i++)PE_StoreU32(0x80000000u|MUSICCON_patches[i][0],MUSICCON_patches[i][1]);
        func_8008CA84();
        uint64_t hash=hit_camera_hash(MUSICCON_ranges,sizeof(MUSICCON_ranges)/sizeof(MUSICCON_ranges[0]));
        if(hash!=MUSICCON_cases[k].hash)fprintf(stderr,"music consumer case%u %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)MUSICCON_cases[k].hash);
        ASSERT(hash==MUSICCON_cases[k].hash,"music consumer state matches original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"music consumer fully native");
    }
    PASS();
}

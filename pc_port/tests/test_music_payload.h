#include "retail_music_payload_cases.h"
static void test_DAY2_music_payload(void)
{
    TEST("DAY2_music_payload");
    for(unsigned k=0;k<sizeof(MP_cases)/sizeof(MP_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(MP_ranges)/sizeof(MP_ranges[0]);i++)memset(PE_Translate(0x80000000u|MP_ranges[i][0],MP_ranges[i][1]),0,MP_ranges[i][1]);
        for(unsigned i=0;i<sizeof(MP_common)/sizeof(MP_common[0]);i++)PE_StoreU32(0x80000000u|MP_common[i][0],MP_common[i][1]);
        for(unsigned i=MP_cases[k].first;i<MP_cases[k].end;i++)PE_StoreU32(0x80000000u|MP_patches[i][0],MP_patches[i][1]);
        ASSERT((uint32_t)func_8008CBA8()==MP_cases[k].result,"music payload original return");
        ASSERT(hit_camera_hash(MP_ranges,sizeof(MP_ranges)/sizeof(MP_ranges[0]))==MP_cases[k].hash,"music payload original RAM");
        ASSERT(!PE_Port_ShouldStop(),"configured mode needs no hardware transition");
    }
    ResetTestState();
    PE_StoreU32(0x800BCD80u,0x10u);PE_StoreU32(0x800BCD84u,0x80150000u);
    PE_StoreU32(0x80150000u,0x4F414B41u);PE_StoreU16(0x80150004u,14);PE_StoreU16(0x80150008u,3);
    PE_StoreU32(0x8009D2C8u,0x800B6980u);PE_StoreU32(0x8009B3A0u,2);
    PE_StoreU32(0x8009B3FCu,0x80151000u);
    ASSERT(func_8008CBA8()==-1 && PE_Port_ShouldStop(),"mode transition without DMA completion provider is explicit");
    ASSERT(PE_LoadU32(0x8009D2F4u)==0,"unported mode cannot publish a playback command");
    PASS();
}

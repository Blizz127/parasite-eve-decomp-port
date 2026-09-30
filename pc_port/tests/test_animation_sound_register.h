#include "retail_animation_sound_register_cases.h"
static void test_DAY1_animation_sound_register(void)
{
    TEST("DAY1_animation_sound_register");
    for (unsigned k=0;k<sizeof(ASR_cases)/sizeof(ASR_cases[0]);k++) {
        uint64_t registered,played;
        ResetTestState();
        for (unsigned i=0;i<sizeof(ASR_common)/sizeof(ASR_common[0]);i++)
            PE_StoreU32(0x80000000u+ASR_common[i][0],ASR_common[i][1]);
        for (unsigned i=ASR_cases[k].first;i<ASR_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ASR_patches[i][0],ASR_patches[i][1]);
        ASSERT(func_80015DAC_default_cut(0x80140200u)==1,"EA animation sound registration completes");
        registered=hit_camera_hash(ASR_ranges,sizeof(ASR_ranges)/sizeof(ASR_ranges[0]));
        ASSERT(registered==ASR_cases[k].registered,"EA406/407 registration differs from original");
        ASSERT(func_8006A318(0x80150000u)==0,"registered sound scan completes");
        played=hit_camera_hash(ASR_ranges,sizeof(ASR_ranges)/sizeof(ASR_ranges[0]));
        ASSERT(played==ASR_cases[k].played,"registered sound playback differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"registration and sound graph complete natively");
    }
    PASS();
}

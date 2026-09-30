#include "retail_animation_sound_cases.h"
static void test_DAY1_animation_sound(void)
{
    TEST("DAY1_animation_sound");
    for (unsigned k=0;k<sizeof(ANS_cases)/sizeof(ANS_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(ANS_common)/sizeof(ANS_common[0]);i++)
            PE_StoreU32(0x80000000u+ANS_common[i][0],ANS_common[i][1]);
        for (unsigned i=ANS_cases[k].first;i<ANS_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ANS_patches[i][0],ANS_patches[i][1]);
        if (ANS_cases[k].entry == 0x8006A318u)
            ASSERT(func_8006A318(0x80150000u)==0,"sound scan completes");
        else func_8001A4AC(0x80150000u);
        hash=hit_camera_hash(ANS_ranges,sizeof(ANS_ranges)/sizeof(ANS_ranges[0]));
        if (hash!=ANS_cases[k].hash) {
            fprintf(stderr,"animation sound %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)ANS_cases[k].hash);
            if (getenv("PE_ANS_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"local/live/animation-sound-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==ANS_cases[k].hash,"animation sound memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"animation sound graph executes natively");
    }
    PASS();
}

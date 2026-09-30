#include "retail_music_volume_cases.h"
static void test_SEW21_music_volume(void)
{
    TEST("SEW21_music_volume");
    for (unsigned k=0;k<sizeof(SEW21_cases)/sizeof(SEW21_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(SEW21_common)/sizeof(SEW21_common[0]);i++)
            PE_StoreU32(0x80000000u+SEW21_common[i][0],SEW21_common[i][1]);
        for (unsigned i=SEW21_cases[k].first;i<SEW21_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SEW21_patches[i][0],SEW21_patches[i][1]);
        ASSERT(func_80015DAC_default_cut(0x80140000u)==1,"sound script completes");
        hash=hit_camera_hash(SEW21_ranges,sizeof(SEW21_ranges)/sizeof(SEW21_ranges[0]));
        if (hash!=SEW21_cases[k].hash) {
            fprintf(stderr,"music volume %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)SEW21_cases[k].hash);
            if (getenv("PE_SEW21_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"local/live/music-volume-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==SEW21_cases[k].hash,"music volume memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"music volume graph executes natively");
    }
    PASS();
}

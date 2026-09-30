#include "retail_script_sound_cases.h"
static void test_SEW19_script_sound(void)
{
    TEST("SEW19_script_sound");
    for (unsigned k=0;k<sizeof(SEW19_cases)/sizeof(SEW19_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(SEW19_common)/sizeof(SEW19_common[0]);i++)
            PE_StoreU32(0x80000000u+SEW19_common[i][0],SEW19_common[i][1]);
        for (unsigned i=SEW19_cases[k].first;i<SEW19_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SEW19_patches[i][0],SEW19_patches[i][1]);
        ASSERT(func_80015DAC_default_cut(0x80140200u)==1,"sound script completes");
        hash=hit_camera_hash(SEW19_ranges,sizeof(SEW19_ranges)/sizeof(SEW19_ranges[0]));
        if (hash!=SEW19_cases[k].hash) {
            fprintf(stderr,"script sound %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)SEW19_cases[k].hash);
            if (getenv("PE_SEW19_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"local/live/script-sound-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==SEW19_cases[k].hash,"script sound memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"script sound graph executes natively");
    }
    PASS();
}

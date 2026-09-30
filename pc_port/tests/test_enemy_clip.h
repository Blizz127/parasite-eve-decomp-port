#include "retail_enemy_clip_cases.h"

static void test_ATK21_retail_enemy_clip(void)
{
    unsigned k,i;
    TEST("ATK21_retail_enemy_clip");
    for (k=0;k<sizeof(ATK21_clip_cases)/sizeof(ATK21_clip_cases[0]);k++) {
        uint32_t result=0;uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(ATK21_clip_common)/sizeof(ATK21_clip_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK21_clip_common[i][0],ATK21_clip_common[i][1]);
        for (i=ATK21_clip_cases[k].first;i<ATK21_clip_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK21_clip_patches[i][0],ATK21_clip_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        if (ATK21_clip_cases[k].entry)func_8002F7D8(0x80141000u);
        else result=(uint32_t)func_8002FAF8(0x80141000u,ATK21_clip_cases[k].code);
        hash=hit_camera_hash(ATK20_exit_ranges,sizeof(ATK20_exit_ranges)/sizeof(ATK20_exit_ranges[0]));
        if(hash!=ATK21_clip_cases[k].hash) {
            fprintf(stderr,"enemy clip %u: %016llX expected %016llX\n",k,
                (unsigned long long)hash,(unsigned long long)ATK21_clip_cases[k].hash);
            if(getenv("PE_ATK21_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk21-native-%u.bin",k);out=fopen(path,"wb");
                if(out){for(i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==ATK21_clip_cases[k].hash,"enemy clip differs from original call graph");
        if(!ATK21_clip_cases[k].entry)ASSERT(result==ATK21_clip_cases[k].result,"enemy clip return differs");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"enemy clip executes natively");
    }
    PASS();
}

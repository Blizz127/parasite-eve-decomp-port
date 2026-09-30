#include "retail_player_animation_cases.h"

static void test_ATK32_retail_player_animation(void)
{
    unsigned k,i,frame;
    TEST("ATK32_retail_player_animation");
    for (k=0;k<sizeof(ATK32_animation_cases)/sizeof(ATK32_animation_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(ATK32_animation_common)/sizeof(ATK32_animation_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK32_animation_common[i][0],ATK32_animation_common[i][1]);
        for (i=ATK32_animation_cases[k].first;i<ATK32_animation_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK32_animation_patches[i][0],ATK32_animation_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        for (frame=0;frame<ATK32_animation_cases[k].frames;frame++) {
            func_80071A64(ATK32_animation_cases[k].seed);
            switch (ATK32_animation_cases[k].entry) {
            case 0:func_800299CC_player_animation();break;
            case 1:func_800306E0(0x80141000u);break;
            case 2:func_80025BD8(0x80141000u);break;
            }
        }
        hash=hit_camera_hash(ATK32_animation_ranges,sizeof(ATK32_animation_ranges)/sizeof(ATK32_animation_ranges[0]));
        if (hash!=ATK32_animation_cases[k].hash) {
            fprintf(stderr,"player animation %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)ATK32_animation_cases[k].hash);
            if (getenv("PE_ATK32_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk32-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==ATK32_animation_cases[k].hash,"player animation memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"player animation call graph executes natively");
    }
    PASS();
}

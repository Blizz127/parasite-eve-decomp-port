#include "retail_player_tick_cases.h"

static void test_ATK31_retail_player_tick(void)
{
    unsigned k,i,frame;
    TEST("ATK31_retail_player_tick");
    for (k=0;k<sizeof(ATK31_player_cases)/sizeof(ATK31_player_cases[0]);k++) {
        const uint32_t *a=ATK31_player_cases[k].args;
        uint32_t result=0;uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(ATK31_player_common)/sizeof(ATK31_player_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK31_player_common[i][0],ATK31_player_common[i][1]);
        for (i=ATK31_player_cases[k].first;i<ATK31_player_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK31_player_patches[i][0],ATK31_player_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        for (frame=0;frame<ATK31_player_cases[k].frames;frame++) {
            func_80071A64(ATK31_player_cases[k].seed);
            switch (ATK31_player_cases[k].entry) {
            case 0:func_8001D340(a[0]);break;
            case 1:func_8001F4D4(a[0]);break;
            case 2:result=(uint32_t)func_8001F814(a[0]);break;
            case 3:func_8001F9C4();break;
            case 4:func_800201DC();break;
            case 5:func_80020288(a[0]);break;
            case 6:func_80020CE4();break;
            }
        }
        hash=hit_camera_hash(ATK31_player_ranges,sizeof(ATK31_player_ranges)/sizeof(ATK31_player_ranges[0]));
        if (hash!=ATK31_player_cases[k].hash || result!=ATK31_player_cases[k].result) {
            fprintf(stderr,"player tick %u result %08X/%08X hash %016llX/%016llX\n",k,result,
                ATK31_player_cases[k].result,(unsigned long long)hash,(unsigned long long)ATK31_player_cases[k].hash);
            if (getenv("PE_ATK31_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk31-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==ATK31_player_cases[k].result,"player tick return differs from original");
        ASSERT(hash==ATK31_player_cases[k].hash,"player tick memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"player tick call graph executes natively");
    }
    PASS();
}

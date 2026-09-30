#include "retail_enemy_tick_cases.h"

static void test_ATK22_retail_enemy_tick(void)
{
    unsigned k,i;
    TEST("ATK22_retail_enemy_tick");
    for(k=0;k<sizeof(ATK22_tick_cases)/sizeof(ATK22_tick_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for(i=0;i<sizeof(ATK22_tick_common)/sizeof(ATK22_tick_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK22_tick_common[i][0],ATK22_tick_common[i][1]);
        for(i=ATK22_tick_cases[k].first;i<ATK22_tick_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK22_tick_patches[i][0],ATK22_tick_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);func_80071A64(1u);
        if(ATK22_tick_cases[k].entry==0)func_80027D14(0x80141000u);
        else if(ATK22_tick_cases[k].entry==1)func_80032B0C(ATK22_tick_cases[k].mode,0x800A5E2Cu);
        else func_80036254(0x80141000u);
        hash=hit_camera_hash(ATK22_tick_ranges,sizeof(ATK22_tick_ranges)/sizeof(ATK22_tick_ranges[0]));
        if(hash!=ATK22_tick_cases[k].hash) {
            fprintf(stderr,"enemy tick %u: %016llX expected %016llX\n",k,
                (unsigned long long)hash,(unsigned long long)ATK22_tick_cases[k].hash);
            if(getenv("PE_ATK22_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk22-native-%u.bin",k);out=fopen(path,"wb");
                if(out){for(i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==ATK22_tick_cases[k].hash,"enemy tick differs from original call graph");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"enemy tick executes natively");
    }
    PASS();
}

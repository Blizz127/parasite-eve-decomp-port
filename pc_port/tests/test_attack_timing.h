#include "retail_attack_timing_cases.h"
static void test_SEW20_attack_timing(void)
{
    TEST("SEW20_attack_timing");
    for (unsigned k=0;k<sizeof(SEW20_cases)/sizeof(SEW20_cases[0]);k++) {
        const uint32_t *a=SEW20_cases[k].args;
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(SEW20_common)/sizeof(SEW20_common[0]);i++)
            PE_StoreU32(0x80000000u+SEW20_common[i][0],SEW20_common[i][1]);
        for (unsigned i=SEW20_cases[k].first;i<SEW20_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SEW20_patches[i][0],SEW20_patches[i][1]);
        D_8009D1A0=0;
        if (SEW20_cases[k].entry==0)func_8002FAD8(a[0],a[1],a[2],a[3]);
        else if (SEW20_cases[k].entry==1) {
            ASSERT(func_800198C4(a[0])==1,"B0 wrapper completes");
        } else func_80017018();
        hash=hit_camera_hash(SEW20_ranges,sizeof(SEW20_ranges)/sizeof(SEW20_ranges[0]));
        if (hash!=SEW20_cases[k].hash) {
            fprintf(stderr,"attack timing %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)SEW20_cases[k].hash);
            if (getenv("PE_SEW20_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"local/live/attack-timing-native-%u.bin",k);out=fopen(path,"wb");
                if(out){for(unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==SEW20_cases[k].hash,"attack timing differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"attack timing executes natively");
    }
    PASS();
}

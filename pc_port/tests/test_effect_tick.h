#include "retail_effect_tick_cases.h"

static void test_ATK23_retail_effect_tick(void)
{
    unsigned k,i;
    TEST("ATK23_retail_effect_tick");
    for (k=0;k<sizeof(ATK23_effect_cases)/sizeof(ATK23_effect_cases[0]);k++) {
        const uint32_t *a=ATK23_effect_cases[k].args;
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(ATK23_effect_common)/sizeof(ATK23_effect_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK23_effect_common[i][0],ATK23_effect_common[i][1]);
        for (i=ATK23_effect_cases[k].first;i<ATK23_effect_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK23_effect_patches[i][0],ATK23_effect_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        switch (ATK23_effect_cases[k].entry) {
        case 0: result=(uint32_t)func_8006F9F0(a[0]);break;
        case 1: result=(uint32_t)func_800D413C(a[0]);break;
        case 2: result=(uint32_t)func_80069594();break;
        case 3: result=(uint32_t)func_800D4704(a[0]);break;
        case 4: result=(uint32_t)func_800D401C((int32_t)a[0]);break;
        case 5: result=func_800CE560(a[0],a[1],(int32_t)a[2],a[3]);break;
        case 6: result=func_800CE5AC(a[0],a[1],a[2],(int32_t)a[3],a[4]);break;
        case 7: result=func_800CE610(a[0]);break;
        case 8: result=(uint32_t)func_800CE688(a[0]);break;
        case 9: result=(uint32_t)func_800CE78C(a[0]);break;
        case 10:func_800CE870(a[0],(int32_t)a[1],a[2]);break;
        case 11:func_800CE8F0(a[0],a[1],a[2],a[3]);break;
        case 12:result=(uint32_t)func_800D3FD8();break;
        case 13:result=(uint32_t)func_8006DC18(a[0]);break;
        }
        hash=hit_camera_hash(ATK23_effect_ranges,sizeof(ATK23_effect_ranges)/sizeof(ATK23_effect_ranges[0]));
        if (hash!=ATK23_effect_cases[k].hash || result!=ATK23_effect_cases[k].result) {
            fprintf(stderr,"effect tick %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,ATK23_effect_cases[k].result,(unsigned long long)hash,
                (unsigned long long)ATK23_effect_cases[k].hash);
            if (getenv("PE_ATK23_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk23-native-%u.bin",k);out=fopen(path,"wb");
                if (out) { for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out); }
            }
        }
        ASSERT(result==ATK23_effect_cases[k].result,"effect return differs from original");
        ASSERT(hash==ATK23_effect_cases[k].hash,"effect memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"effect call graph executes natively");
    }
    PASS();
}

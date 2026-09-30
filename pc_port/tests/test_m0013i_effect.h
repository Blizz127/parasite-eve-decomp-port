#include "retail_m0013i_effect_cases.h"

static void test_SEW14_m0013i_effect(void)
{
    unsigned k,i,j;
    TEST_RETAIL_DISC1("SEW14_m0013i_effect"); TEST_RETAIL_FIXUPS(RETAILFIX_m0013i_effect);
    for (k=0;k<sizeof(SEW14_cases)/sizeof(SEW14_cases[0]);k++) {
        const uint32_t *a=SEW14_cases[k].args;
        uint32_t result=0;uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(SEW14_common)/sizeof(SEW14_common[0]);i++)
            PE_StoreU32(0x80000000u+SEW14_common[i][0],SEW14_common[i][1]);
        for (i=SEW14_cases[k].first;i<SEW14_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SEW14_patches[i][0],SEW14_patches[i][1]);
        PE_GTE_LoadRT33(0x80148000u);g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;
        g_pe_gte.h=256;g_pe_gte.dqa=-0x1062;g_pe_gte.dqb=0x1400000;
        switch (SEW14_cases[k].entry) {
        case 0:result=(uint32_t)PE_EffectCallback(0x8018F20Cu,(int32_t)a[0],a[1],a[2]);break;
        case 1:result=(uint32_t)PE_EffectCallback(0x8018F004u,(int32_t)a[0],a[1],0);break;
        case 2:result=(uint32_t)func_800C6B90(a[0],(int32_t)a[1]);break;
        case 3:result=(uint32_t)func_800D3F64(a[0],a[1]);break;
        case 4:result=(uint32_t)func_800CE688(a[0]);break;
        case 6:result=(uint32_t)func_800187C0(a[0]);break;
        case 5:result=(uint32_t)func_800CE78C(a[0]);break;
        }
        hash=ATK26_flare_hash();
        for (i=0;i<sizeof(SEW14_extra_ranges)/sizeof(SEW14_extra_ranges[0]);i++)
            for (j=0;j<SEW14_extra_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(0x80000000u+SEW14_extra_ranges[i][0]+j))*UINT64_C(1099511628211);
        if (hash!=SEW14_cases[k].hash || result!=SEW14_cases[k].result) {
            fprintf(stderr,"M0013I %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,SEW14_cases[k].result,(unsigned long long)hash,(unsigned long long)SEW14_cases[k].hash);
            if (getenv("PE_SEW14_DUMP")) {
                char path[128];FILE *out;
                snprintf(path,sizeof(path),"local/live/m0013i-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==SEW14_cases[k].result,"M0013I return differs from original");
        ASSERT(hash==SEW14_cases[k].hash,"M0013I state or GPU packets differ from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"M0013I graph executes natively");
    }
    PASS();
}

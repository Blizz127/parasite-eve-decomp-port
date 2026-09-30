#include "retail_m28_projectile_cases.h"

static void test_DAY1_m28_projectile(void)
{
    unsigned k,i,j;
    TEST_RETAIL_DISC1("DAY1_m28_projectile"); TEST_RETAIL_FIXUPS(RETAILFIX_m28_projectile);
    for (k=0;k<sizeof(DAY1_m28_projectile_cases)/sizeof(DAY1_m28_projectile_cases[0]);k++) {
        const uint32_t *a=DAY1_m28_projectile_cases[k].args;
        uint32_t result=0;uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(DAY1_m28_projectile_common)/sizeof(DAY1_m28_projectile_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_m28_projectile_common[i][0],DAY1_m28_projectile_common[i][1]);
        for (i=DAY1_m28_projectile_cases[k].first;i<DAY1_m28_projectile_cases[k].end;i++)
            PE_StoreU32(0x80000000u+DAY1_m28_projectile_patches[i][0],DAY1_m28_projectile_patches[i][1]);
        PE_GTE_LoadRT33(0x80148000u);g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;
        g_pe_gte.h=256;g_pe_gte.dqa=-0x1062;g_pe_gte.dqb=0x1400000;
        switch (DAY1_m28_projectile_cases[k].entry) {
        case 0:result=(uint32_t)PE_EffectCallback(0x80192700u,(int32_t)a[0],a[1],a[2]);break;
        case 1:result=(uint32_t)PE_EffectCallback(0x801924F8u,(int32_t)a[0],a[1],0);break;
        case 2:result=(uint32_t)func_800C6B90(a[0],(int32_t)a[1]);break;
        case 3:result=(uint32_t)func_800D3F64(a[0],a[1]);break;
        case 4:result=(uint32_t)func_800CE688(a[0]);break;
        case 6:result=(uint32_t)func_800187C0(a[0]);break;
        case 5:result=(uint32_t)func_800CE78C(a[0]);break;
        }
        hash=ATK26_flare_hash();
        for (i=0;i<sizeof(DAY1_m28_projectile_extra_ranges)/sizeof(DAY1_m28_projectile_extra_ranges[0]);i++)
            for (j=0;j<DAY1_m28_projectile_extra_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(0x80000000u+DAY1_m28_projectile_extra_ranges[i][0]+j))*UINT64_C(1099511628211);
        if (hash!=DAY1_m28_projectile_cases[k].hash || result!=DAY1_m28_projectile_cases[k].result) {
            fprintf(stderr,"M0028I %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,DAY1_m28_projectile_cases[k].result,(unsigned long long)hash,(unsigned long long)DAY1_m28_projectile_cases[k].hash);
            if (getenv("PE_DAY1_m28_projectile_DUMP")) {
                char path[128];FILE *out;
                snprintf(path,sizeof(path),"local/live/m28-projectile-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==DAY1_m28_projectile_cases[k].result,"M0028I return differs from original");
        ASSERT(hash==DAY1_m28_projectile_cases[k].hash,"M0028I state or GPU packets differ from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"M0028I graph executes natively");
    }
    PASS();
}

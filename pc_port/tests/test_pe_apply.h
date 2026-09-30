#include "retail_pe_apply_cases.h"

static void test_INV18_retail_pe_apply(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV18_retail_pe_apply"); TEST_RETAIL_FIXUPS(RETAILFIX_pe_apply);
    for (k=0;k<sizeof(INV18_pe_apply_cases)/sizeof(INV18_pe_apply_cases[0]);k++) {
        const uint32_t *args=INV18_pe_apply_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();func_80071A64(INV18_pe_apply_cases[k].seed);
        for (i=0;i<sizeof(INV18_pe_apply_common)/sizeof(INV18_pe_apply_common[0]);i++)
            PE_StoreU32(0x80000000u+INV18_pe_apply_common[i][0],INV18_pe_apply_common[i][1]);
        for (i=INV18_pe_apply_cases[k].first;i<INV18_pe_apply_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV18_pe_apply_patches[i][0],INV18_pe_apply_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        if (INV18_pe_apply_cases[k].entry==0) func_80024250((int32_t)a,args[1]);
        else func_80051770((int32_t)a);
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV18_pe_apply_ranges,sizeof(INV18_pe_apply_ranges)/sizeof(INV18_pe_apply_ranges[0]));
        if (hash!=INV18_pe_apply_cases[k].hash) {
            fprintf(stderr,"PE application %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV18_pe_apply_cases[k].hash,
                result,INV18_pe_apply_cases[k].result);
            if (getenv("PE_INV18_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv18-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV18_pe_apply_cases[k].hash,"PE application memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"PE application call graph executes natively");
    }
    PASS();
}

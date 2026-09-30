#include "retail_pe_action_cases.h"

static void test_INV20_retail_pe_action(void)
{
    unsigned k,i,failures=0;
    TEST_RETAIL_DISC1("INV20_retail_pe_action"); TEST_RETAIL_FIXUPS(RETAILFIX_pe_action);
    for (k=0;k<sizeof(INV20_pe_action_cases)/sizeof(INV20_pe_action_cases[0]);k++) {
        const uint32_t *args=INV20_pe_action_cases[k].args;
        uint32_t result=0u;
        (void)args;
        uint64_t hash;
        ResetTestState();func_80071A64(INV20_pe_action_cases[k].seed);
        for (i=0;i<sizeof(INV20_pe_action_common)/sizeof(INV20_pe_action_common[0]);i++)
            PE_StoreU32(0x80000000u+INV20_pe_action_common[i][0],INV20_pe_action_common[i][1]);
        for (i=INV20_pe_action_cases[k].first;i<INV20_pe_action_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV20_pe_action_patches[i][0],INV20_pe_action_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        if (INV20_pe_action_cases[k].entry==0) func_80022394();
        else func_80020D50();
        PE_StoreU32(0x8009D018u,D_8009D018);
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV20_pe_action_ranges,sizeof(INV20_pe_action_ranges)/sizeof(INV20_pe_action_ranges[0]));
        if (hash!=INV20_pe_action_cases[k].hash || g_stub_order_count || PE_Port_ShouldStop()) {
            fprintf(stderr,"PE battle action %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV20_pe_action_cases[k].hash,
                result,INV20_pe_action_cases[k].result);
            if (getenv("PE_INV20_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv20-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        if (hash!=INV20_pe_action_cases[k].hash || g_stub_order_count || PE_Port_ShouldStop()) failures++;
    }
    ASSERT(!failures,"PE battle action original comparisons or native call graphs failed");
    PASS();
}

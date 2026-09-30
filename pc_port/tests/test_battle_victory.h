#include "retail_battle_victory_cases.h"

static void test_VICTORY_original_entries(void)
{
    TEST_RETAIL_DISC1("VICTORY_original_entries"); TEST_RETAIL_FIXUPS(RETAILFIX_battle_victory);
    for (unsigned k=0;k<sizeof(VICTORY_cases)/sizeof(VICTORY_cases[0]);k++) {
        const uint32_t *args=VICTORY_cases[k].args;
        uint32_t a=args[0],b=args[1],c=args[2],d=args[3],result=0u;
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(VICTORY_common)/sizeof(VICTORY_common[0]);i++)
            PE_StoreU32(0x80000000u+VICTORY_common[i][0],VICTORY_common[i][1]);
        for (unsigned i=VICTORY_cases[k].first;i<VICTORY_cases[k].end;i++)
            PE_StoreU32(0x80000000u+VICTORY_patches[i][0],VICTORY_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        switch (VICTORY_cases[k].entry) {
case 0:func_8002B0E8();break;
}
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        hash=hit_camera_hash(VICTORY_ranges,sizeof(VICTORY_ranges)/sizeof(VICTORY_ranges[0]));
        if (hash!=VICTORY_cases[k].hash || result!=VICTORY_cases[k].result)
            fprintf(stderr,"victory %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)VICTORY_cases[k].hash,result,VICTORY_cases[k].result);
        ASSERT(hash==VICTORY_cases[k].hash,"victory memory differs from original");
        ASSERT(result==VICTORY_cases[k].result,"victory result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"victory graph executes natively");
    }
    PASS();
}

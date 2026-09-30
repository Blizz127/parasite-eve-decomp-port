#include "retail_battle_level_cases.h"

static void test_LEVEL_original_entries(void)
{
    TEST_RETAIL_DISC1("LEVEL_original_entries"); TEST_RETAIL_FIXUPS(RETAILFIX_battle_level);
    for (unsigned k=0;k<sizeof(LEVEL_cases)/sizeof(LEVEL_cases[0]);k++) {
        const uint32_t *args=LEVEL_cases[k].args;
        uint32_t a=args[0],b=args[1],c=args[2],d=args[3],result=0u;
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(LEVEL_common)/sizeof(LEVEL_common[0]);i++)
            PE_StoreU32(0x80000000u+LEVEL_common[i][0],LEVEL_common[i][1]);
        for (unsigned i=LEVEL_cases[k].first;i<LEVEL_cases[k].end;i++)
            PE_StoreU32(0x80000000u+LEVEL_patches[i][0],LEVEL_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        switch (LEVEL_cases[k].entry) {
case 0:func_8005BA78((int32_t)a,(int32_t)b,c,d);break;
case 1:func_800437B4(a);break;
case 2:func_8006062C((int32_t)a,(int32_t)b);break;
case 3:func_80050D20(a);break;
case 4:func_8004FFF8(a);break;
case 5:func_8004BF40();break;
case 6:func_80062FEC();break;
case 7:result=func_8004C1E0(a,b);break;
}
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        hash=hit_camera_hash(LEVEL_ranges,sizeof(LEVEL_ranges)/sizeof(LEVEL_ranges[0]));
        if (hash!=LEVEL_cases[k].hash || result!=LEVEL_cases[k].result)
            fprintf(stderr,"level %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)LEVEL_cases[k].hash,result,LEVEL_cases[k].result);
        ASSERT(hash==LEVEL_cases[k].hash,"level memory differs from original");
        ASSERT(result==LEVEL_cases[k].result,"level result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"level graph executes natively");
    }
    PASS();
}

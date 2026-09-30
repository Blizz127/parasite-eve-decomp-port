#include "retail_battle_loot_cases.h"

static void test_LOOT_original_entries(void)
{
    TEST_RETAIL_DISC1("LOOT_original_entries"); TEST_RETAIL_FIXUPS(RETAILFIX_battle_loot);
    for (unsigned k=0;k<sizeof(LOOT_cases)/sizeof(LOOT_cases[0]);k++) {
        const uint32_t *args=LOOT_cases[k].args;
        uint32_t a=args[0],b=args[1],c=args[2],d=args[3],result=0u;
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(LOOT_common)/sizeof(LOOT_common[0]);i++)
            PE_StoreU32(0x80000000u+LOOT_common[i][0],LOOT_common[i][1]);
        for (unsigned i=LOOT_cases[k].first;i<LOOT_cases[k].end;i++)
            PE_StoreU32(0x80000000u+LOOT_patches[i][0],LOOT_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        switch (LOOT_cases[k].entry) {
case 0:func_8004FCF8(a);break;
case 1:func_8004FD68(a);break;
case 2:func_80050B94(a);break;
case 3:func_80050BE8(a);break;
case 4:func_80057F14(a);break;
case 5:result=func_80058030(a,(int32_t)b,c,(int32_t)d);break;
case 6:result=func_8005833C((int32_t)a);break;
case 7:func_80058454();break;
case 8:func_80058670();break;
case 9:result=func_80048838(a,b);break;
case 10:func_80062FEC();break;
case 11:func_8005E30C();break;
case 12:func_80052C6C();break;
}
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        hash=hit_camera_hash(LOOT_ranges,sizeof(LOOT_ranges)/sizeof(LOOT_ranges[0]));
        if (hash!=LOOT_cases[k].hash || result!=LOOT_cases[k].result)
            fprintf(stderr,"loot %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)LOOT_cases[k].hash,result,LOOT_cases[k].result);
        ASSERT(hash==LOOT_cases[k].hash,"loot memory differs from original");
        ASSERT(result==LOOT_cases[k].result,"loot result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"loot graph executes natively");
    }
    PASS();
}

#include "retail_battle_reward_cases.h"

/* The old mode-2 tests must supply real window/stat resources now that reward
 * setup no longer bypasses allocation, bonus and inventory initialization. */
static void reward_seed_resources(void)
{
    for (unsigned i=0;i<sizeof(REWARD_common)/sizeof(REWARD_common[0]);i++)
        PE_StoreU32(0x80000000u+REWARD_common[i][0],REWARD_common[i][1]);
    /* Victory setup now clears both real effect pools before opening rewards. */
    PE_StoreU32(0x800942E4u,0x80180000u);PE_StoreU32(0x800942E8u,0x80190000u);
    for (unsigned i=0;i<22u;i++) {
        pe_addr_t slot=i<11u?0x80180000u+i*0xA0Cu:0x80190000u+(i-11u)*0x10Cu;
        PE_StoreU32(slot,0u);PE_StoreU32(slot+4u,0u);PE_StoreU32(slot+8u,0u);
    }
    D_8009D1A0=PE_LoadU32(0x8009D1A0u);
    PE_StoreU32(0x800C0E00u,0u);PE_StoreU16(0x800C0E1Eu,0u);
}

static void test_REWARD_original_entries(void)
{
    TEST_RETAIL_DISC1("REWARD_original_entries"); TEST_RETAIL_FIXUPS(RETAILFIX_battle_reward);
    for (unsigned k=0;k<sizeof(REWARD_cases)/sizeof(REWARD_cases[0]);k++) {
        const uint32_t *args=REWARD_cases[k].args;
        uint32_t a=args[0],b=args[1],c=args[2],result=0u;
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(REWARD_common)/sizeof(REWARD_common[0]);i++)
            PE_StoreU32(0x80000000u+REWARD_common[i][0],REWARD_common[i][1]);
        for (unsigned i=REWARD_cases[k].first;i<REWARD_cases[k].end;i++)
            PE_StoreU32(0x80000000u+REWARD_patches[i][0],REWARD_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        switch (REWARD_cases[k].entry) {
case 0:func_8004B70C(a,b,c);break;
case 1:func_8004B90C();break;
case 2:func_8004B970();break;
case 3:result=func_8004BB80(a,b);break;
case 4:func_8004BC80();break;
case 5:func_8004BCB4();break;
case 6:func_8004C4B4(a);break;
case 7:func_8004C520();break;
case 8:result=func_80055668(a);break;
case 9:result=func_8004C1E0(a,b);break;
case 10:result=func_8005382C((int32_t)a);break;
case 11:func_80048654();break;
case 12:func_80063D78(a,(int32_t)b,(int32_t)c);break;
case 13:result=func_8005B8A8((int32_t)a,b);break;
case 14:result=func_80051DF8((int32_t)a);break;
case 15:result=func_80057ECC();break;
case 16:func_80052764();break;
case 17:func_8004BE4C();break;
case 18:func_8004BF08();break;
}
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        hash=hit_camera_hash(REWARD_ranges,sizeof(REWARD_ranges)/sizeof(REWARD_ranges[0]));
        if (hash!=REWARD_cases[k].hash || result!=REWARD_cases[k].result)
            fprintf(stderr,"reward %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)REWARD_cases[k].hash,result,REWARD_cases[k].result);
        ASSERT(hash==REWARD_cases[k].hash,"reward memory differs from original");
        ASSERT(result==REWARD_cases[k].result,"reward result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"reward entry graph executes natively");
    }
    PASS();
}

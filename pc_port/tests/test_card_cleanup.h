#include "retail_card_cleanup_cases.h"
static void test_DAY1_card_cleanup(void)
{
    TEST_RETAIL_DISC1("DAY1_card_cleanup"); TEST_RETAIL_FIXUPS(RETAILFIX_card_cleanup);
    for(unsigned n=0;n<256;n++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_card_cleanup_common)/sizeof(DAY1_card_cleanup_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_card_cleanup_common[i][0],DAY1_card_cleanup_common[i][1]);
        for(unsigned i=DAY1_card_cleanup_cases[n].first;i<DAY1_card_cleanup_cases[n].end;i++)
            PE_StoreU32(0x80000000u+DAY1_card_cleanup_patches[i][0],DAY1_card_cleanup_patches[i][1]);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        if(n<192)func_80040F80(DAY1_card_cleanup_cases[n].argument);else func_8004D5CC(DAY1_card_cleanup_cases[n].argument);
        uint64_t hash=hit_camera_hash(DAY1_card_cleanup_ranges,sizeof(DAY1_card_cleanup_ranges)/sizeof(DAY1_card_cleanup_ranges[0]));
        if(hash!=DAY1_card_cleanup_cases[n].hash)fprintf(stderr,"card cleanup %u differs: %016llX expected %016llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_card_cleanup_cases[n].hash);
        ASSERT(hash==DAY1_card_cleanup_cases[n].hash,"card cleanup state differs from original");
        if(!card_operation_boundary_matches(DAY1_card_cleanup_cases[n].target,DAY1_card_cleanup_cases[n].mask,DAY1_card_cleanup_cases[n].args,DAY1_card_cleanup_cases[n].fifth))fprintf(stderr,"cleanup boundary %u expected %08X mask%u got stop%d calls%d stubs%d target%08X\n",n,DAY1_card_cleanup_cases[n].target,DAY1_card_cleanup_cases[n].mask,PE_Port_ShouldStop(),g_bootstrap_arg4_call_count,g_stub_order_count,g_bootstrap_arg4_call_count?(unsigned)g_bootstrap_arg4_calls[0].target:0u);
        ASSERT(card_operation_boundary_matches(DAY1_card_cleanup_cases[n].target,DAY1_card_cleanup_cases[n].mask,DAY1_card_cleanup_cases[n].args,DAY1_card_cleanup_cases[n].fifth),"cleanup operation boundary differs");
    }
    PASS();
}

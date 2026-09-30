#include "retail_card_driver_cases.h"
static void test_DAY1_card_driver(void)
{
    TEST_RETAIL_DISC1("DAY1_card_driver"); TEST_RETAIL_FIXUPS(RETAILFIX_card_driver);
    for(unsigned n=0;n<112;n++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_card_driver_common)/sizeof(DAY1_card_driver_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_card_driver_common[i][0],DAY1_card_driver_common[i][1]);
        for(unsigned i=DAY1_card_driver_cases[n].first;i<DAY1_card_driver_cases[n].end;i++)
            PE_StoreU32(0x80000000u+DAY1_card_driver_patches[i][0],DAY1_card_driver_patches[i][1]);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        if(n<96)func_800425DC();else func_80042928();
        uint64_t hash=hit_camera_hash(DAY1_card_driver_ranges,sizeof(DAY1_card_driver_ranges)/sizeof(DAY1_card_driver_ranges[0]));
        if(hash!=DAY1_card_driver_cases[n].hash)fprintf(stderr,"card driver %u differs: %016llX expected %016llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_card_driver_cases[n].hash);
        ASSERT(hash==DAY1_card_driver_cases[n].hash,"card driver state differs from original");
        ASSERT(card_operation_boundary_matches(DAY1_card_driver_cases[n].target,DAY1_card_driver_cases[n].mask,DAY1_card_driver_cases[n].args,DAY1_card_driver_cases[n].fifth),"driver operation boundary differs");
    }
    PASS();
}

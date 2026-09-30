#include "retail_card_confirmation_cases.h"
static void test_DAY1_card_confirmation(void)
{
    TEST_RETAIL_DISC1("DAY1_card_confirmation"); TEST_RETAIL_FIXUPS(RETAILFIX_card_confirmation);
    for(unsigned n=0;n<128;n++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_card_confirm_common)/sizeof(DAY1_card_confirm_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_card_confirm_common[i][0],DAY1_card_confirm_common[i][1]);
        for(unsigned i=DAY1_card_confirm_cases[n].first;i<DAY1_card_confirm_cases[n].end;i++)
            PE_StoreU32(0x80000000u+DAY1_card_confirm_patches[i][0],DAY1_card_confirm_patches[i][1]);
        for(unsigned i=0;i<(n/2%2)*8;i++)PE_GPU_VBlankStep();
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        ASSERT(func_80044E98(DAY1_card_confirm_cases[n].window,DAY1_card_confirm_cases[n].event)==1,"confirmation return differs");
        if(n>=64) {
            for(unsigned i=0;i<3;i++)func_80042B6C();
            if(n%4==0) {
                if(n/4%3)PE_StoreU32(0x8009D000u,0x41u);
                func_8004DA04();ASSERT(func_8004DA9C()==1,"progress input differs");
            } else func_8004CFD4();
        }
        uint64_t hash=hit_camera_hash(DAY1_card_confirm_ranges,sizeof(DAY1_card_confirm_ranges)/sizeof(DAY1_card_confirm_ranges[0]));
        if(hash!=DAY1_card_confirm_cases[n].hash)fprintf(stderr,"card confirmation %u: %016llX expected %016llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_card_confirm_cases[n].hash);
        ASSERT(hash==DAY1_card_confirm_cases[n].hash,"card confirmation state differs from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"card confirmation graph hit boundary");
    }
    PASS();
}

#include "retail_card_modal_cases.h"
static void test_DAY1_card_modal(void)
{
    TEST_RETAIL_DISC1("DAY1_card_modal"); TEST_RETAIL_FIXUPS(RETAILFIX_card_modal);
    for(unsigned n=0;n<112;n++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_card_modal_common)/sizeof(DAY1_card_modal_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_card_modal_common[i][0],DAY1_card_modal_common[i][1]);
        for(unsigned i=DAY1_card_modal_cases[n].first;i<DAY1_card_modal_cases[n].end;i++)
            PE_StoreU32(0x80000000u+DAY1_card_modal_patches[i][0],DAY1_card_modal_patches[i][1]);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);D_8009D048=PE_LoadU32(0x8009D048u);D_8009D050=PE_LoadU32(0x8009D050u);
        unsigned result=0;
        if(n<96)result=(unsigned)func_80042848(DAY1_card_modal_cases[n].index);
        else func_8004DAA4();
        uint64_t hash=hit_camera_hash(DAY1_card_modal_ranges,sizeof(DAY1_card_modal_ranges)/sizeof(DAY1_card_modal_ranges[0]));
        if(hash!=DAY1_card_modal_cases[n].hash)fprintf(stderr,"card modal %u: %016llX expected %016llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_card_modal_cases[n].hash);
        if(hash!=DAY1_card_modal_cases[n].hash && getenv("PE_MODAL_DUMP")) {
            FILE *out=fopen("/tmp/pe-modal-native.bin","wb");
            if(out){for(unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
        }
        ASSERT(hash==DAY1_card_modal_cases[n].hash && result==DAY1_card_modal_cases[n].result,"card gate/modal state differs from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"card modal stopped");
    }
    PASS();
}

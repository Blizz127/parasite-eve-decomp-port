#include "retail_exit_menu_draw_cases.h"
static void test_DAY1_exit_menu_draw(void)
{
    TEST_RETAIL_DISC1("DAY1_exit_menu_draw"); TEST_RETAIL_FIXUPS(RETAILFIX_exit_menu_draw);
    for(unsigned n=0;n<sizeof(DAY1_exit_draw_predicates)/sizeof(DAY1_exit_draw_predicates[0]);n++) {
        ResetTestState();uint32_t index=DAY1_exit_draw_predicates[n][0];
        PE_StoreU8(0x800A0ED4u+index*0x418u,DAY1_exit_draw_predicates[n][1]);
        ASSERT((uint32_t)func_8004FDA4(index)==DAY1_exit_draw_predicates[n][2],"exit menu predicate differs from original");
    }
    for(unsigned n=0;n<sizeof(DAY1_exit_draw_cases)/sizeof(DAY1_exit_draw_cases[0]);n++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_exit_draw_common)/sizeof(DAY1_exit_draw_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_exit_draw_common[i][0],DAY1_exit_draw_common[i][1]);
        for(unsigned i=DAY1_exit_draw_cases[n].first;i<DAY1_exit_draw_cases[n].end;i++)
            PE_StoreU32(0x80000000u+DAY1_exit_draw_patches[i][0],DAY1_exit_draw_patches[i][1]);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);D_8009D048=PE_LoadU32(0x8009D048u);D_8009D050=PE_LoadU32(0x8009D050u);
        for(unsigned i=0;i<DAY1_exit_draw_cases[n].frame;i++)PE_GPU_VBlankStep();
        func_8004FDE8(DAY1_exit_draw_cases[n].node);
        uint64_t hash=hit_camera_hash(DAY1_exit_draw_ranges,sizeof(DAY1_exit_draw_ranges)/sizeof(DAY1_exit_draw_ranges[0]));
        if(hash!=DAY1_exit_draw_cases[n].hash)fprintf(stderr,"exit draw %u: %016llX expected %016llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_exit_draw_cases[n].hash);
        ASSERT(hash==DAY1_exit_draw_cases[n].hash,"exit menu packet/state differs from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"exit menu drawing stopped");
        ASSERT(PE_GPU_VSyncQuery()==DAY1_exit_draw_cases[n].frame,"drawing advanced VBlank");
    }
    PASS();
}

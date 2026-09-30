#include "retail_exit_menu_cases.h"
static void test_DAY1_exit_menu(void)
{
    TEST_RETAIL_DISC1("DAY1_exit_menu"); TEST_RETAIL_FIXUPS(RETAILFIX_exit_menu);
    for(unsigned n=0;n<sizeof(DAY1_exit_menu_cases)/sizeof(DAY1_exit_menu_cases[0]);n++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_exit_menu_common)/sizeof(DAY1_exit_menu_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_exit_menu_common[i][0],DAY1_exit_menu_common[i][1]);
        for(unsigned i=DAY1_exit_menu_cases[n].first;i<DAY1_exit_menu_cases[n].end;i++)
            PE_StoreU32(0x80000000u+DAY1_exit_menu_patches[i][0],DAY1_exit_menu_patches[i][1]);
        uint32_t result=func_80015AF0(0x80150100u);
        uint64_t hash=hit_camera_hash(DAY1_exit_menu_ranges,sizeof(DAY1_exit_menu_ranges)/sizeof(DAY1_exit_menu_ranges[0]));
        if(hash!=DAY1_exit_menu_cases[n].hash)fprintf(stderr,"exit menu case %u: %016llX expected %016llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_exit_menu_cases[n].hash);
        ASSERT(result==DAY1_exit_menu_cases[n].result && hash==DAY1_exit_menu_cases[n].hash,"E7 initial state differs from original");
        if(DAY1_exit_menu_cases[n].second!=0xFFFFFFFFu) {
            PE_StoreU32(0x8009CE00u,0x801910C8u);
            result=func_80015AF0(0x80150100u);
            hash=hit_camera_hash(DAY1_exit_menu_ranges,sizeof(DAY1_exit_menu_ranges)/sizeof(DAY1_exit_menu_ranges[0]));
            if(hash!=DAY1_exit_menu_cases[n].hash2)fprintf(stderr,"exit menu retry %u: %016llX expected %016llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_exit_menu_cases[n].hash2);
            ASSERT(result==DAY1_exit_menu_cases[n].second && hash==DAY1_exit_menu_cases[n].hash2,"E7 retry/constructor differs from original");
        }
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"E7 constructor stopped");
    }
    ResetTestState();
    const pe_addr_t actor=0x800BEA90u,task=0x8009D310u,stream=0x80122000u;
    PE_StoreU32(0x800910A0u+0xE7u*4,0x80015AF0u);PE_StoreU32(0x800910A4u,0x800172BCu);
    PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D300u,task);
    PE_StoreU32(task,stream);PE_StoreU32(task+16u,1);PE_StoreU32(actor+0x98u,0x100000E0u);
    PE_StoreU32(stream,0xE7u);PE_StoreU32(stream+8u,1);
    func_80017018();
    ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"E7 VM boundary remains");
    ASSERT(PE_LoadU32(task)==stream && (PE_LoadU16(task+8u)&0x20u),"E7 VM did not rewind/yield");
    /* A new scheduler pass republishes the actor/task after walk_next. */
    PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D300u,task);
    PE_StoreU32(0x800B0CD8u,0x1000u);
    func_80017018();
    ASSERT(PE_LoadU32(actor+0x98u)&0x10u,"E7 VM did not continue after bypass");
    ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"E7 VM continuation stopped");
    PASS();
}

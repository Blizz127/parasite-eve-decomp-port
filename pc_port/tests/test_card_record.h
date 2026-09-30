#include "retail_card_record_cases.h"
static void test_DAY1_card_record(void)
{
    TEST("DAY1_card_record");
    static const uint32_t ranges[][2]={{0xA0ED4,0xA00},{0x9D15C,8}};
    for(unsigned n=0;n<256;n++) {
        ResetTestState();
        for(unsigned i=0;i<0xA00;i++)PE_StoreU8(0x800A0ED4u+i,0xA5);
        unsigned index=n&1;pe_addr_t record=0x800A0ED4u+index*0x418u,other=0x800A0ED4u+(1-index)*0x418u;
        PE_StoreU8(record+1u,n);PE_StoreU8(other+1u,n*5);
        PE_StoreU32(0x8009D15Cu,0x80140000u+n*4);PE_StoreU32(0x8009D160u,0xDEADBEEFu);
        PE_StoreU32(0x800A1860u,n*0x1020304u);
        func_8004298C(index,n*2654435761u);
        ASSERT(func_80042AD8(index)==DAY1_card_record_cases[n].active,"record active predicate differs");
        ASSERT(func_800428C4()==DAY1_card_record_cases[n].selected,"record selection wrap differs");
        func_80042910();
        ASSERT(func_80042B28()==DAY1_card_record_cases[n].busy,"record busy getter differs");
        ASSERT(hit_camera_hash(ranges,2)==DAY1_card_record_cases[n].before,"record enqueue/selection/reset state differs");
        func_80042A10();
        ASSERT(hit_camera_hash(ranges,2)==DAY1_card_record_cases[n].after,"cleanup mutated state beyond original return/stop frontier");
        ASSERT((unsigned)PE_Port_ShouldStop()==DAY1_card_record_cases[n].stopped,"cleanup stop differs");
        ASSERT(g_bootstrap_arg4_call_count==(int)DAY1_card_record_cases[n].stopped,"cleanup continued to later close call");
    }
    PASS();
}

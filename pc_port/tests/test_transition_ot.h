#include "retail_transition_ot_cases.h"
static void test_DAY1_transition_ot(void)
{
    TEST("DAY1_transition_ot");
    for(unsigned k=0;k<sizeof(DAY1_transition_ot_cases)/sizeof(DAY1_transition_ot_cases[0]);k++) {
        ResetTestState();
        unsigned mode=DAY1_transition_ot_cases[k].mode,seed=DAY1_transition_ot_cases[k].seed;
        for(unsigned i=0;i<4096;i++) {
            int empty;
            switch(mode) {
            case 0:empty=0;break;case 1:empty=1;break;
            case 2:empty=(i%2==0);break;case 3:empty=(i%2==1);break;
            case 4:empty=i>seed*64;break;case 5:empty=i<seed*64;break;
            default:empty=((i*2654435761u+seed)&15u)!=0;break;
            }
            uint32_t value=empty?0x160000u+i*4u-4u:0x145000u+(i*28u)%4096u;
            if(!i)value=0xFFFFFF;
            PE_StoreU32(0x80160000u+i*4u,value);
        }
        pe_addr_t descriptor=mode==7?0x8015FFFCu:0x80150000u;
        PE_StoreU32(descriptor+4u,0x80160000u);PE_StoreU32(0x8019C9C0u,descriptor);
        uint32_t result=PE_TransitionCompactOT(DAY1_transition_ot_cases[k].initial);
        static const uint32_t ranges[][2]={{0x150000,0x14000},{0x19C9C0,4}};
        ASSERT(result==DAY1_transition_ot_cases[k].result,"OT empty-run state differs from original");
        ASSERT(hit_camera_hash(ranges,2)==DAY1_transition_ot_cases[k].hash,"OT compaction links differ from original");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"OT compaction stopped or hit stub");
    }
    PASS();
}

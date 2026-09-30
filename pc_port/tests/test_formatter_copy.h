#include "retail_formatter_copy_cases.h"
static void test_DAY1_formatter_copy(void)
{
    TEST("DAY1_formatter_copy");
    static const uint32_t ranges[][2]={{0x140000,512}};
    for(unsigned n=0;n<2048;n++) {
        ResetTestState();
        for(unsigned i=0;i<512;i++)PE_StoreU8(0x80140000u+i,(uint8_t)(i*73u+19u));
        uint32_t result=func_80072334(DAY1_formatter_copy_cases[n].dst,DAY1_formatter_copy_cases[n].src,DAY1_formatter_copy_cases[n].count);
        ASSERT(result==DAY1_formatter_copy_cases[n].result,"formatter copy return differs");
        ASSERT(hit_camera_hash(ranges,1)==DAY1_formatter_copy_cases[n].hash,"formatter copy alias/write order differs");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"formatter copy hit boundary");
    }
    PASS();
}

#include "retail_formatter_frame_cases.h"
static void test_DAY1_formatter_frame(void)
{
    TEST_RETAIL_DISC1("DAY1_formatter_frame"); TEST_RETAIL_FIXUPS(RETAILFIX_formatter_frame);
    for(unsigned n=0;n<sizeof(DAY1_formatter_frame_cases)/sizeof(DAY1_formatter_frame_cases[0]);n++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_formatter_frame_common)/sizeof(DAY1_formatter_frame_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_formatter_frame_common[i][0],DAY1_formatter_frame_common[i][1]);
        for(unsigned i=0;i<=strlen(DAY1_formatter_frame_cases[n].format);i++)
            PE_StoreU8(0x80140000u+i,DAY1_formatter_frame_cases[n].format[i]);
        const uint8_t data[]={3,'a','b','c','X','Y','Z',0};
        for(unsigned i=0;i<sizeof(data);i++)PE_StoreU8(0x80142000u+i,data[i]);
        uint32_t saved[9]={0};for(unsigned i=0;i<8;i++)saved[i]=0x13570000u+i*0x1111u;
        for(unsigned i=0;i<3;i++)PE_StoreU32(0x80094528u+i*4u,DAY1_formatter_frame_cases[n].defaults[i]);
        PE_StoreU32(DAY1_formatter_frame_cases[n].stack+16u,DAY1_formatter_frame_cases[n].args[2]);
        uint32_t result=PE_FormatterFrame(DAY1_formatter_frame_cases[n].destination,0x80140000u,
            DAY1_formatter_frame_cases[n].args[0],DAY1_formatter_frame_cases[n].args[1],DAY1_formatter_frame_cases[n].stack,saved);
        uint64_t hash=hit_camera_hash(DAY1_formatter_frame_ranges,2);
        if(hash!=DAY1_formatter_frame_cases[n].hash)fprintf(stderr,"formatter frame %u (%s): %016llX expected %016llX\n",n,DAY1_formatter_frame_cases[n].format,(unsigned long long)hash,(unsigned long long)DAY1_formatter_frame_cases[n].hash);
        ASSERT(hash==DAY1_formatter_frame_cases[n].hash,"formatter guest output/frame differs from original");
        if(DAY1_formatter_frame_cases[n].target) {
            ASSERT(PE_Port_ShouldStop() && g_bootstrap_arg4_call_count==1,"formatter BIOS boundary missing");
            ASSERT(g_bootstrap_arg4_calls[0].target==DAY1_formatter_frame_cases[n].target,"formatter BIOS target differs");
            ASSERT(g_bootstrap_arg4_calls[0].arg0==DAY1_formatter_frame_cases[n].boundary[0],"formatter BIOS pointer differs");
            if(DAY1_formatter_frame_cases[n].target==0x80072324u)ASSERT(g_bootstrap_arg4_calls[0].arg1==0u && g_bootstrap_arg4_calls[0].arg2==DAY1_formatter_frame_cases[n].boundary[2],"formatter BIOS precision differs");
        } else {
            ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"formatter stopped on returning original path");
            ASSERT(result==DAY1_formatter_frame_cases[n].result,"formatter count differs from original");
        }
    }
    PASS();
}

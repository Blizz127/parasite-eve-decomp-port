#include "retail_card_operation_frame_cases.h"
static void test_DAY1_card_operation_frame(void)
{
    TEST_RETAIL_DISC1("DAY1_card_operation_frame"); TEST_RETAIL_FIXUPS(RETAILFIX_card_operation_frame);
    for(unsigned n=0;n<1280;n++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_card_operation_frame_common)/sizeof(DAY1_card_operation_frame_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_card_operation_frame_common[i][0],DAY1_card_operation_frame_common[i][1]);
        for(unsigned i=DAY1_card_operation_frame_cases[n].first;i<DAY1_card_operation_frame_cases[n].end;i++)
            PE_StoreU32(0x80000000u+DAY1_card_operation_frame_patches[i][0],DAY1_card_operation_frame_patches[i][1]);
        uint32_t incoming[32]={0};for(unsigned i=0;i<8;i++)incoming[16+i]=0x13570000u+i*0x1111u;
        incoming[7]=0x13579BDFu;incoming[29]=DAY1_card_operation_frame_cases[n].stack;
        if(DAY1_card_operation_frame_cases[n].entry==0x80040F80u)
            PE_CardCleanupFrame(DAY1_card_operation_frame_cases[n].argument,incoming[29],incoming);
        else PE_CardOperationFrame(DAY1_card_operation_frame_cases[n].argument,incoming[29],incoming);
        /* Observable region only: the fixed record / buffer ranges plus the caller-visible words at and above the dispatcher's sp; the private frames
         * below it (dispatcher and formatter spills, stack residue) are not retail-observable behaviour. */
        uint32_t observable[sizeof(DAY1_card_operation_frame_ranges)/sizeof(DAY1_card_operation_frame_ranges[0])+1][2];
        const unsigned fixed=sizeof(DAY1_card_operation_frame_ranges)/sizeof(DAY1_card_operation_frame_ranges[0]);
        for(unsigned i=0;i<fixed;i++){observable[i][0]=DAY1_card_operation_frame_ranges[i][0];observable[i][1]=DAY1_card_operation_frame_ranges[i][1];}
        observable[fixed][0]=DAY1_card_operation_frame_cases[n].stack&0x1FFFFFu;observable[fixed][1]=0x40u;
        uint64_t hash=hit_camera_hash(observable,fixed+1u);
        if(hash!=DAY1_card_operation_frame_cases[n].hash)fprintf(stderr,"card frame %u: %016llX expected %016llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_card_operation_frame_cases[n].hash);
        ASSERT(hash==DAY1_card_operation_frame_cases[n].hash,"card formatter/frame effects differ from original");
        if(DAY1_card_operation_frame_cases[n].target==0x80072314u) {
            ASSERT(PE_Port_ShouldStop() && g_bootstrap_arg4_call_count==1,"cleanup string BIOS stop missing");
            ASSERT(g_bootstrap_arg4_calls[0].target==0x80072314u && g_bootstrap_arg4_calls[0].arg0==DAY1_card_operation_frame_cases[n].args[0],"cleanup string BIOS pointer differs");
        } else ASSERT(card_operation_boundary_matches(DAY1_card_operation_frame_cases[n].target,DAY1_card_operation_frame_cases[n].mask,DAY1_card_operation_frame_cases[n].args,0u),"card post-format boundary differs from original");
    }
    PASS();
}

#include "retail_exit_menu_input_cases.h"
static void test_DAY1_exit_menu_input(void)
{
    TEST_RETAIL_DISC1("DAY1_exit_menu_input"); TEST_RETAIL_FIXUPS(RETAILFIX_exit_menu_input);
    for(unsigned n=0;n<328;n++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_exit_input_common)/sizeof(DAY1_exit_input_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_exit_input_common[i][0],DAY1_exit_input_common[i][1]);
        for(unsigned i=DAY1_exit_input_cases[n].first;i<DAY1_exit_input_cases[n].end;i++)
            PE_StoreU32(0x80000000u+DAY1_exit_input_patches[i][0],DAY1_exit_input_patches[i][1]);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        int result=n<320?func_8004D2DC(DAY1_exit_input_cases[n].window,DAY1_exit_input_cases[n].event):func_8004D030(DAY1_exit_input_cases[n].window,DAY1_exit_input_cases[n].event);
        uint64_t hash=hit_camera_hash(DAY1_exit_input_ranges,sizeof(DAY1_exit_input_ranges)/sizeof(DAY1_exit_input_ranges[0]));
        if(hash!=DAY1_exit_input_cases[n].hash)fprintf(stderr,"exit input %u: %016llX expected %016llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_exit_input_cases[n].hash);
        if(hash!=DAY1_exit_input_cases[n].hash && getenv("PE_EXIT_INPUT_DUMP")) {
            FILE *out=fopen("/tmp/pe-exit-input-native.bin","wb");
            if(out){for(unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
        }
        ASSERT(hash==DAY1_exit_input_cases[n].hash,"exit input/notice state differs from original");
        ASSERT((unsigned)PE_Port_ShouldStop()==DAY1_exit_input_cases[n].stopped,"input I/O stop differs");
        if(!DAY1_exit_input_cases[n].stopped)ASSERT(result==1 && !g_stub_order_count,"returning input graph hit boundary");
        else ASSERT(g_bootstrap_arg4_call_count==1 && g_bootstrap_arg4_calls[0].target==0x80072774u,"input stopped at wrong call");
        if(n==0) {
            /* Exercise the actual callback dispatcher through a queued event. */
            PE_StoreU32(0x8009D15Cu,DAY1_exit_input_cases[n].window);PE_StoreU32(0x8009D0ECu,1);
            PE_StoreU32(0x8009D0E0u,0x80158000u);PE_StoreU32(0x8009D0E4u,0x80158000u);
            PE_StoreU32(0x80158000u,0);PE_StoreU32(0x80158004u,1);PE_StoreU32(0x80158008u,0);
            func_8005E30C();
            ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"exit input dispatch still unresolved");
            ASSERT(PE_LoadU32(0x8009D0E0u)==0,"queued input was not consumed");
        }
    }
    PASS();
}

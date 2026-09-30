#include "retail_script_turn_cases.h"
static void test_DAY1_script_turn(void)
{
    TEST("DAY1_script_turn");
    for(unsigned k=0;k<sizeof(DAY1_turn_cases)/sizeof(DAY1_turn_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_turn_common)/sizeof(DAY1_turn_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_turn_common[i][0],DAY1_turn_common[i][1]);
        for(unsigned i=DAY1_turn_cases[k].first;i<DAY1_turn_cases[k].end;i++)
            PE_StoreU32(0x80000000u+DAY1_turn_patches[i][0],DAY1_turn_patches[i][1]);
        D_8009D1A0=0;
        int result=0;
        for(unsigned frame=0;frame<DAY1_turn_cases[k].frames;frame++) {
            PE_StoreU32(0x8009D300u,0x80140100u);
            if(DAY1_turn_cases[k].vm)func_80017018();
            else result=func_80014BA0(0x80140000u);
        }
        uint64_t hash=hit_camera_hash(DAY1_turn_ranges,sizeof(DAY1_turn_ranges)/sizeof(DAY1_turn_ranges[0]));
        if(hash!=DAY1_turn_cases[k].hash || (unsigned)result!=DAY1_turn_cases[k].result) {
            fprintf(stderr,"turn case%u result%d/%u hash%016llX/%016llX\n",k,result,
                DAY1_turn_cases[k].result,(unsigned long long)hash,(unsigned long long)DAY1_turn_cases[k].hash);
            if(getenv("PE_DAY1_TURN_DUMP")) {
                FILE *out=fopen("local/live/script-turn-native-mismatch.bin","wb");
                if(out){for(unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT((unsigned)result==DAY1_turn_cases[k].result,"turn return differs from original");
        ASSERT(hash==DAY1_turn_cases[k].hash,"turn memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"turn executes natively");
    }
    PASS();
}

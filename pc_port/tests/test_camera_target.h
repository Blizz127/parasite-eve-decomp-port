#include "retail_camera_target_cases.h"
static void test_DAY1_camera_target(void)
{
    TEST("DAY1_camera_target");
    static const uint32_t shifts[]={0,1,15,16,31,32,255,0x10001};
    static const uint32_t ranges[][2]={{0x19C020u,0xA0},{0x19C330u,16},{0x19C810u,16},{0x150000u,32}};
    for(unsigned k=0;k<sizeof(DAY1_target_cases)/sizeof(DAY1_target_cases[0]);k++){
        unsigned seed=DAY1_target_cases[k].seed;
        if(!DAY1_target_cases[k].step){
            ResetTestState();
            for(unsigned i=0;i<4;i++)for(unsigned j=0;j<ranges[i][1];j++)PE_StoreU8(0x80000000u+ranges[i][0]+j,(uint8_t)(seed*31+j*29));
        }
        func_80195BC8(DAY1_target_cases[k].target,DAY1_target_cases[k].eye,shifts[seed],shifts[7-seed]);
        uint64_t hash=hit_camera_hash(ranges,4);
        if(hash!=DAY1_target_cases[k].hash)fprintf(stderr,"camera target case%u %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)DAY1_target_cases[k].hash);
        ASSERT(hash==DAY1_target_cases[k].hash && !PE_Port_ShouldStop(),"direct camera target differs from original");
    }
    PASS();
}

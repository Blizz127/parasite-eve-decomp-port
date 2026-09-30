#include "retail_script_effect_spawn_cases.h"
static void test_DAY2_script_effect_spawn(void)
{
    TEST("DAY2_script_effect_spawn");
    for(unsigned k=0;k<sizeof(ESP_cases)/sizeof(ESP_cases[0]);k++) {
        uint64_t color,spawn;
        ResetTestState();
        for(unsigned i=0;i<sizeof(ESP_common)/sizeof(ESP_common[0]);i++)PE_StoreU32(0x80000000u+ESP_common[i][0],ESP_common[i][1]);
        for(unsigned i=ESP_cases[k].first;i<ESP_cases[k].end;i++)PE_StoreU32(0x80000000u+ESP_patches[i][0],ESP_patches[i][1]);
        ASSERT(func_80016910_key2900_cut(0x80154000u)==1,"effect color setup completes");
        color=hit_camera_hash(ESP_ranges,sizeof(ESP_ranges)/sizeof(ESP_ranges[0]));
        ASSERT(color==ESP_cases[k].color,"effect color setup differs from original");
        for(unsigned i=0;i<6;i++)PE_StoreU32(0x80154040u+i*4u,PE_LoadU32(0x80154080u+i*4u));
        ASSERT(func_80016910_key2900_cut(0x80154000u)==1,"effect allocation completes");
        spawn=hit_camera_hash(ESP_ranges,sizeof(ESP_ranges)/sizeof(ESP_ranges[0]));
        if(spawn!=ESP_cases[k].spawn)fprintf(stderr,"effect spawn%u %016llX/%016llX\n",k,(unsigned long long)spawn,(unsigned long long)ESP_cases[k].spawn);
        ASSERT(spawn==ESP_cases[k].spawn,"effect allocation differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"effect allocation completes natively");
    }
    PASS();
}

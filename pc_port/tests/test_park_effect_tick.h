#include "retail_park_effect_tick_cases.h"
static void test_DAY2_park_effect_tick(void)
{
    TEST("DAY2_park_effect_tick");
    for(unsigned k=0;k<sizeof(PET_cases)/sizeof(PET_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<0x400;i++)PE_StoreU8(0x1F800000u+i,0);
        for(unsigned i=0;i<sizeof(PET_common)/sizeof(PET_common[0]);i++)PE_StoreU32(0x80000000u+PET_common[i][0],PET_common[i][1]);
        for(unsigned i=PET_cases[k].first;i<PET_cases[k].end;i++)PE_StoreU32(0x80000000u+PET_patches[i][0],PET_patches[i][1]);
        g_pe_gte.ofx=160<<16;g_pe_gte.ofy=112<<16;g_pe_gte.h=256;
        for(unsigned frame=0;frame<8;frame++) {
            func_800E01BC();
            uint64_t h=hit_camera_hash(PET_ranges,sizeof(PET_ranges)/sizeof(PET_ranges[0]));
            for(unsigned i=0;i<0x38;i++){h^=PE_LoadU8(0x1F800000u+i);h*=UINT64_C(1099511628211);}
            if(h!=PET_cases[k].hash[frame])fprintf(stderr,"park effect %u frame%u %016llX/%016llX\n",k,frame,(unsigned long long)h,(unsigned long long)PET_cases[k].hash[frame]);
            ASSERT(h==PET_cases[k].hash[frame],"park effect update/packet differs from original");
            ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"park effect graph completes natively");
        }
    }
    PASS();
}

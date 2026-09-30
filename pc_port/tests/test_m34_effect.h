#include "retail_m34_effect_cases.h"

static void test_DAY1_m34_effect_core(void)
{
    TEST("DAY1_m34_effect_core");
    for(unsigned k=0;k<sizeof(m34_effect_cases)/sizeof(m34_effect_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(m34_effect_common)/sizeof(m34_effect_common[0]);i++)
            PE_StoreU32(0x80000000u+m34_effect_common[i][0],m34_effect_common[i][1]);
        for(unsigned i=m34_effect_cases[k].first;i<m34_effect_cases[k].end;i++)
            PE_StoreU32(0x80000000u+m34_effect_patches[i][0],m34_effect_patches[i][1]);
        const uint32_t *a=m34_effect_cases[k].args;int result=0;
        switch(m34_effect_cases[k].fn) {
        case 0x8018F00C:result=PE_M34BossEffectInit(a[0]);break;
        case 0x8018F0B8:result=PE_M34BossEffectCommand(a[0],a[1],a[2],a[3],0,0);break;
        case 0x8018F0E4:result=PE_M34BossEffectDraw(a[0]);break;
        case 0x8018F12C:result=PE_M34BossEffectUpdate(a[0]);break;
        case 0x8018F1B8:result=PE_M34BossEffectCleanup(a[0]);break;
        case 0x8006F39C:result=func_8006F39C(a[0],a[1]);break;
        default:ASSERT(PE_M34BossEffectChild(m34_effect_cases[k].fn,a[0],a[1],a[2]),"child entry translated");
        }
        uint64_t hash=hit_camera_hash(m34_effect_ranges,sizeof(m34_effect_ranges)/sizeof(m34_effect_ranges[0]));
        if(hash!=m34_effect_cases[k].hash)
            fprintf(stderr,"M34 core case%u hash%016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)m34_effect_cases[k].hash);
        ASSERT(hash==m34_effect_cases[k].hash,"effect storage and flags match original");
        ASSERT(result==m34_effect_cases[k].result,"effect return matches original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"M34 core call executes natively");
    }
    ResetTestState();PASS();
}

#include "retail_m28_movement_cases.h"
static void test_DAY1_m28_movement(void)
{
    TEST_RETAIL_DISC1("DAY1_m28_movement"); TEST_RETAIL_FIXUPS(RETAILFIX_m28_movement);
    for(unsigned k=0;k<sizeof(m28_movement_cases)/sizeof(m28_movement_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(m28_movement_common)/sizeof(m28_movement_common[0]);i++)
            PE_StoreU32(0x80000000u+m28_movement_common[i][0],m28_movement_common[i][1]);
        for(unsigned i=m28_movement_cases[k].first;i<m28_movement_cases[k].end;i++)
            PE_StoreU32(0x80000000u+m28_movement_patches[i][0],m28_movement_patches[i][1]);
        const uint32_t *a=m28_movement_cases[k].args;
        uint32_t result=0;
        switch(m28_movement_cases[k].kind) {
        case 0:result=PE_M28MovementInit(a[0]);break;
        case 1:result=PE_M28MovementCommand(a[0],a[1],a[2],a[3],a[4],a[5]);break;
        case 2:result=PE_M28MovementUpdate(a[0]);break;
        case 3:result=PE_M28MovementCleanup(a[0]);break;
        case 4:result=func_8006F39C(a[0],a[1]);break;
        case 5:result=func_8006F6D4(a[0],a[1],a[2],a[3],a[4],a[5]);break;
        case 6:result=func_80069660();break;
        case 7:func_80017018();break;
        case 8:PE_EffectCallback_SetExtra1(a[3]);result=PE_EffectCallback(0x80193148u,a[0],a[1],a[2]);break;
        case 9:result=func_8006FC18(a[0],a[1],a[2]);break;
        }
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<sizeof(m28_movement_ranges)/sizeof(m28_movement_ranges[0]);i++)
            for(unsigned j=0;j<m28_movement_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(0x80000000u+m28_movement_ranges[i][0]+j))*UINT64_C(1099511628211);
        if(hash!=m28_movement_cases[k].hash)fprintf(stderr,"m28 movement case%u hash%016llX/%016llX\n",k,
            (unsigned long long)hash,(unsigned long long)m28_movement_cases[k].hash);
        ASSERT(hash==m28_movement_cases[k].hash,"original movement, allocation, query and pump RAM");
        ASSERT(result==m28_movement_cases[k].result,"original handler result");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"movement graph executes natively");
    }
    ResetTestState();PASS();
}

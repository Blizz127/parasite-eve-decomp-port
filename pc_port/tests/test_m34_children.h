#include "retail_m34_children_cases.h"

static void test_DAY1_m34_projectile_movement_and_drawing(void)
{
    TEST("DAY1_m34_projectile_movement_and_drawing");
    for(unsigned k=0;k<sizeof(m34_children_cases)/sizeof(m34_children_cases[0]);k++) {
        ResetTestState();
        const uint32_t (*common)[3];unsigned count;
        switch(m34_children_cases[k].family) {
        case 0:common=m34_children_common0;count=sizeof(m34_children_common0)/sizeof(common[0]);break;
        case 1:common=m34_children_common1;count=sizeof(m34_children_common1)/sizeof(common[0]);break;
        default:common=m34_children_common2;count=sizeof(m34_children_common2)/sizeof(common[0]);break;
        }
        for(unsigned i=0;i<count;i++)for(unsigned j=0;j<common[i][1];j++)
            PE_StoreU32(0x80000000u+common[i][0]+j*4u,common[i][2]);
        for(unsigned i=m34_children_cases[k].first;i<m34_children_cases[k].end;i++)
            PE_StoreU32(0x80000000u+m34_children_patches[i][0],m34_children_patches[i][1]);
        g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;g_pe_gte.h=256;g_pe_gte.zsf3=0x155;
        ASSERT(PE_M34BossEffectChild(m34_children_cases[k].fn,0x80150000u,0x80150080u,0x80150200u),"M34 child translated");
        uint64_t hash=hit_camera_hash(m34_children_ranges,sizeof(m34_children_ranges)/sizeof(m34_children_ranges[0]));
        for(unsigned i=0;i<1024;i++)hash=(hash^PE_LoadU8(0x1F800000u+i))*UINT64_C(1099511628211);
        if(hash!=m34_children_cases[k].hash)
            fprintf(stderr,"M34 children case%u fn%08X hash%016llX/%016llX\n",k,m34_children_cases[k].fn,
                    (unsigned long long)hash,(unsigned long long)m34_children_cases[k].hash);
        ASSERT(hash==m34_children_cases[k].hash,"movement, trails, collision flags, GPU packets and scratch match original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"M34 child graph executes natively");
    }
    ResetTestState();PASS();
}

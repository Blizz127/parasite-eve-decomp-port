#include "retail_transition_visibility_cases.h"
static void test_DAY1_transition_visibility(void)
{
    TEST("DAY1_transition_visibility");
    static const uint32_t ranges[][2]={{0x140000,16},{0x19CA90,0x190}};
    for(unsigned k=0;k<sizeof(DAY1_visibility_cases)/sizeof(DAY1_visibility_cases[0]);k++) {
        ResetTestState();
        for(unsigned j=0;j<2;j++)for(unsigned i=0;i<ranges[j][1];i++)PE_StoreU8(0x80000000u+ranges[j][0]+i,(uint8_t)(i*17+5));
        for(unsigned i=0;i<24;i++)PE_StoreU32(0x80000000u+DAY1_visibility_fields[i],DAY1_visibility_cases[k].fields[i]);
        unsigned kind=DAY1_visibility_cases[k].kind,alias=DAY1_visibility_cases[k].alias;
        pe_addr_t point=alias==0?0x80140000u:alias==1?0x8019CBB0u:0x8019CB48u;
        for(unsigned i=0;i<3;i++) {
            if(kind==1)PE_StoreU16(point+i*2,(uint16_t)DAY1_visibility_cases[k].point[i]);
            else PE_StoreU32(point+i*4,DAY1_visibility_cases[k].point[i]);
        }
        uint32_t radius=DAY1_visibility_cases[k].radius;
        int ret=kind==0?func_8018FFF4(point):kind==1?func_80190124(point):kind==2?func_80190254(point,radius):func_801904B0(point,radius);
        if(!DAY1_visibility_cases[k].trap && (uint32_t)ret!=DAY1_visibility_cases[k].ret)fprintf(stderr,"visibility%u kind%u ret%d/%u\n",k,kind,ret,DAY1_visibility_cases[k].ret);
        ASSERT(!!PE_Port_ShouldStop()==!!DAY1_visibility_cases[k].trap,"visibility DIV/BREAK boundary differs");
        if(!DAY1_visibility_cases[k].trap)ASSERT((uint32_t)ret==DAY1_visibility_cases[k].ret,"visibility result differs from original");
        ASSERT(hit_camera_hash(ranges,2)==DAY1_visibility_cases[k].hash,"visibility mutated persistent RAM");
        ASSERT(!g_stub_order_count,"visibility used stub");
    }
    PASS();
}

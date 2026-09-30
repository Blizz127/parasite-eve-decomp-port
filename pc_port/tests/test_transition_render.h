#include "retail_transition_render_cases.h"
static void test_DAY1_transition_render(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_render"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_view);
    for(unsigned k=0;k<sizeof(DAY1_transition_render_cases)/sizeof(DAY1_transition_render_cases[0]);k++) {
        ResetTestState();memset(&g_pe_gte,0,sizeof(g_pe_gte));
        for(unsigned j=0;j<sizeof(DAY1_view_tables);j++)PE_StoreU8(0x80095F00u+j,DAY1_view_tables[j]);
        uint32_t seed=DAY1_transition_render_cases[k].seed;
        for(unsigned j=0;j<sizeof(DAY1_transition_render_ranges)/sizeof(DAY1_transition_render_ranges[0]);j++) {
            uint32_t a=DAY1_transition_render_ranges[j][0],length=DAY1_transition_render_ranges[j][1];
            for(unsigned i=0;i<length;i+=4)PE_StoreU32((a?0x80000000u+a:0x1F800000u)+i,
                (a==0 || a==0x160000u || a==0x170000u)?(a+i)*2654435761u+seed:0);
        }
        for(unsigned j=0;j<sizeof(DAY1_transition_render_init)/sizeof(DAY1_transition_render_init[0]);j++)
            PE_StoreU32(0x80000000u+DAY1_transition_render_init[j][0],DAY1_transition_render_init[j][1]);
        for(unsigned j=0;j<DAY1_transition_render_cases[k].patch_count;j++)
            PE_StoreU32(0x80000000u+DAY1_transition_render_cases[k].patches[j][0],DAY1_transition_render_cases[k].patches[j][1]);
        g_pe_gte.ofx=160<<16;g_pe_gte.ofy=120<<16;g_pe_gte.h=1000;
        g_pe_gte.dqa=-4194;g_pe_gte.dqb=0x1400000;g_pe_gte.zsf3=341;g_pe_gte.zsf4=256;
        func_80071A64(seed);func_80192800();
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned j=0;j<sizeof(DAY1_transition_render_ranges)/sizeof(DAY1_transition_render_ranges[0]);j++) {
            uint32_t a=DAY1_transition_render_ranges[j][0],length=DAY1_transition_render_ranges[j][1];
            for(unsigned i=0;i<length;i++)hash=(hash^PE_LoadU8((a?0x80000000u+a:0x1F800000u)+i))*UINT64_C(1099511628211);
        }
        uint32_t state[28]={g_pe_gte.otz,(uint32_t)g_pe_gte.ir0,(uint32_t)g_pe_gte.ir[0],(uint32_t)g_pe_gte.ir[1],(uint32_t)g_pe_gte.ir[2],
            g_pe_gte.sxy[0],g_pe_gte.sxy[1],g_pe_gte.sxy[2],g_pe_gte.sz[0],g_pe_gte.sz[1],g_pe_gte.sz[2],g_pe_gte.sz[3],
            (uint32_t)g_pe_gte.mac0,(uint32_t)g_pe_gte.mac[0],(uint32_t)g_pe_gte.mac[1],(uint32_t)g_pe_gte.mac[2]};
        for(unsigned j=0;j<9;j++)state[16+j]=(uint32_t)(int32_t)g_pe_gte.rt[j/3][j%3];
        for(unsigned j=0;j<3;j++)state[25+j]=(uint32_t)g_pe_gte.tr[j];
        if(hash!=DAY1_transition_render_cases[k].hash)fprintf(stderr,"transition renderer %u hash%llX/%llX\n",k,(unsigned long long)hash,(unsigned long long)DAY1_transition_render_cases[k].hash);
        ASSERT(hash==DAY1_transition_render_cases[k].hash,"transition renderer packets/objects/globals differ from original");
        ASSERT(!memcmp(state,DAY1_transition_render_cases[k].state,sizeof(state)),"transition renderer GTE differs from original");
        ASSERT(func_80071A54()==DAY1_transition_render_cases[k].next_rand,"transition renderer RNG advancement differs");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"transition renderer stopped or hit stub");
    }
    PASS();
}

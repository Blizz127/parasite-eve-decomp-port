#include "retail_transition_view_cases.h"
static void test_DAY1_transition_view(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_view"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_view);
    static const uint32_t ranges[][2]={{0x140000,256},{0x19BFC4,8},{0x19C330,12},{0x19C810,12},{0x19CC30,32},{0x19CDF0,32}};
    for(unsigned k=0;k<sizeof(DAY1_view_cases)/sizeof(DAY1_view_cases[0]);k++) {
        ResetTestState();
        memset(&g_pe_gte,0,sizeof(g_pe_gte));
        for(unsigned i=0;i<sizeof(DAY1_view_tables);i++)PE_StoreU8(0x80095F00u+i,DAY1_view_tables[i]);
        for(unsigned i=0;i<sizeof(DAY1_view_template);i++)PE_StoreU8(0x8018EFF4u+i,DAY1_view_template[i]);
        for(unsigned j=0;j<6;j++)for(unsigned i=0;i<ranges[j][1];i++)PE_StoreU8(0x80000000u+ranges[j][0]+i,(uint8_t)(DAY1_view_cases[k].seed+i*17));
        for(unsigned i=0;i<128;i++)PE_StoreU8(0x1F800280u+i,(uint8_t)(i*13+7));
        const uint32_t *v=DAY1_view_cases[k].input;
        unsigned kind=DAY1_view_cases[k].kind;
        if(!kind)for(unsigned i=0;i<3;i++) {PE_StoreU32(0x8019C330u+i*4,v[i]);PE_StoreU32(0x8019C810u+i*4,v[3+i]);}
        else if(kind==1)for(unsigned i=0;i<6;i++)PE_StoreU32(0x80140000u+(i<3?20+i*4:40+i*4),v[i]);
        else for(unsigned i=0;i<3;i++)PE_StoreU16(0x80140000u+i*2,(uint16_t)v[i]);
        if(kind==1 && DAY1_view_cases[k].seed>=128)for(unsigned j=0;j<2;j++)for(unsigned i=0;i<9;i++)PE_StoreU16(0x80140000u+j*32+i*2,(uint16_t)(i%4==0?4096:0));
        uint32_t ret=0,state[18]={0},offset=DAY1_view_cases[k].offset;
        if(!kind)func_8018F05C();
        else if(kind==1)ret=func_800787D4(0x80140000u,0x80140020u,0x80140000u+offset);
        else ret=func_800799E4(0x80140000u,0x80140000u+offset);
        uint64_t h=hit_camera_hash(ranges,6);
        if(h!=DAY1_view_cases[k].hash)fprintf(stderr,"view%u kind%u hash %016llX/%016llX\n",k,kind,(unsigned long long)h,(unsigned long long)DAY1_view_cases[k].hash);
        ASSERT(h==DAY1_view_cases[k].hash,"view RAM differs from original graph");
        ASSERT(!!PE_Port_ShouldStop()==!!DAY1_view_cases[k].trap,"view overflow boundary differs");
        for(unsigned i=0;i<128;i++)ASSERT(PE_LoadU8(0x1F800280u+i)==(uint8_t)(i*13+7),"view scratch leaked");
        if(!DAY1_view_cases[k].trap) {
            if(kind)ASSERT(ret==DAY1_view_cases[k].ret,"SDK matrix return differs");
            for(unsigned i=0;i<9;i++)state[i/2]|=(uint32_t)(uint16_t)g_pe_gte.rt[i/3][i%3]<<((i%2)*16);
            for(unsigned i=0;i<3;i++)state[5+i]=(uint32_t)g_pe_gte.tr[i];
            state[8]=(uint16_t)g_pe_gte.ir0;
            for(unsigned i=0;i<3;i++) {state[9+i]=(uint16_t)g_pe_gte.ir[i];state[12+i]=g_pe_gte.rgb_fifo[i];state[15+i]=(uint32_t)g_pe_gte.mac[i];}
            for(unsigned i=0;i<18;i++) {
                if(state[i]!=DAY1_view_cases[k].state[i])fprintf(stderr,"view%u GTE%u %08X/%08X\n",k,i,state[i],DAY1_view_cases[k].state[i]);
                ASSERT(state[i]==DAY1_view_cases[k].state[i],"view terminal GTE state differs");
            }
        }
        ASSERT(!g_stub_order_count,"view graph has a stub");
    }
    PASS();
}

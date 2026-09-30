#include "retail_transition_basis_cases.h"
static void test_DAY1_transition_basis(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_basis"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_basis);
    for(unsigned k=0;k<sizeof(DAY1_basis_cases)/sizeof(DAY1_basis_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_basis_table);i++)PE_StoreU8(0x800961D0u+i,DAY1_basis_table[i]);
        for(unsigned i=0;i<256;i++)PE_StoreU8(0x80140000u+i,(uint8_t)(DAY1_basis_cases[k].seed+i*17));
        const uint32_t *v=DAY1_basis_cases[k].vectors;
        if(DAY1_basis_cases[k].normal)for(unsigned i=0;i<3;i++)PE_StoreU32(0x80140000u+i*4,v[i]);
        else for(unsigned i=0;i<3;i++) {
            PE_StoreU16(0x80140000u+i*2,(uint16_t)v[i]);
            PE_StoreU16(0x80140010u+i*2,(uint16_t)v[3+i]);
            PE_StoreU32(0x80140020u+i*4,v[6+i]);
        }
        uint32_t ret=0,state[18]={0};
        if(DAY1_basis_cases[k].normal)ret=func_80078134(0x80140000u,0x80140000u+DAY1_basis_cases[k].offset);
        else func_8018F344(0x80140000u+DAY1_basis_cases[k].offset,0x80140000u,0x80140010u,0x80140020u);
        uint64_t h=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<256;i++)h=(h^PE_LoadU8(0x80140000u+i))*UINT64_C(1099511628211);
        if(h!=DAY1_basis_cases[k].hash)fprintf(stderr,"basis%u hash %016llX/%016llX\n",k,(unsigned long long)h,(unsigned long long)DAY1_basis_cases[k].hash);
        ASSERT(h==DAY1_basis_cases[k].hash,"basis RAM differs from original graph");
        ASSERT(!!PE_Port_ShouldStop()==!!DAY1_basis_cases[k].trap,"trapping ADD boundary differs");
        if(!DAY1_basis_cases[k].trap) {
            if(DAY1_basis_cases[k].normal)ASSERT(ret==DAY1_basis_cases[k].ret,"normalizer return differs");
            for(unsigned i=0;i<9;i++)state[i/2]|=(uint32_t)(uint16_t)g_pe_gte.rt[i/3][i%3]<<((i%2)*16);
            for(unsigned i=0;i<3;i++)state[5+i]=(uint32_t)g_pe_gte.tr[i];
            state[8]=(uint16_t)g_pe_gte.ir0;
            for(unsigned i=0;i<3;i++) {
                state[9+i]=(uint16_t)g_pe_gte.ir[i];state[12+i]=g_pe_gte.rgb_fifo[i];state[15+i]=(uint32_t)g_pe_gte.mac[i];
            }
            for(unsigned i=0;i<18;i++) {
                if(state[i]!=DAY1_basis_cases[k].state[i])fprintf(stderr,"basis%u GTE%u %08X/%08X\n",k,i,state[i],DAY1_basis_cases[k].state[i]);
                ASSERT(state[i]==DAY1_basis_cases[k].state[i],"basis terminal GTE state differs");
            }
        }
        ASSERT(!g_stub_order_count,"basis graph contains a stub");
    }
    PASS();
}

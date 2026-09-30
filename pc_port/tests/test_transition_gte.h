#include "retail_transition_gte_cases.h"

static void test_DAY1_transition_gte(void)
{
    TEST("DAY1_transition_gte");
    for (unsigned k=0;k<sizeof(DAY1_tgte_cases)/sizeof(DAY1_tgte_cases[0]);k++) {
        uint32_t got[18]={0};
        const uint32_t *a=DAY1_tgte_cases[k].args;
        ResetTestState();
        for (unsigned i=0;i<32u;i++) PE_StoreU8(0x80140000u+i,(uint8_t)(DAY1_tgte_cases[k].seed+i*17u));
        for (unsigned i=0;i<9u;i++) {
            g_pe_gte.llm[i/3u][i%3u]=(int16_t)(DAY1_tgte_initial[i/2u]>>((i%2u)*16u));
            g_pe_gte.lcm[i/3u][i%3u]=(int16_t)(DAY1_tgte_initial[8u+i/2u]>>((i%2u)*16u));
        }
        for (unsigned i=0;i<3u;i++) {
            g_pe_gte.bk[i]=(int32_t)DAY1_tgte_initial[5u+i];
            g_pe_gte.fc[i]=(int32_t)DAY1_tgte_initial[13u+i];
        }
        g_pe_gte.dqa=(int16_t)DAY1_tgte_initial[16];
        g_pe_gte.dqb=(int32_t)DAY1_tgte_initial[17];
        switch (DAY1_tgte_cases[k].entry) {
        case 0x80078E34u:func_80078E34(a[0]);break;
        case 0x80078E64u:func_80078E64(a[0]);break;
        case 0x80078FC4u:func_80078FC4(a[0],a[1],a[2]);break;
        case 0x80078FE4u:func_80078FE4(a[0],a[1],a[2]);break;
        case 0x80077E64u:func_80077E64(a[0],a[1],(int32_t)a[2]);break;
        default:ASSERT(0,"unknown GTE setup entry");
        }
        for (unsigned i=0;i<9u;i++) {
            got[i/2u]|=(uint32_t)(uint16_t)g_pe_gte.llm[i/3u][i%3u]<<((i%2u)*16u);
            got[8u+i/2u]|=(uint32_t)(uint16_t)g_pe_gte.lcm[i/3u][i%3u]<<((i%2u)*16u);
        }
        for (unsigned i=0;i<3u;i++) {
            got[5u+i]=(uint32_t)g_pe_gte.bk[i];got[13u+i]=(uint32_t)g_pe_gte.fc[i];
        }
        got[16]=(uint16_t)g_pe_gte.dqa;got[17]=(uint32_t)g_pe_gte.dqb;
        for (unsigned i=0;i<18u;i++) {
            if (got[i]!=DAY1_tgte_cases[k].control[i])
                fprintf(stderr,"transition GTE case %u control %u %08X/%08X\n",k,i,got[i],DAY1_tgte_cases[k].control[i]);
            ASSERT(got[i]==DAY1_tgte_cases[k].control[i],"GTE setup differs from original control readback");
        }
        ASSERT(!!PE_Port_ShouldStop()==!!DAY1_tgte_cases[k].trap,"original fog traps stay explicit");
        ASSERT(!g_stub_order_count,"GTE setup has no missing calls");
    }
    PASS();
}

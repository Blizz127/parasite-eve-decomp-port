#include "retail_ncct_cases.h"
static void test_DAY1_ncct(void)
{
    TEST("DAY1_ncct");
    for (unsigned k=0;k<sizeof(NCCT_cases)/sizeof(NCCT_cases[0]);k++) {
        const uint32_t *v=NCCT_cases[k].input;
        uint32_t got[9];
        ResetTestState();
        for (unsigned i=0;i<9;i++) {
            g_pe_gte.llm[i/3][i%3]=(int16_t)(v[6+i/2]>>((i%2)*16));
            g_pe_gte.lcm[i/3][i%3]=(int16_t)(v[11+i/2]>>((i%2)*16));
        }
        PE_GTE_SetV0((int16_t)v[0],(int16_t)(v[0]>>16),(int16_t)v[1]);
        PE_GTE_SetV1((int16_t)v[2],(int16_t)(v[2]>>16),(int16_t)v[3]);
        PE_GTE_SetV2((int16_t)v[4],(int16_t)(v[4]>>16),(int16_t)v[5]);
        for (unsigned i=0;i<3;i++)g_pe_gte.bk[i]=(int32_t)v[16+i];
        PE_GTE_SetRGBC(v[19]);PE_GTE_NCCT();
        for (unsigned i=0;i<3;i++) {
            got[i]=g_pe_gte.rgb_fifo[i];got[3+i]=(uint32_t)g_pe_gte.mac[i];got[6+i]=(uint32_t)g_pe_gte.ir[i];
        }
        for (unsigned i=0;i<9;i++) {
            if (got[i]!=NCCT_cases[k].output[i])fprintf(stderr,"NCCT case%u field%u %08X/%08X\n",k,i,got[i],NCCT_cases[k].output[i]);
            ASSERT(got[i]==NCCT_cases[k].output[i],"NCCT result differs from hardware arithmetic oracle");
        }
    }
    PASS();
}

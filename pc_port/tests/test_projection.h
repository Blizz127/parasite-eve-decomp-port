#include "retail_projection_cases.h"
static void test_DAY1_projection(void)
{
    TEST("DAY1_projection");
    static const uint32_t ranges[][2]={{0x140000,256}};
    for (unsigned k=0;k<sizeof(DAY1_projection_cases)/sizeof(DAY1_projection_cases[0]);k++) {
        ResetTestState(); memset(&g_pe_gte,0,sizeof(g_pe_gte));
        const uint32_t *c=DAY1_projection_cases[k].ctrl;
        for (unsigned i=0;i<9;i++)g_pe_gte.rt[i/3][i%3]=(int16_t)(c[i/2]>>((i%2)*16));
        for (unsigned i=0;i<3;i++)g_pe_gte.tr[i]=(int32_t)c[5+i];
        g_pe_gte.ofx=(int32_t)c[24];g_pe_gte.ofy=(int32_t)c[25];g_pe_gte.h=(int32_t)c[26];
        g_pe_gte.dqa=(int32_t)c[27];g_pe_gte.dqb=(int32_t)c[28];g_pe_gte.zsf3=(int32_t)c[29];g_pe_gte.zsf4=(int32_t)c[30];
        for(unsigned i=0;i<256;i++)PE_StoreU8(0x80140000u+i,(uint8_t)(i*13+7));
        for(unsigned i=0;i<8;i++)PE_StoreU32(0x80140000u+i*4,DAY1_projection_cases[k].vertices[i]);
        pe_addr_t p[4]={0x80140000,0x80140008,0x80140010,0x80140018};
        pe_addr_t o[7]={0x80140040,0x80140044,0x80140048,0x8014004C,0x80140050,0x80140054,0x80140058};
        switch(DAY1_projection_cases[k].alias) {
        case 1:o[0]=p[3];break;
        case 2:o[1]=p[3]+4;break;
        case 3:o[6]=p[3];break;
        case 4:o[6]=o[0];break;
        case 5:o[4]=o[6];break;
        case 6:o[5]=o[6];break;
        case 7:for(unsigned i=0;i<7;i++)o[i]=p[3];break;
        case 8:o[2]=p[0];o[3]=p[1];break;
        }
        int32_t ret=DAY1_projection_cases[k].kind?
            func_80079414(p[0],p[1],p[2],p[3],o[0],o[1],o[2],o[3],o[4],o[5],o[6]):
            func_80079384(p[0],p[1],p[2],o[0],o[1],o[2],o[4],o[5],o[6]);
        uint32_t state[]={g_pe_gte.otz,(uint32_t)g_pe_gte.ir0,
            (uint32_t)g_pe_gte.ir[0],(uint32_t)g_pe_gte.ir[1],(uint32_t)g_pe_gte.ir[2],
            g_pe_gte.sxy[0],g_pe_gte.sxy[1],g_pe_gte.sxy[2],
            g_pe_gte.sz[0],g_pe_gte.sz[1],g_pe_gte.sz[2],g_pe_gte.sz[3],
            (uint32_t)g_pe_gte.mac0,(uint32_t)g_pe_gte.mac[0],(uint32_t)g_pe_gte.mac[1],(uint32_t)g_pe_gte.mac[2],
            (uint16_t)g_pe_gte.v0[0]|((uint32_t)(uint16_t)g_pe_gte.v0[1]<<16),(uint16_t)g_pe_gte.v0[2]};
        uint64_t hash=hit_camera_hash(ranges,1);
        if ((uint32_t)ret!=DAY1_projection_cases[k].ret || hash!=DAY1_projection_cases[k].hash)
            fprintf(stderr,"projection %u kind%u ret %X/%X hash %llX/%llX\n",k,DAY1_projection_cases[k].kind,(unsigned)ret,DAY1_projection_cases[k].ret,(unsigned long long)hash,(unsigned long long)DAY1_projection_cases[k].hash);
        ASSERT((uint32_t)ret==DAY1_projection_cases[k].ret,"projection normal clip differs from original");
        ASSERT(hash==DAY1_projection_cases[k].hash,"projection output/flags or alias write order differs from original");
        ASSERT(!memcmp(state,DAY1_projection_cases[k].state,sizeof(state)),"projection hardware registers differ from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"projection used unresolved boundary");
    }
    PASS();
}

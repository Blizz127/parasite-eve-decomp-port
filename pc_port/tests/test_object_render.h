#include "retail_object_render_cases.h"
static uint32_t object_render_random(uint32_t *seed)
{
    *seed^=*seed<<13;*seed^=*seed>>17;*seed^=*seed<<5;return *seed;
}
static void test_DAY1_object_render(void)
{
    TEST_RETAIL_DISC1("DAY1_object_render"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_view);
    static const unsigned strides[2][8]={{28,36,36,48,64,72,60,76},{28,36,36,48,40,48,48,60}},vertices[2][8]={{4,4,12,16,40,40,36,44},{4,4,12,16,16,16,24,28}};
    static const uint32_t ranges[][2]={{0x1F800000,1024},{0x80140000,0x1800},{0x80145000,0x1000},{0x80146000,0x5000},{0x8014B000,16},{0x8019C9C0,4},{0x801EA5E0,4}};
    for(unsigned k=0;k<sizeof(DAY1_object_render_cases)/sizeof(DAY1_object_render_cases[0]);k++) {
        const unsigned *stride=strides[DAY1_object_render_cases[k].kind!=0],*vertex=vertices[DAY1_object_render_cases[k].kind!=0];
        ResetTestState();memset(&g_pe_gte,0,sizeof(g_pe_gte));
        for(unsigned j=0;j<sizeof(DAY1_view_tables);j++)PE_StoreU8(0x80095F00u+j,DAY1_view_tables[j]);
        uint32_t seed=DAY1_object_render_cases[k].seed,mask=DAY1_object_render_cases[k].mask,mode=DAY1_object_render_cases[k].mode;
        for(unsigned j=0;j<7;j++)for(unsigned i=0;i<ranges[j][1];i+=4)PE_StoreU32(ranges[j][0]+i,object_render_random(&seed));
        uint32_t bias=DAY1_object_render_cases[k].bias,shape=DAY1_object_render_cases[k].shape;
        g_pe_gte.rt[0][0]=g_pe_gte.rt[1][1]=g_pe_gte.rt[2][2]=4096;
        g_pe_gte.tr[0]=(int32_t)(object_render_random(&seed)%201)-100;
        g_pe_gte.tr[1]=(int32_t)(object_render_random(&seed)%201)-100;
        g_pe_gte.tr[2]=(int32_t)(object_render_random(&seed)%3000)+1000;
        g_pe_gte.ofx=160<<16;g_pe_gte.ofy=120<<16;g_pe_gte.h=1000;
        g_pe_gte.dqa=-4194;g_pe_gte.dqb=0x1400000;g_pe_gte.zsf3=341;g_pe_gte.zsf4=256;
        if(shape){g_pe_gte.tr[0]=g_pe_gte.tr[1]=0;g_pe_gte.tr[2]=(int32_t)DAY1_object_render_cases[k].depth;}
        PE_StoreU32(0x80140000,0x40);PE_StoreU32(0x80140004,0x40);
        for(unsigned kind=0;kind<8;kind++) {
            unsigned count=(mask>>kind&1)?1+object_render_random(&seed)%3:0;
            if(shape)count=(mask>>kind&1)?1:0;
            PE_StoreU16(0x80140040+kind*2,(uint16_t)count);
            PE_StoreU32(0x80140050+kind*4,0xC0+kind*0x200);
            for(unsigned i=0;i<3;i++) {
                pe_addr_t p=0x80140100+kind*0x200+i*stride[kind]+vertex[kind];
                unsigned order=object_render_random(&seed)%4;
                if(shape)order=shape-1;
                int32_t coords[4][2]={{-200,-200},{200,-200},{-200,200},{200,200}};
                if(order==1){coords[1][0]=-200;coords[1][1]=200;coords[2][0]=200;coords[2][1]=-200;}
                if(order==2)memset(coords,0,sizeof(coords));
                if(order==3)for(unsigned j=0;j<4;j++)for(unsigned n=0;n<2;n++)coords[j][n]=(int32_t)(object_render_random(&seed)%65536)-32768;
                for(unsigned j=0;j<((kind==1||kind==3||kind==5||kind==7)?4u:3u);j++) {
                    PE_StoreU16(p+j*8,(uint16_t)coords[j][0]);PE_StoreU16(p+j*8+2,(uint16_t)coords[j][1]);
                    int32_t z=(int32_t)(object_render_random(&seed)%4096)-1024;
                    PE_StoreU16(p+j*8+4,(uint16_t)(shape?0:z));PE_StoreU16(p+j*8+6,0);
                }
            }
        }
        pe_addr_t packet=0x80145000,ot=0x80146000;
        if(mode==1||mode==2){unsigned high=0;for(unsigned j=0;j<8;j++)if(mask>>j&1)high=j;packet=0x80140100+high*0x200+(mode==1?4:16);}
        unsigned high=0;for(unsigned j=0;j<8;j++)if(mask>>j&1)high=j;
        unsigned index=((high==1?256u:255u)+bias)>>2;
        if(mode==3)ot=packet-index*4;
        if(mode==4)ot=0x80140100-index*4;
        pe_addr_t descriptor=mode==5?0x80140100+high*0x200:0x8014B000;
        PE_StoreU32(0x8019C9C0,descriptor);PE_StoreU32(descriptor,packet);PE_StoreU32(descriptor+4,ot);PE_StoreU32(0x801EA5E0,bias);
        for(unsigned j=0;j<DAY1_object_render_cases[k].patch_count;j++)PE_StoreU32(0x80000000u+DAY1_object_render_cases[k].patches[j][0],DAY1_object_render_cases[k].patches[j][1]);
        const uint32_t *args=DAY1_object_render_cases[k].args;
        unsigned kind=DAY1_object_render_cases[k].kind;
        if(kind==0)func_80190D3C(0x80150000,0x80149999);
        else if(kind==1)func_80190E04(0x80150000,0x80149999,args[0],args[1],args[2]);
        else func_80191114(0x80150000,0x80149999,args[0],args[1]);
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned j=0;j<7;j++)for(unsigned i=0;i<ranges[j][1];i++)hash=(hash^PE_LoadU8(ranges[j][0]+i))*UINT64_C(1099511628211);
        static const uint32_t extra[][2]={{0x80150000,0x140},{0x8019BFF0,4},{0x8019CC30,32},{0x8018F014,32},{0x8019CA90,0x190},{0x801EA5E4,4}};
        for(unsigned j=0;j<6;j++)for(unsigned i=0;i<extra[j][1];i++)hash=(hash^PE_LoadU8(extra[j][0]+i))*UINT64_C(1099511628211);
        uint32_t state[28]={g_pe_gte.otz,(uint32_t)g_pe_gte.ir0,(uint32_t)g_pe_gte.ir[0],(uint32_t)g_pe_gte.ir[1],(uint32_t)g_pe_gte.ir[2],
            g_pe_gte.sxy[0],g_pe_gte.sxy[1],g_pe_gte.sxy[2],g_pe_gte.sz[0],g_pe_gte.sz[1],g_pe_gte.sz[2],g_pe_gte.sz[3],
            (uint32_t)g_pe_gte.mac0,(uint32_t)g_pe_gte.mac[0],(uint32_t)g_pe_gte.mac[1],(uint32_t)g_pe_gte.mac[2]};
        for(unsigned j=0;j<9;j++)state[16+j]=(uint32_t)(int32_t)g_pe_gte.rt[j/3][j%3];
        for(unsigned j=0;j<3;j++)state[25+j]=(uint32_t)g_pe_gte.tr[j];
        if(hash!=DAY1_object_render_cases[k].hash)fprintf(stderr,"object wrapper %u mask%X mode%u hash%llX/%llX\n",k,mask,mode,(unsigned long long)hash,(unsigned long long)DAY1_object_render_cases[k].hash);
        ASSERT(hash==DAY1_object_render_cases[k].hash,"object wrapper packets/scratch/OT/source differ from original");
        if(!DAY1_object_render_cases[k].trap)ASSERT(!memcmp(state,DAY1_object_render_cases[k].state,sizeof(state)),"object wrapper GTE state differs from original");
        ASSERT(!!PE_Port_ShouldStop()==!!DAY1_object_render_cases[k].trap&&!g_stub_order_count,"object wrapper stop prefix differs");
    }
    PASS();
}

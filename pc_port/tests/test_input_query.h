#include "retail_input_query_cases.h"
static void test_DAY1_input_query(void)
{
    TEST("DAY1_input_query");
    static const uint32_t extra[]={0,0xFFFFFFFFu,0x7FFFFFFFu,0x80000001u,0xC0000000u,0x40000001u,0x01000100u,0x80150040u};
    static const uint32_t ranges[][2]={{0x150000,128},{0xA776C,132},{0x9D26C,4},{0x9D1F4,4},{0x9D1E4,4}};
    for(unsigned n=0;n<2560;n++) {
        ResetTestState();
        for(unsigned i=0;i<128;i+=4)PE_StoreU32(0x80150000u+i,0xA5A50000u+i);
        for(unsigned i=0;i<33;i++)PE_StoreU32(0x800A776Cu+i*4,0x12340000u+i*0x101u+n);
        uint32_t code=n<1280?3u:(n/40)%8,bit=n%40;
        uint32_t mask=bit<32?1u<<bit:extra[bit-32];
        pe_addr_t maskptr=0x80150044u;
        if(n/640%2){maskptr=0x80150000u;mask=0x80150040u;}
        uint32_t flags[]={mask,~mask,0,0xFFFFFFFFu};
        PE_StoreU32(0x8009D26Cu,flags[n/40%4]);PE_StoreU32(0x8009D1F4u,flags[n/40%4]^0x55555555u);PE_StoreU32(0x8009D1E4u,flags[n/40%4]^0xAAAAAAAAu);
        PE_StoreU32(0x80150040u,code);PE_StoreU32(0x80150044u,mask);
        PE_StoreU32(0x80150000u,0x80150040u);PE_StoreU32(0x80150004u,maskptr);
        pe_addr_t destinations[]={0x80150048u,maskptr,0x80150000u,0x80150004u,0x80150008u,0x8009D26Cu,0x800A776Cu,0x800A77ECu};
        PE_StoreU32(0x80150008u,destinations[n/160%8]);
        PE_GTE_SetLZCS(0x80000000u);g_pe_gte.projection_flags=0xA5A51234u;
        PeGteState expected=g_pe_gte;
        expected.lzcs=DAY1_input_query_cases[n].lzcs;expected.lzcr=DAY1_input_query_cases[n].lzcr;
        ASSERT(func_800130B4(0x80150000u)==1,"input query return differs");
        ASSERT(hit_camera_hash(ranges,5)==DAY1_input_query_cases[n].hash,"input query aliased RAM effects differ from original");
        ASSERT(memcmp(&expected,&g_pe_gte,sizeof(expected))==0,"input query GTE effects differ from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"input query stopped");
    }
    ResetTestState();
    const pe_addr_t actor=0x800BEA90u,task=0x8009D310u,stream=0x80122000u;
    PE_StoreU32(0x800910A0u+0x11u*4,0x800130B4u);PE_StoreU32(0x800910A4u,0x800172BCu);
    PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D300u,task);
    PE_StoreU32(task,stream);PE_StoreU32(task+0x10u,1);PE_StoreU32(actor+0x98u,0x100000E0u);
    PE_StoreU32(stream,0x6011u);PE_StoreU32(stream+8u,3);PE_StoreU32(stream+12u,0x100);PE_StoreU32(stream+20u,1);
    PE_StoreU32(0x8009D26Cu,0x100);PE_StoreU32(0x800A7790u,1234);
    func_80017018();
    ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"opcode11 reached VM boundary");
    ASSERT(PE_LoadU32(stream+12u)==23 && PE_LoadU32(stream+16u)==1234,"VM held-query destructive mask/output differ");
    ASSERT(PE_LoadU32(actor+0x98u)&0x10u,"VM did not continue after input query");
    PASS();
}

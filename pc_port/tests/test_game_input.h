#include "retail_game_input_cases.h"
static void test_DAY1_game_input(void)
{
    TEST("DAY1_game_input");
    static const uint32_t ranges[][2]={{0x9D140,0x1A0},{0xA5B70,0x1E0},{0xA76F0,0x100},{0xB0CD8,0xF0},{0xBE9A0,0x44},{0x150000,0x40},{0x9B738,12},{0x92200,36}};
    for(unsigned n=0;n<sizeof(DAY1_game_input_hashes)/sizeof(DAY1_game_input_hashes[0]);n++) {
        ResetTestState();uint32_t seed=57+n*101;
        for(unsigned j=0;j<8;j++)for(unsigned i=0;i<ranges[j][1];i+=4)
            PE_StoreU32(0x80000000u+ranges[j][0]+i,object_render_random(&seed));
        PE_StoreU32(0x8009B738u,0x80084B20u);PE_StoreU32(0x8009B740u,0x80084F8Cu);
        PE_StoreU32(0x800A5B80u,0x800A5B70u);PE_StoreU32(0x800A5BA0u,0x800BE9A0u);PE_StoreU32(0x800A5BA4u,0);
        PE_StoreU8(0x800A5BB9u,(uint8_t)((n/32)%8));PE_StoreU8(0x800A5BB6u,(n&16)?255:1);
        PE_StoreU16(0x800A5C56u,(n&512)?0:2);
        static const uint32_t flags[]={0,1,0x4000,0x4001,0xC000,0xC001,0x8000,0x8001};
        static const uint16_t types[]={0,0x4100,0x7300,0xFFFF};
        PE_StoreU32(0x8009D1A0u,flags[n/4%8]);PE_StoreU16(0x800BE9A0u,types[n%4]);
        PE_StoreU16(0x800BE9A2u,(uint16_t)object_render_random(&seed));
        static const uint8_t edges[]={0,19,20,89,90,160,161,230,231,255};
        PE_StoreU8(0x800BE9A6u,edges[n/256%10]);PE_StoreU8(0x800BE9A7u,edges[n/25%10]);
        PE_StoreU32(0x8009D154u,(n&128)?0x80150000u:0);PE_StoreU32(0x80150000u,0);
        PE_StoreU32(0x80150020u,1);PE_StoreU32(0x80150024u,0);
        PE_StoreU32(0x800B0CD8u,(n/8%4)*0x200);PE_StoreU8(0x800B0DBFu,(uint8_t)(n/16%3));
        PE_StoreU32(0x8009D2A8u,n%9);
        for(unsigned i=0;i<32;i++)PE_StoreU32(0x800A76F0u+i*4,1u<<(i%16));
        for(unsigned i=0;i<9;i++)PE_StoreU32(0x80092200u+i*4,(n&1)?8:0x40);
        if(n>=2560) {
            PE_StoreU16(0x800BE9A0u,0x4100);PE_StoreU16(0x800BE9A2u,0x7FEF);
            PE_StoreU32(0x8009D1A0u,1);PE_StoreU32(0x8009D2D4u,0);
            for(unsigned i=0;i<32;i++)PE_StoreU32(0x800A76F0u+i*4,0);
            PE_StoreU32(0x800A76F0u+31*4,0x8000);PE_StoreU32(0x800A76F0u+3*4,0x10);
        }
        func_8003EB04();uint64_t hash=hit_camera_hash(ranges,8);
        if(hash!=DAY1_game_input_hashes[n])fprintf(stderr,"game input %u hash%llX/%llX\n",n,(unsigned long long)hash,(unsigned long long)DAY1_game_input_hashes[n]);
        ASSERT(hash==DAY1_game_input_hashes[n],"full input state differs from original");
        ASSERT(D_8009D1A0==PE_LoadU32(0x8009D1A0u)&&D_8009D280==PE_LoadU32(0x8009D280u),"game flag aliases diverged");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"full input stopped or hit stub");
    }
    PASS();
}

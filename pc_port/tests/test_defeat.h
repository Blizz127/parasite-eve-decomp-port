#include "retail_defeat_cases.h"

static void test_ATK18_retail_defeat_results(void)
{
    static const uint32_t ranges[][2]={{0x140000,0x7000},{0xA5D58,7*220},{0xA7FF0,40},{0xB00E8,0xD4},
        {0xB6920,0x38},{0xBCEA8,224},{0xB8628,72},{0xBCD80,32},{0x9D1A0,4},{0x9D1D0,8},
        {0x9CE3C,1},{0x9D21C,2},{0x9D234,1},{0x9D244,1},{0x9D268,4},{0x9D28C,4},
        {0x9D298,12},{0x9D304,4},{0x9D2E8,4},{0x9D2F4,4},{0xB0CD8,4}};
    unsigned k,i;
    TEST("ATK18_retail_defeat_results");
    for (k=0;k<sizeof(ATK18_defeat_cases)/sizeof(ATK18_defeat_cases[0]);k++) {
        const int *c=ATK18_defeat_cases[k]; pe_addr_t b=0x800A5D5Cu; uint64_t hash;
        ResetTestState(); D_8009D1A0=(uint32_t)c[10];
        PE_StoreU32(0x8009D254u,0x80140000u);PE_StoreU32(0x8009D278u,0x80144000u);PE_StoreU32(0x8009D20Cu,0x80141000u);
        PE_StoreU32(0x80140000u,0x80144000u);PE_StoreU32(0x80140004u,c[8]?0x80142800u:0u);
        PE_StoreU32(0x80141000u,b);PE_StoreU32(0x80141004u,0x80142000u);PE_StoreU32(0x80142004u,0x80140000u);
        PE_StoreU32(0x8014218Cu,0x80141000u);PE_StoreU32(0x80142800u,0x80144100u);PE_StoreU32(b-4u,0x80141000u);
        PE_StoreU8(b+0xACu,(uint8_t)c[1]);PE_StoreU8(b+0xAFu,(uint8_t)c[2]);PE_StoreU8(b+0xADu,(uint8_t)c[6]);
        PE_StoreU8(b+5u,(uint8_t)c[5]);PE_StoreU8(b+0xAEu,1u);PE_StoreU8(b+0x91u,8u);PE_StoreU32(b+8u,255u);
        PE_StoreU16(b+14u,20u);PE_StoreU16(b+0x98u,100u);PE_StoreU16(b+0x9Au,3u);PE_StoreU16(b+0x9Cu,10u);
        PE_StoreU16(b+0xA0u,c[9]?200u:0u);PE_StoreU16(b+0xA2u,c[9]==2?65535u:3u);PE_StoreU8(b+0x9Eu,c[9]?5u:0u);
        if(c[9]==3) for(i=0;i<10;i++) PE_StoreU16(0x800A7FF0u+i*4u,(uint16_t)(50u+i));
        PE_StoreU8(0x8014100Eu,(uint8_t)c[4]);PE_StoreU16(0x80141016u,(uint16_t)c[3]);
        PE_StoreU8(0x80141252u,(uint8_t)c[7]);PE_StoreU32(0x80141068u,0x10000u);
        for(i=0;i<2;i++) {
            pe_addr_t a=0x801411B4u+i*0x1000u,p=0x80146000u+i*0x100u;
            PE_StoreU32(a,0x80145000u);PE_StoreU16(a+0xBAu,1u);PE_StoreU32(a+0x54u,p);PE_StoreU8(p+7u,0x3Cu);
        }
        PE_StoreU16(0x80145008u,1u);PE_StoreU32(0x8009D1A0u,(uint32_t)c[10]);PE_StoreU8(0x8009D2A0u,c[1]==0?1u:0u);
        PE_StoreU32(0x8009D2E8u,65535u);PE_StoreU32(0x8009D298u,0xFFFFFFFFu);PE_StoreU32(0x8009D29Cu,0xFFFFFFFFu);
        PE_StoreU16(0x8014400Cu,45u);PE_StoreU16(0x8014401Cu,40u);PE_StoreU32(0x8014404Cu,0xFFFFFFFFu);
        PE_StoreU32(0x80140068u,0xFFFFFFFFu);PE_StoreU32(0x80140098u,0x100u);
        PE_StoreU32(0x800B0E98u+20u*4u,0x80145080u);PE_StoreU8(0x80145082u,8u);
        if(c[0]==0)func_80028E94(0x80141000u);
        else if(c[0]==1)func_800293F4((uint32_t)c[11]);
        else func_8002F300_mode2_cut();
        hash=hit_camera_hash(ranges,sizeof(ranges)/sizeof(ranges[0]));
        if(hash!=ATK18_defeat_expected[k])fprintf(stderr,"defeat %u: %016llX\n",k,(unsigned long long)hash);
        ASSERT(hash==ATK18_defeat_expected[k],"defeat/results differs from original instructions");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"defeat/results executes natively");
    }
    PASS();
}

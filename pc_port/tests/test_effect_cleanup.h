#include "retail_cleanup_cases.h"

static void test_ATK16_retail_effect_cleanup(void)
{
    static const uint32_t ranges[][2]={{0x150000,11*0xA0C},{0x160000,11*0x10C},{0xE10A0,28},{0xB0CD8,4}};
    static const uint32_t codes[]={2,4,0x55};
    static const uint32_t callbacks[]={0x800C9C00u,0x800CD960u,0x800D4850u};
    unsigned k,i,j;
    TEST("ATK16_retail_effect_cleanup");
    for (k=0;k<sizeof(ATK16_cleanup_cases)/sizeof(ATK16_cleanup_cases[0]);k++) {
        const int *c=ATK16_cleanup_cases[k]; uint32_t result; uint64_t hash;
        ResetTestState();
        for (i=0;i<4;i++) for (j=0;j<ranges[i][1];j++)
            PE_StoreU8(0x80000000u+ranges[i][0]+j,(uint8_t)(j*37u+19u));
        PE_StoreU32(0x800942E4u,0x80150000u); PE_StoreU32(0x800942E8u,0x80160000u);
        PE_StoreU32(0x800942E0u,0x80170000u);
        for (j=0;j<3;j++) {
            PE_StoreU32(0x80170000u+codes[j]*4u,c[5]==1?0u:0x80171000u+codes[j]*24u);
            PE_StoreU32(0x80171000u+codes[j]*24u+20u,c[5]==2?0u:callbacks[j]);
        }
        PE_StoreU32(0x8009D254u,0x80140000u); PE_StoreU32(0x8009D20Cu,0x80141000u);
        for (j=0;j<3;j++) {
            pe_addr_t a=0x80141000u+j*0x400u;
            PE_StoreU32(a+4u,j<2?a+0x400u:0u); PE_StoreU8(a+12u,2u);
            PE_StoreU8(a+13u,j==2?1u:0u); PE_StoreU32(a+0x98u,j==0?0x10u:0u);
        }
        PE_StoreU32(0x80142000u,0x80142010u); PE_StoreU32(0x80142004u,0x80142014u);
        PE_StoreU32(0x80142010u,(uint32_t)c[1]); PE_StoreU32(0x80142014u,(uint32_t)c[2]);
        for (j=0;j<22;j++) {
            static const pe_addr_t owners[]={0x80140000u,0x80141000u,0x80141400u,0x80141800u};
            pe_addr_t a=j<11?0x80150000u+j*0xA0Cu:0x80160000u+(j-11u)*0x10Cu;
            PE_StoreU8(a,(uint8_t)c[3]); PE_StoreU8(a+1u,(uint8_t)c[4]); PE_StoreU32(a+8u,owners[j%4]);
        }
        result=(uint32_t)(c[0]?func_8001930C(0x80142000u):func_8006FE14(c[1]?0x80141400u:0u));
        hash=hit_camera_hash(ranges,4u);
        if (hash!=ATK16_cleanup_expected[k]) fprintf(stderr,"cleanup %u: %016llX\n",k,(unsigned long long)hash);
        ASSERT(hash==ATK16_cleanup_expected[k] && result==ATK16_cleanup_returns[k],"actor effect cleanup differs from original instructions");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"cleanup executes natively");
    }
    PASS();
}

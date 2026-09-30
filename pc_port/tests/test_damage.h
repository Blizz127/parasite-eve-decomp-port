#include "retail_damage_cases.h"

static void test_ATK17_retail_damage_reaction(void)
{
    static const uint32_t ranges[][2]={{0x140000,0x6000}};
    unsigned k,i;
    TEST("ATK17_retail_damage_reaction");
    for (k=0;k<sizeof(ATK17_damage_cases)/sizeof(ATK17_damage_cases[0]);k++) {
        const int *c=ATK17_damage_cases[k]; uint64_t hash;
        ResetTestState(); func_80071A64((uint32_t)c[10]);
        PE_StoreU32(0x8009D254u,0x80140000u); PE_StoreU32(0x8009D278u,0x80144000u);
        PE_StoreU32(0x80140028u,100u<<16); PE_StoreU32(0x80140030u,200u<<16);
        PE_StoreU16(0x8014003Au,(uint16_t)c[9]); PE_StoreU32(0x80141000u,0x80143000u);
        PE_StoreU8(0x8014100Cu,2u); PE_StoreU8(0x8014100Eu,6u); PE_StoreU8(0x8014100Fu,7u);
        PE_StoreU32(0x80141030u,1000u<<16); PE_StoreU16(0x8014103Au,(uint16_t)c[9]);
        PE_StoreU32(0x80141098u,0x200u); PE_StoreU32(0x80141014u,0x20000u);
        PE_StoreU32(0x80141018u,0x30000u); PE_StoreU32(0x8014101Cu,0x8000u);
        PE_StoreU32(0x80143000u,(uint32_t)c[3]|(c[8]?0x100000u:0u)); PE_StoreU32(0x80143010u,1000u);
        PE_StoreU8(0x80143005u,(uint8_t)c[6]); PE_StoreU8(0x80143006u,2u);
        PE_StoreU16(0x8014308Cu,(uint16_t)c[2]); PE_StoreU8(0x801430BCu,(uint8_t)c[7]);
        PE_StoreU8(0x801430A4u,(uint8_t)c[8]); PE_StoreU16(0x801430A6u,10u); PE_StoreU32(0x801430CCu,(uint32_t)c[5]);
        PE_StoreU32(0x8014404Cu,(uint32_t)c[1]); PE_StoreU16(0x80144004u,7u);
        PE_StoreU16(0x8014400Au,8u); PE_StoreU16(0x8014402Au,20u); PE_StoreU16(0x8014401Eu,(uint16_t)c[13]);
        PE_StoreU32(0x80144068u,0x80144100u); PE_StoreU16(0x80144100u,(uint16_t)c[12]);
        PE_StoreU16(0x80144106u,1u); PE_StoreU32(0x80144110u,(uint32_t)c[4]);
        PE_StoreU8(0x8009D2B0u,(uint8_t)c[11]);
        for (i=0;i<3;i++) {
            PE_StoreU32(0x800B0E98u+2u*192u+i*4u,0x80145000u+i*8u);
            PE_StoreU8(0x80145002u+i*8u,8u);
        }
        for (i=0;i<1025;i++) {
            PE_StoreU16(0x8009A6ECu+i*2u,(uint16_t)(i/2u)); PE_StoreU16(0x8009589Cu+i*2u,(uint16_t)(i*4u));
        }
        if (c[0]) func_80028C48(0x80141000u); else func_80028574(0x80141000u);
        hash=hit_camera_hash(ranges,1u);
        if (hash!=ATK17_damage_expected[k]) fprintf(stderr,"damage %u: %016llX HP %d\n",k,(unsigned long long)hash,(int32_t)PE_LoadU32(0x80143010u));
        ASSERT(hash==ATK17_damage_expected[k],"damage or hit reaction differs from original instructions");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"damage executes natively");
    }
    PASS();
}

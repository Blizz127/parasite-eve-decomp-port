/* Native fixtures paired with the independent original-instruction oracle. */
#include "retail_hit_camera_cases.h"

static uint64_t hit_camera_hash(const uint32_t (*ranges)[2],unsigned count)
{
    uint64_t hash=UINT64_C(14695981039346656037); unsigned i,j;
    for (i=0;i<count;i++) for (j=0;j<ranges[i][1];j++) {
        hash^=PE_LoadU8(0x80000000u+ranges[i][0]+j); hash*=UINT64_C(1099511628211);
    }
    return hash;
}

static void test_ATK15_retail_hit_frames(void)
{
    static const uint32_t ranges[][2]={{0x140000,0x4800},{0x9CE54,2},{0x9D274,1},{0x9D294,1}};
    unsigned k,i;
    TEST("ATK15_retail_hit_frames");
    for (k=0;k<sizeof(ATK15_hit_cases)/sizeof(ATK15_hit_cases[0]);k++) {
        const int *c=ATK15_hit_cases[k]; uint64_t hash;
        ResetTestState(); func_80071A64((uint32_t)c[9]);
        PE_StoreU32(0x8009D254u,0x80140000u); PE_StoreU32(0x8009D278u,0x80144000u);
        PE_StoreU32(0x8009D20Cu,0x80141000u); PE_StoreU16(0x8014003Au,(uint16_t)c[11]);
        PE_StoreU32(0x80141000u,0x80143000u); PE_StoreU32(0x80141004u,0x80142000u);
        PE_StoreU32(0x80142000u,0x80143100u); PE_StoreU32(0x80143010u,100u); PE_StoreU32(0x80143110u,100u);
        PE_StoreU16(0x80141268u,(uint16_t)c[2]); PE_StoreU16(0x8014126Cu,(uint16_t)c[3]);
        PE_StoreU16(0x80142268u,300u); PE_StoreU16(0x8014226Cu,300u);
        PE_StoreU32(0x80141098u,(uint32_t)c[7]); PE_StoreU32(0x80143000u,0x800FC010u);
        PE_StoreU32(0x80143100u,0x800FC010u);
        PE_StoreU16(0x80141218u,70u); PE_StoreU16(0x8014121Au,50u);
        PE_StoreU16(0x80142218u,90u); PE_StoreU16(0x8014221Au,60u);
        PE_StoreU32(0x80144068u,0x80144100u); PE_StoreU32(0x8014404Cu,(uint32_t)c[6]);
        PE_StoreU16(0x80144102u,(uint16_t)c[4]); PE_StoreU16(0x80144106u,(uint16_t)c[1]);
        PE_StoreU32(0x80144108u,2u); PE_StoreU32(0x8014410Cu,0x300006u);
        PE_StoreU32(0x80144110u,(uint32_t)c[5]); PE_StoreU32(0x800BE830u,0x80141000u);
        PE_StoreU8(0x8009D274u,(uint8_t)c[10]); PE_StoreU8(0x8009D294u,1u);
        for (i=0;i<1025;i++) PE_StoreU16(0x8009A6ECu+i*2u,(uint16_t)(i/2u));
        PE_StoreU32(0x800BCFA4u,0x80146000u); PE_StoreU32(0x800BCFA8u,0x80146100u);
        PE_StoreU32(0x80146100u,256u); PE_StoreU32(0x8014601Cu,1024u);
        for (i=0;i<3;i++) PE_StoreU16(0x80146000u+i*8u,4096u);
        PE_StoreU16(0x800BCF94u,160u); PE_StoreU16(0x800BCF96u,112u);
        PE_StoreU16(0x800B0DD0u,100u); PE_StoreU16(0x800B0DD2u,1000u);
        PE_StoreU8(0x800B0DCEu,20u); PE_StoreU8(0x800B0DCFu,120u);
        if (c[0]) func_800236E8(); else func_80021278(0x80141000u,0x8009CE54u,c[8]);
        hash=hit_camera_hash(ranges,4u);
        if (hash!=ATK15_hit_expected[k]) fprintf(stderr,"hit frame %u: %016llX\n",k,(unsigned long long)hash);
        ASSERT(hash==ATK15_hit_expected[k],"shot frame / hit judgement differs from original instructions");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"hit judgement and sound execute natively");
    }
    PASS();
}

static void test_STG8_retail_camera_pan(void)
{
    static const uint32_t ranges[][2]={{0xBCF88,0x1C}};
    unsigned k,i;
    TEST("STG8_retail_camera_pan");
    for (k=0;k<sizeof(STG8_camera_cases)/sizeof(STG8_camera_cases[0]);k++) {
        const int *c=STG8_camera_cases[k]; uint64_t hash;
        ResetTestState(); PE_StoreU32(0x800BCF88u,(uint32_t)c[0]);
        PE_StoreU16(0x800BCFA0u,(uint16_t)c[1]); PE_StoreU16(0x800BCFA2u,(uint16_t)c[2]);
        PE_StoreU16(0x800BCF8Cu,(uint16_t)c[3]); PE_StoreU16(0x800BCF8Eu,(uint16_t)c[4]);
        PE_StoreU16(0x800BCF98u,(uint16_t)c[3]); PE_StoreU16(0x800BCF9Au,(uint16_t)c[4]);
        PE_StoreU16(0x800BCF9Cu,(uint16_t)c[5]); PE_StoreU16(0x800BCF9Eu,(uint16_t)c[6]);
        PE_StoreU32(0x800B1624u,0x80150000u); PE_StoreU32(0x8015001Cu,0x100u);
        PE_StoreU16(0x80150128u,(uint16_t)c[7]); PE_StoreU16(0x8015012Au,(uint16_t)c[8]);
        for (i=0;i<1025;i++) PE_StoreU16(0x8009589Cu+i*2u,(uint16_t)(i*4u));
        (void)func_80066268(); hash=hit_camera_hash(ranges,1u);
        if (hash!=STG8_camera_expected[k]) fprintf(stderr,"camera pan %u: %016llX\n",k,(unsigned long long)hash);
        ASSERT(hash==STG8_camera_expected[k],"scripted camera pan differs from original instructions");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"camera pan executes natively");
    }
    PASS();
}

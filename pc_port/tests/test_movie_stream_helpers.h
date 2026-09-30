#include "retail_movie_stream_helpers_cases.h"
static void test_DAY2_movie_stream_helpers(void)
{
    TEST("DAY2_movie_stream_helpers");
    for(unsigned k=0;k<sizeof(MOVSTREAM_convert)/sizeof(MOVSTREAM_convert[0]);k++) {
        ResetTestState();PE_StoreU32(0x80130000u,0xA5A5A5A5u);
        ASSERT(func_8007A930((int32_t)MOVSTREAM_convert[k].lba,0x80130000u)==0x80130000u &&
            PE_LoadU32(0x80130000u)==MOVSTREAM_convert[k].word,"signed CD location conversion differs from original");
    }
    for(unsigned k=0;k<sizeof(MOVSTREAM_inverse)/sizeof(MOVSTREAM_inverse[0]);k++) {
        ResetTestState();PE_StoreU32(0x80130000u,MOVSTREAM_inverse[k].word);
        ASSERT((uint32_t)func_8007AA34(0x80130000u)==MOVSTREAM_inverse[k].result,"BCD inverse differs from original");
    }
    for(unsigned k=0;k<sizeof(MOVSTREAM_retry)/sizeof(MOVSTREAM_retry[0]);k++) {
        ResetTestState();
        for(unsigned j=0;j<sizeof(MOVSTREAM_ranges)/sizeof(MOVSTREAM_ranges[0]);j++)
            memset(PE_Translate(0x80000000u+MOVSTREAM_ranges[j][0],MOVSTREAM_ranges[j][1]),0xA5,MOVSTREAM_ranges[j][1]);
        PE_StoreU32(0x800A3490u,MOVSTREAM_retry[k].word);PE_StoreU32(0x800A3494u,0x12345678u);
        PE_StoreU32(0x800A8020u,MOVSTREAM_retry[k].guard);
        ASSERT((uint32_t)func_8007C2A0(MOVSTREAM_retry[k].dest)==MOVSTREAM_retry[k].result &&
            hit_camera_hash(MOVSTREAM_ranges,sizeof(MOVSTREAM_ranges)/sizeof(MOVSTREAM_ranges[0]))==MOVSTREAM_retry[k].hash,
            "stream retry position/result differs from original");
    }
    for(unsigned k=0;k<sizeof(MOVSTREAM_close)/sizeof(MOVSTREAM_close[0]);k++) {
        ResetTestState();func_80073C94();
        (void)func_800824F0(0x8007C214u);
        PE_StoreU32(0x8009AFD8u,MOVSTREAM_close[k].lane);PE_StoreU32(0x8009AFB8u,0x89ABCDEFu);PE_StoreU32(0x800B8AB4u,0x12345678u);
        PE_StoreU32(0x8009AF1Cu,0x80130000u);PE_StoreU32(0x8009AF28u,MOVSTREAM_close[k].alias?0x80130000u:0x80130004u);
        memset(PE_Translate(0x80130000u,8u),0xA5,8u);func_8007A2A4();
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x80130000u)==MOVSTREAM_close[k].first &&
            PE_LoadU32(0x80130004u)==MOVSTREAM_close[k].second && PE_LoadU32(0x8009AFB8u)==MOVSTREAM_close[k].low_callback &&
            PE_LoadU32(0x800B8AB4u)==MOVSTREAM_close[k].high_callback,"stream teardown RAM differs from original");
        ASSERT(func_800824F0(0u)==0u && !g_stub_order_count,"stream teardown failed to unregister actual DMA callback");
    }
    /* The same original register pointers may name physical CD registers. */
    ResetTestState();func_80073C94();(void)func_800824F0(0x8007C214u);
    PE_StoreU32(0x8009AFD8u,1u);PE_StoreU32(0x800B8AB4u,0x800813E8u);
    PE_StoreU32(0x8009AF1Cu,0x1F801800u);PE_StoreU32(0x8009AF28u,0x1F801803u);
    PE_CdStoreU8(0x1F801800u,3u);func_8007A2A4();
    ASSERT(!PE_Port_ShouldStop() && !(PE_CdLoadU8(0x1F801800u)&3u) && !PE_LoadU32(0x800B8AB4u) &&
        func_800824F0(0u)==0u,"physical CD teardown did not clear bank/callbacks");
    PASS();
}

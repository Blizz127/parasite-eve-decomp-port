#include "retail_fade_wait_cases.h"
static void DAY1_FadeSeed(unsigned n)
{
    ResetTestState();
    static const uint16_t colors[]={0x7FFF,0x8000,255,0x8000,0x7FFF,0};
    for(unsigned i=0;i<6;i++)PE_StoreU16(0x800BCFE8u+(i<3?i:i+1)*2,colors[i]);
    PE_StoreU32(0x8009CDDCu,n&1);PE_StoreU32(0x800B0E38u,0x80150000u);PE_StoreU32(0x800B0E3Cu,0x80150020u);
    PE_StoreU32(0x8009D300u,0x80150080u);PE_StoreU32(0x80150060u,0x80150064u);
    PE_StoreU32(0x8015000Cu,0xA5FFFFFFu);PE_StoreU32(0x8015002Cu,0x5AFFFFFFu);
}
static void test_DAY1_fade_wait(void)
{
    TEST("DAY1_fade_wait");
    static const uint32_t ranges[][2]={{0xBCF88,128},{0x150000,160},{0x9CE00,4}};
    unsigned row=0;
#define FADE_CHECK(call) do { \
    uint32_t ret=(uint32_t)(call); \
    ASSERT(ret==DAY1_fade_wait_cases[row].result,"fade/wait return differs"); \
    ASSERT(hit_camera_hash(ranges,3)==DAY1_fade_wait_cases[row].hash,"fade packet/timer/wait state differs from original"); \
    ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"fade stopped");row++; \
} while(0)
    for(unsigned n=0;n<512;n++) {
        DAY1_FadeSeed(n);
        static const uint16_t durations[]={0,1,2,60,32768,65535},times[]={0,1,59,60,32768,65535};
        PE_StoreU8(0x800BCFEEu,n%8);PE_StoreU8(0x800BCFEFu,n/8%4);
        PE_StoreU16(0x800BCFF6u,durations[n/8%6]);PE_StoreU16(0x800BCFF8u,times[n/48%6]);
        FADE_CHECK(func_80068E24());
    }
    for(unsigned n=0;n<16;n++) {
        DAY1_FadeSeed(n);static const unsigned durations[]={0,1,2,60};
        unsigned duration=durations[n/2%4],ticks=duration?duration:1;
        PE_StoreU32(0x80150064u,duration);
        FADE_CHECK(n<8?func_80018EB4(0x80150060u):func_80018EE0(0x80150060u));
        for(unsigned frame=0;frame<=ticks;frame++) {
            PE_StoreU32(0x8009CE00u,0x801910DCu);PE_StoreU32(0x80150090u,0xBAD);
            FADE_CHECK(func_80019410());
            if(frame==ticks)break;
            PE_StoreU32(0x8009CDDCu,frame&1);PE_StoreU32(0x8015000Cu+(frame&1)*32,0xFFFFFF);
            FADE_CHECK(func_80068E24());
        }
    }
    ASSERT(row==sizeof(DAY1_fade_wait_cases)/sizeof(DAY1_fade_wait_cases[0]),"fade checkpoints omitted");
#undef FADE_CHECK
    PASS();
}

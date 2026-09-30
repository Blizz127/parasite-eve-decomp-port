#include "retail_card_status_cases.h"
static void test_DAY1_card_status(void)
{
    TEST("DAY1_card_status");
    static const uint32_t ranges[][2]={{0xA0ED4,0xA00},{0x9D154,16}};
    static const uint8_t states[]={0,1,2,3,4,5,255,3},flags[]={0,1,4,255},operations[]={0,12,8};
    static const uint32_t timers[]={0,1,2,0xFFFFFFFFu,0x80000000u,0x7FFFFFFFu,0x80000001u,99};
    for(unsigned n=0;n<4096;n++) {
        ResetTestState();
        for(unsigned i=0;i<0xA00;i++)PE_StoreU8(0x800A0ED4u+i,0xA5);
        unsigned index=n/2048;pe_addr_t record=0x800A0ED4u+index*0x418u;
        PE_StoreU8(record,flags[n/512%4]);PE_StoreU8(record+8u,states[n%8]);PE_StoreU8(record+1u,operations[n/32%3]);
        unsigned mask=n/8%64;
        for(unsigned i=0;i<6;i++)PE_StoreU32(0x800A1820u+4u*i,mask&(1u<<i)?0x12345678u:0u);
        PE_StoreU32(0x800A1838u,n/128%2);PE_StoreU32(0x800A183Cu,n/64%3);PE_StoreU32(0x800A1840u,timers[n/256%8]);
        PE_StoreU32(0x8009D154u,n/16%2?0x80158000u:0u);PE_StoreU32(0x8009D158u,0x12345678u);
        PE_StoreU32(0x8009D15Cu,0x80159000u);PE_StoreU32(0x8009D160u,0xDEADBEEFu);
        PE_StoreU32(0x80158000u,0u);PE_StoreU32(0x80158020u,1u);PE_StoreU32(0x80158024u,36u);
        for(unsigned i=0;i<8;i++)PE_StoreU32(0x800BCDA8u+4u*i,0xF1000000u+i);
        func_800405A4(index);
        uint64_t hash=hit_camera_hash(ranges,2);
        if(hash!=DAY1_card_status_cases[n].hash)fprintf(stderr,"card status %u differs\n",n);
        ASSERT(hash==DAY1_card_status_cases[n].hash,"card status effects differ from original");
        ASSERT((unsigned)PE_Port_ShouldStop()==DAY1_card_status_cases[n].stopped,"card status stop differs");
        if(DAY1_card_status_cases[n].stopped) {
            ASSERT(g_bootstrap_arg4_call_count==1 && g_bootstrap_arg4_calls[0].target==0x800726F4u &&
                   g_bootstrap_arg4_calls[0].arg0==DAY1_card_status_cases[n].argument,"card status BIOS frontier differs");
        } else ASSERT(!g_stub_order_count,"returning card status path logged a stub");
    }
    PASS();
}

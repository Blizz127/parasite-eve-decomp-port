#include "retail_audio_commands_cases.h"

static void ATK19_SeedAudio(const int64_t *c)
{
    unsigned i;
    ResetTestState();
    PE_StoreU32(0x8009D2C8u,0x800B6980u);PE_StoreU32(0x8009D2C4u,0x80u);
    PE_StoreU32(0x8009D2DCu,(uint32_t)c[3]);PE_StoreU32(0x8009D2F4u,1u);
    for(i=0;i<2;i++) {
        pe_addr_t s=0x800B6980u+i*0x68u;
        PE_StoreU32(s+4u,(uint32_t)c[2]);PE_StoreU32(s+0x1Cu,(uint32_t)c[2]);
        PE_StoreU32(s+0x14u,0xFFFFFFu);PE_StoreU32(s+0x50u,0xAABBCCDDu);
    }
    PE_StoreU16(0x800B69D4u,7u);PE_StoreU16(0x800B6A3Cu,8u);
    for(i=0;i<10;i++) if(i!=6u && i!=5u) PE_StoreU32(0x800BCD50u+i*4u,(uint32_t)c[1]);
    for(i=0;i<48;i++) {
        pe_addr_t a=0x800B8AC0u+i*0x11Cu;
        PE_StoreU32(a+0xF0u,i%24u);PE_StoreU32(a+0xF4u,0x80000000u);
    }
    for(i=0;i<12;i++) {
        pe_addr_t a=0x800BC000u+i*0x11Cu;
        PE_StoreU32(a+0xF0u,i+12u);PE_StoreU32(a+0xF4u,0x40000000u);
        PE_StoreU32(a+0x28u,0x400u+i);
        PE_StoreU32(a+0x2Cu,i%3u==0u?0u:(i%3u==1u?0xFFu:0x2000000u));
        PE_StoreU32(a+0x38u,c[8]?0x100000u:0u);
        PE_StoreU32(a+0x50u,(uint32_t)(c[9]>=0?c[9]+i:c[9]));
        if(c[0]==0xA1 || c[0]==0xA9)PE_StoreU16(a+0xD8u,(uint16_t)(i*7919u+0x8000u));
    }
    for(i=0;i<16;i++)PE_StoreU8(0x800B2900u+i,(uint8_t)(i*17u+3u));
    for(i=0;i<112;i++)PE_StoreU8(0x800B8A20u+i,(uint8_t)(i+0x60u));
    PE_StoreU16(0x80148000u,c[6]==2 || c[6]==3?0xFFFFu:0u);
    PE_StoreU16(0x80148002u,c[6]==0 || c[6]==3?0xFFFFu:16u);
    PE_StoreU32(0x800B8628u,(uint32_t)c[0]);
    PE_StoreU32(0x800B862Cu,c[0]==0x24?0x80148000u:(c[0]==0xC0?0x1F3u:(uint32_t)c[5]));
    PE_StoreU32(0x800B8630u,(uint32_t)c[4]);PE_StoreU32(0x800B8634u,(uint32_t)c[10]);
    PE_StoreU32(0x800B8638u,(c[0]==0xC0||c[0]==0xC1||c[0]==0xC2||c[0]==0xA1)?(uint32_t)c[7]:0x7Fu);
    PE_StoreU32(0x800B863Cu,(uint32_t)c[5]);
    for(i=0;i<sizeof(ATK19_audio_callbacks)/sizeof(ATK19_audio_callbacks[0]);i++)
        PE_StoreU32(0x8009C0C0u+ATK19_audio_callbacks[i][0]*4u,ATK19_audio_callbacks[i][1]);
}

static void test_ATK19_retail_audio_commands(void)
{
    static const uint32_t ranges[][2]={{0xB6980,0xD0},{0xB8628,72},{0xB8A20,112},
        {0xB8AC0,0x42B8},{0x9D2C0,0x24},{0x9D2F4,4}};
    unsigned k;
    TEST("ATK19_retail_audio_commands");
    for(k=0;k<sizeof(ATK19_audio_cases)/sizeof(ATK19_audio_cases[0]);k++) {
        uint64_t hash;
        ATK19_SeedAudio(ATK19_audio_cases[k]);
        func_8008CA84();
        hash=hit_camera_hash(ranges,sizeof(ranges)/sizeof(ranges[0]));
        if(hash!=ATK19_audio_expected[k])fprintf(stderr,"audio %u: %016llX\n",k,(unsigned long long)hash);
        ASSERT(hash==ATK19_audio_expected[k],"audio FIFO/voice state differs from original instructions");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"audio command graph executes natively");
    }
    for(k=0;k<sizeof(DAY1_audio_fade_faults)/sizeof(DAY1_audio_fade_faults[0]);k++){
        int64_t c[]={0xA9,0xFFF000,0,0,127,DAY1_audio_fade_faults[k].duration,0,0,0,7,0};
        ATK19_SeedAudio(c);func_8008CA84();
        ASSERT(PE_Port_ShouldStop(),"A9 signed-zero duration must stop at original DIV");
        ASSERT(hit_camera_hash(ranges,sizeof(ranges)/sizeof(ranges[0]))==DAY1_audio_fade_faults[k].hash,"A9 BREAK prefix differs from original");
    }
    for(k=0;k<sizeof(DAY1_audio_selected_fade_faults)/sizeof(DAY1_audio_selected_fade_faults[0]);k++){
        int64_t c[]={0xA1,0xFFF000,0,0,DAY1_audio_selected_fade_faults[k].group,0x405,0,127,0,7,DAY1_audio_selected_fade_faults[k].duration};
        ATK19_SeedAudio(c);func_8008CA84();
        ASSERT(PE_Port_ShouldStop(),"A1 signed-zero duration must stop at original DIV");
        ASSERT(hit_camera_hash(ranges,sizeof(ranges)/sizeof(ranges[0]))==DAY1_audio_selected_fade_faults[k].hash,"A1 BREAK prefix differs from original");
    }
    PASS();
}

static void test_ATK19_audio_timer_preserves_battle_record(void)
{
    unsigned i,n;
    int event;
    TEST("ATK19_audio_timer_preserves_battle_record");
    ATK19_SeedAudio(ATK19_audio_cases[0]);
    HostFB_VSync(2);
    ASSERT(PE_LoadU32(0x8009D2F4u)==1u,"unregistered timer cannot consume");
    event=PE_Event_Open(0xF2000002u,2u,0x1000u,0x8008E23Cu);
    HostFB_VSync(2);
    ASSERT(PE_LoadU32(0x8009D2F4u)==1u,"disabled timer cannot consume");
    PE_Event_Enable(event);
    PE_StoreU32(0x8009D268u,1u);
    HostFB_VSync(2);
    ASSERT(PE_LoadU32(0x8009D2F4u)==1u,"producer owns the FIFO while filling it");
    PE_StoreU32(0x8009D268u,0u);
    func_80072714();
    HostFB_VSync(2);
    ASSERT(PE_LoadU32(0x8009D2F4u)==1u,"critical section defers audio commands");
    func_80072724();
    HostFB_VSync(-1);
    ASSERT(PE_LoadU32(0x8009D2F4u)==1u,"nonblocking counter query does not service timer");
    HostFB_VSync(2);
    ASSERT(PE_LoadU32(0x8009D2F4u)==0u,"enabled timer drains FIFO");
    for(n=0;n<80;n++) {
        PE_StoreU32(0x800BCD80u,0x80u);
        func_8008CBA8();
        PE_StoreU32(0x800BCD80u,0xC0u);
        PE_StoreU32(0x800BCD84u,n);
        PE_StoreU32(0x800BCD90u,0u);
        func_8008CBA8();
        PE_StoreU32(0x800BCD80u,0xF1u);
        func_8008CBA8();
        HostFB_VSync(2);
        ASSERT(PE_LoadU32(0x8009D2F4u)==0u,"commands do not accumulate across frames");
        ASSERT(PE_LoadU32(0x800B69C8u)==n*65536u,"middle command in each batch is applied");
        for(i=0;i<112;i++)
            ASSERT(PE_LoadU8(0x800B8A20u+i)==(uint8_t)(i+0x60u),"audio must preserve Aya's battle record");
    }
    PE_Sdk_ResetState();
    PE_StoreU32(0x8009D2F4u,1u);
    HostFB_VSync(2);
    ASSERT(PE_LoadU32(0x8009D2F4u)==1u,"reset revokes the previous event");
    PASS();
}

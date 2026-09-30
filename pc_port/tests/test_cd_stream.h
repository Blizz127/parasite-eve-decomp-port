#include "retail_cd_stream_cases.h"
#include "pe_cdreg.h"
static void test_DAY2_cd_stream(void)
{
    TEST("DAY2_cd_stream");
    for(unsigned k=0;k<sizeof(CDSTREAM_poll)/sizeof(CDSTREAM_poll[0]);k++) {
        static const uint32_t ranges[][2]={{0xA3460u,68u},{0x9B294u,3u},{0x140000u,16u}};
        ResetTestState();HostFB_Init();PE_GPU_Init();
        for(unsigned i=0;i<16;i++) {
            PE_StoreU8(0x80140000u+i,0xCCu);
            PE_StoreU8(0x800A3468u+i,(uint8_t)(37u+i*17u));
        }
        PE_StoreU8(0x8009B296u,(uint8_t)CDSTREAM_poll[k].hi);
        PE_StoreU8(0x8009B295u,(uint8_t)CDSTREAM_poll[k].lo);
        ASSERT((uint32_t)func_8007A488(CDSTREAM_poll[k].mode,CDSTREAM_poll[k].dst)==CDSTREAM_poll[k].result,"stream poll result priority");
        ASSERT(hit_camera_hash(ranges,3)==CDSTREAM_poll[k].hash,"stream poll RAM differs from original");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"stream poll unexpected boundary");
    }
    for(unsigned k=0;k<sizeof(CDSTREAM_cases)/sizeof(CDSTREAM_cases[0]);k++) {
        uint64_t hash;
        pe_addr_t incoming;
        ResetTestState();HostFB_Init();PE_GPU_Init();
        for(unsigned j=0;j<sizeof(CDSTREAM_ranges)/sizeof(CDSTREAM_ranges[0]);j++)
            for(unsigned i=0;i<CDSTREAM_ranges[j][1];i++)
                PE_StoreU8(0x80000000u+CDSTREAM_ranges[j][0]+i,j==14u?0xCCu:0u);
        for(unsigned i=0;i<sizeof(CDSTREAM_seed_addresses)/sizeof(CDSTREAM_seed_addresses[0]);i++)
            PE_StoreU32(0x80000000u+CDSTREAM_seed_addresses[i],CDSTREAM_cases[k].init[i]);
        PE_StoreU32(0x800C0DC8u,0x80150000u);
        for(unsigned i=0;i<8;i++) PE_StoreU16(0x80150000u+i*32u,4u);
        PE_StoreU16(0x80150000u,(uint16_t)CDSTREAM_cases[k].first);
        PE_StoreU16(0x80150000u+CDSTREAM_cases[k].init[9]*32u,(uint16_t)CDSTREAM_cases[k].occupied);
        for(unsigned i=0;i<6144;i++) PE_StoreU8(0x80160000u+i,(uint8_t)(CDSTREAM_cases[k].seed+i*17u));
        incoming=0x80160000u+CDSTREAM_cases[k].init[4]*2048u;
        for(unsigned i=0;i<5;i++) PE_StoreU16(incoming+i*2u,(uint16_t)CDSTREAM_cases[k].header[i]);
        PE_StoreU32(0x8009B27Cu,0x1F801800u);PE_StoreU32(0x8009B32Cu,0x1F801800u);
        PE_StoreU32(0x8009B280u,0x1F801801u);PE_StoreU32(0x8009B284u,0x1F801802u);
        PE_StoreU32(0x8009B288u,0x1F801803u);
        PE_StoreU32(0x8009B338u,0x1F801803u);PE_StoreU32(0x8009B33Cu,0x1F801018u);
        PE_StoreU32(0x8009B340u,0x1F801020u);PE_StoreU32(0x8009B35Cu,0x1F8010B8u);
        /* Busy MDEC fixtures redirect the same table entry as the original
         * interpreter. Idle fixtures read the actual MDEC register owner. */
        PE_StoreU32(0x8009B34Cu,CDSTREAM_cases[k].busy?0x8013001Cu:0x1F801098u);
        PE_StoreU32(0x8013001Cu,CDSTREAM_cases[k].busy?0x01000000u:0u);
        PE_StoreU32(0x801FFE88u,0x55667788u);
        func_8007C564();
        hash=hit_camera_hash(CDSTREAM_ranges,sizeof(CDSTREAM_ranges)/sizeof(CDSTREAM_ranges[0]));
        if(hash!=CDSTREAM_cases[k].hash)fprintf(stderr,"stream case%u status%u hash%016llX expected%016llX\n",k,PE_LoadU32(0x8009B374u),(unsigned long long)hash,(unsigned long long)CDSTREAM_cases[k].hash);
        ASSERT(hash==CDSTREAM_cases[k].hash,"stream assembly RAM differs from original");
        ASSERT(PE_CdLoadU32(0x1F801018u)==CDSTREAM_cases[k].bus &&
               PE_CdLoadU32(0x1F801020u)==CDSTREAM_cases[k].mailbox &&
               PE_CdLoadU8(0x1F801803u)==CDSTREAM_cases[k].request,"stream register writes differ");
        if(CDSTREAM_cases[k].boundary) {
            const char *names[]={"", "func_8007C564_sector_fifo","func_8007CEAC","func_8007AAB4","func_8007C564_auxiliary_callback"};
            ASSERT(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
                CountOrderLog(names[CDSTREAM_cases[k].boundary])==1,"stream unsupported callee was not stopped");
        } else ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"stream RAM path unexpected boundary");
    }
    PASS();
}

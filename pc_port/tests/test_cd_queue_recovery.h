#include "retail_cd_queue_recovery_cases.h"
static void CdRecoverySeed(void)
{
    for(unsigned j=0;j<sizeof(CDREC_ranges)/sizeof(CDREC_ranges[0]);j++)
        for(unsigned i=0;i<CDREC_ranges[j][1];i++)
            PE_StoreU8(0x80000000u+CDREC_ranges[j][0]+i,(uint8_t)(37u+i*17u));
    for(unsigned i=0;i<8;i++) PE_StoreU8(0x80140000u+i,(uint8_t)(165u+i*17u));
}
static void test_DAY2_cd_queue_recovery(void)
{
    TEST("DAY2_cd_queue_recovery");
    for(unsigned k=0;k<sizeof(CDREC_writers)/sizeof(CDREC_writers[0]);k++) {
        ResetTestState();CdRecoverySeed();
        PE_StoreU32(0x800A3690u,CDREC_writers[k].ring);
        func_8007EB88(0x12345678u,CDREC_writers[k].status,CDREC_writers[k].null?0u:0x80140000u);
        ASSERT(hit_camera_hash(CDREC_ranges,4)==CDREC_writers[k].hash,"completion writer differs from original");
    }
    for(unsigned k=0;k<sizeof(CDREC_cases)/sizeof(CDREC_cases[0]);k++) {
        static const uint32_t mixed[]={0,0,11,11,0,22,0,0};
        uint64_t hash;
        ResetTestState();CdRecoverySeed();
        PE_StoreU32(0x800A3600u,CDREC_cases[k].head);PE_StoreU32(0x800A3604u,5u);
        PE_StoreU32(0x800A3608u,CDREC_cases[k].count);PE_StoreU32(0x800A3690u,CDREC_cases[k].ring);
        for(unsigned i=0;i<8;i++) {
            uint32_t seq=CDREC_cases[k].pattern==0u?11u:CDREC_cases[k].pattern==1u?mixed[i]:1u+i%2u;
            pe_addr_t cb=CDREC_cases[k].callbacks==0u?0u:CDREC_cases[k].callbacks==1u || i==0u?0x800813E8u:0x80170000u;
            pe_addr_t addr=0x800A3540u+((CDREC_cases[k].head+i)%8u)*24u;
            PE_StoreU32(addr,seq);PE_StoreU32(addr+16u,cb);
        }
        PE_StoreU32(0x800B89F4u,0u);PE_StoreU32(0x800A801Cu,1u);
        PE_StoreU32(0x8009B34Cu,0x80130000u);PE_StoreU32(0x80130000u,0x01000000u);
        PE_StoreU32(0x800C0DB8u,0x80160000u);PE_StoreU32(0x800BCD7Cu,0xFFFFFFFFu);
        func_8007E704(0x105u,CDREC_cases[k].null?0u:0x80140000u);
        hash=hit_camera_hash(CDREC_ranges,4);
        if(hash!=CDREC_cases[k].hash)fprintf(stderr,"queue recovery case%u hash%016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)CDREC_cases[k].hash);
        ASSERT(hash==CDREC_cases[k].hash,"queue recovery differs from original");
        if(CDREC_cases[k].boundary) {
            ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_data_callback")==1 &&
                g_bootstrap_arg4_calls[0].target==0x80170000u &&
                g_bootstrap_arg4_calls[0].arg0==CDREC_cases[k].arg0 &&
                g_bootstrap_arg4_calls[0].arg1==CDREC_cases[k].arg1,"recovery callback boundary contract");
        } else ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"queue recovery unexpected boundary");
    }
    ResetTestState();CdRecoverySeed();PE_StoreU32(0x800A3608u,9u);
    { uint64_t before=hit_camera_hash(CDREC_ranges,4);
      func_8007E704(5u,0u);
      ASSERT(PE_Port_ShouldStop() && CountOrderLog("func_8007E704_queue_count")==1 &&
        hit_camera_hash(CDREC_ranges,4)==before,"corrupt queue count must stop before host callback-array overflow"); }
    PASS();
}

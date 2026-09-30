#include "retail_cd_queue_completion_cases.h"
static void CdQueueSeed(uint32_t head,uint32_t count,uint32_t pattern,uint32_t current,uint32_t retries,uint32_t cb,uint32_t gcb,uint32_t lane)
{
    static const pe_addr_t targets[]={0u,0x800813E8u,0x80170000u};
    for(unsigned j=0;j<5;j++) for(unsigned i=0;i<CDQUEUE_ranges[j][1];i++)
        PE_StoreU8(0x80000000u+CDQUEUE_ranges[j][0]+i,(uint8_t)(37u+i*17u));
    PE_StoreU32(0x800A3600u,head);PE_StoreU32(0x800A3604u,current);PE_StoreU32(0x800A3608u,count);PE_StoreU32(0x800A3690u,7u);
    for(unsigned i=0;i<8;i++) {
        pe_addr_t a=0x800A3540u+((head+i)%8u)*24u;
        uint32_t seq=pattern==0u?0u:pattern==1u || i<2u?11u:22u+i;
        PE_StoreU32(a,seq);PE_StoreU8(a+4u,1u);PE_StoreU32(a+12u,0x80140000u);
        PE_StoreU32(a+16u,targets[cb]);PE_StoreU32(a+20u,retries);
    }
    PE_StoreU32(0x800B8AB0u,gcb==2u?0x80170010u:targets[gcb]);
    PE_StoreU32(0x8009B574u,lane);PE_StoreU32(0x8009B598u,1u);PE_StoreU32(0x8009B554u,1u);
    PE_StoreU32(0x800B89F4u,0u);PE_StoreU32(0x800A801Cu,1u);PE_StoreU32(0x8009B34Cu,0x80130000u);PE_StoreU32(0x80130000u,0x01000000u);
    PE_StoreU32(0x800C0DB8u,0x80160000u);PE_StoreU32(0x800BCD7Cu,0xFFFFFFFFu);
    for(unsigned i=0;i<8;i++) PE_StoreU8(0x80140000u+i,(uint8_t)(165u+i*17u));
}
static void test_DAY2_cd_queue_completion(void)
{
    TEST("DAY2_cd_queue_completion");
    for(unsigned k=0;k<sizeof(CDQUEUE_removals)/sizeof(CDQUEUE_removals[0]);k++) {
        ResetTestState();CdQueueSeed(CDQUEUE_removals[k].head,CDQUEUE_removals[k].count,CDQUEUE_removals[k].pattern,5u,0u,0u,0u,2u);
        func_8007E5C4();
        ASSERT(hit_camera_hash(CDQUEUE_ranges,5)==CDQUEUE_removals[k].hash,"queue removal differs from original");
    }
    for(unsigned k=0;k<sizeof(CDQUEUE_cases)/sizeof(CDQUEUE_cases[0]);k++) {
        ResetTestState();CdQueueSeed(CDQUEUE_cases[k].head,CDQUEUE_cases[k].count,CDQUEUE_cases[k].pattern,CDQUEUE_cases[k].current,CDQUEUE_cases[k].retries,CDQUEUE_cases[k].cb,CDQUEUE_cases[k].gcb,CDQUEUE_cases[k].lane);
        func_8007E964(CDQUEUE_cases[k].status,CDQUEUE_cases[k].null?0u:0x80140000u);
        uint64_t hash=hit_camera_hash(CDQUEUE_ranges,5);
        if(hash!=CDQUEUE_cases[k].hash) fprintf(stderr,"CD queue case%u hash%016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)CDQUEUE_cases[k].hash);
        ASSERT(hash==CDQUEUE_cases[k].hash,"queue completion differs from original");
        if(CDQUEUE_cases[k].boundary) {
            ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_data_callback")==1 &&
                g_bootstrap_arg4_calls[0].target==CDQUEUE_cases[k].boundary &&
                g_bootstrap_arg4_calls[0].arg0==CDQUEUE_cases[k].arg0 &&
                g_bootstrap_arg4_calls[0].arg1==CDQUEUE_cases[k].arg1,"queue callback boundary contract");
        } else ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"queue completion unexpected boundary");
    }
    PASS();
}

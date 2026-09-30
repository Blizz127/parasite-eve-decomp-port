#include "retail_cd_dispatch_cases.h"
static void test_DAY2_cd_dispatch(void)
{
    TEST("DAY2_cd_dispatch");
    for(unsigned k=0;k<sizeof(CDDISP_cases)/sizeof(CDDISP_cases[0]);k++) {
        unsigned kind=CDDISP_cases[k].kind;
        uint8_t payload[8]={2,0x80,0x20,0x82,0,0,0,0};
        uint64_t hash;
        ResetTestState();HostFB_Init();PE_GPU_Init();B558_PlantPointers();
        for(unsigned j=0;j<sizeof(CDDISP_ranges)/sizeof(CDDISP_ranges[0]);j++)
            for(unsigned i=0;i<CDDISP_ranges[j][1];i++)
                PE_StoreU8(0x80000000u+CDDISP_ranges[j][0]+i,CDDISP_ranges[j][0]==0x150000u?0xCCu:0u);
        for(unsigned i=0;i<sizeof(CDDISP_seeds)/sizeof(CDDISP_seeds[0]);i++)
            PE_StoreU32(0x80000000u+CDDISP_seeds[i][0],CDDISP_seeds[i][1]);
        for(unsigned i=0;i<5;i++) PE_StoreU32(0x80011B8Cu+i*4u,CDACK_jumps[i]);
        for(unsigned i=0;i<8;i++) PE_StoreU16(0x80150000u+i*32u,0u);
        for(unsigned i=0;i<2048;i++) PE_StoreU8(0x80160000u+i,(uint8_t)(CDDISP_cases[k].seed+i*17u));
        PE_StoreU16(0x80160000u,0x160u);PE_StoreU16(0x80160002u,3u<<10u);
        PE_StoreU16(0x80160004u,0u);PE_StoreU16(0x80160006u,1u);PE_StoreU16(0x80160008u,7u);
        if(kind==1) PE_StoreU16(0x80160006u,2u);
        if(kind==2) PE_StoreU32(0x800B89F4u,1u);
        if(kind==3) PE_StoreU32(0x8009B554u,0u);
        if(kind==4) PE_StoreU32(0x800B8AB4u,0u);
        if(kind==5) PE_StoreU32(0x8009AFB8u,0u);
        if(kind==6) PE_StoreU16(0x80160000u,0x161u);
        if(kind==7) payload[0]=16u;
        if(kind==8) PE_StoreU32(0x800B8AB4u,0x80170000u);
        if(kind==10) PE_StoreU32(0x8009AFB4u,0x80080164u);
        if(kind==11) PE_StoreU32(0x8009B624u,0u);
        if(kind==12) PE_StoreU32(0x8009B624u,4u);
        PE_StoreU32(0x8009B32Cu,0x1F801800u);PE_StoreU32(0x8009B338u,0x1F801803u);
        PE_StoreU32(0x8009B33Cu,0x1F801018u);PE_StoreU32(0x8009B340u,0x1F801020u);
        PE_StoreU32(0x8009B35Cu,0x1F8010B8u);PE_StoreU32(0x8009B34Cu,0x1F801098u);
        PE_StoreU32(0x801FFE88u,0x55667788u);
        PE_CdReg_WriteU8(0x1F801800u,(uint8_t)CDDISP_cases[k].index);
        ASSERT(PE_CdReg_PushResponse((uint8_t)CDDISP_cases[k].tag,payload,CDDISP_cases[k].size),"dispatch response ingress");
        /* Enter through the existing CPU IRQ service, with a real source2
         * assertion and a registered original callback identity. */
        PE_StoreU16(0x800945E4u,1u);PE_StoreU16(0x80094614u,0u);
        PE_StoreU32(0x800945F0u,0u);(void)PE_IRQ_ExchangeMask(0u);
        ASSERT(func_80073CC4(2u,0x8007C13Cu)==0u && !PE_Port_ShouldStop(),"CD source registration");
        ASSERT(PE_LoadU32(0x800945F0u)==0x8007C13Cu &&
            PE_LoadU16(0x80094614u)==4u && PE_IRQ_GetMask()==4u,"CD registered mask/table");
        PE_IRQ_AssertSources(4u);
        ASSERT(PE_IRQ_ServicePendingForGeneration(PE_IRQ_Generation())==
            (CDDISP_cases[k].boundary?PE_IRQ_SERVICE_BOUNDARY:PE_IRQ_SERVICE_RETURNED),"CD CPU IRQ result");
        hash=hit_camera_hash(CDDISP_ranges,sizeof(CDDISP_ranges)/sizeof(CDDISP_ranges[0]));
        if(hash!=CDDISP_cases[k].hash)fprintf(stderr,"CD dispatch case%u kind%u hash%016llX expected%016llX\n",k,kind,(unsigned long long)hash,(unsigned long long)CDDISP_cases[k].hash);
        ASSERT(hash==CDDISP_cases[k].hash,"CD callback chain differs from original");
        ASSERT(!(PE_IRQ_ReadStatus()&4u),"CD CPU source was not acknowledged");
        if(CDDISP_cases[k].boundary) {
            ASSERT(PE_Port_ShouldStop() && PE_LoadU16(0x800945E6u)==1u,"CD boundary incorrectly ran IRQ cleanup");
            if(CDDISP_cases[k].boundary==0x8007E704u)
                ASSERT(CountOrderLog("func_8007E704")==1 && g_bootstrap_arg_calls[0].arg0==5u,"CD error recovery boundary");
            else ASSERT(CountOrderLog("CD_data_callback")==1 &&
                g_bootstrap_arg4_calls[0].target==CDDISP_cases[k].boundary &&
                g_bootstrap_arg4_calls[0].arg0==CDDISP_cases[k].arg0 &&
                g_bootstrap_arg4_calls[0].arg1==CDDISP_cases[k].arg1,"CD unknown callback contract");
        } else {
            ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count && !PE_LoadU16(0x800945E6u),"CD IRQ normal completion");
            ASSERT((PE_CdReg_ReadU8(0x1F801800u)&3u)==CDDISP_cases[k].index,"CD register bank not restored");
        }
    }
    PASS();
}

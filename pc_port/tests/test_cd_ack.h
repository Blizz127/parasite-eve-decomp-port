#include "retail_cd_ack_cases.h"
static void test_DAY2_cd_ack(void)
{
    TEST("DAY2_cd_ack");
    for(unsigned k=0;k<sizeof(CDACK_cases)/sizeof(CDACK_cases[0]);k++) {
        uint8_t payload[16];
        ResetTestState();B558_PlantPointers();
        for(unsigned j=0;j<3;j++)
            for(unsigned i=0;i<CDACK_ranges[j][1];i++)
                PE_StoreU8(0x80000000u+CDACK_ranges[j][0]+i,(uint8_t)(37u+i*17u));
        PE_StoreU32(0x8009AFC4u,CDACK_cases[k].old);PE_StoreU32(0x8009AFCCu,0xFFFFFFFFu);
        PE_StoreU32(0x8009AFC0u,CDACK_cases[k].debug);PE_StoreU8(0x8009AFD5u,2u);
        PE_StoreU32(0x8009B184u,CDACK_cases[k].table);PE_StoreU32(0x8009B084u,CDACK_cases[k].table);
        for(unsigned i=0;i<5;i++) PE_StoreU32(0x80011B8Cu+i*4u,CDACK_jumps[i]);
        if(CDACK_cases[k].dirty) PE_StoreU32(0x80011B94u,0x80170000u);
        for(unsigned i=0;i<16;i++) payload[i]=(uint8_t)(i?37u+i*17u:CDACK_cases[k].first);
        if(CDACK_cases[k].tag)
            ASSERT(PE_CdReg_PushResponse((uint8_t)CDACK_cases[k].tag,payload,CDACK_cases[k].size),"CD response ingress failed");
        ASSERT((uint32_t)func_8007AAB4()==CDACK_cases[k].result,"CD acknowledge result differs");
        ASSERT(hit_camera_hash(CDACK_ranges,3)==CDACK_cases[k].hash,"CD acknowledge RAM differs from original");
        if(CDACK_cases[k].boundary) {
            const char *names[]={"","func_80071A74","func_80073C5C","func_8007AAB4_jump_table"};
            ASSERT(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
                CountOrderLog(names[CDACK_cases[k].boundary])==1,"CD acknowledge diagnostic boundary differs");
        } else ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"CD acknowledge unexpected boundary");
        PE_CdReg_WriteU8(0x1F801800u,1u);
        ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==0u,"CD interrupt was not acknowledged");
        if(CDACK_cases[k].size>CDACK_cases[k].consumed)
            ASSERT(!PE_CdReg_PushResponse(1u,payload,1u),"unread response was overwritten");
        for(unsigned i=CDACK_cases[k].consumed;i<CDACK_cases[k].size;i++) {
            ASSERT(PE_CdReg_ReadU8(0x1F801800u)&0x20u,"remaining response lost FIFO-ready bit");
            ASSERT(PE_CdReg_ReadU8(0x1F801801u)==payload[i],"remaining FIFO bytes differ");
        }
        ASSERT(!(PE_CdReg_ReadU8(0x1F801800u)&0x20u),"response FIFO did not drain");
    }
    /* Exercise the real nested acknowledge loop and subsequent status copy. */
    ResetTestState();HostFB_Init();PE_GPU_Init();B558_PlantPointers();
    for(unsigned i=0;i<5;i++) PE_StoreU32(0x80011B8Cu+i*4u,CDACK_jumps[i]);
    PE_StoreU16(0x800945E6u,1u);
    { uint8_t response=0x1Du;
      ASSERT(PE_CdReg_PushResponse(1u,&response,1u),"data-ready response ingress"); }
    ASSERT(func_8007B290(1u,0x80140000u)==1 && !PE_Port_ShouldStop(),"real data-ready poll failed");
    ASSERT(PE_LoadU8(0x80140000u)==0x1Du && !PE_LoadU8(0x8009B295u),"data-ready response consumption failed");
    for(unsigned i=1;i<8;i++) ASSERT(!PE_LoadU8(0x80140000u+i),"short response was not zero padded");
    ASSERT(func_8007B290(1u,0x80140000u)==0 && !PE_Port_ShouldStop(),"acknowledged response was replayed");
    ASSERT(!PE_CdReg_PushResponse(0u,NULL,0u) && !PE_CdReg_PushResponse(1u,NULL,1u) &&
        !PE_CdReg_PushResponse(1u,(const uint8_t *)"x",17u),"invalid response ingress accepted");
    PASS();
}

/* B290's retail ready/sync jalr sites, exercised through real CD responses.
 * No disc data or CPU interpreter. Low-RAM printf formats are synthetic
 * witnesses: callback a0 is a byte status, a1 is the guest response address. */
static void CDSYNC_Seed(unsigned tag, pe_addr_t ready, pe_addr_t sync)
{
    uint8_t payload[8]={0,0xE2,0x37,0x49,0x53,0x61,0x72,0x83};
    ResetTestState();HostFB_Init();PE_GPU_Init();B558_PlantPointers();
    PE_Bios_PrintfLogReset();
    for(unsigned i=0;i<5;i++) PE_StoreU32(0x80011B8Cu+i*4u,CDACK_jumps[i]);
    PE_StoreU16(0x800945E6u,1u);
    PE_StoreU32(0x8009AFB8u,ready);PE_StoreU32(0x8009AFB4u,sync);
    PE_StoreU32(0x8009B624u,2u); /* callback reads payload[1] */
    PE_StoreU32(0x8009B598u,0x12345678u);PE_StoreU32(0x8009B59Cu,0x87654321u);
    PE_CdReg_WriteU8(0x1F801800u,2u);
    /* Known valid payload, checked by the enclosing test's status/buffer. */
    (void)PE_CdReg_PushResponse((uint8_t)tag,payload,sizeof(payload));
}

static void test_CDSYNC_installed_callbacks(void)
{
    TEST("CDSYNC_installed_callbacks");
    for(unsigned tag=1;tag<=5;tag++) {
        if(tag==3 || tag==4) continue;
        CDSYNC_Seed(tag,0x80080778u,0x80080164u); /* CdInit identities */
        int result=func_8007B290(1u,0x80140000u);
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,
               "CdInit callback stopped or used a stub");
        ASSERT(result==(tag==2?0:tag==5?5:1),"poll return after callback differs");
        ASSERT(PE_LoadU8(0x8009B564u)==0 && PE_LoadU8(0x8009B565u)==0xE2u,
               "installed callback did not consume response");
        ASSERT(PE_LoadU8(0x8009B56Cu)==(tag==5?0:0xE2u),
               "callback did not receive correct status/response");
        if(tag!=1) {
            ASSERT(!PE_LoadU32(0x8009B598u) && !PE_LoadU32(0x8009B59Cu) &&
                   PE_LoadU32(0x8009B570u)==33u,"sync callback tail missing");
        } else ASSERT(PE_LoadU32(0x8009B598u)==0x12345678u,
                      "ready-only event incorrectly called sync hook");
        if(tag==5) ASSERT(PE_LoadU32(0x8009B574u)==1u &&
                          PE_LoadU32(0x8009B578u)==11u,"INT5 sync handling missing");
        ASSERT((PE_CdReg_ReadU8(0x1F801800u)&3u)==2u,"bank was not restored");
        ASSERT(!PE_LoadU8(0x8009B295u),"ready status was not consumed");
        if(tag!=2) {
            ASSERT(PE_LoadU8(0x80140001u)==0xE2u && PE_LoadU8(0x80140007u)==0x83u,
                   "poll result buffer not copied after callback");
            ASSERT(func_8007B290(1u,0u)==0 && !PE_Port_ShouldStop(),"event replayed");
        }
    }
    PASS();
}

static void test_CDSYNC_registry_arguments(void)
{
    TEST("CDSYNC_registry_arguments");
    for(unsigned tag=1;tag<=2;tag++) {
        pe_addr_t fmt;uint32_t args[4];
        CDSYNC_Seed(tag,tag==1?0x80071A74u:0u,tag==2?0x80071A74u:0u);
        /* Physical low RAM aliases the guest address represented by status. */
        PE_StoreU8(0x80000000u+tag,'%');PE_StoreU8(0x80000001u+tag,'x');
        PE_StoreU8(0x80000002u+tag,0);
        ASSERT(func_8007B290(1u,0u)==(tag==1?1:0) && !PE_Port_ShouldStop(),
               "registered callback did not return to poll");
        ASSERT(PE_Bios_PrintfLog(&fmt,args)==1u && fmt==tag &&
               args[0]==(tag==1?0x800A3468u:0x800A3460u),
               "registry callback did not receive (status, response)");
        ASSERT(!g_stub_order_count,"registry callback used a stub");
    }
    PE_Bios_PrintfLogReset();
    PASS();
}

static void test_CDSYNC_unknown_callbacks(void)
{
    TEST("CDSYNC_unknown_callbacks");
    for(unsigned tag=1;tag<=2;tag++) {
        CDSYNC_Seed(tag,tag==1?0x80012222u:0u,tag==2?0x80012222u:0u);
        PE_StoreU8(0x80140000u,0xCCu);
        ASSERT(func_8007B290(1u,0x80140000u)==0 &&
               PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY,
               "unknown callback did not stop");
        ASSERT(CountOrderLog("CD_data_callback")==1 && g_bootstrap_arg4_call_count==1 &&
               g_bootstrap_arg4_calls[0].target==0x80012222u &&
               g_bootstrap_arg4_calls[0].arg0==tag &&
               g_bootstrap_arg4_calls[0].arg1==(tag==1?0x800A3468u:0x800A3460u),
               "unknown callback boundary arguments differ");
        ASSERT(PE_LoadU8(0x80140000u)==0xCCu,"stop ran result-copy tail");
    }
    PASS();
}

static void test_CDSYNC_nested_stop(void)
{
    TEST("CDSYNC_nested_stop");
    CDSYNC_Seed(5u,0x80080778u,0x80080164u);
    PE_StoreU32(0x8009B554u,1u);PE_StoreU32(0x800A36A8u,0x80012222u);
    PE_StoreU8(0x80140000u,0xCCu);
    ASSERT(func_8007B290(1u,0x80140000u)==0 && PE_Port_ShouldStop(),
           "nested ready callback stop did not propagate");
    ASSERT(PE_LoadU32(0x8009B598u)==0x12345678u &&
           PE_LoadU32(0x8009B59Cu)==0x87654321u && PE_LoadU8(0x80140000u)==0xCCu,
           "nested stop continued into sync callback or poll tail");
    ASSERT(CountOrderLog("CD_data_callback")==1,"nested stop boundary missing");
    PASS();
}

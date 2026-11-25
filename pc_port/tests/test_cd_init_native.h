/* Real-engine callback-map failure, quit-poll and requested-stop contracts. */
static void CDINIT_SeedHost(void)
{
    ResetTestState();HostFB_Init();PE_GPU_Init();PE_Callback_Init();B558_PlantPointers();
    PE_StoreU16(0x800945E4u,1u);PE_StoreU16(0x800945E6u,0u);
    PE_StoreU16(0x80094614u,0u);PE_StoreU32(0x8009AFC0u,0u);
    PE_StoreU32(0x8009B290u,0x1F801C00u);PE_CdReg_WriteU8(0x1F801803u,0u);
}
static void CDINIT_ConflictingHandler(void) { }
static void test_CDINIT_native_binding_conflict(void)
{
    TEST("CDINIT_native_binding_conflict");
    CDINIT_SeedHost();
    ASSERT(PE_Callback_Bind(0x8007FE24u,CDINIT_ConflictingHandler)==0,"test registry seed failed");
    int result=func_8007F994();
    ASSERT(result==0 && PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "initializer ignored host callback binding conflict");
    ASSERT(PE_LoadU32(0x8009AFB4u)==0x80080164u && PE_LoadU32(0x8009AFB8u)==0x80080778u,
           "failure happened before the callback binding stage");
    ASSERT(PE_Callback_Bind(0x8007FE24u,CDINIT_ConflictingHandler)==0,
           "failed initializer replaced the existing host binding");
    PASS();
}
static int CDINIT_Quit(void) { return 1; }
static void test_CDINIT_native_stop(void)
{
    TEST("CDINIT_native_stop");
    for (int requested=0; requested<2; requested++) {
        CDINIT_SeedHost();
        PE_StoreU32(0x800A36A8u,0x12345678u);
        PE_StoreU32(0x8009AFB4u,0x76543210u);
        if (requested) PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
        else PE_Port_SetQuitPoll(CDINIT_Quit);
        int result=func_8007F994();
        PE_Port_SetQuitPoll(NULL);
        /* Real sync clears AFB4 before returning stopped; initializer's later
         * A36A8 clear and AFB4 callback installation must not execute. */
        ASSERT(result==0 && PE_Port_GetStopReason()==PE_PORT_STOP_HOST_QUIT &&
               PE_LoadU32(0x800A36A8u)==0x12345678u && PE_LoadU32(0x8009AFB4u)==0u,
               "stopped sync allowed later initializer callback writes");
    }
    PASS();
}

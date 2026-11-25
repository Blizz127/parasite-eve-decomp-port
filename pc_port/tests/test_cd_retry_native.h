/* Retry integration through the actual CD register/command providers. */
static void test_CDRETRY_native_issue(void)
{
    TEST("CDRETRY_native_issue");
    for (unsigned i=0;i<2;i++) {
        ResetTestState(); B558_PlantChain();
        PE_StoreU8(0x8009B558u,9u);
        PE_StoreU32(0x8009B560u,B558_D);
        PE_StoreU32(0x8009B59Cu,0xFFFFFFFFu);
        PE_StoreU32(0x8009B598u,0x12345678u);
        PE_StoreU32(0x8009B5A4u+9u*4u,i ? 0x80000000u : 0u);
        func_800800F4();
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x8009B59Cu)==0u &&
               PE_LoadU32(0x8009B598u)==(i ? 960u : 30u),
               "retry counter wrap/whole-word timeout selection differs");
        ASSERT(PE_LoadU8(0x8009AFD5u)==9u && PE_CdReg_ReadU8(PE_CDREG_BASE+1u)==9u,
               "retry did not reach the real command issuer");
    }
    PASS();
}

static int CDRETRY_QuitAfterFlush(void)
{
    return PE_LoadU8(0x8009B294u)==2u;
}

static void test_CDRETRY_stop_after_flush(void)
{
    TEST("CDRETRY_stop_after_flush");
    ResetTestState(); B558_PlantPointers();
    PE_StoreU8(0x8009B294u,0u);
    PE_StoreU32(0x8009B59Cu,0x11223344u);
    PE_StoreU32(0x8009B598u,0x55667788u);
    PE_Port_SetQuitPoll(CDRETRY_QuitAfterFlush);
    func_800800F4();
    PE_Port_SetQuitPoll(NULL);
    ASSERT(PE_Port_GetStopReason()==PE_PORT_STOP_HOST_QUIT &&
           PE_LoadU8(0x8009B294u)==2u &&
           PE_LoadU32(0x8009B59Cu)==0x11223344u &&
           PE_LoadU32(0x8009B598u)==0x55667788u,
           "flush stop allowed retry state changes");
    PASS();
}

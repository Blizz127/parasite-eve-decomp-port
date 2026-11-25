/* Real-engine checks for independent status helpers and retained boundaries. */
static void test_CDSTATUS_redirected_table(void)
{
    TEST("CDSTATUS_redirected_table");
    ResetTestState();
    PE_StoreU32(0x800A36A4u,0u);
    PE_StoreU32(0x8009B558u,0xAB12CD04u);
    PE_StoreU32(0x8009B55Cu,0x12345678u);
    PE_StoreU32(0x8009B580u,0x87654321u);
    PE_StoreU32(0x8009B584u,0xABCD9876u);
    PE_StoreU32(0x80011D04u+4u*4u,0x80080318u);
    pe_plat_cd_command_complete(2u,0x80140000u);
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x8009B580u)==0x12CD4321u &&
           PE_LoadU32(0x8009B584u)==0xABCD78ABu,
           "redirected table must select the location-copy handler");
    PE_StoreU32(0x80011D04u+4u*4u,0x80080340u);
    pe_plat_cd_command_complete(2u,0x80140000u);
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU8(0x8009B586u)==4u &&
           PE_LoadU32(0x8009B574u)==1u && PE_LoadU32(0x8009B578u)==11u,
           "redirected table must select the command-latch handler");
    PASS();
}
static void test_CDSTATUS_unknown_table_stops(void)
{
    TEST("CDSTATUS_unknown_table_stops");
    ResetTestState();
    PE_StoreU32(0x800A36A4u,0u);PE_StoreU8(0x8009B56Cu,0u);
    PE_StoreU8(0x8009B558u,4u);PE_StoreU32(0x8009B570u,31u);
    PE_StoreU32(0x8009B598u,0x12345678u);PE_StoreU32(0x8009B59Cu,0x87654321u);
    PE_StoreU32(0x80011D04u+4u*4u,0xDEADBEEFu);
    func_80080164(2u,0x80140000u);
    ASSERT(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           PE_LoadU32(0x8009B570u)==31u && !PE_LoadU32(0x8009B598u) &&
           !PE_LoadU32(0x8009B59Cu),"table boundary lost or callback continued after stop");
    ASSERT(CountOrderLog("CD_command_table")==1 &&
           g_bootstrap_arg4_calls[0].target==0xDEADBEEFu &&
           g_bootstrap_arg4_calls[0].arg0==2u &&
           g_bootstrap_arg4_calls[0].arg1==0x80140000u,
           "table boundary target or arguments changed");
    PASS();
}
static void test_CDSTATUS_mode_change_bit(void)
{
    TEST("CDSTATUS_mode_change_bit");
    for(unsigned i=0;i<2;i++) {
        ResetTestState();PE_StoreU8(0x8009B558u,14u);
        PE_StoreU8(0x8009B581u,0u);PE_StoreU8(0x8009B559u,i?0x80u:0x20u);
        PE_StoreU32(0x8009B574u,0x55u);PE_StoreU32(0x8009B578u,0x66u);
        PE_StoreU32(0x8009B594u,0x77u);
        pe_plat_cd_unexpected_status(2u,0x80140000u);
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU8(0x8009B581u)==(i?0x80u:0x20u) &&
               PE_LoadU32(0x8009B574u)==(i?2u:0x55u) &&
               PE_LoadU32(0x8009B578u)==(i?15u:0x66u) &&
               PE_LoadU32(0x8009B594u)==(i?3u:0x77u),
               "mode change must test bit80, not bit20");
    }
    PASS();
}
static void test_CDSTATUS_stopped_entry(void)
{
    TEST("CDSTATUS_stopped_entry");
    ResetTestState();PE_StoreU32(0x8009B570u,31u);
    PE_StoreU32(0x8009B598u,0x12345678u);PE_StoreU32(0x8009B59Cu,0x87654321u);
    PE_Port_RequestStop(PE_PORT_STOP_EXPLICIT);
    func_80080164(2u,0x80140000u);
    ASSERT(PE_LoadU32(0x8009B570u)==31u && PE_LoadU32(0x8009B598u)==0x12345678u &&
           PE_LoadU32(0x8009B59Cu)==0x87654321u,"stopped callback changed state");
    PASS();
}

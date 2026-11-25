/* Host scratch padding and timer distinctions while command issue is blocked. */
static void test_CDPUMP_native_blocked_mode_and_counter(void)
{
    TEST("CDPUMP_native_blocked_mode_and_counter");
    ResetTestState();
    PE_StoreU32(0x8009B574u,2);PE_StoreU32(0x8009B578u,12);
    PE_StoreU32(0x8009B598u,2);PE_StoreU32(0x8009B594u,0x80000000u);
    PE_StoreU32(0x8009B5A0u,3);PE_StoreU32(0x8009B6A4u,1);
    PE_StoreU32(0x8009B570u,0x11223344u);PE_StoreU32(0x801FFE90u,0xA5A5A5A5u);
    func_8007FE24();
    ASSERT(PE_LoadU32(0x8009B598u)==1 && PE_LoadU32(0x8009B594u)==0x80000000u && PE_LoadU32(0x8009B5A0u)==2,"timer decrement or signed timer condition changed");
    ASSERT(PE_LoadU32(0x801FFE90u)==0xA5A5A500u && PE_LoadU32(0x8009B6A4u)==0,"blocked mode lost scratch padding or toggle");
    ASSERT(PE_LoadU32(0x8009B570u)==0x11223344u,"pending command must suppress issue marker");
    PE_StoreU32(0x8009B578u,14);PE_StoreU32(0x8009B57Cu,22);PE_StoreU32(0x8009B598u,2);PE_StoreU32(0x8009B58Cu,0xFFFFFFFFu);
    func_8007FE24();
    ASSERT(PE_LoadU32(0x8009B58Cu)==0 && PE_LoadU32(0x8009B570u)==0x11223344u,"blocked phase counter wrap changed");
    PE_StoreU32(0x8009B578u,15);PE_StoreU32(0x8009B598u,2);
    func_8007FE24();
    ASSERT(PE_LoadU32(0x8009B574u)==2 && PE_LoadU32(0x8009B578u)==15,"negative timer incorrectly treated as expired");
    ASSERT(!PE_Port_ShouldStop(),"blocked timer pump requested stop");
    PASS();
}

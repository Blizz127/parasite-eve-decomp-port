/* Libpad communication start (func_80082534), clean-room cdgen45 contract.
 * Empty hook slot (what the vectors record): the running word stays 0 and no
 * counter/record is touched.  Installed slot (the boot path: func_800844E4
 * stores the record-reset hook first): records reset, counters cleared, running
 * word 1.  No device-input claim. */
static void test_PADINIT_native_slots_and_enable(void)
{
    TEST("PADINIT_native_slots_and_enable");
    ResetTestState();
    /* empty slot */
    PE_StoreU32(0x800A5AC0u,0xFFFFFFFF);PE_StoreU32(0x800A5AC4u,0xFFFFFFFF);
    PE_StoreU32(0x800A5B70u,0x11223344);
    PE_StoreU32(0x8009B75Cu,0xFFFFFFFF);
    PE_StoreU32(0x8009B728u,0);
    func_80082534();
    ASSERT(PE_LoadU32(0x8009B75Cu)==0,"empty slot: running word must be cleared and stay 0");
    ASSERT(PE_LoadU32(0x800A5AC0u)==0xFFFFFFFFu&&PE_LoadU32(0x800A5AC4u)==0xFFFFFFFFu,"empty slot: counters must be untouched");
    ASSERT(PE_LoadU32(0x800A5B70u)==0x11223344u,"empty slot: record must be untouched");
    ASSERT(!PE_Port_ShouldStop(),"pad initialization requested unexpected stop");
    /* installed slot: the real boot order */
    ResetTestState();
    PE_StoreU8(0x800A5BB9u,1);PE_StoreU8(0x800A5CA9u,1);
    PE_StoreU32(0x800A5B70u,0x11223344);PE_StoreU32(0x800A5C60u,0x55667788);
    PE_StoreU32(0x800A5AC0u,0xFFFFFFFF);PE_StoreU32(0x800A5AC4u,0xFFFFFFFF);
    func_800844E4(0x800BE9A0u,0x800BE9C2u);
    PE_StoreU32(0x800A5AC0u,0xFFFFFFFF);PE_StoreU32(0x800A5AC4u,0xFFFFFFFF);
    PE_StoreU8(0x800A5BB9u,1);PE_StoreU8(0x800A5CA9u,1);
    PE_StoreU32(0x800A5B70u,0x11223344);PE_StoreU32(0x800A5C60u,0x55667788);
    PE_StoreU32(0x8009B75Cu,0xFFFFFFFF);
    func_80082534();
    ASSERT(PE_LoadU32(0x8009B75Cu)==1,"pad initialization did not enable host state");
    ASSERT(!PE_LoadU32(0x800A5AC0u)&&!PE_LoadU32(0x800A5AC4u),"pad initialization left host counters");
    ASSERT(!PE_LoadU8(0x800A5BB9u)&&!PE_LoadU8(0x800A5CA9u),"pad initialization skipped active slot");
    ASSERT(!PE_LoadU32(0x800A5B70u)&&!PE_LoadU32(0x800A5C60u),"pad initialization retained active slot words");
    ASSERT(PE_LoadU8(0x800A5BCDu)==255&&PE_LoadU8(0x800A5BD2u)==255&&PE_LoadU8(0x800A5CBDu)==255&&PE_LoadU8(0x800A5CC2u)==255,"slot reset sentinel range changed");
    ASSERT(!PE_Port_ShouldStop(),"pad initialization requested unexpected stop");
    PASS();
}

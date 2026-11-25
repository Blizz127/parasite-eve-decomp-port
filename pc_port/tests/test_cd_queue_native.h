/* NULL parameters preserve record padding and suppress preliminary Setloc. */
static void test_CDQUEUE_native_null_and_serial_wrap(void)
{
    TEST("CDQUEUE_native_null_and_serial_wrap");
    ResetTestState();
    for(uint32_t p=0x800A3540u;p<0x800A3600u;p++)PE_StoreU8(p,0xA5);
    PE_StoreU32(0x8009B4C8u,1);PE_StoreU32(0x8009B53Cu,0xFFFFFFFFu);
    PE_StoreU32(0x800A3600u,0);PE_StoreU32(0x800A3604u,0);PE_StoreU32(0x800A3608u,0);
    PE_StoreU32(0x8009B574u,0);
    ASSERT(func_8007EE84(0x103u,0,0x12345678u,0x87654321u)==1,"queue serial did not skip zero");
    ASSERT(PE_LoadU32(0x8009B53Cu)==1&&PE_LoadU32(0x800A3608u)==1,"NULL parameter added a preliminary command");
    ASSERT(PE_LoadU32(0x800A3540u)==1&&PE_LoadU8(0x800A3544u)==3,"queue record serial/command incorrect");
    for(uint32_t p=0x800A3545u;p<=0x800A354Bu;p++)ASSERT(PE_LoadU8(p)==0xA5,"NULL command changed parameter bytes or padding");
    ASSERT(PE_LoadU32(0x800A354Cu)==0&&PE_LoadU32(0x800A3550u)==0x12345678u&&PE_LoadU32(0x800A3554u)==0x87654321u,"queue arguments changed");
    PE_StoreU32(0x800A3608u,8);
    ASSERT(func_8007EE84(3,0,0,0)==0&&PE_LoadU32(0x8009B53Cu)==1,"full queue advanced the serial");
    ASSERT(!PE_Port_ShouldStop(),"non-device queue requested a stop");
    PASS();
}

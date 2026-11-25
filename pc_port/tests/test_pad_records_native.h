/* Host buffer caching, slot clear and callback records; no device-input claim. */
static void test_PADRECORD_native_alias_and_tables(void)
{
    TEST("PADRECORD_native_alias_and_tables");
    ResetTestState();
    PE_StoreU32(0x800A5B84u,0xFFFFFFFFu);PE_StoreU32(0x800A5C74u,0xFFFFFFFFu);
    PE_StoreU8(0x80150000u,0x12);PE_StoreU8(0x80150001u,0x34);
    func_800844E4(0x800A5BA0u,0x80150000u);
    ASSERT(PE_LoadU32(0x800A5BA0u)==0x800A00FFu,"self-alias buffer pointer was not cached");
    ASSERT(PE_LoadU8(0x80150000u)==255&&!PE_LoadU8(0x80150001u),"second buffer header changed");
    ASSERT(!PE_LoadU32(0x800A5B84u)&&!PE_LoadU32(0x800A5C74u),"record clear skipped");
    ASSERT(PE_LoadU32(0x8009B744u)==0x80084C4Cu&&PE_LoadU32(0x8009B758u)==0x800A5B70u,"record hooks changed");
    ASSERT(PE_LoadU32(0x800A5AB4u)==0x80082B70u&&PE_LoadU32(0x800A5AB8u)==0x80082B08u,"card callbacks changed");
    ASSERT(PE_LoadU32(0x8009B75Cu)==1&&!PE_Port_ShouldStop(),"record setup failed to enable cleanly");
    PASS();
}

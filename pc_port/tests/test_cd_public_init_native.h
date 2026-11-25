/* The public wrapper must not install later handlers after device init stops. */
static void CDPUBLIC_Conflict(void) { }
static void test_CDPUBLIC_native_stop_before_handlers(void)
{
    TEST_RETAIL_DISC1("CDPUBLIC_native_stop_before_handlers");
    TEST_RETAIL_FIXUPS(RETAILFIX_cd_device_tables);
    DiscFixture fx={0};
    ASSERT(FxBuild(&fx,0),"disc fixture allocation failed");
    for(unsigned conflict=0;conflict<2;conflict++){
        ResetTestState();PE_Disc_SetActive(fx.disc);CdDeviceSeed();
        ASSERT(PE_CdReg_EnableDevice(7u),"device fixture attachment failed");
        PE_StoreU32(0x8009B554u,0);PE_StoreU32(0x800A36A4u,0x12345678u);
        if(conflict)ASSERT(PE_Callback_Bind(0x8007FE24u,CDPUBLIC_Conflict)==0,"callback fixture conflict failed");
        else PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
        pe_plat_cd_store_setloc_hint(0xA1B2C3D4u);
        ASSERT(func_8007EC14()==0,"public initializer ignored a host stop");
        ASSERT(PE_Port_GetStopReason()==(conflict?PE_PORT_STOP_UNRESOLVED_BOUNDARY:PE_PORT_STOP_HOST_QUIT),"stop reason changed");
        ASSERT(PE_Cd_GetSetlocRaw()==0,"public initializer left the host Setloc cache armed");
        ASSERT(PE_LoadU32(0x800A36A4u)!=0x8007E964u,"public handler installed after stopped initialization");
    }
    PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}

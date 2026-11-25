/* Real guest printf dispatch and signed rectangle bounds; no device claim. */
static void test_GPURECT_native_diagnostics(void)
{
    TEST("GPURECT_native_diagnostics");
    ResetTestState();
    PE_Bios_PrintfLogReset();
    RECT rect={1,2,3,4};
    pe_addr_t fmt=0;uint32_t args[4]={0};
    PE_StoreU32(0x80095748u,0x80071A74u);
    PE_StoreU8(0x8009574Eu,1);
    PE_StoreU16(0x80095750u,640);PE_StoreU16(0x80095752u,256);
    func_80074E28(0x80140000u,&rect);
    ASSERT(PE_Bios_PrintfLog(NULL,NULL)==0,"valid rectangle unexpectedly logged");
    PE_StoreU16(0x80095750u,0xFFFFu);
    func_80074E28(0x80140000u,&rect);
    ASSERT(PE_Bios_PrintfLog(&fmt,args)==2&&fmt==0x800118A4u,"signed limit or second print lost");
    ASSERT(args[0]==1&&args[1]==2&&args[2]==3&&args[3]==4,"rectangle print arguments changed");
    PE_Bios_PrintfLogReset();PE_StoreU8(0x8009574Eu,2);rect.x=-7;
    func_80074E28(0x80140000u,&rect);
    ASSERT(PE_Bios_PrintfLog(&fmt,args)==2&&args[0]==(uint32_t)-7,"debug2 signed rectangle lost");
    ASSERT(!PE_Port_ShouldStop(),"diagnostics unexpectedly stopped runtime");
    PASS();
}

/* Ring lookup, cached payload, pending and expired query distinctions. */
static void test_CDRESPONSE_native_latest_and_missing(void)
{
    TEST("CDRESPONSE_native_latest_and_missing");
    ResetTestState();HostFB_Init();
    PE_StoreU32(0x800A3690u,0);PE_StoreU32(0x8009B574u,0);
    for(unsigned i=0;i<8;i++){
        uint32_t record=0x800A3610u+16u*i;
        PE_StoreU32(record,10+i);PE_StoreU8(record+4,2);
        for(unsigned j=0;j<8;j++)PE_StoreU8(record+5+j,(uint8_t)(16*i+j));
    }
    PE_StoreU32(0x800A3610u,42);
    ASSERT(func_8007F418(0,0x80150000u)==2 && PE_LoadU32(0x800A3500u)==17,"latest response lookup");
    for(unsigned j=0;j<8;j++)ASSERT(PE_LoadU8(0x80150000u+j)==112+j,"latest response bytes");
    ASSERT(func_8007F418(42,0x80150000u)==2 && PE_LoadU32(0x800A3500u)==42,"matching response lookup");
    ASSERT(func_8007F418(43,0)==0,"pending query must return zero");
    ASSERT(func_8007F418(1,0)==6,"expired query must return six");
    PE_StoreU32(0x800A3680u,0);
    ASSERT(func_8007F418(0,0)==6,"empty latest record must return six");
    ASSERT(!PE_Port_ShouldStop(),"response query requested stop");
    PASS();
}

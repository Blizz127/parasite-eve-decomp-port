/* Preserve host four-record sequence, mode padding and rejection side effects. */
static void test_CDSEQUENCE_native_records_and_capacity(void)
{
    TEST("CDSEQUENCE_native_records_and_capacity");
    ResetTestState();
    for(uint32_t a=0x800A3540u;a<0x800A3600u;a++)PE_StoreU8(a,0xA5);
    PE_StoreU32(0x8009B53Cu,0xFFFFFFFFu);
    PE_StoreU32(0x800A3600u,0);PE_StoreU32(0x800A3604u,0);PE_StoreU32(0x800A3608u,0);
    PE_StoreU32(0x8009B574u,0);PE_StoreU32(0x80150000u,0x00000200u);
    ASSERT(func_8007F0C8(0x1E0,0x80150000u,0x11B,0x12345678u,0x87654321u)==1,"sequence serial wrap");
    ASSERT(PE_LoadU32(0x800A3608u)==4,"sequence must append four records");
    const uint8_t commands[4]={9,14,2,27};
    for(unsigned i=0;i<4;i++){
        uint32_t r=0x800A3540u+24u*i;
        ASSERT(PE_LoadU32(r)==1 && PE_LoadU8(r+4)==commands[i],"sequence command/serial");
        ASSERT(PE_LoadU32(r+12)==((i==1||i==2)?r+5:0),"sequence parameter pointer");
        ASSERT(PE_LoadU32(r+16)==(i==3?0x12345678u:0) && PE_LoadU32(r+20)==0x87654321u,"sequence callback/extra");
    }
    ASSERT(PE_LoadU32(0x800A355Du)==0xE0 && PE_LoadU32(0x800A3575u)==0x00000200u,"sequence mode/location payload");
    ASSERT(PE_LoadU8(0x800A3545u)==0xA5 && PE_LoadU8(0x800A358Du)==0xA5,"sequence empty payload changed");
    PE_StoreU32(0x800A3608u,5);PE_StoreU32(0x801FFE80u,0xFFFFFFFFu);
    ASSERT(func_8007F0C8(0,0x80150000u,27,0,0)==0 && PE_LoadU32(0x8009B53Cu)==1,"full sequence queue changed serial");
    ASSERT(PE_LoadU32(0x801FFE80u)==0x00000200u,"rejected sequence lost location scratch side effect");
    PASS();
}

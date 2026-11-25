/* Check reset transitions and record holes through the bound host provider. */
static void test_CDRESET_native_state_and_padding(void)
{
    TEST("CDRESET_native_state_and_padding");
    const uint32_t states[]={0u,2u,0xFFFFFFFFu},subs[]={0u,0xBu,0xFu,0x10u,0x11u,0x12u};
    for(unsigned a=0;a<3;a++)for(unsigned b=0;b<6;b++){
        ResetTestState();
        for(uint32_t p=0x800A3500u;p<0x800A36A0u;p++)PE_StoreU8(p,0xA5);
        for(uint32_t p=0x800B8AA0u;p<0x800B8AC0u;p++)PE_StoreU8(p,0xA5);
        PE_StoreU32(0x8009B554u,0x12345678u);PE_StoreU32(0x8009B574u,states[a]);
        PE_StoreU32(0x8009B578u,subs[b]);PE_StoreU32(0x8009B708u,7u);PE_StoreU32(0x8009B70Cu,1u);
        pe_plat_cd_store_setloc_hint(0xA1B2C3D4u);
        ASSERT(func_8007ED58()==1,"reset failed");
        ASSERT(PE_LoadU32(0x8009B554u)==1&&PE_LoadU32(0x8009B574u)==1,"reset did not enable idle state");
        int aborting=states[a]==2u&&(subs[b]==0xBu||subs[b]==0x10u||subs[b]==0x11u);
        ASSERT(PE_LoadU32(0x8009B578u)==(aborting?0xBu:subs[b]),"abort substate transition mismatch");
        ASSERT(!PE_LoadU32(0x8009B708u)&&!PE_LoadU32(0x8009B70Cu),"ready state not cleared");
        for(uint32_t i=0;i<8;i++){
            uint32_t q=0x800A3540u+24*i;
            for(uint32_t j=0;j<24;j++)ASSERT(PE_LoadU8(q+j)==(j>=9&&j<=11?0xA5:0),"queue clear changed padding or left payload");
        }
        for(uint32_t i=0;i<3;i++){
            uint32_t p=0x800A3510u+16*i;
            for(uint32_t j=0;j<16;j++)ASSERT(PE_LoadU8(p+j)==(j<=12?0:0xA5),"result lane clear mismatch");
            ASSERT(!PE_LoadU32(0x800B8AB0u+4*i),"result counter not cleared");
        }
        ASSERT(PE_LoadU8(0x800A350Fu)==0xA5&&PE_LoadU8(0x800B8ABCu)==0xA5,"reset wrote outside state range");
        ASSERT(PE_Cd_GetSetlocRaw()==0,"reset left the host Setloc cache armed");
        ASSERT(!PE_Port_ShouldStop(),"reset requested a host stop");
    }
    PASS();
}

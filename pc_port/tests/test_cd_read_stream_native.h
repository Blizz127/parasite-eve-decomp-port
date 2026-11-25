/* Synthetic XA sector exercises the real host feeder without owner assets. */
static void test_CDREAD_native_stream_sequence(void)
{
    TEST("CDREAD_native_stream_sequence");
    uint8_t *img=calloc(64u,PE_DISC_RAW_SECTOR);
    ASSERT(img!=NULL,"disc fixture allocation failed");
    for(unsigned i=0;i<64;i++){
        uint8_t *raw=img+i*PE_DISC_RAW_SECTOR;
        memset(raw+1,255,10);raw[15]=2;raw[18]=4;
    }
    uint8_t *p=img+16u*PE_DISC_RAW_SECTOR+PE_DISC_USER_OFFSET;
    p[0]=1;memcpy(p+1,"CD001",5);p[6]=1;
    PE_Disc *disc=PE_Disc_OpenMemory(img,64u*PE_DISC_RAW_SECTOR);
    ASSERT(disc!=NULL,"synthetic disc rejected");
    const uint32_t modes[]={0x100u,0x120u},sequences[]={0u,1u,0xFFFFFFFFu};
    for(unsigned m=0;m<2;m++)for(unsigned q=0;q<3;q++){
        CDINIT_SeedHost();PE_Disc_SetActive(disc);
        PE_StoreU32(0x80150000u,0x00000200u);
        PE_StoreU32(0x8009B53Cu,sequences[q]);
        uint32_t expected=sequences[q]+1u;if(!expected)expected=1u;
        int result=func_80081314(0x80150000u,modes[m]);
        ASSERT((uint32_t)result==expected && PE_LoadU32(0x8009B53Cu)==expected,
               "host read sequence/wrap mismatch");
        ASSERT(PE_StrFeed_Active() && !PE_Port_ShouldStop(),"host stream did not open");
        ASSERT(PE_LoadU32(0x800A8020u)==(m?0u:1u),"stream flag differs from mode bit");
        PE_StrFeed_Close();
    }
    PE_Disc_SetActive(NULL);PE_Disc_Close(disc);free(img);PASS();
}

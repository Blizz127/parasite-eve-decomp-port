/* Real ISO9660 lookup through the bound native search adapter; synthetic data. */
static void test_CDSEARCH_native_lookup_record(void)
{
    TEST("CDSEARCH_native_lookup_record");
    uint8_t *img=calloc(64u,PE_DISC_RAW_SECTOR);
    ASSERT(img!=NULL,"fixture allocation failed");
    for(unsigned i=0;i<64;i++){
        uint8_t *raw=img+i*PE_DISC_RAW_SECTOR;
        memset(raw+1,255,10);raw[15]=2;
    }
#define CDSEARCH_USER(l) (img+(size_t)(l)*PE_DISC_RAW_SECTOR+PE_DISC_USER_OFFSET)
    uint8_t *p=CDSEARCH_USER(16);
    p[0]=1;memcpy(p+1,"CD001",5);p[6]=1;
    plat_test_dirrec(p+156,20,2048,2,"\0",1);
    int n=plat_test_dirrec(CDSEARCH_USER(20),20,2048,2,"\0",1);
    n+=plat_test_dirrec(CDSEARCH_USER(20)+n,20,2048,2,"\1",1);
    n+=plat_test_dirrec(CDSEARCH_USER(20)+n,21,2048,2,"DIR",3);
    plat_test_dirrec(CDSEARCH_USER(20)+n,22,3000,0,"DATA.BIN;1",10);
    n=plat_test_dirrec(CDSEARCH_USER(21),21,2048,2,"\0",1);
    n+=plat_test_dirrec(CDSEARCH_USER(21)+n,20,2048,2,"\1",1);
    plat_test_dirrec(CDSEARCH_USER(21)+n,30,1234,0,"ABCDEFGHIJKLMNOPQRST.BIN;1",26);
#undef CDSEARCH_USER
    PE_Disc *before=PE_Disc_GetActive(),*disc=PE_Disc_OpenMemory(img,64u*PE_DISC_RAW_SECTOR);
    ASSERT(disc!=NULL,"synthetic disc rejected");
    const char *paths[]={NULL,"","A","\\","\\DATA.BIN;1",
        "\\DIR\\ABCDEFGHIJKLMNOPQRST.BIN;1","\\NO.BIN;1"};
    for(unsigned active=0;active<2;active++)for(unsigned k=0;k<7;k++){
        PE_Disc_SetActive(active?disc:NULL);
        const uint32_t out=0x80150000u;
        for(unsigned j=0;j<32;j++)PE_StoreU8(out+j,0xA5);
        int found=func_80081414(out,paths[k]);
        int want=active&&(k==4||k==5);
        ASSERT(found==want,"lookup result mismatch");
        if(want){
            ASSERT(PE_LoadU32(out)==(k==4?0x00220200u:0x00300200u),"BCD location/pad mismatch");
            ASSERT(PE_LoadU32(out+4)==(k==4?3000u:1234u),"file size mismatch");
            const char *name=k==4?"DATA.BIN;1":"ABCDEFGHIJKLMNO";
            for(unsigned j=0;j<16;j++){
                uint8_t want_byte=j<strlen(name)?(uint8_t)name[j]:0;
                ASSERT(PE_LoadU8(out+8+j)==want_byte,"filename truncation/padding mismatch");
            }
        }else{
            for(unsigned j=0;j<24;j++)ASSERT(PE_LoadU8(out+j)==0xA5,"failure modified output");
        }
        for(unsigned j=24;j<32;j++)ASSERT(PE_LoadU8(out+j)==0xA5,"record overwrite");
    }
    PE_Disc_SetActive(before);PE_Disc_Close(disc);free(img);PASS();
}

/* Host Setloc compatibility path: real disc/range/readiness/position/cache. */
static void test_CDWAIT_native_setloc_cache(void)
{
    TEST("CDWAIT_native_setloc_cache");
    uint8_t *img=calloc(64u,PE_DISC_RAW_SECTOR);
    ASSERT(img!=NULL,"fixture allocation failed");
    for(unsigned i=0;i<64;i++){
        memset(img+i*PE_DISC_RAW_SECTOR+1u,255,10);
        img[i*PE_DISC_RAW_SECTOR+15u]=2;
    }
    uint8_t *p=img+16u*PE_DISC_RAW_SECTOR+PE_DISC_USER_OFFSET;
    p[0]=1;memcpy(p+1,"CD001",5);p[6]=1;
    PE_Disc *disc=PE_Disc_OpenMemory(img,64u*PE_DISC_RAW_SECTOR);
    ASSERT(disc!=NULL,"synthetic disc rejected");
    CDINIT_SeedHost();PE_Disc_SetActive(disc);
    PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x800A3608u,0u);
    PE_StoreU32(0x80150000u,0xAA000200u);
    ASSERT(func_80080D5C(2,0x80150000u,0u)==1,"valid Setloc failed");
    ASSERT(PE_Cd_GetSetlocRaw()==0xAA000200u,"Setloc lost fourth byte");
    PE_StoreU32(0x80150000u,0xBB640200u);
    ASSERT(func_80080D5C(2,0x80150000u,0u)==0,"out-of-disc Setloc accepted");
    ASSERT(PE_Cd_GetSetlocRaw()==0xAA000200u,"rejected position changed cache");
    PE_StoreU32(0x80150000u,0xCC010200u);PE_StoreU32(0x800A3608u,1u);
    ASSERT(func_80080D5C(2,0x80150000u,0u)==0,"busy queue accepted Setloc");
    ASSERT(PE_Cd_GetSetlocRaw()==0xAA000200u,"busy queue changed cache");
    PE_StoreU32(0x800A3608u,0u);PE_Disc_SetActive(NULL);
    ASSERT(func_80080D5C(2,0x80150000u,0u)==0,"missing disc accepted Setloc");
    ASSERT(PE_Cd_GetSetlocRaw()==0xAA000200u,"missing disc changed cache");
    PE_Disc_Close(disc);free(img);PASS();
}

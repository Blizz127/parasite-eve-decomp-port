#include "retail_m0034i_setup_cases.h"
static void test_DAY1_m0034i_setup(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY1_m0034i_setup");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for (unsigned i=0;i<100;i++)
        ASSERT(PE_Disc_ReadUserSector(disc,M34_CHUNK_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0034I chunk sector");
    PE_Disc_Close(disc);
    seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"setup seed allocation");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for (unsigned k=0;k<sizeof(M34S_cases)/sizeof(M34S_cases[0]);k++) {
        const pe_addr_t actor=0x80150000u,task=0x80152000u;
        uint64_t hash;
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        PE_StoreU32(actor,0x80153000u);PE_StoreU8(actor+12u,4u);
        PE_StoreU32(actor+0x98u,0x2000u);PE_StoreU32(actor+0x9Cu,0x801B8BCCu);
        PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D300u,task);PE_StoreU32(0x8009D1A0u,0);
        PE_StoreU32(task,0x801B8BE0u);PE_StoreU32(task+16u,1);
        PE_StoreU32(0x800A7930u,M34S_cases[k].persist);
        PE_StoreU8(0x8009D2ECu,255);PE_StoreU8(0x8009D2A0u,31);
        for (unsigned i=0;i<7;i++)PE_StoreU32(0x800A5D58u+i*220u,i<M34S_cases[k].occupied);
        PE_StoreU32(0x80070E04u,0);PE_StoreU32(0x80070E08u,8);
        for (unsigned i=0;i<17;i++)PE_StoreU32(0x80070E0Cu+i*4u,0);
        PE_StoreU32(0x80070E0Cu,(M34S_cases[k].first*65536u+99u)/100u);
        PE_StoreU32(0x80070E4Cu,(M34S_cases[k].second*65536u+99u)/100u);
        func_80017018();
        hash=hit_camera_hash(M34S_ranges,sizeof(M34S_ranges)/sizeof(M34S_ranges[0]));
        if (hash!=M34S_cases[k].hash)fprintf(stderr,"M34 setup%u %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)M34S_cases[k].hash);
        ASSERT(hash==M34S_cases[k].hash,"M0034I setup differs from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"M0034I setup completed natively");
    }
    free(seed);PASS();
}

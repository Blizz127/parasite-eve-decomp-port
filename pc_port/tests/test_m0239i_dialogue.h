#include "retail_m0239i_dialogue_cases.h"
static void test_DAY2_m0239i_dialogue(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_m0239i_dialogue");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for(unsigned i=0;i<M239D_SECTORS;i++)
        ASSERT(PE_Disc_ReadUserSector(disc,M239D_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0239I sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"dialogue seed allocation");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(M239D_cases)/sizeof(M239D_cases[0]);k++) {
        unsigned bank=M239D_cases[k].bank;uint64_t hash;
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        func_800371B0(0x801E465Cu);PE_StoreU32(0x8009CDDCu,bank);
        PE_StoreU32(0x800B0E38u+bank*4u,0x80155000u);PE_StoreU32(0x800B0E44u+bank*4u,0x80150000u);
        for(unsigned i=0;i<4;i++)PE_StoreU32(0x80155000u+i*4u,i?0x155000u+(i-1u)*4u:0xFFFFFFu);
        PE_StoreU16(0x80156000u,0xFFFFu);func_800375E0((int)M239D_cases[k].message,0,0x80156000u);
        PE_StoreU32(0x80156020u,0x80156040u);PE_StoreU32(0x80156024u,0x80156044u);
        PE_StoreU32(0x80156040u,2300);PE_StoreU32(0x80156044u,M239D_cases[k].background);
        func_80016910_key2900_cut(0x80156020u);func_80037870();
        hash=hit_camera_hash(M239D_ranges,sizeof(M239D_ranges)/sizeof(M239D_ranges[0]));
        if(hash!=M239D_cases[k].hash)fprintf(stderr,"M239 dialogue%u %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)M239D_cases[k].hash);
        ASSERT(hash==M239D_cases[k].hash,"real M0239I first-page text differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"real dialogue first page completed natively");
    }
    free(seed);PASS();
}

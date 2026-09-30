#include "retail_mode5_cleanup_cases.h"
static void test_DAY1_mode5_cleanup(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY1_mode5_cleanup");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);PE_Disc_Close(disc);
    seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"mode5 seed allocation");memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for (unsigned k=0;k<sizeof(M5_cases)/sizeof(M5_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for (unsigned i=0;i<sizeof(M5_ranges)/sizeof(M5_ranges[0]);i++)memset(PE_Translate(0x80000000u|M5_ranges[i][0],M5_ranges[i][1]),0,M5_ranges[i][1]);
        for (unsigned i=0;i<sizeof(M5_common)/sizeof(M5_common[0]);i++)PE_StoreU32(0x80000000u|M5_common[i][0],M5_common[i][1]);
        for (unsigned i=M5_cases[k].first;i<M5_cases[k].end;i++)PE_StoreU32(0x80000000u|M5_patches[i][0],M5_patches[i][1]);
        func_800299CC_mode_switch_cut();
        hash=hit_camera_hash(M5_ranges,sizeof(M5_ranges)/sizeof(M5_ranges[0]));
        if (hash!=M5_cases[k].hash)fprintf(stderr,"mode5 case%u %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)M5_cases[k].hash);
        ASSERT(hash==M5_cases[k].hash,"mode5 cleanup differs from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"mode5 graph completed natively");
    }
    free(seed);PASS();
}

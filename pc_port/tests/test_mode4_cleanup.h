#include "retail_mode4_cleanup_cases.h"
static void test_DAY1_mode4_cleanup(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY1_mode4_cleanup");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);PE_Disc_Close(disc);
    seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"mode4 seed allocation");memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for (unsigned k=0;k<sizeof(M4_cases)/sizeof(M4_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for (unsigned i=0;i<sizeof(M4_ranges)/sizeof(M4_ranges[0]);i++)memset(PE_Translate(0x80000000u|M4_ranges[i][0],M4_ranges[i][1]),0,M4_ranges[i][1]);
        for (unsigned i=0;i<sizeof(M4_common)/sizeof(M4_common[0]);i++)PE_StoreU32(0x80000000u|M4_common[i][0],M4_common[i][1]);
        for (unsigned i=M4_cases[k].first;i<M4_cases[k].end;i++)PE_StoreU32(0x80000000u|M4_patches[i][0],M4_patches[i][1]);
        func_800299CC_mode_switch_cut();
        hash=hit_camera_hash(M4_ranges,sizeof(M4_ranges)/sizeof(M4_ranges[0]));
        if (hash!=M4_cases[k].hash)fprintf(stderr,"mode4 case%u %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)M4_cases[k].hash);
        ASSERT(hash==M4_cases[k].hash,"mode4 cleanup differs from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"mode4 graph completed natively");
    }
    free(seed);PASS();
}

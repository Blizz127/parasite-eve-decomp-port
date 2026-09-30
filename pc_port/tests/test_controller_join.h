#include "retail_controller_join_cases.h"
static void test_DAY2_controller_join(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_controller_join"); TEST_RETAIL_FIXUPS(RETAILFIX_controller_join);
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);PE_Disc_Close(disc);
    seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"controller join seed");memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(CJ_cases)/sizeof(CJ_cases[0]);k++) {
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for(unsigned i=0;i<sizeof(CJ_ranges)/sizeof(CJ_ranges[0]);i++)memset(PE_Translate(0x80000000u|CJ_ranges[i][0],CJ_ranges[i][1]),0,CJ_ranges[i][1]);
        for(unsigned i=0;i<sizeof(CJ_common)/sizeof(CJ_common[0]);i++)PE_StoreU32(0x80000000u|CJ_common[i][0],CJ_common[i][1]);
        for(unsigned i=CJ_cases[k].first;i<CJ_cases[k].end;i++)PE_StoreU32(0x80000000u|CJ_patches[i][0],CJ_patches[i][1]);
        func_8002A7F8_join_cut();
        uint64_t h=hit_camera_hash(CJ_ranges,sizeof(CJ_ranges)/sizeof(CJ_ranges[0]));
        if(h!=CJ_cases[k].hash)fprintf(stderr,"controller join%u %016llX/%016llX\n",k,(unsigned long long)h,(unsigned long long)CJ_cases[k].hash);
        ASSERT(h==CJ_cases[k].hash,"controller join/message packets differ from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"controller join completes natively");
    }
    free(seed);PASS();
}

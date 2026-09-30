#include "retail_message_number_cases.h"
static void test_DAY2_message_number(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_message_number");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);PE_Disc_Close(disc);
    seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"message number seed");memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(MN_cases)/sizeof(MN_cases[0]);k++) {
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for(unsigned i=0;i<sizeof(MN_ranges)/sizeof(MN_ranges[0]);i++)memset(PE_Translate(0x80000000u|MN_ranges[i][0],MN_ranges[i][1]),0,MN_ranges[i][1]);
        for(unsigned i=0;i<sizeof(MN_common)/sizeof(MN_common[0]);i++)PE_StoreU32(0x80000000u|MN_common[i][0],MN_common[i][1]);
        for(unsigned i=MN_cases[k].first;i<MN_cases[k].end;i++)PE_StoreU32(0x80000000u|MN_patches[i][0],MN_patches[i][1]);
        func_800375E0(7,2u,0x80156080u);
        ASSERT(hit_camera_hash(MN_ranges,sizeof(MN_ranges)/sizeof(MN_ranges[0]))==MN_cases[k].opened,"numeric message setup differs from original");
        func_80037870();
        uint64_t h=hit_camera_hash(MN_ranges,sizeof(MN_ranges)/sizeof(MN_ranges[0]));
        if(h!=MN_cases[k].draw)fprintf(stderr,"message number%u %016llX/%016llX\n",k,(unsigned long long)h,(unsigned long long)MN_cases[k].draw);
        ASSERT(h==MN_cases[k].draw,"message number packets differ from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"message number graph completes natively");
    }
    free(seed);PASS();
}

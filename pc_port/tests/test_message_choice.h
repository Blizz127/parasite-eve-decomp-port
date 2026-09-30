#include "retail_message_choice_cases.h"
static void test_DAY2_message_choice(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_message_choice");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);PE_Disc_Close(disc);
    seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"message choice seed");memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(MC_cases)/sizeof(MC_cases[0]);k++) {
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for(unsigned i=0;i<sizeof(MC_ranges)/sizeof(MC_ranges[0]);i++)memset(PE_Translate(0x80000000u|MC_ranges[i][0],MC_ranges[i][1]),0,MC_ranges[i][1]);
        for(unsigned i=0;i<sizeof(MC_common)/sizeof(MC_common[0]);i++)PE_StoreU32(0x80000000u|MC_common[i][0],MC_common[i][1]);
        for(unsigned i=MC_cases[k].first;i<MC_cases[k].end;i++)PE_StoreU32(0x80000000u|MC_patches[i][0],MC_patches[i][1]);
        for(unsigned frame=0;frame<2;frame++) {
            for(unsigned i=0;i<2;i++)for(unsigned j=0;j<4;j++)PE_StoreU32(0x80155000u+i*32u+j*4u,j?0x155000u+i*32u+(j-1u)*4u:0xFFFFFFu);
            func_80037870();
            uint64_t h=hit_camera_hash(MC_ranges,sizeof(MC_ranges)/sizeof(MC_ranges[0]));
            if(h!=MC_cases[k].hash[frame])fprintf(stderr,"message choice%u frame%u %016llX/%016llX\n",k,frame,(unsigned long long)h,(unsigned long long)MC_cases[k].hash[frame]);
            ASSERT(h==MC_cases[k].hash[frame],"message choice packets/state differ from original");
            ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"message choice graph completes natively");
        }
    }
    free(seed);PASS();
}

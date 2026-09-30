#include "retail_message_wait_cases.h"
static void test_DAY2_message_wait(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_message_wait");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);PE_Disc_Close(disc);
    seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"message wait seed");memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(MW_cases)/sizeof(MW_cases[0]);k++) {
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for(unsigned i=0;i<sizeof(MW_ranges)/sizeof(MW_ranges[0]);i++)memset(PE_Translate(0x80000000u|MW_ranges[i][0],MW_ranges[i][1]),0,MW_ranges[i][1]);
        for(unsigned i=0;i<sizeof(MW_common)/sizeof(MW_common[0]);i++)PE_StoreU32(0x80000000u|MW_common[i][0],MW_common[i][1]);
        for(unsigned i=MW_cases[k].pfirst;i<MW_cases[k].pend;i++)PE_StoreU32(0x80000000u|MW_patches[i][0],MW_patches[i][1]);
        for(unsigned n=MW_cases[k].first;n<MW_cases[k].end;n++) {
            unsigned frame=n-MW_cases[k].first;
            PE_StoreU32(0x8009CDDCu,frame&1u);
            PE_StoreU32(0x8009D1F4u,MW_cases[k].pattern==1u || (MW_cases[k].pattern==2u && frame%3u==2u)?0x100u:0u);
            for(unsigned i=0;i<2;i++)for(unsigned j=0;j<4;j++)PE_StoreU32(0x80155000u+i*32u+j*4u,j?0x155000u+i*32u+(j-1u)*4u:0xFFFFFFu);
            func_80037870();
            uint64_t h=hit_camera_hash(MW_ranges,sizeof(MW_ranges)/sizeof(MW_ranges[0]));
            if(h!=MW_frames[n])fprintf(stderr,"message wait%u frame%u %016llX/%016llX\n",k,frame,(unsigned long long)h,(unsigned long long)MW_frames[n]);
            ASSERT(h==MW_frames[n],"message wait packets/state differ from original");
            ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"message wait graph completes natively");
        }
    }
    free(seed);PASS();
}

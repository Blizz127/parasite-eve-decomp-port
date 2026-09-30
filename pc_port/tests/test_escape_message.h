#include "retail_escape_message_cases.h"
static void test_DAY2_escape_message(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_escape_message"); TEST_RETAIL_FIXUPS(RETAILFIX_escape_message);
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);PE_Disc_Close(disc);
    seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"escape message seed");memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(EM_cases)/sizeof(EM_cases[0]);k++) {
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for(unsigned i=0;i<sizeof(EM_ranges)/sizeof(EM_ranges[0]);i++)memset(PE_Translate(0x80000000u|EM_ranges[i][0],EM_ranges[i][1]),0,EM_ranges[i][1]);
        for(unsigned i=0;i<sizeof(EM_common)/sizeof(EM_common[0]);i++)PE_StoreU32(0x80000000u|EM_common[i][0],EM_common[i][1]);
        for(unsigned i=EM_cases[k].pfirst;i<EM_cases[k].pend;i++)PE_StoreU32(0x80000000u|EM_patches[i][0],EM_patches[i][1]);
        func_80071A64(1);
        ASSERT((uint32_t)(int32_t)func_800255E4()==EM_cases[k].result,"escape judgement differs from original");
        ASSERT(hit_camera_hash(EM_ranges,sizeof(EM_ranges)/sizeof(EM_ranges[0]))==EM_cases[k].producer,"escape message producer differs from original");
        for(unsigned n=EM_cases[k].first;n<EM_cases[k].end;n++) {
            unsigned frame=n-EM_cases[k].first;
            PE_StoreU32(0x8009CDDCu,(frame+EM_cases[k].bank)&1u);PE_StoreU32(0x8009D100u,0x80160000u);
            for(unsigned i=0;i<2;i++)for(unsigned j=0;j<4;j++)PE_StoreU32(0x80185000u+i*32u+j*4u,j?0x185000u+i*32u+(j-1u)*4u:0xFFFFFFu);
            func_8002A7F8_join_cut();func_80037870();
            uint64_t h=hit_camera_hash(EM_ranges,sizeof(EM_ranges)/sizeof(EM_ranges[0]));
            if(h!=EM_frames[n])fprintf(stderr,"escape message%u frame%u %016llX/%016llX\n",k,frame,(unsigned long long)h,(unsigned long long)EM_frames[n]);
            ASSERT(h==EM_frames[n],"escape status lifetime/render differs from original");
            ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"escape status graph completes natively");
        }
    }
    free(seed);PASS();
}

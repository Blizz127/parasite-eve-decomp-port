#include "retail_m0040i_transition_cases.h"
static void test_DAY2_m0040i_transition(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_m0040i_transition");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for(unsigned i=0;i<M40T_SECTORS;i++)ASSERT(PE_Disc_ReadUserSector(disc,M40T_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0040I sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"M0040I choice seed");memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(M40T_cases)/sizeof(M40T_cases[0]);k++) {
        unsigned waiting=0;
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for(unsigned i=0;i<sizeof(M40T_ranges)/sizeof(M40T_ranges[0]);i++)memset(PE_Translate(0x80000000u|M40T_ranges[i][0],M40T_ranges[i][1]),0,M40T_ranges[i][1]);
        for(unsigned i=0;i<sizeof(M40T_common)/sizeof(M40T_common[0]);i++)PE_StoreU32(0x80000000u|M40T_common[i][0],M40T_common[i][1]);
        for(unsigned i=M40T_cases[k].pfirst;i<M40T_cases[k].pend;i++)PE_StoreU32(0x80000000u|M40T_patches[i][0],M40T_patches[i][1]);
        for(unsigned n=M40T_cases[k].first;n<M40T_cases[k].end;n++) {
            unsigned frame=n-M40T_cases[k].first,pad=0;
            if(PE_LoadU8(0x800BCEA8u)==2u) {
                waiting++;
                if(waiting>M40T_cases[k].delay)pad=(PE_LoadU32(0x800BCEB4u)&0x200000u) && PE_LoadU8(0x8009CEA0u)<M40T_cases[k].choice?0x20u:0x100u;
            }
            PE_StoreU32(0x8009D1F4u,pad);PE_StoreU32(0x8009D300u,0x80156300u);PE_StoreU32(0x8009CDDCu,(frame+M40T_cases[k].bank)&1u);
            for(unsigned i=0;i<2;i++)for(unsigned j=0;j<4;j++)PE_StoreU32(0x80155000u+i*32u+j*4u,j?0x155000u+i*32u+(j-1u)*4u:0xFFFFFFu);
            func_80017018();func_80037870();func_80068E24();
            uint64_t h=hit_camera_hash(M40T_ranges,sizeof(M40T_ranges)/sizeof(M40T_ranges[0]));
            if(h!=M40T_frames[n].hash)fprintf(stderr,"M40 choice%u frame%u pc%08X/%08X %016llX/%016llX\n",k,frame,PE_LoadU32(0x80156300u),M40T_frames[n].pc,(unsigned long long)h,(unsigned long long)M40T_frames[n].hash);
            ASSERT(h==M40T_frames[n].hash,"M0040I choice VM/render differs from original");
            ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"M0040I choice graph completes natively");
        }
        ASSERT(PE_LoadU32(0x800A7918u)==0xD8u && PE_LoadU32(0x8009D280u)==0xA8000048u && PE_LoadU32(0x800A77F4u)==999u,"D8 and map destination published");
        ASSERT(PE_LoadU32(0x801560BCu)==M40T_cases[k].choice,"VM reads confirmed selection");
        ASSERT(PE_LoadU32(0x80156300u)==0x801C9FA4u,"choice transition reaches original transfer");
    }
    free(seed);PASS();
}

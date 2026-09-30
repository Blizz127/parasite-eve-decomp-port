#include "retail_m0059i_battle_ready_cases.h"
static void test_DAY2_m0059i_battle_ready(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_m0059i_battle_ready");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for(unsigned i=0;i<M59B_SECTORS;i++)ASSERT(PE_Disc_ReadUserSector(disc,M59B_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0059I sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"battle readiness seed");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(M59B_cases)/sizeof(M59B_cases[0]);k++) {
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for(unsigned i=0;i<sizeof(M59B_ranges)/sizeof(M59B_ranges[0]);i++)memset(PE_Translate(0x80000000u|M59B_ranges[i][0],M59B_ranges[i][1]),0,M59B_ranges[i][1]);
        for(unsigned i=0;i<sizeof(M59B_common)/sizeof(M59B_common[0]);i++)PE_StoreU32(0x80000000u|M59B_common[i][0],M59B_common[i][1]);
        for(unsigned i=M59B_cases[k].pfirst;i<M59B_cases[k].pend;i++)PE_StoreU32(0x80000000u|M59B_patches[i][0],M59B_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        for(unsigned n=M59B_cases[k].first;n<M59B_cases[k].end;n++) {
            PE_StoreU32(0x8009D300u,PE_LoadU32(0x801780A8u));func_80017018();
            uint64_t h=hit_camera_hash(M59B_ranges,sizeof(M59B_ranges)/sizeof(M59B_ranges[0]));
            if(h!=M59B_frames[n].vm)fprintf(stderr,"M59 readiness%u frame%u VM %016llX/%016llX\n",k,n-M59B_cases[k].first,(unsigned long long)h,(unsigned long long)M59B_frames[n].vm);
            ASSERT(h==M59B_frames[n].vm,"M0059I battle VM differs from original");
            if(PE_LoadU32(0x8009D28Cu)==6)func_8002BC90();
            h=hit_camera_hash(M59B_ranges,sizeof(M59B_ranges)/sizeof(M59B_ranges[0]));
            if(h!=M59B_frames[n].controller)fprintf(stderr,"M59 readiness%u frame%u controller %016llX/%016llX\n",k,n-M59B_cases[k].first,(unsigned long long)h,(unsigned long long)M59B_frames[n].controller);
            ASSERT(h==M59B_frames[n].controller,"M0059I battle readiness controller differs from original");
            ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"M0059I readiness completes natively");
        }
    }
    free(seed);PASS();
}

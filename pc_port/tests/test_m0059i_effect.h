#include "retail_m0059i_effect_cases.h"
static void test_DAY2_m0059i_effect(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_m0059i_effect");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for(unsigned i=0;i<M59E_SECTORS;i++)ASSERT(PE_Disc_ReadUserSector(disc,M59E_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0059I sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"effect loop seed");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(M59E_cases)/sizeof(M59E_cases[0]);k++) {
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        for(unsigned i=0;i<0x400;i++)PE_StoreU8(0x1F800000u+i,0);
        for(unsigned i=0;i<sizeof(M59E_ranges)/sizeof(M59E_ranges[0]);i++)for(unsigned j=0;j<M59E_ranges[i][1];j++)PE_StoreU8(0x80000000u+M59E_ranges[i][0]+j,0);
        for(unsigned i=0;i<sizeof(M59E_common)/sizeof(M59E_common[0]);i++)PE_StoreU32(0x80000000u+M59E_common[i][0],M59E_common[i][1]);
        for(unsigned i=M59E_cases[k].pfirst;i<M59E_cases[k].pend;i++)PE_StoreU32(0x80000000u+M59E_patches[i][0],M59E_patches[i][1]);
        g_pe_gte.ofx=160<<16;g_pe_gte.ofy=112<<16;g_pe_gte.h=256;
        for(unsigned n=M59E_cases[k].first;n<M59E_cases[k].end;n++) {
            unsigned frame=n-M59E_cases[k].first;
            PE_StoreU32(0x8009CDDCu,(M59E_cases[k].bank+frame)&1u);PE_StoreU32(0x8009CDD8u,0);
            for(unsigned i=0;i<0x2000;i+=4)PE_StoreU32(0x80152000u+i,0xABFFFFFFu);
            PE_StoreU32(0x8009D300u,0x80156300u);func_80017018();func_800E01BC();
            uint64_t h=hit_camera_hash(M59E_ranges,sizeof(M59E_ranges)/sizeof(M59E_ranges[0]));
            for(unsigned i=0;i<0x38;i++){h^=PE_LoadU8(0x1F800000u+i);h*=UINT64_C(1099511628211);}
            if(h!=M59E_frames[n].hash)fprintf(stderr,"M59 effect%u frame%u pc%08X/%08X count%u/%u bytes%u/%u hash%016llX/%016llX\n",k,frame,PE_LoadU32(0x80156300u),M59E_frames[n].pc,PE_LoadU16(0x800E21A4u),M59E_frames[n].count,PE_LoadU32(0x8009CDD8u),M59E_frames[n].bytes,(unsigned long long)h,(unsigned long long)M59E_frames[n].hash);
            ASSERT(h==M59E_frames[n].hash,"M0059I effect loop differs from original");
            ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"M0059I effect loop completes natively");
        }
    }
    free(seed);PASS();
}

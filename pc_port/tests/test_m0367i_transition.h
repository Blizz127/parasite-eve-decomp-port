#include "retail_m0367i_transition_cases.h"
static void test_DAY2_m0367i_transition(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_m0367i_transition");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for(unsigned i=0;i<M367T_SECTORS;i++)
        ASSERT(PE_Disc_ReadUserSector(disc,M367T_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0367I sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"transition seed allocation");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for(unsigned k=0;k<sizeof(M367T_cases)/sizeof(M367T_cases[0]);k++) {
        const pe_addr_t actor=0x80156000u,task=0x80156300u;unsigned waiting=0;
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        func_800371B0(M367T_TEXT);
        PE_StoreU32(actor+0x9Cu,0x801AA43Cu);PE_StoreU32(actor+0xA8u,task);PE_StoreU32(task,0x801AA8ACu);PE_StoreU32(task+16u,1);
        PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D1A0u,0);PE_StoreU32(0x800A7918u,0xCDu);
        for(unsigned i=0;i<4;i++)PE_StoreU8(0x800B0DB4u+i,255);
        for(unsigned i=0;i<2;i++){PE_StoreU32(0x800B0E38u+i*4u,0x80155000u+i*32u);PE_StoreU32(0x800B0E44u+i*4u,0x80150000u);}
        for(unsigned n=M367T_cases[k].first;n<M367T_cases[k].end;n++) {
            uint64_t hash;unsigned frame=n-M367T_cases[k].first;
            PE_StoreU32(0x8009CDDCu,(frame+M367T_cases[k].bank)&1u);
            for(unsigned i=0;i<2;i++)for(unsigned j=0;j<4;j++)PE_StoreU32(0x80155000u+i*32u+j*4u,j?0x155000u+i*32u+(j-1u)*4u:0xFFFFFFu);
            waiting=PE_LoadU8(0x800BCEA8u)==2?waiting+1u:0;
            PE_StoreU32(0x8009D1F4u,waiting>M367T_cases[k].delay?0x100u:0);
            PE_StoreU32(0x8009D300u,task);func_80017018();func_80037870();func_80068E24();
            hash=hit_camera_hash(M367T_ranges,sizeof(M367T_ranges)/sizeof(M367T_ranges[0]));
            if(hash!=M367T_frames[n].hash)fprintf(stderr,"M367 transition%u frame%u pc%08X/%08X hash%016llX/%016llX\n",k,frame,PE_LoadU32(task),M367T_frames[n].pc,(unsigned long long)hash,(unsigned long long)M367T_frames[n].hash);
            ASSERT(hash==M367T_frames[n].hash,"M0367I transition frame differs from original");
            ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"transition component completed natively");
        }
        ASSERT(PE_LoadU32(0x800A7918u)==0xCEu && PE_LoadU32(0x8009D280u)==0xA80434C8u,"CD transition publishes CE and M0239I");
    }
    free(seed);PASS();
}

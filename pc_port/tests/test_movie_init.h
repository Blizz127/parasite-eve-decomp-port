#include "retail_movie_init_cases.h"
static void test_DAY2_movie_init(void)
{
    TEST_RETAIL_DISC1("DAY2_movie_init"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    for (unsigned k=0;k<sizeof(MOVINIT_cases)/sizeof(MOVINIT_cases[0]);k++) {
        PeGpuState gpu;
        uint64_t hash;
        ResetTestState();HostFB_Init();PE_GPU_Init();B54KR_SeedGpuStatic();
        for (unsigned j=0;j<sizeof(MOVINIT_ranges)/sizeof(MOVINIT_ranges[0]);j++)
            for (unsigned i=0;i<MOVINIT_ranges[j][1];i++)
                PE_StoreU8(0x80000000u+MOVINIT_ranges[j][0]+i,(uint8_t)(MOVINIT_cases[k].seed+i*17u));
        PE_StoreU32(0x80150000u,MOVINIT_cases[k].missing==1?0u:0x80122D00u);
        PE_StoreU32(0x80150004u,MOVINIT_cases[k].missing==2?0u:0x80180000u);
        PE_StoreU8(0x800B0DBAu,(uint8_t)MOVINIT_cases[k].active);
        PE_StoreU32(0x800B0CD8u,MOVINIT_cases[k].special?0x08000000u:0u);
        ASSERT(PE_GPU_WriteGP0(0x020000FFu) && PE_GPU_WriteGP0(320u) &&
               PE_GPU_WriteGP0(192u|(256u<<16)),"seed upper movie source");
        ASSERT(PE_GPU_WriteGP0(0x0200FF00u) && PE_GPU_WriteGP0(448u<<16) &&
               PE_GPU_WriteGP0(320u|(64u<<16)),"seed lower movie source");
        ASSERT(func_801216C4((int32_t)MOVINIT_cases[k].mode,0x80150000u)==(int)MOVINIT_cases[k].result,"movie init return");
        hash=hit_camera_hash(MOVINIT_ranges,sizeof(MOVINIT_ranges)/sizeof(MOVINIT_ranges[0]));
        if (hash!=MOVINIT_cases[k].hash) fprintf(stderr,"movie init %u hash %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)MOVINIT_cases[k].hash);
        ASSERT(hash==MOVINIT_cases[k].hash,"movie buffer/display state differs from original");
        PE_GPU_GetState(&gpu);
        ASSERT(gpu.move_count==(MOVINIT_cases[k].result?(MOVINIT_cases[k].special?1u:2u):0u),"movie backup copy count");
        for (unsigned y=0;y<256;y++) for(unsigned x=0;x<192;x++)
            ASSERT(B54KR_PixelIs(512u+x,y,MOVINIT_cases[k].result?31u:0u),"upper movie backup pixels");
        for (unsigned y=0;y<64;y++) for(unsigned x=0;x<320;x++)
            ASSERT(B54KR_PixelIs(512u+x,256u+y,MOVINIT_cases[k].result&&!MOVINIT_cases[k].special?992u:0u),"lower movie backup pixels");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"movie initializer boundary");
        for (unsigned i=0;i<224;i++) PE_StoreU8(0x800BCDC8u+i,238u);
        ASSERT(PE_GPU_WriteGP0(0x02000000u) && PE_GPU_WriteGP0(320u) &&
               PE_GPU_WriteGP0(192u|(256u<<16)),"erase upper movie source");
        ASSERT(PE_GPU_WriteGP0(0x02000000u) && PE_GPU_WriteGP0(448u<<16) &&
               PE_GPU_WriteGP0(320u|(64u<<16)),"erase lower movie source");
        func_80121A00();
        hash=hit_camera_hash(MOVINIT_ranges,sizeof(MOVINIT_ranges)/sizeof(MOVINIT_ranges[0]));
        if (hash!=MOVINIT_cases[k].restore_hash) fprintf(stderr,"movie restore %u hash %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)MOVINIT_cases[k].restore_hash);
        ASSERT(hash==MOVINIT_cases[k].restore_hash,"movie display restoration differs from original");
        PE_GPU_GetState(&gpu);
        ASSERT(gpu.move_count==((MOVINIT_cases[k].result?1u:0u)+
               (MOVINIT_cases[k].result||MOVINIT_cases[k].active?1u:0u))*(MOVINIT_cases[k].special?1u:2u),"movie restore copy count");
        for (unsigned y=0;y<256;y++) for(unsigned x=0;x<192;x++)
            ASSERT(B54KR_PixelIs(320u+x,y,MOVINIT_cases[k].result?31u:0u),"restored upper pixels");
        for (unsigned y=0;y<64;y++) for(unsigned x=0;x<320;x++)
            ASSERT(B54KR_PixelIs(x,448u+y,MOVINIT_cases[k].result&&!MOVINIT_cases[k].special?992u:0u),"restored lower pixels");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"movie restoration boundary");
    }
    for(unsigned k=0;k<sizeof(MOVINIT_flags)/sizeof(MOVINIT_flags[0]);k++) {
        PE_StoreU8(0x800B0DBBu,(uint8_t)MOVINIT_flags[k][1]);
        PE_StoreU8(0x801223F8u,(uint8_t)MOVINIT_flags[k][2]);
        func_801223A8((int32_t)MOVINIT_flags[k][0]);
        ASSERT(PE_LoadU8(0x801223F8u)==MOVINIT_flags[k][3],"movie completion flag differs from original");
    }
    PASS();
}

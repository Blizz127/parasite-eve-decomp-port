#include "retail_movie_frame_cases.h"
static void test_DAY2_movie_frame(void)
{
    TEST_RETAIL_DISC1("DAY2_movie_frame"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    for (unsigned k=0;k<sizeof(MOVFRAME_display)/sizeof(MOVFRAME_display[0]);k++) {
        ResetTestState();
        for (unsigned j=0;j<sizeof(MOVFRAME_display_ranges)/sizeof(MOVFRAME_display_ranges[0]);j++)
            for (unsigned i=0;i<MOVFRAME_display_ranges[j][1];i++)
                PE_StoreU8(0x80000000u+MOVFRAME_display_ranges[j][0]+i,(uint8_t)(MOVFRAME_display[k].seed+i*17u));
        PE_StoreU32(0x800956ECu,MOVFRAME_display[k].std);
        func_80121004((int32_t)MOVFRAME_display[k].bank,(int32_t)MOVFRAME_display[k].wide);
        ASSERT(hit_camera_hash(MOVFRAME_display_ranges,sizeof(MOVFRAME_display_ranges)/sizeof(MOVFRAME_display_ranges[0]))==MOVFRAME_display[k].hash,"movie display setup differs from original");
    }
    for (unsigned k=0;k<sizeof(MOVFRAME_cases)/sizeof(MOVFRAME_cases[0]);k++) {
        uint64_t hash;PeGpuState gpu;
        ResetTestState();HostFB_Init();PE_GPU_Init();B54KR_SeedGpuStatic();
        for (unsigned j=0;j<sizeof(MOVFRAME_frame_ranges)/sizeof(MOVFRAME_frame_ranges[0]);j++)
            for (unsigned i=0;i<MOVFRAME_frame_ranges[j][1];i++)
                PE_StoreU8(0x80000000u+MOVFRAME_frame_ranges[j][0]+i,0u);
        for(unsigned i=0;i<64;i++) PE_StoreU8(0x80140000u+i,(uint8_t)(37u+i*17u));
        PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x800C0DC8u,0x80150000u);
        PE_StoreU32(0x800C20C4u,2u);PE_StoreU32(0x800BE9ECu,0u);
        PE_StoreU16(0x80150000u,MOVFRAME_cases[k].timeout?0u:2u);
        PE_StoreU32(0x80150008u,MOVFRAME_cases[k].frame);
        PE_StoreU16(0x80150010u,(uint16_t)MOVFRAME_cases[k].width);
        PE_StoreU16(0x80150012u,(uint16_t)MOVFRAME_cases[k].height);
        PE_StoreU32(0x801227E4u,0x80151000u);PE_StoreU16(0x80151008u,(uint16_t)MOVFRAME_cases[k].limit);
        PE_StoreU16(0x801227E8u,(uint16_t)MOVFRAME_cases[k].last);
        PE_StoreU16(0x80122418u,MOVFRAME_cases[k].same?(uint16_t)MOVFRAME_cases[k].width:1u);
        PE_StoreU16(0x8012241Au,MOVFRAME_cases[k].same?(uint16_t)MOVFRAME_cases[k].height:1u);
        PE_StoreU8(0x800B0DBBu,(uint8_t)MOVFRAME_cases[k].wide);
        PE_StoreU8(0x800B0DBEu,152u);PE_StoreU32(0x8009D2C0u,2u);
        PE_StoreU32(0x8009B27Cu,0x1F801800u);
        PE_StoreU32(0x8009B280u,0x1F801801u);
        PE_StoreU32(0x8009B284u,0x1F801802u);
        PE_StoreU32(0x8009B288u,0x1F801803u);
        ASSERT(PE_GPU_WriteGP0(0x02FFFFFFu)&&PE_GPU_WriteGP0(0u)&&PE_GPU_WriteGP0(640u|(480u<<16)),"movie frame background");
        ASSERT((uint32_t)func_80121270(0x80140000u)==MOVFRAME_cases[k].result,"movie frame result");
        hash=hit_camera_hash(MOVFRAME_frame_ranges,sizeof(MOVFRAME_frame_ranges)/sizeof(MOVFRAME_frame_ranges[0]));
        if(hash!=MOVFRAME_cases[k].hash)fprintf(stderr,"movie frame %u hash%016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)MOVFRAME_cases[k].hash);
        ASSERT(hash==MOVFRAME_cases[k].hash,"movie frame RAM differs from original");
        PE_GPU_GetState(&gpu);
        ASSERT(gpu.fill_count==1u+(MOVFRAME_cases[k].wide?0u:MOVFRAME_cases[k].clears) &&
               gpu.mono_rectangle_count==(MOVFRAME_cases[k].wide?MOVFRAME_cases[k].clears:0u),"movie frame clear packet count");
        ASSERT(B54KR_PixelIs(0u,0u,MOVFRAME_cases[k].clears?0u:0x7FFFu),"movie frame clear origin");
        ASSERT(B54KR_PixelIs(MOVFRAME_cases[k].wide?479u:319u,479u,MOVFRAME_cases[k].clears?0u:0x7FFFu),"movie frame clear far corner");
        ASSERT(B54KR_PixelIs(MOVFRAME_cases[k].wide?480u:320u,479u,0x7FFFu),"movie frame clear outside");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"movie frame unresolved boundary");
    }
    PASS();
}

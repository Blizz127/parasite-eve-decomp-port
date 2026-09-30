#include "retail_movie_callback_cases.h"
static void test_DAY2_movie_callback(void)
{
    TEST_RETAIL_DISC1("DAY2_movie_callback"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    for (unsigned k=0;k<sizeof(MOVCB_cases)/sizeof(MOVCB_cases[0]);k++) {
        unsigned width=MOVCB_cases[k].wide?24u:16u;
        unsigned bank=MOVCB_cases[k].bank;
        uint64_t hash;
        ResetTestState();HostFB_Init();PE_GPU_Init();B54KR_SeedGpuStatic();
        for(unsigned j=0;j<sizeof(MOVCB_ranges)/sizeof(MOVCB_ranges[0]);j++)
            for(unsigned i=0;i<MOVCB_ranges[j][1];i++)
                PE_StoreU8(0x80000000u+MOVCB_ranges[j][0]+i,
                    j==4u?(uint8_t)(37u+i*17u):0u);
        PE_StoreU8(0x800B0DBBu,(uint8_t)MOVCB_cases[k].wide);
        PE_StoreU16(0x800B0CD0u,(uint16_t)MOVCB_cases[k].pending);
        PE_StoreU8(0x801223F8u,(uint8_t)MOVCB_cases[k].flag);
        PE_StoreU8(0x801228E0u,(uint8_t)MOVCB_cases[k].buffer);
        PE_StoreU8(0x801228F2u,(uint8_t)bank);
        PE_StoreU32(0x801228D8u,0x80160000u);PE_StoreU32(0x801228DCu,0x80161000u);
        for(unsigned b=0;b<2;b++) {
            PE_StoreU16(0x801228E2u+b*8u,32u);
            PE_StoreU16(0x801228E4u+b*8u,(uint16_t)(40u+b*8u));
            PE_StoreU16(0x801228E6u+b*8u,(uint16_t)(width*(MOVCB_cases[k].last?1u:3u)));
            PE_StoreU16(0x801228E8u+b*8u,4u);
            for(unsigned i=0;i<width*4u;i++)
                PE_StoreU16(0x80160000u+b*0x1000u+i*2u,(uint16_t)(0x1000u+b*0x2000u+i));
        }
        PE_StoreU16(0x801228F4u,32u);PE_StoreU16(0x801228F6u,(uint16_t)(40u+bank*8u));
        PE_StoreU16(0x801228F8u,(uint16_t)width);PE_StoreU16(0x801228FAu,4u);
        PE_StoreU32(0x8009CDDCu,bank);PE_StoreU32(0x800956ECu,0u);
        PE_StoreU32(0x800B89F4u,1u); /* real stream-handler active guard */
        func_801214D4();
        hash=hit_camera_hash(MOVCB_ranges,sizeof(MOVCB_ranges)/sizeof(MOVCB_ranges[0]));
        if(hash!=(MOVCB_cases[k].boundary?MOVCB_cases[k].prefix_hash:MOVCB_cases[k].full_hash))
            fprintf(stderr,"movie callback case %u boundary%u hash%016llX\n",k,MOVCB_cases[k].boundary,(unsigned long long)hash);
        ASSERT(hash==(MOVCB_cases[k].boundary?MOVCB_cases[k].prefix_hash:MOVCB_cases[k].full_hash),"callback RAM differs from original execution/prefix");
        if(MOVCB_cases[k].boundary) {
            ASSERT(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY,"callback must stop before an unported callee");
            ASSERT(CountOrderLog(MOVCB_cases[k].boundary==1u?"func_8007C564":"func_8010C01C")==1,"callback boundary identity");
            if(MOVCB_cases[k].boundary==2u) {
                ASSERT(g_bootstrap_arg4_call_count==1 &&
                    g_bootstrap_arg4_calls[0].arg0==MOVCB_cases[k].arg0 &&
                    g_bootstrap_arg4_calls[0].arg1==MOVCB_cases[k].arg1,"next pixel buffer/output length differs");
            }
            ASSERT(B54KR_PixelIs(32u,40u+bank*8u,0u),"unported decoder must not upload stale pixels");
        } else {
            ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"completed last slice encountered a boundary");
            ASSERT(PE_GPU_DMA2Pending(),"last slice did not schedule its GPU DMA");
            ASSERT(func_80074DC0(0)==0 && !PE_GPU_DMA2Pending() && !PE_Port_ShouldStop(),"slice GPU DMA completion failed");
            for(unsigned y=0;y<4;y++)
                for(unsigned x=0;x<width;x++)
                    ASSERT(B54KR_PixelIs(32u+x,40u+bank*8u+y,
                        (uint16_t)(0x1000u+MOVCB_cases[k].buffer*0x2000u+y*width+x)),"callback uploaded wrong buffer, rectangle or pixel");
            ASSERT(B54KR_PixelIs(31u,40u+bank*8u,0u) && B54KR_PixelIs(32u+width,40u+bank*8u,0u),"slice upload touched outside rectangle");
        }
    }
    PASS();
}

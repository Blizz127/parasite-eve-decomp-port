#include "retail_mdec_io_cases.h"
static void test_DAY2_mdec_dma(void)
{
    TEST_RETAIL_DISC1("DAY2_mdec_dma"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv); TEST_RETAIL_FIXUPS(RETAILFIX_mdec_pixel_tables);
    PeMdecState state;
    for(unsigned k=0;k<sizeof(MDECIO_input)/sizeof(MDECIO_input[0]);k++) {
        ResetTestState();PE_StoreU32(0x80140000u,MDECIO_input[k].command);
        func_8010BFA0(0x80140000u,MDECIO_input[k].mode);PE_MDEC_GetState(&state);
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x80140000u)==MDECIO_input[k].changed &&
            state.command_last_write==MDECIO_input[k].changed && state.uploads[0].source==MDECIO_input[k].address+4u &&
            state.uploads[0].word_count==MDECIO_input[k].words,"libpress input differs from original graph");
    }
    for(unsigned k=0;k<sizeof(MDECIO_output)/sizeof(MDECIO_output[0]);k++) {
        ResetTestState();PE_GPU_WriteDPCR(0x12345678u);
        func_8010C01C(MDECIO_output[k].destination,MDECIO_output[k].words);PE_MDEC_GetState(&state);
        ASSERT(!PE_Port_ShouldStop() && PE_GPU_ReadDPCR()==MDECIO_output[k].dpcr && state.dma1_chcr==MDECIO_output[k].chcr &&
            state.dma1_madr==(MDECIO_output[k].madr&0xFFFFFFu) && state.dma1_bcr==MDECIO_output[k].bcr && !state.completed_output_count,"libpress output differs from original register graph");
    }
    for(unsigned wide=0;wide<2u;wide++) {
        ResetTestState();HostFB_Init();PE_GPU_Init();func_80073C94();B54KR_SeedGpuStatic();
        for(unsigned i=0;i<33u;i++) {
            PE_StoreU32(0x8010DA0Cu+i*4u,MDECPIX_quant[i]);PE_StoreU32(0x8010DA90u+i*4u,MDECPIX_scale[i]);
        }
        func_8010BE3C(0);func_8010C0D8(0x801214D4u);
        PE_StoreU32(0x80140000u,0x38000020u);
        for(unsigned i=0;i<64u;i++) PE_StoreU16(0x80140004u+i*2u,0xFE00u);
        for(unsigned macro=0;macro<2u;macro++) for(unsigned block=0;block<6u;block++) {
            int dc=block<2u?0:macro?-64:64;
            PE_StoreU16(0x80140004u+(macro*12u+block*2u)*2u,(uint16_t)((8u<<10u)|((unsigned)dc&1023u)));
        }
        unsigned width=wide?24u:16u,bytes=width*16u*2u;
        PE_StoreU8(0x800B0DBBu,(uint8_t)wide);
        PE_StoreU32(0x801228D8u,0x80160000u);PE_StoreU32(0x801228DCu,0x80161000u);
        for(unsigned bank=0;bank<2u;bank++) {
            PE_StoreU16(0x801228E2u+bank*8u,32u);PE_StoreU16(0x801228E4u+bank*8u,(uint16_t)(40u+bank*40u));
            PE_StoreU16(0x801228E6u+bank*8u,(uint16_t)(width*2u));PE_StoreU16(0x801228E8u+bank*8u,16u);
        }
        PE_StoreU16(0x801228F4u,32u);PE_StoreU16(0x801228F6u,40u);
        PE_StoreU16(0x801228F8u,(uint16_t)width);PE_StoreU16(0x801228FAu,16u);
        for(unsigned i=0;i<bytes;i++) PE_StoreU8(0x80160000u+i,0xCCu);
        func_8010BFA0(0x80140000u,wide);func_8010C01C(0x80160000u,bytes/4u);
        PE_MDEC_GetState(&state);
        ASSERT(!PE_Port_ShouldStop() && state.dma0_active && (state.dma1_chcr&0x01000000u) && !state.completed_output_count && PE_LoadU8(0x80160000u)==0xCCu,"MDEC output completed during submission");
        uint32_t dpcr=PE_GPU_ReadDPCR();PE_GPU_WriteDPCR(dpcr&~0x80u);
        HostFB_VSync(-1);PE_MDEC_GetState(&state);
        ASSERT(!state.dma0_active && !state.completed_output_count && PE_LoadU8(0x80160000u)==0xCCu,"DMA1 ignored DPCR gate");
        PE_GPU_WriteDPCR(dpcr);uint16_t mask=PE_IRQ_GetMask();(void)PE_IRQ_ExchangeMask((uint16_t)(mask&~8u));
        HostFB_VSync(-1);PE_MDEC_GetState(&state);
        ASSERT(!PE_Port_ShouldStop() && state.completed_output_count==1u && !(state.dma1_chcr&0x01000000u) && !PE_LoadU8(0x801228E0u) && (PE_IRQ_ReadStatus()&8u),"DMA output/masked IRQ separation");
        (void)PE_IRQ_ExchangeMask(mask);HostFB_VSync(-1);PE_MDEC_GetState(&state);
        ASSERT(!PE_Port_ShouldStop() && state.output_count==2u && state.completed_output_count==1u && PE_LoadU8(0x801228E0u)==1u && !PE_LoadU8(0x801228FCu),"first slice callback did not queue second output");
        HostFB_VSync(-1);
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU8(0x801228FCu)==1u && PE_MDEC_DecodedMacroblocks()==2u,"second slice did not finish frame");
        ASSERT(func_80074DC0(0)==0 && !PE_Port_ShouldStop(),"movie GPU upload completion");
        for(unsigned y=0;y<16u;y++) for(unsigned x=0;x<width*2u;x++) {
            uint16_t pixel=wide?(x<width?0x9090u:0x7070u):(x<width?0x4A52u:0x39CEu);
            uint16_t actual=0;PE_GPU_ReadVRAM(32u+x,40u+y,&actual);
            if(actual!=pixel) fprintf(stderr,"MDEC DMA wide%u pixel%u,%u actual%04X expected%04X buffers%04X/%04X\n",wide,x,y,actual,pixel,PE_LoadU16(0x80160000u),PE_LoadU16(0x80161000u));
            ASSERT(actual==pixel,"decoded movie slices differ in VRAM");
        }
        ASSERT(B54KR_PixelIs(31u,40u,0u) && B54KR_PixelIs(32u+width*2u,40u,0u),"movie upload exceeded frame rectangle");
    }
    PASS();
}

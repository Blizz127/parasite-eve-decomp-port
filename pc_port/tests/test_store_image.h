#include "retail_store_image_cases.h"

static uint16_t nam10_pixel(uint32_t ordinal) {return (uint16_t)(ordinal*37u+0x123u);}

static void nam10_seed_vram(uint32_t position,uint32_t width,uint32_t height)
{
    uint32_t i,pixels=width*height;
    if (!pixels) return;
    (void)PE_GPU_WriteGP0(0xA0000000u);(void)PE_GPU_WriteGP0(position);
    (void)PE_GPU_WriteGP0(width|(height<<16u));
    for (i=0;i<pixels;i+=2u)
        (void)PE_GPU_WriteGP0((uint32_t)nam10_pixel(i)|((uint32_t)nam10_pixel(i+1u)<<16u));
}

static void test_NAM10_retail_store_image(void)
{
    const pe_addr_t rect=0x801E0000u,destination=0x800D0000u;
    uint32_t k,i,result;
    TEST("NAM10_retail_store_image");
    for (k=0;k<sizeof(NAM10_store_image_cases)/sizeof(NAM10_store_image_cases[0]);k++) {
        uint32_t width=NAM10_store_image_cases[k].width,height=NAM10_store_image_cases[k].height;
        uint32_t pixels=width*height,words=(pixels+1u)/2u,prefix=NAM10_store_image_cases[k].count;
        uint32_t position=(uint16_t)NAM10_store_image_cases[k].rect[0]|
            ((uint32_t)(uint16_t)NAM10_store_image_cases[k].rect[1]<<16u);
        ResetTestState();B53E_SetupRetailGpu();
        for (i=0;i<31u;i++) PE_GPU_VBlankStep();
        PE_StoreU16(0x80095750u,(uint16_t)NAM10_store_image_cases[k].limits[0]);
        PE_StoreU16(0x80095752u,(uint16_t)NAM10_store_image_cases[k].limits[1]);
        for (i=0;i<4u;i++) PE_StoreU16(rect+i*2u,(uint16_t)NAM10_store_image_cases[k].rect[i]);
        for (i=0;i<words+1u;i++) PE_StoreU32(destination+i*4u,0xA5A5A5A5u);
        nam10_seed_vram(position,width,height);
        result=(uint32_t)func_800768A0(rect,destination);
        ASSERT(result==NAM10_store_image_cases[k].result,"StoreImage worker result differs from original");
        ASSERT(PE_LoadU16(rect+4u)==width && PE_LoadU16(rect+6u)==height,"StoreImage rectangle clamp differs");
        ASSERT(PE_LoadU32(0x80095888u)==271u && !PE_LoadU32(0x8009588Cu),"StoreImage timeout initialization differs");
        for (i=0;i<prefix;i++)
            ASSERT(PE_LoadU32(destination+i*4u)==NAM10_store_image_cases[k].prefix[i],"CPU prefix differs from original GPUREAD sequence");
        for (i=prefix;i<words+1u;i++)
            ASSERT(PE_LoadU32(destination+i*4u)==0xA5A5A5A5u,"readback wrote outside its CPU prefix before DMA completion");
        if (NAM10_store_image_cases[k].dma[2]) {
            uint64_t token=PE_GPU_DMA2EventToken();uint32_t value=0u;
            ASSERT(PE_GPU_DMA2Pending() && PE_GPU_ReadDMA2MADR()==NAM10_store_image_cases[k].dma[0] &&
                PE_GPU_ReadDMA2BCR()==NAM10_store_image_cases[k].dma[1] &&
                PE_GPU_ReadDMA2CHCR()==NAM10_store_image_cases[k].dma[2],"readback DMA registers differ from original");
            ASSERT(!PE_GPU_ReadGP0(&value),"CPU stole pixels from an active readback DMA");
            ASSERT(!PE_GPU_ServiceDMA2Completion(token+1u) && PE_GPU_DMA2Pending(),"stale event completed the readback");
            ASSERT(PE_GPU_ServiceDMA2Completion(token),"readback DMA did not complete");
        } else ASSERT(!PE_GPU_DMA2Pending(),"CPU-only readback created a DMA");
        for (i=0;i<pixels;i++) {
            if (PE_LoadU16(destination+i*2u)!=nam10_pixel(i))
                fprintf(stderr,"StoreImage case %u pixel %u: %04X/%04X\n",k,i,PE_LoadU16(destination+i*2u),nam10_pixel(i));
            ASSERT(PE_LoadU16(destination+i*2u)==nam10_pixel(i),"readback changed pixel order or VRAM wrapping");
        }
        ASSERT(PE_LoadU32(destination+words*4u)==0xA5A5A5A5u,"readback overran the destination");
        ASSERT(!(PE_GPU_ReadStatus()&PE_GPU_STATUS_READY_READ),"readback retained ready status after its last pixel");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"readback uses the native GPU without substitutions");
    }
    PASS();
}

static void test_NAM10_queued_store_image(void)
{
    RECT rect={9,480,32,2};uint32_t i;
    TEST("NAM10_queued_store_image");
    ResetTestState();B53E_SetupRetailGpu();B52_SeedGpuDispatch();
    PE_StoreU32(B52_JTB+28u,0x800768A0u);
    for (i=0;i<64u;i++) PE_StoreU16(0x80180000u+i*2u,nam10_pixel(i));
    PE_Fill(0x800E0000u,128u,0xA5u);
    (void)func_8007506C(&rect,0x80180000u);
    ASSERT(PE_GPU_DMA2Pending(),"initial load did not issue DMA");
    (void)func_800750CC(&rect,0x800E0000u);
    rect.x=320;rect.y=8;
    ASSERT(PE_GPU_ReadDMA2CHCR()==PE_GPU_DMA2_CHCR_LOAD && PE_LoadU32(0x800E0000u)==0xA5A5A5A5u,
        "queued readback ran before the preceding upload completed");
    {int sync=func_80074DC0(0);
     if (sync) fprintf(stderr,"StoreImage queue sync=%d pending=%d CHCR=%08X stop=%d first=%08X\n",sync,
        PE_GPU_DMA2Pending(),PE_GPU_ReadDMA2CHCR(),PE_Port_GetStopReason(),PE_LoadU32(0x800E0000u));
     ASSERT(sync==0,"DrawSync failed to drain upload then readback");}
    for (i=0;i<64u;i++) ASSERT(PE_LoadU16(0x800E0000u+i*2u)==nam10_pixel(i),"queued readback lost its copied rectangle or upload order");
    ASSERT(!PE_GPU_DMA2Pending() && !g_stub_order_count && !PE_Port_ShouldStop(),"queued readback left an unresolved dependency");
    PASS();
}

#include "retail_clear_image_cases.h"
static void test_DAY1_clear_image_packets(void)
{
    TEST("DAY1_clear_image_packets");
    for (unsigned i=0;i<sizeof(DAY1_clear_cases)/sizeof(DAY1_clear_cases[0]);i++) {
        ResetTestState();PE_GPU_Init();
        PE_Fill(0x800A3300u,56u,0xA5u);
        PE_StoreU32(0x80140000u,DAY1_clear_cases[i].xy);
        PE_StoreU32(0x80140004u,DAY1_clear_cases[i].wh);
        PE_StoreU32(0x80095750u,DAY1_clear_cases[i].limit);
        ASSERT(PE_GPU_WriteGP0(0xE1000000u|DAY1_clear_cases[i].mode) &&
               PE_GPU_WriteGP0(0xE3000000u|DAY1_clear_cases[i].area[0]) &&
               PE_GPU_WriteGP0(0xE4000000u|DAY1_clear_cases[i].area[1]) &&
               PE_GPU_WriteGP0(0xE5000000u|DAY1_clear_cases[i].area[2]),"GPU seed rejected");
        ASSERT(func_80076434(0x80140000u,DAY1_clear_cases[i].color)==0 &&
               !PE_Port_ShouldStop(),"clear worker did not return");
        ASSERT(PE_LoadU32(0x80140000u)==DAY1_clear_cases[i].xy &&
               PE_LoadU32(0x80140004u)==DAY1_clear_cases[i].rect_out,"retail RECT clamp differs");
        for (unsigned j=0;j<14;j++) {
            if (PE_LoadU32(0x800A3300u+j*4u)!=DAY1_clear_cases[i].packet[j])
                fprintf(stderr,"clear case %u word %u: %08X/%08X\n",i,j,
                    PE_LoadU32(0x800A3300u+j*4u),DAY1_clear_cases[i].packet[j]);
            ASSERT(PE_LoadU32(0x800A3300u+j*4u)==DAY1_clear_cases[i].packet[j],"packet differs from original instructions");
        }
    }
    PASS();
}

static void test_DAY1_clear_image_dispatch_pixels(void)
{
    TEST_RETAIL_DISC1("DAY1_clear_image_dispatch_pixels"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    for (unsigned queued=0;queued<2;queued++) {
        PeGpuState gpu;
        int returned=0;
        ResetTestState();PE_GPU_Init();B54KR_SeedGpuStatic();
        ASSERT(PE_GPU_WriteGP0(0x02FFFFFFu) && PE_GPU_WriteGP0(0u) &&
               PE_GPU_WriteGP0(0x01E00140u),"seed both frame buffers");
        PE_StoreU32(0x80140000u,0u);PE_StoreU32(0x80140004u,0x00F00140u);
        ASSERT(func_80076C34(0x80076434u,0x80140000u,8,0x0000FFu)==0 &&
               !PE_Port_ShouldStop(),"direct quick clear failed");
        ASSERT(B54KR_PixelIs(0,0,0x001F) && B54KR_PixelIs(319,239,0x001F) &&
               B54KR_PixelIs(0,240,0x7FFF),"quick clear affected wrong frame buffer");
        ASSERT(PE_GPU_WriteGP0(0xE300500Au) && PE_GPU_WriteGP0(0xE400A014u) &&
               PE_GPU_WriteGP0(0xE5000801u) && PE_GPU_WriteGP0(0xE6000003u),"seed restrictive environment");
        PE_StoreU32(0x80140000u,0x00F00001u);PE_StoreU32(0x80140004u,0x00030005u);
        if (queued) {
            PE_StoreU32(0x800BD030u,0x80076434u);
            PE_StoreU32(0x800BD034u,0x80140000u);
            PE_StoreU32(0x800BD038u,0xFF0000u);
            PE_StoreU32(0x80095874u,1u);PE_StoreU32(0x80095878u,0u);
            ASSERT(PE_func_80076EE4_Pump(&returned)==0 && returned &&
                   PE_LoadU32(0x80095878u)==1u,"queued clear not consumed");
        } else ASSERT(func_80076C34(0x80076434u,0x80140000u,8,0xFF0000u)==0,"direct rectangle clear failed");
        ASSERT(!PE_Port_ShouldStop(),"clear reached unresolved boundary");
        for (unsigned y=239;y<=243;y++) for (unsigned x=0;x<=6;x++) {
            uint16_t expected=y==239?0x001F:(y<=242 && x>=1 && x<=5?0x7C00:0x7FFF);
            ASSERT(B54KR_PixelIs(x,y,expected),"precise clear extent/color differs");
        }
        PE_GPU_GetState(&gpu);
        ASSERT(gpu.drawing_area_top_left==0xE300500Au &&
               gpu.drawing_area_bottom_right==0xE400A014u &&
               gpu.drawing_offset==0xE5000801u && gpu.mask_setting==0xE6000000u,
               "retail drawing state restore/mask reset differs");
    }
    PASS();
}

static void test_DAY1_gpu_drawing_status(void)
{
    static const uint32_t cases[][2]={{0,0},{0x7FF,0x7FF},{0x800,0x8000},
                                     {0xFFF,0x87FF},{0x3000,0},{0x3555,0x555}};
    TEST("DAY1_gpu_drawing_status");
    ResetTestState();PE_GPU_Init();
    for (unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        uint32_t preserved=PE_GPU_ReadStatus()&~0x9FFFu;
        ASSERT(PE_GPU_WriteGP0(0xE6000003u) &&
               PE_GPU_WriteGP0(0xE1000000u|cases[i][0]),"GPU mode rejected");
        ASSERT(PE_GPU_ReadStatus()==(preserved|cases[i][1]|0x1800u),"E1/E6 GPUSTAT readback differs");
        ASSERT(PE_GPU_WriteGP0(0xE6000000u) &&
               PE_GPU_ReadStatus()==(preserved|cases[i][1]),"mask reset damaged draw mode/status");
    }
    PASS();
}

static void test_DAY1_clear_image_queue_ownership(void)
{
    RECT upload={40,50,16,2};
    TEST_RETAIL_DISC1("DAY1_clear_image_queue_ownership"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    ResetTestState();HostFB_Init();PE_GPU_Init();B54KR_SeedGpuStatic();
    for (unsigned i=0;i<16;i++) PE_StoreU32(0x80170000u+i*4u,0x7FFF7FFFu);
    ASSERT(func_8007506C(&upload,0x80170000u)==0 && PE_GPU_DMA2Pending(),"DMA seed failed");
    PE_StoreU32(0x80140000u,0x00F00000u);PE_StoreU32(0x80140004u,0x00010400u);
    ASSERT(func_80076C34(0x80076434u,0x80140000u,8,0xFFu)==1 &&
           PE_LoadU32(0x80095874u)==1 && PE_LoadU32(0x80095878u)==0 &&
           PE_LoadU32(0x800BD034u)==0x800BD03Cu &&
           PE_LoadU32(0x800BD040u)==0x00010400u,"queued RECT not copied before return");
    PE_StoreU32(0x80140000u,0x12345678u);PE_StoreU32(0x80140004u,0xABCDEF01u);
    ASSERT(func_80074DC0(0)==0 && !PE_GPU_DMA2Pending() && !PE_Port_ShouldStop(),"queued clear did not drain");
    ASSERT(PE_LoadU32(0x80095878u)==1 && PE_LoadU32(0x800BD040u)==0x000103FFu &&
           PE_LoadU32(0x80140000u)==0x12345678u && PE_LoadU32(0x80140004u)==0xABCDEF01u,
           "clear clamped caller storage instead of retained payload");
    ASSERT(B54KR_PixelIs(0,240,0x001F) && B54KR_PixelIs(1022,240,0x001F) &&
           B54KR_PixelIs(1023,240,0) && B54KR_PixelIs(40,50,0x7FFF),"queued clear lost copied geometry/order");
    PASS();
}

static void test_DAY1_clear_sdk_lifetime(void)
{
    RECT rect={0,240,1024,1},upload={40,50,16,2};
    TEST_RETAIL_DISC1("DAY1_clear_sdk_lifetime"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    ResetTestState();HostFB_Init();PE_GPU_Init();B54KR_SeedGpuStatic();
    HostFB_ClearImage(0,0,320,240,17,33,65);
    ASSERT(func_80074F44(&rect,255,0,0)==0 && rect.w==1023 && rect.h==1 &&
           B54KR_PixelIs(0,240,0x001F) && B54KR_PixelIs(1022,240,0x001F) &&
           B54KR_PixelIs(1023,240,0),"SDK direct clear/clamp differs");
    ASSERT(HostFB_GetPixels()[0]==17 && HostFB_GetPixels()[1]==33 && HostFB_GetPixels()[2]==65,
           "SDK clear incorrectly modified presented host buffer");
    for (unsigned i=0;i<16;i++) PE_StoreU32(0x80170000u+i*4u,0x7FFF7FFFu);
    ASSERT(func_8007506C(&upload,0x80170000u)==0 && PE_GPU_DMA2Pending(),"SDK queued seed failed");
    rect.x=0;rect.y=241;rect.w=1024;rect.h=1;
    ASSERT(func_80074F44(&rect,0,0,255)==1 && rect.w==1024 && rect.h==1,
           "queued SDK changed caller RECT before issue");
    rect.x=123;rect.y=100;rect.w=12;rect.h=3;
    ASSERT(func_80074DC0(0)==0 && !PE_Port_ShouldStop() &&
           rect.x==123 && rect.y==100 && rect.w==12 && rect.h==3 &&
           PE_LoadU32(0x800BD040u)==0x000103FFu &&
           B54KR_PixelIs(0,241,0x7C00) && B54KR_PixelIs(1022,241,0x7C00),
           "queued SDK retained caller pointer or lost copied geometry");
    PASS();
}

static void test_DAY1_clear_sdk_dispatch_boundary(void)
{
    RECT rect={0,0,320,240};
    const BootstrapArgCall4 *call;
    TEST_RETAIL_DISC1("DAY1_clear_sdk_dispatch_boundary"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    ResetTestState();PE_GPU_Init();B54KR_SeedGpuStatic();
    PE_StoreU32(0x8009570Cu,0xDEADBEEFu);
    ASSERT(func_80074F44(&rect,0x12,0x34,0x56)==0 && PE_Port_ShouldStop(),"dirty dispatch silently accepted");
    call=&g_bootstrap_arg4_calls[g_bootstrap_arg4_call_count-1];
    ASSERT(call->target==0xDEADBEEFu && call->arg0==0x80076434u &&
           call->arg2==8u && call->arg3==0x563412u && rect.w==320 &&
           PE_LoadU32(0x800A3300u)==0 && B54KR_PixelIs(0,0,0),"SDK argument packing or boundary mutated clear state");
    PASS();
}

#include "retail_clear_sdk_cases.h"
static void test_DAY1_clear_sdk_original_arguments(void)
{
    TEST_RETAIL_DISC1("DAY1_clear_sdk_original_arguments"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    for (unsigned i=0;i<sizeof(DAY1_clear_sdk_colors)/sizeof(DAY1_clear_sdk_colors[0]);i++) {
        RECT rect={0,240,64,1};
        const uint32_t *c=DAY1_clear_sdk_colors[i];
        const BootstrapArgCall4 *call;
        ResetTestState();PE_GPU_Init();B54KR_SeedGpuStatic();
        ASSERT(func_80074F44(&rect,(uint8_t)c[0],(uint8_t)c[1],(uint8_t)c[2])==0 &&
               !PE_Port_ShouldStop(),"SDK did not return original direct result");
        ASSERT(g_bootstrap_arg4_call_count==1,"SDK dispatch count differs");
        call=&g_bootstrap_arg4_calls[0];
        ASSERT(call->target==0x80076C34u && call->arg0==0x80076434u &&
               call->arg2==8 && call->arg3==c[3] && rect.w==64 && rect.h==1,
               "SDK arguments differ from original executable");
    }
    PASS();
}

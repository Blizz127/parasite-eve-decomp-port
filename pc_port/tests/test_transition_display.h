#include "retail_transition_display_cases.h"
static void test_DAY1_transition_display(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_display"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    for (unsigned k=0;k<sizeof(DAY1_display_cases)/sizeof(DAY1_display_cases[0]);k++) {
        unsigned step=DAY1_display_cases[k].step;
        uint64_t hash;
        int mask,presented;
        if (!step) {
            ResetTestState();HostFB_Init();PE_GPU_Init();B54KR_SeedGpuStatic();
            PE_StoreU32(0x80095730u,0x80076354u);
            for (unsigned j=0;j<sizeof(DAY1_display_ranges)/sizeof(DAY1_display_ranges[0]);j++)
                for (unsigned i=0;i<DAY1_display_ranges[j][1];i++)
                    PE_StoreU8(0x80000000u+DAY1_display_ranges[j][0]+i,
                        (uint8_t)(DAY1_display_cases[k].seed+i*17u+(i>>8)));
            PE_StoreU32(0x800B0E38u,DAY1_display_cases[k].swap?0x80145000u:0x80140000u);
            PE_StoreU32(0x800B0E3Cu,DAY1_display_cases[k].swap?0x80140000u:0x80145000u);
            PE_StoreU32(0x800B0E4Cu,DAY1_display_cases[k].arena);
            ASSERT(PE_GPU_WriteGP0(0x02FFFFFFu) && PE_GPU_WriteGP0(0u) &&
                   PE_GPU_WriteGP0(0x01FF03FFu),"VRAM poison seed failed");
        }
        if (step==0 || step==4) func_8019BD78();
        else {
            pe_addr_t ctx=step==1?0x8019C1F8u:step==2?0x8019C270u:0x80150000u;
            PE_StoreU32(0x8019C9C0u,ctx);PE_StoreU32(ctx,0xDEADBEEFu);
            if (step==3) PE_StoreU32(ctx+4u,0x8014A000u);
            func_8019BF50();
        }
        hash=hit_camera_hash(DAY1_display_ranges,sizeof(DAY1_display_ranges)/sizeof(DAY1_display_ranges[0]));
        if (hash!=DAY1_display_cases[k].hash)
            fprintf(stderr,"display case%u step%u %016llX/%016llX\n",k,step,
                (unsigned long long)hash,(unsigned long long)DAY1_display_cases[k].hash);
        ASSERT(hash==DAY1_display_cases[k].hash,"display/OT state differs from original instructions");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"display setup crossed a boundary");
        HostFB_GetState(NULL,NULL,&presented,&mask);
        ASSERT(mask==1 && presented==0,"display mask/presentation sequencing differs");
        ASSERT(g_pe_gte.ofx==160*65536 && g_pe_gte.ofy==120*65536 &&
               g_pe_gte.h==768 && (uint16_t)g_pe_gte.dqa==0xEC89 &&
               g_pe_gte.dqb==0x024CC000,"geometry/fog setup differs from original GTE case");
        for (unsigned i=0;i<3;i++) ASSERT(g_pe_gte.bk[i]==0x800 && g_pe_gte.fc[i]==0,"lighting setup differs");
        if (!step || step==4) {
            PeGpuState gpu;PE_GPU_GetState(&gpu);
            ASSERT(gpu.fill_count==(step==0?3u:5u),"both buffers were not cleared once per setup");
            for (unsigned y=0;y<512;y++) for (unsigned x=0;x<1024;x++) {
                uint16_t expected=y==511?0:(x<320 && y<480?0:0x7FFFu);
                ASSERT(B54KR_PixelIs(x,y,expected),"display clear modified wrong VRAM region");
            }
        }
    }
    PASS();
}

static void test_DAY1_transition_display_boundary(void)
{
    int mask;
    TEST_RETAIL_DISC1("DAY1_transition_display_boundary"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    ResetTestState();HostFB_Init();PE_GPU_Init();B54KR_SeedGpuStatic();
    PE_StoreU32(0x800B0E38u,0x80140000u);PE_StoreU32(0x800B0E3Cu,0x80145000u);
    PE_StoreU32(0x800B0E4Cu,0x80100000u);PE_StoreU32(0x8019C9C0u,0xDEADBEEFu);
    PE_StoreU32(0x8019C200u,0xA5A5A5A5u);
    PE_StoreU32(0x80095730u,0xBADu);
    func_8019BD78();HostFB_GetState(NULL,NULL,NULL,&mask);
    ASSERT(PE_Port_ShouldStop() && mask==0 && PE_LoadU32(0x8019C9C0u)==0xDEADBEEFu &&
           PE_LoadU32(0x8019C200u)==0xA5A5A5A5u && PE_LoadU32(0x800A3300u)==0,
           "setup advanced beyond unresolved first OTC call");
    ASSERT(PE_LoadU32(0x8019C1FCu)==0x80140000u && PE_LoadU32(0x8019C274u)==0x80145000u,
           "setup lost original prefix before OTC call");
    PE_Port_RunControlReset();PE_StoreU32(0x8019C9C0u,0x8019C270u);
    PE_StoreU32(0x8019C270u,0x12345678u);func_8019BF50();
    ASSERT(PE_Port_ShouldStop() && PE_LoadU32(0x8019C270u)==0x12345678u,
           "bank reset advanced beyond unresolved OTC call");
    PASS();
}

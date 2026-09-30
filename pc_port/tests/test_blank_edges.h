static unsigned blank_edge_callbacks;
static void blank_edge_callback(void) {blank_edge_callbacks++;}
static void test_DAY1_blank_edges(void)
{
    TEST("DAY1_blank_edges");
    for(unsigned n=0;n<32;n++) {
        ResetTestState();PE_Callback_Init();func_80073C94();
        PeIrqGeneration generation=PE_IRQ_Generation();
        PE_Callback_Bind(0x80130000u,blank_edge_callback);PE_Callback_SetSlot(0,0x80130000u);
        blank_edge_callbacks=0;
        PE_IRQ_ExchangeMask(0);
        unsigned locks=n%4;
        for(unsigned i=0;i<locks;i++)func_80072714();
        ASSERT(PE_GPU_SetVBlank(1,generation),"initial high level rejected");
        ASSERT(!PE_IRQ_ReadStatus(),"initial/repeated blanking must not assert a new edge");
        PE_GPU_SetHBlank(0,generation);PE_GPU_SetHBlank(1,generation);
        ASSERT(!PE_Timer1_ReadCounter(),"HBlank before first VBlank must remain gated");
        for(unsigned frame=0;frame<1+n/4;frame++) {
            PE_GPU_SetVBlank(0,generation);PE_GPU_SetVBlank(1,generation);PE_GPU_SetVBlank(1,generation);
            for(unsigned line=0;line<3;line++) {
                PE_GPU_SetHBlank(0,generation);PE_GPU_SetHBlank(1,generation);PE_GPU_SetHBlank(1,generation);
            }
        }
        ASSERT(PE_IRQ_ReadStatus()==1 && !blank_edge_callbacks && !PE_LoadU32(0x800956ACu),"edge producer must latch without dispatch");
        ASSERT(PE_Timer1_ReadCounter()==3*(1+n/4),"duplicate levels must not tick timer twice");
        PeGpuState before,after;PE_GPU_GetState(&before);
        ASSERT(before.vblank_edge_count==1+n/4 && before.hblank_edge_count==1+3*(1+n/4),"physical edge counts differ");
        ASSERT(!PE_GPU_SetVBlank(0,generation-1) && !PE_GPU_SetHBlank(0,generation-1),"stale edge accepted");
        PE_GPU_GetState(&after);
        ASSERT(after.vblank_level==before.vblank_level && after.hblank_level==before.hblank_level,"stale input changed signal levels");
        ASSERT(PE_IRQ_ServicePendingForGeneration(generation)==PE_IRQ_SERVICE_RETURNED && !blank_edge_callbacks,"masked/locked pending IRQ dispatched");
        PE_IRQ_ExchangeMask(1);
        for(unsigned i=0;i<locks;i++) {
            ASSERT(PE_IRQ_ServicePendingForGeneration(generation)==PE_IRQ_SERVICE_RETURNED && !blank_edge_callbacks && PE_IRQ_ReadStatus()==1,"critical section must retain pending IRQ");
            func_80072724();
        }
        ASSERT(PE_IRQ_ServicePendingForGeneration(generation)==PE_IRQ_SERVICE_RETURNED && blank_edge_callbacks==1 && !PE_IRQ_ReadStatus() && PE_LoadU32(0x800956ACu)==1,"coalesced VBlank must dispatch once after unmask/unlock");
        ASSERT(!PE_GPU_VSyncQuery(),"physical edges must not change legacy synthetic frame counter");
    }
    ResetTestState();
    for(unsigned counter=0;counter<4;counter++) {
        ASSERT(func_80073C84(counter,7)==0,"clear policy must return previous zero");
        ASSERT(func_80073C84(counter,1)==7,"clear policy must preserve full flag word");
    }
    func_80073C74(1);
    func_80082534();
    PeIrqSource0BiosState bios;PE_Irq_GetSource0BiosState(&bios);
    ASSERT(bios.vblank_clear_mode==0 && bios.pad_clear_mode==1 && bios.vblank_counter==3,"controller reset must clear only VBlank timer policy");
    func_80073C74(0);PE_Irq_GetSource0BiosState(&bios);
    ASSERT(!bios.pad_clear_mode,"PAD clear setter not reflected");
    PE_Sdk_ResetState();
    ASSERT(func_80073C84(0,1)==0 && func_80073C84(3,1)==0,"SDK reset must clear timer policies");
    PASS();
}

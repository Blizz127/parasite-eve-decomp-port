static void test_DAY2_spu_dma_event(void)
{
    TEST("DAY2_spu_dma_event");
    ResetTestState();
    PE_SpuDma_InstallIrq(PE_SPU_DMA_IRQ_HANDLER);
    ASSERT(!PE_SpuDma_Begin(0x80150000u,0x70000u,0x400u,0),"event transfer needs registered event");
    int wrong=PE_Event_Open(0xF0000009u,0x21u,0x2000u,0);
    PE_Event_Enable(wrong);
    ASSERT(!PE_Event_SpuDmaEnabled(),"different event spec is not DMA completion");
    int handle=PE_Event_Open(0xF0000009u,0x20u,0x2000u,0);
    ASSERT(!PE_SpuDma_Begin(0x80150000u,0x70000u,0x400u,0),"registration alone does not enable event");
    PE_Event_Enable(handle);
    for(unsigned step=0;step<3;step++) {
        for(unsigned i=0;i<0x400;i++)PE_StoreU8(0x80150000u+i,(uint8_t)(i+step+1));
        ASSERT(PE_SpuDma_Begin(0x80150000u,0x70000u+step*0x400u,0x400u,0),"event-backed DMA starts");
        ASSERT(!PE_Event_ConsumeSpuDma(handle),"issue cannot fabricate completion");
        ASSERT(PE_SpuRam_LoadU8(0x70000u+step*0x400u)==0,"data remains pending until service");
        ASSERT(PE_SpuDma_Service()==1,"DMA service delivers event");
        for(unsigned i=0;i<0x400;i++)ASSERT(PE_SpuRam_LoadU8(0x70000u+step*0x400u+i)==(uint8_t)(i+step+1),"event follows actual data transfer");
        ASSERT(!PE_Event_ConsumeSpuDma(wrong),"wrong handle cannot consume completion");
        ASSERT(PE_Event_ConsumeSpuDma(handle),"registered handle consumes completion");
        ASSERT(!PE_Event_ConsumeSpuDma(handle),"completion cannot be reused");
        PeSpuDmaState state;PE_SpuDma_GetState(&state);
        ASSERT(state.callback_at_issue==0 && state.callback_order>state.data_order,"event dispatch ordering");
        ASSERT(!PE_LoadU32(0x8009D24Cu),"event path does not invent callback busy state");
    }
    ASSERT(PE_SpuDma_Begin(0x80150000u,0,0x400u,0),"start pending transfer before reset");
    PE_Sdk_ResetState();
    ASSERT(!PE_Event_ConsumeSpuDma(handle) && !PE_Event_SpuDmaEnabled(),"reset removes completion registration");
    ASSERT(!PE_SpuDma_Service(),"reset cancels pending transfer");
    PASS();
}

#include "retail_cd_dma_issue_cases.h"
static void test_DAY2_cd_dma(void)
{
    TEST_RETAIL_DISC1("DAY2_cd_dma"); TEST_RETAIL_FIXUPS(RETAILFIX_cd_device_tables);
    for(unsigned k=0;k<sizeof(CDDMA_cases)/sizeof(CDDMA_cases[0]);k++) {
        ResetTestState();
        PE_StoreU32(0x8009B348u,0x80130000u);PE_StoreU32(0x80130000u,CDDMA_cases[k].enable<<16u|0x1234u);
        PE_StoreU32(0x8009B344u,0x80130004u);PE_StoreU32(0x80130004u,0x12345678u);
        PE_StoreU32(0x8009B32Cu,0x80130008u);PE_StoreU8(0x80130008u,0x40u);
        func_8007CEAC(3u,0x80150000u,0u,CDDMA_cases[k].words,0x11000000u,CDDMA_cases[k].interrupt,0u);
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x80130000u)==CDDMA_cases[k].dicr && PE_LoadU32(0x80130004u)==CDDMA_cases[k].dpcr &&
            PE_CdReg_ReadU32(PE_CDREG_DMA3)==CDDMA_cases[k].madr && PE_CdReg_ReadU32(PE_CDREG_DMA3+4u)==CDDMA_cases[k].bcr &&
            PE_CdReg_ReadU32(PE_CDREG_DMA3+8u)==CDDMA_cases[k].chcr,"DMA issuer differs from original");
    }
    DiscFixture fx={0};ASSERT(FxBuild(&fx,0),"DMA disc fixture");PE_Disc_SetActive(fx.disc);
    for(unsigned chopped=0;chopped<2;chopped++) {
        ResetTestState();PE_Disc_SetActive(fx.disc);CdDeviceSeed();
        uint8_t *raw=fx.img+20u*2352u;
        for(unsigned i=0;i<2352u;i++) raw[i]=(uint8_t)(i*13u+(i>>8u));
        for(unsigned i=0;i<32u;i++) raw[24u+i]=0;
        raw[24]=0x60;raw[25]=1;raw[30]=1;raw[32]=1;
        ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,"DMA startup");
        for(unsigned i=0;i<10u && PE_LoadU32(0x8009B574u)!=1u && !PE_Port_ShouldStop();i++) HostFB_VSync(0);
        PE_StoreU32(0x80140000u,0x00200200u);ASSERT(func_8007FB44(2u,0x80140000u)==1,"DMA Setloc");
        for(unsigned i=0;i<2048u && PE_LoadU32(0x8009B598u) && !PE_Port_ShouldStop();i++) HostFB_VSync(-1);
        ASSERT(func_8007FB44(27u,0u)==1,"DMA ReadS");
        for(unsigned i=0;i<2048u && !PE_LoadU32(0x800A3520u) && !PE_Port_ShouldStop();i++) HostFB_VSync(-1);
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x800A3520u)==1u,"DMA sector arrival");
        PE_StoreU32(0x8009B32Cu,PE_CDREG_BASE);PE_StoreU32(0x8009B334u,PE_CDREG_BASE+2u);
        PE_StoreU32(0x8009B338u,PE_CDREG_BASE+3u);PE_StoreU32(0x8009B33Cu,PE_CDREG_BUS_CONTROL);
        PE_StoreU32(0x8009B340u,PE_CDREG_MAILBOX);PE_StoreU32(0x8009B344u,0x1F8010F0u);PE_StoreU32(0x8009B348u,0x1F8010F4u);
        PE_StoreU32(0x8009B34Cu,0x1F801098u);PE_StoreU32(0x8009B35Cu,PE_CDREG_DMA3+8u);
        PE_StoreU32(0x800C0DC8u,0x80150000u);PE_StoreU32(0x800C20C4u,8u);PE_StoreU32(0x800A801Cu,!chopped);
        (void)func_800824F0(0x8007C214u);
        PE_GPU_WriteDICR(0x00800000u);
        func_8007C564();
        ASSERT(!PE_Port_ShouldStop(),"physical stream stopped");
        ASSERT(PE_LoadU16(0x80150000u)==3u && PE_LoadU32(0x800BE998u)==1u && PE_LoadU32(0x800B89F4u)==1u,"physical stream publication");
        for(unsigned i=0;i<4u;i++) ASSERT(PE_LoadU8(0x8015001Cu+i)==raw[12u+i],"physical location bytes");
        for(unsigned i=0;i<2016u;i++) ASSERT(PE_LoadU8(0x80150100u+i)==raw[56u+i],"physical stream payload");
        ASSERT((PE_GPU_ReadDICR()&0x08800000u)==0x08800000u && (PE_IRQ_ReadStatus()&8u),"DMA3 completion IRQ");
        ASSERT(!(PE_CdReg_ReadU32(PE_CDREG_DMA3+8u)&0x11000000u),"DMA3 stayed busy");
        ASSERT(PE_CdReg_ReadU32(PE_CDREG_DMA3)==(chopped?0x1508E0u:0x150100u) &&
            PE_CdReg_ReadU32(PE_CDREG_DMA3+4u)==(chopped?0u:504u),"DMA3 final registers");
        PeCdDeviceState state;PE_CdReg_GetDeviceState(&state);ASSERT(state.data_remaining==280u,"DMA consumed wrong sector span");
        (void)PE_IRQ_ServicePendingForGeneration(PE_IRQ_Generation());
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU16(0x80150000u)==2u && PE_LoadU32(0x800BE9E4u)==1u && !PE_LoadU32(0x800B89F4u),"DMA3 callback did not finalize frame");
        /* Validate before transfer: an oversized DMA must preserve FIFO/RAM. */
        PE_CdReg_WriteU32(PE_CDREG_DMA3,0x80160000u);PE_CdReg_WriteU32(PE_CDREG_DMA3+4u,71u);
        PE_CdReg_WriteU32(PE_CDREG_DMA3+8u,0x11000000u);
        PE_CdReg_GetDeviceState(&state);
        ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_dma3_fifo_range")==1 && state.data_remaining==280u && !PE_LoadU32(0x80160000u),"DMA overflow partially transferred");
    }
    /* Idle B0CD0 pump: busy defer leaves the sector pending; when output
     * DMA clears, ServiceDevice retries 7C564 instead of overrunning. */
    ResetTestState();PE_Disc_SetActive(fx.disc);CdDeviceSeed();
    {
        uint8_t *raw=fx.img+20u*2352u;
        for(unsigned i=0;i<2352u;i++) raw[i]=(uint8_t)(i*13u+(i>>8u));
        for(unsigned i=0;i<32u;i++) raw[24u+i]=0;
        raw[24]=0x60;raw[25]=1;raw[30]=1;raw[32]=1;
        ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,"catchup DMA startup");
        for(unsigned i=0;i<10u && PE_LoadU32(0x8009B574u)!=1u && !PE_Port_ShouldStop();i++) HostFB_VSync(0);
        PE_StoreU32(0x80140000u,0x00200200u);ASSERT(func_8007FB44(2u,0x80140000u)==1,"catchup Setloc");
        for(unsigned i=0;i<2048u && PE_LoadU32(0x8009B598u) && !PE_Port_ShouldStop();i++) HostFB_VSync(-1);
        ASSERT(func_8007FB44(27u,0u)==1,"catchup ReadS");
        for(unsigned i=0;i<2048u && !PE_LoadU32(0x800A3520u) && !PE_Port_ShouldStop();i++) HostFB_VSync(-1);
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x800A3520u)==1u,"catchup sector arrival");
        PE_StoreU32(0x8009B32Cu,PE_CDREG_BASE);PE_StoreU32(0x8009B334u,PE_CDREG_BASE+2u);
        PE_StoreU32(0x8009B338u,PE_CDREG_BASE+3u);PE_StoreU32(0x8009B33Cu,PE_CDREG_BUS_CONTROL);
        PE_StoreU32(0x8009B340u,PE_CDREG_MAILBOX);PE_StoreU32(0x8009B344u,0x1F8010F0u);PE_StoreU32(0x8009B348u,0x1F8010F4u);
        PE_StoreU32(0x8009B34Cu,0x8013001Cu);PE_StoreU32(0x8013001Cu,0x01000000u);
        PE_StoreU32(0x8009B35Cu,PE_CDREG_DMA3+8u);
        PE_StoreU32(0x800C0DC8u,0x80150000u);PE_StoreU32(0x800C20C4u,8u);PE_StoreU32(0x800A801Cu,1u);
        (void)func_800824F0(0x8007C214u);
        PE_GPU_WriteDICR(0x00800000u);
        func_8007C564();
        ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x800B0CD0u)==1u &&
            PE_LoadU16(0x80150000u)!=3u,"busy defer must set B0CD0 without publishing");
        PE_StoreU32(0x8013001Cu,0u);
        PE_CdReg_ServiceDevice(225792u);
        ASSERT(!PE_Port_ShouldStop(),"idle B0CD0 catchup stopped");
        ASSERT(PE_LoadU16(0x80150000u)==3u && PE_LoadU32(0x800BE998u)==1u,
            "idle B0CD0 catchup must assemble the held sector");
    }
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}

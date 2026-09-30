#include "retail_cd_device_tables.h"
#include "retail_cd_startup_notify_cases.h"
static void CdDeviceSeed(void)
{
    HostFB_Init();PE_GPU_Init();PE_Callback_Init();B558_PlantPointers();
    PE_StoreU32(0x8009B290u,0x1F801C00u);
    for(unsigned j=0;j<6;j++) for(unsigned i=0;i<32;i++) PE_StoreU32(0x80000000u+CDDEV_tables[j].address+i*4u,CDDEV_tables[j].values[i]);
    for(unsigned i=0;i<26;i++) PE_StoreU32(0x80011D0Cu+i*4u,CDDEV_command_jumps[i]);
    for(unsigned i=0;i<5;i++) PE_StoreU32(0x80011B8Cu+i*4u,CDACK_jumps[i]);
}

/* Store-handler and DMA register pointers the stream state machine reaches
 * through the retail .rodata constants (asm/disc1/data/818A0.rodata.s).
 * CdDeviceSeed()/B558_PlantPointers() already planted the B27C command
 * family; these are the data/BFRD/bus/index/mailbox/DMA family the 7C564
 * assembly and the 7CEAC DMA3 issue use. Plain pointer values, not planted
 * guest state. Shared by the movie player and movie updater fixtures. */
static void CdStreamSeedRegisters(void)
{
    PE_StoreU32(0x8009B34Cu,0x1F801098u);   /* MDEC DMA1 CHCR (busy check) */
    PE_StoreU32(0x8009B32Cu,0x1F801800u);   /* index/status   */
    PE_StoreU32(0x8009B334u,0x1F801802u);   /* data FIFO      */
    PE_StoreU32(0x8009B338u,0x1F801803u);   /* BFRD / IRQ     */
    PE_StoreU32(0x8009B33Cu,0x1F801018u);   /* bus control    */
    PE_StoreU32(0x8009B340u,0x1F801020u);   /* result mailbox */
    PE_StoreU32(0x8009B344u,0x1F8010F0u);   /* DPCR           */
    PE_StoreU32(0x8009B348u,0x1F8010F4u);   /* DICR           */
    PE_StoreU32(0x8009B35Cu,0x1F8010B8u);   /* DMA3 CHCR      */
}

/* Write the retail Form-1 video chunk header (raw+24: chunk magic 0x0160,
 * 0x8001 channel selector, chunk index, total chunks, frame word) into one
 * fixture sector. payload 0 leaves the rest of the sector zero (a benign
 * decoder input); payload 1 fills MOVAU's deterministic pattern so the
 * assembled 2016-byte slice can be byte-compared with the on-disc bytes. */
static void CdStreamWriteVideoSector(uint8_t *img,uint32_t lba,uint32_t chunk,
                                    uint32_t chunks,uint32_t frame,int payload)
{
    uint8_t *raw=img+(size_t)lba*PE_DISC_RAW_SECTOR;
    for(unsigned i=0;i<PE_DISC_RAW_SECTOR;i++) raw[i]=0u;
    raw[16]=0u;raw[17]=0u;raw[18]=0x48u;raw[19]=0u;
    raw[20]=0u;raw[21]=0u;raw[22]=0x48u;raw[23]=0u;
    raw[24]=0x60u;raw[25]=0x01u;      /* chunk magic 0x0160 */
    raw[26]=0x01u;raw[27]=0x80u;      /* 0x8001: channel selector low bits 0 */
    raw[28]=(uint8_t)chunk;raw[29]=0u;
    raw[30]=(uint8_t)chunks;raw[31]=0u;
    raw[32]=(uint8_t)frame;raw[33]=(uint8_t)(frame>>8);
    raw[34]=(uint8_t)(frame>>16);raw[35]=(uint8_t)(frame>>24);
    if(payload) for(unsigned i=36u;i<PE_DISC_USER_SECTOR;i++)
        raw[24u+i]=(uint8_t)(i*13u+5u);
}
static void test_DAY2_cd_device(void)
{
    TEST_RETAIL_DISC1("DAY2_cd_device"); TEST_RETAIL_FIXUPS(RETAILFIX_cd_device_tables);
    for(unsigned k=0;k<sizeof(CDNOTIFY_cases)/sizeof(CDNOTIFY_cases[0]);k++) {
        ResetTestState();PE_StoreU32(0x800B8AB8u,CDNOTIFY_cases[k].target);
        func_8007F960(CDNOTIFY_cases[k].status,CDNOTIFY_cases[k].response);
        if(CDNOTIFY_cases[k].target) {
            ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_data_callback")==1 &&
                g_bootstrap_arg4_calls[0].target==CDNOTIFY_cases[k].target &&
                g_bootstrap_arg4_calls[0].arg0==CDNOTIFY_cases[k].arg0 &&
                g_bootstrap_arg4_calls[0].arg1==CDNOTIFY_cases[k].arg1,"startup notification differs from original callback prefix");
        } else ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"null startup notification must return");
    }
    DiscFixture fx={0};PeCdDeviceState device;
    ResetTestState();PE_Disc_SetActive(NULL);
    ASSERT(!PE_CdReg_EnableDevice(7u),"CD device accepted absent disc");
    ASSERT(FxBuild(&fx,0),"CD device fixture");PE_Disc_SetActive(fx.disc);
    CdDeviceSeed();ASSERT(PE_CdReg_EnableDevice(7u),"CD device attachment");
    ASSERT(func_8007EC14()==1 && !PE_Port_ShouldStop(),"real CD startup did not complete");
    PE_CdReg_GetDeviceState(&device);
    ASSERT(device.commands==3u && device.responses==4u && device.command_log[0]==1u && device.command_log[1]==10u && device.command_log[2]==12u,"startup command sequence");
    ASSERT(device.response_log[0]==3u && device.response_log[1]==3u && device.response_log[2]==2u && device.response_log[3]==3u,"startup response ordering");
    ASSERT(PE_LoadU8(0x8009B294u)==2u && PE_LoadU32(0x800945F0u)==0x8007C13Cu && PE_LoadU32(0x8009568Cu)==0x8007FE24u,"startup callback/status publication");
    ASSERT(PE_LoadU32(0x800A36ACu)==0x8007F960u && PE_LoadU32(0x800A36A0u)==0x8007F7E8u,"public CD handlers");
    /* A masked VBlank advances time but cannot invoke the SDK updater. */
    uint16_t mask=PE_IRQ_GetMask();
    (void)PE_IRQ_ExchangeMask((uint16_t)(mask&~1u));
    uint32_t vblanks=PE_LoadU32(0x800956ACu);
    HostFB_VSync(0);PE_CdReg_GetDeviceState(&device);
    ASSERT(device.commands==3u && PE_LoadU32(0x800956ACu)==vblanks && (PE_IRQ_ReadStatus()&1u),"masked VBlank dispatched startup");
    (void)PE_IRQ_ExchangeMask(mask);
    for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u && !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x800956ACu)>vblanks,"host VBlank startup did not dispatch");
    ASSERT(PE_LoadU32(0x8009B574u)==1u && PE_LoadU32(0x8009B578u)==11u,"CD startup state machine did not become ready");
    PE_CdReg_GetDeviceState(&device);
    ASSERT(device.commands==7u && device.command_log[3]==1u && device.command_log[4]==1u && device.command_log[5]==19u && device.command_log[6]==1u,"startup VBlank command progression");
    /* Transport IRQ masking and Init's held second response. */
    PE_CdReg_Reset();(void)PE_IRQ_ExchangeMask(0u);PE_IRQ_WriteStatus(0u);
    ASSERT(PE_CdReg_EnableDevice(0u),"device masked attachment");
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,10u);
    PE_CdReg_ServiceDevice(0x13CCDu);
    ASSERT(!(PE_CdReg_ReadU8(0x1F801800u)&0x20u),"command responded before scheduled cycle");
    PE_CdReg_ServiceDevice(1u);ASSERT(!(PE_IRQ_ReadStatus()&4u),"CD IRQ ignored device mask");
    PE_CdReg_WriteU8(0x1F801800u,1u);ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==3u,"Init first tag");
    PE_CdReg_WriteU8(0x1F801802u,7u);PE_CdReg_ServiceDevice(0u);
    ASSERT(PE_IRQ_ReadStatus()&4u,"pending CD response did not assert after enabling mask");
    PE_CdReg_ServiceDevice(0x200000u);PE_CdReg_GetDeviceState(&device);
    ASSERT(device.responses==1u,"second response overwrote unacknowledged first");
    ASSERT(PE_CdReg_ReadU8(0x1F801801u)==2u,"Init first status byte");
    PE_CdReg_WriteU8(0x1F801803u,7u);PE_CdReg_ServiceDevice(0u);
    ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==2u && PE_CdReg_ReadU8(0x1F801801u)==2u,"Init completion tag/status");
    PE_CdReg_WriteU8(0x1F801803u,7u);
    /* SetMode and Getparam transfer parameters through actual FIFOs. */
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801802u,0xA0u);PE_CdReg_WriteU8(0x1F801801u,14u);PE_CdReg_ServiceDevice(0xC4E1u);
    PE_CdReg_WriteU8(0x1F801800u,1u);PE_CdReg_WriteU8(0x1F801803u,7u);
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,15u);PE_CdReg_ServiceDevice(0xC4E1u);
    ASSERT(PE_CdReg_ReadU8(0x1F801801u)==2u && PE_CdReg_ReadU8(0x1F801801u)==0xA0u,"mode parameter round trip");
    PE_CdReg_WriteU8(0x1F801800u,1u);PE_CdReg_WriteU8(0x1F801803u,7u);
    /* Bad parameter counts are device errors, unported commands are boundaries. */
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,14u);PE_CdReg_ServiceDevice(0xC4E1u);
    PE_CdReg_WriteU8(0x1F801800u,1u);
    ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==5u && PE_CdReg_ReadU8(0x1F801801u)==3u && PE_CdReg_ReadU8(0x1F801801u)==0x20u,"bad parameter error response");
    PE_CdReg_WriteU8(0x1F801803u,7u);PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,3u);
    ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_device_unported_command")==1,"unported Play must stop");
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}

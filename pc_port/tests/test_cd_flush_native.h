/* Direct provider tests complement the production-binding CD tests. */
static void test_CDFLUSH_native_idle(void)
{
    TEST("CDFLUSH_native_idle");
    ResetTestState(); B558_PlantPointers();
    PE_StoreU8(0x8009B295u, 9u); PE_StoreU8(0x8009B296u, 8u);
    pe_plat_cd_flush();
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU8(0x8009B294u)==2u &&
           !PE_LoadU8(0x8009B295u) && !PE_LoadU8(0x8009B296u), "native flush latches");
    ASSERT(PE_CdReg_ReadU32(PE_CDREG_MAILBOX)==0x1325u &&
           PE_CdReg_ReadU8(PE_CDREG_BASE)==0u, "native flush mailbox/bank");
    PASS();
}

static void test_CDFLUSH_native_response_only(void)
{
    TEST("CDFLUSH_native_response_only");
    for(unsigned tag=1;tag<=7;tag++) {
        uint8_t payload[]={0x22u,0x71u,0x93u};
        ResetTestState(); B558_PlantPointers();
        PE_CdReg_WriteU8(PE_CDREG_BASE,1u);
        PE_CdReg_WriteU8(PE_CDREG_BASE+2u,0x15u);
        ASSERT(PE_CdReg_PushResponse(tag,payload,sizeof(payload)), "response ingress");
        PE_CdReg_WriteU8(PE_CDREG_BASE,2u);
        PE_CdReg_WriteU8(PE_CDREG_BASE+3u,7u);
        PE_CdReg_WriteU8(PE_CDREG_BASE,3u);
        PE_CdReg_WriteU8(PE_CDREG_BASE+3u,0x21u);
        ASSERT((PE_CdReg_ReadU8(PE_CDREG_BASE+3u)&7u)==tag, "audio acknowledged pending IRQ");
        PE_CdReg_WriteU8(PE_CDREG_BASE,0u);
        ASSERT((PE_CdReg_ReadU8(PE_CDREG_BASE+3u)&31u)==0x15u, "audio changed IRQ mask");
        pe_plat_cd_flush();
        PE_CdReg_WriteU8(PE_CDREG_BASE,1u);
        ASSERT(!PE_Port_ShouldStop() && !PE_CdReg_DeviceEnabled() &&
               !(PE_CdReg_ReadU8(PE_CDREG_BASE+3u)&7u), "native response IRQ acknowledgement");
        ASSERT(!PE_CdReg_PushResponse(1u,payload,1u), "unread response overwritten");
        for(unsigned i=0;i<sizeof(payload);i++) {
            ASSERT(PE_CdReg_ReadU8(PE_CDREG_BASE)&0x20u, "unread response lost");
            ASSERT(PE_CdReg_ReadU8(PE_CDREG_BASE+1u)==payload[i], "response data changed");
        }
        ASSERT(PE_CdReg_PushResponse(1u,payload,1u), "drained response cannot be replaced");
    }
    PASS();
}

static void test_CDFLUSH_native_device(void)
{
    TEST("CDFLUSH_native_device");
    DiscFixture fx={0};
    ResetTestState(); ASSERT(FxBuild(&fx,0), "fixture");
    PE_Disc_SetActive(fx.disc); CdDeviceSeed();
    ASSERT(PE_CdReg_EnableDevice(0u), "device attachment");
    for(unsigned tag=1;tag<=7;tag++) {
        uint8_t payload=0x22u;
        ASSERT(PE_CdReg_PushResponse(tag,&payload,1u), "device response ingress");
        pe_plat_cd_flush();
        PE_CdReg_WriteU8(PE_CDREG_BASE,1u);
        ASSERT(!PE_Port_ShouldStop() && PE_CdReg_DeviceEnabled() &&
               !(PE_CdReg_ReadU8(PE_CDREG_BASE+3u)&7u) &&
               !(PE_CdReg_ReadU8(PE_CDREG_BASE)&0x20u), "device response not acknowledged/drained");
    }
    PE_CdReg_Reset(); PE_Disc_SetActive(NULL); FxFree(&fx); PASS();
}

static void test_CDFLUSH_detach_contract(void)
{
    TEST("CDFLUSH_detach_contract");
    uint8_t gains[4];
    ResetTestState();
    PE_CdReg_WriteU8(PE_CDREG_BASE,2u);
    PE_CdReg_WriteU8(PE_CDREG_BASE+2u,0x81u);
    PE_CdReg_WriteU8(PE_CDREG_BASE+3u,0x42u);
    PE_CdReg_WriteU8(PE_CDREG_BASE,3u);
    PE_CdReg_WriteU8(PE_CDREG_BASE+1u,0x63u);
    PE_CdReg_WriteU8(PE_CDREG_BASE+2u,0x24u);
    PE_CdReg_WriteU8(PE_CDREG_BASE+3u,0x21u);
    ASSERT(PE_CdReg_ReadU32(PE_CDREG_BASE)==0x21246303u, "raw audio shadow changed");
    /* Audio apply/mute must not manufacture an IRQ in detached mode. */
    PE_CdReg_WriteU8(PE_CDREG_BASE,1u);
    ASSERT((PE_CdReg_ReadU8(PE_CDREG_BASE+3u)&7u)==0u, "audio write manufactured IRQ");
    B558_PlantPointers(); pe_plat_cd_flush();
    PE_CdReg_DetachDevice(); PE_CdReg_GetAudioVolumes(gains);
    ASSERT(gains[0]==0x81u && gains[1]==0x42u && gains[2]==0x63u &&
           gains[3]==0x24u && PE_CdReg_XaMuted(), "detach lost audio state");
    ASSERT(!PE_CdReg_DeviceEnabled() && !(PE_CdReg_ReadU8(PE_CDREG_BASE+3u)&7u), "detach register reset");
    B558_PlantPointers(); pe_plat_cd_flush();
    PE_CdReg_GetAudioVolumes(gains);
    ASSERT(!PE_Port_ShouldStop() && gains[0]==0x81u && gains[1]==0x42u &&
           gains[2]==0x63u && gains[3]==0x24u && PE_CdReg_XaMuted(), "idle native flush lost audio state");
    PASS();
}

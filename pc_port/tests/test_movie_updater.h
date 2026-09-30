/* DAY2-156: complete movie updater 122040 and abort 22354.
 * Original control-flow authority: pe_movie_update_oracle.py (192 graphs,
 * explicit call/timeout-counter providers). The native test executes the
 * real SDK/MDEC/CD implementations; the readiness/Setloc retry branch
 * needs the enabled CD device (stage 155 queue), so device-disabled runs
 * stop at the explicit movie_retry_wait boundary after the recorded
 * 2000-poll exhaustion and the 7C2A0 position write. */
static void MOVUPD_SeedSuccess(uint32_t bank)
{
    /* Stream record pool + one ready state-2 slot (7C484 promotes it). */
    PE_StoreU32(0x800C0DC8u,0x80150000u);
    PE_StoreU32(0x800C20C4u,2u);
    PE_StoreU32(0x800BE9ECu,0u);
    PE_StoreU16(0x80150000u,2u);              /* slot status 2 */
    PE_StoreU32(0x80150008u,0u);              /* frame 0 */
    PE_StoreU16(0x80150010u,320u);            /* slot width */
    PE_StoreU16(0x80150012u,240u);            /* slot height */
    /* Decoder stream record at next (base+count*32): MV1D bound pattern. */
    PE_StoreU32(0x80150040u,0u);
    PE_StoreU16(0x80150044u,0u);
    PE_StoreU16(0x80150046u,0u);
    PE_StoreU16(0x80150048u,0u);
    PE_StoreU16(0x8015004Au,0u);
    PE_StoreU16(0x8015004Cu,0x1234u);
    PE_StoreU32(0x8011EB8Cu,0u);              /* VLC bound seed */
    /* Frame metadata: no sound (frame < limit-16), no ClearImage,
     * end flag untouched by 121270. */
    PE_StoreU32(0x801227E4u,0x80151000u);
    PE_StoreU16(0x801227E8u,0u);
    PE_StoreU16(0x80151008u,0x7FFFu);
    PE_StoreU16(0x80122418u,320u);
    PE_StoreU16(0x8012241Au,240u);
    /* RLE arenas, output buffers, slice geometry. */
    PE_StoreU32(0x801228CCu,0x80160000u);
    PE_StoreU32(0x801228D0u,0x80164000u);
    PE_StoreU32(0x801228D8u,0x80170000u);
    PE_StoreU32(0x801228DCu,0x80174000u);
    PE_StoreU8(0x801228D4u,(uint8_t)bank);
    PE_StoreU8(0x801228E0u,(uint8_t)bank);
    PE_StoreU32(0x80160000u+bank*0x4000u,0x60000100u);
    PE_StoreU16(0x801228F8u,24u);
    PE_StoreU16(0x801228FAu,240u);
    PE_StoreU8(0x801228F2u,0u);
    PE_StoreU16(0x801228EAu,17u);             /* expiry restore x */
    PE_StoreU16(0x801228ECu,240u);            /* expiry restore y */
    PE_StoreU32(0x80122430u,0x80130000u);     /* VLC table */
    PE_StoreU8(0x800B0DBBu,1u);               /* wide */
    PE_StoreU32(0x8009CDDCu,1u);              /* display bank */
    PE_StoreU32(0x801223FCu,0x11223344u);     /* stream location copy */
    PE_StoreU8(0x801223F6u,3u);               /* submit mode */
}

static void MOVUPD_SeedTeardown(void)
{
    PE_StoreU32(0x8009AFD8u,0u);              /* low-lane teardown */
    PE_StoreU32(0x8009AF1Cu,0x80120000u);
    PE_StoreU32(0x8009AF28u,0x80120004u);
    PE_StoreU32(0x80120000u,0xA5A5A5A5u);
    PE_StoreU32(0x80120004u,0xA5A5A5A5u);
    PE_StoreU32(0x8009AFB8u,0x89ABCDEFu);
}

static void MOVUPD_RunContinue(unsigned active,unsigned bank,
                               unsigned change,unsigned timeout)
{
    static const unsigned arenas[2]={0x80160000u,0x80164000u};
    static const unsigned buffers[2]={0x80170000u,0x80174000u};
    PeMdecState mdec;
    PeC89CTelemetry c89c;

    ResetTestState();HostFB_Init();PE_GPU_Init();
    MOVUPD_SeedSuccess(bank);
    PE_StoreU8(0x800B0DBAu,(uint8_t)active);
    PE_StoreU16(0x800B0DBCu,65535u);
    PE_StoreU8(0x801223F5u,0u);
    PE_StoreU8(0x801223F8u,(uint8_t)change);
    PE_StoreU8(0x801228FCu,(uint8_t)(timeout?0u:1u));
    MOVUPD_SeedTeardown();
    ASSERT(func_80122040()==1,"updater continue return");
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"updater boundary");
    ASSERT(PE_LoadU8(0x800B0DBAu)==(uint8_t)active &&
        PE_LoadU16(0x800B0DBCu)==0u,"updater counters");
    ASSERT(PE_LoadU8(0x801228D4u)==(uint8_t)(bank^1u),"updater bank flip");
    ASSERT(PE_LoadU32(0x80122414u)==0x11223344u &&
        PE_LoadU8(0x801228FCu)==0u &&
        PE_LoadU8(0x801228F2u)==(uint8_t)timeout,"updater wait state");
    if(timeout) {
        ASSERT(PE_LoadU16(0x801228F4u)==17u &&
            PE_LoadU16(0x801228F6u)==240u,"updater expiry slice restore");
    }
    PE_C89C_GetTelemetry(&c89c);
    ASSERT(c89c.a0==0x80150040u && c89c.a1==arenas[bank^1u] &&
        c89c.a2==0x80130000u && c89c.ret==1,"updater C89C decode");
    ASSERT(PE_LoadU32(0x8011EBB4u)==arenas[bank^1u]+4u,"updater C89C bound");
    PE_MDEC_GetState(&mdec);
    ASSERT(mdec.upload_count==1u &&
        mdec.uploads[0].command==0x62000100u &&
        mdec.uploads[0].source==arenas[bank]+4u,"updater BFA0 submission");
    ASSERT(mdec.output_count==1u && mdec.dma1_madr==(buffers[bank]&0x00FFFFFFu) &&
        mdec.dma1_bcr==0x005A0020u,"updater C01C submission");
    ASSERT(PE_LoadU8(0x801223F6u)==3u,"updater mode byte");
    ASSERT(PE_LoadU8(0x801223F8u)==0u,"updater completion flag");
    if(change==2u) {
        ASSERT(PE_LoadU8(0x800BCE91u)==1u,"updater bank reconfigure");
    } else {
        ASSERT(PE_LoadU8(0x800BCE91u)==0u,"updater env untouched");
    }
    ASSERT(PE_LoadU32(0x80120000u)==0xA5A5A5A5u &&
        PE_LoadU32(0x8009AFB8u)==0x89ABCDEFu,"updater no early teardown");
}

static void test_DAY2_movie_updater(void)
{
    PeMdecState mdec;
    unsigned end,bank,k;

    TEST_RETAIL_DISC1("DAY2_movie_updater"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv); TEST_RETAIL_FIXUPS(RETAILFIX_cd_device_tables);
    /* Early return: B0DBA < 2 returns 0 with no calls and no mutation. */
    for(unsigned active=0u;active<2u;active++) for(end=0u;end<3u;end++) {
        ResetTestState();HostFB_Init();PE_GPU_Init();
        PE_StoreU8(0x800B0DBAu,(uint8_t)active);
        PE_StoreU16(0x800B0DBCu,5u);
        PE_StoreU8(0x801223F5u,(uint8_t)end);
        ASSERT(func_80122040()==0,"updater early return");
        ASSERT(PE_LoadU8(0x800B0DBAu)==(uint8_t)active &&
            PE_LoadU16(0x800B0DBCu)==5u &&
            !PE_Port_ShouldStop()&&!g_stub_order_count,"updater early state");
    }
    /* Full path, device disabled, end flag 0: prepare next frame, wait,
     * and return 1 without teardown. timeout 0 skips the wait because
     * the recorded completion flag is already set. */
    for(unsigned active=2u;active<256u;active+=(255u-2u))
    for(bank=0u;bank<2u;bank++) for(unsigned change=0u;change<3u;change+=2u)
        MOVUPD_RunContinue(active,bank,change,0u);
    /* Recorded 0x800000 wait-countdown expiry arm: pure guest-RAM slice
     * restore, device independent; the device stays disabled so the tick
     * loop is cheap. Exercised once per bank. */
    for(bank=0u;bank<2u;bank++) MOVUPD_RunContinue(2u,bank,0u,1u);
    /* Readiness/Setloc retry: with the device disabled the recorded
     * boundary stops after the 2000-poll exhaustion and the 7C2A0
     * position write; B0DBA/B0DBC and the bank byte stay unflipped. */
    for(end=0u;end<2u;end++) for(bank=0u;bank<2u;bank++) {
        ResetTestState();HostFB_Init();PE_GPU_Init();
        MOVUPD_SeedSuccess(bank);
        PE_StoreU16(0x80150000u,0u);              /* no ready record */
        PE_StoreU8(0x800B0DBAu,2u);
        PE_StoreU16(0x800B0DBCu,65535u);
        PE_StoreU8(0x801223F5u,(uint8_t)end);
        PE_StoreU8(0x801223F8u,0u);
        PE_StoreU8(0x801228FCu,1u);
        PE_StoreU32(0x800A3490u,0x00000200u);
        PE_StoreU8(0x800A3493u,0x77u);
        PE_StoreU32(0x800A3494u,0x12345678u);
        PE_StoreU32(0x800A8020u,0u);
        ASSERT(func_80122040()==0 && PE_Port_ShouldStop(),"updater retry stop");
        ASSERT(g_stub_order_count==1u &&
            strcmp(g_stub_order_log[0],"movie_retry_wait")==0,
            "updater retry boundary identity");
        ASSERT(PE_LoadU32(0x80122414u)==0x11010200u,
            "updater retry stream position");
        ASSERT(PE_LoadU8(0x800B0DBAu)==2u &&
            PE_LoadU16(0x800B0DBCu)==65535u &&
            PE_LoadU8(0x801228D4u)==(uint8_t)bank &&
            PE_LoadU8(0x801228FCu)==1u,"updater retry state preserved");
        PE_MDEC_GetState(&mdec);
        ASSERT(mdec.upload_count==1u && mdec.output_count==1u,
            "updater retry prior submissions");
    }
    /* Enabled-device readiness/Setloc retry completion: the acquisition
     * timeout repositions through 7C2A0 (D_800A3490 -> LBA 1), re-issues
     * Setloc through the real 80D5C -> 80DC4 queue with the stack response
     * buffer, opens the stream through 81314(location, 0x1E0), and the
     * physical stream delivers the frame so 121270 hands it off. The
     * fixture sector at LBA 1 carries the retail Form-1 chunk header with a
     * zero payload (a benign decoder input). */
    for(k=0u;k<2u;k++) {
        DiscFixture fx={0};
        ResetTestState();
        ASSERT(FxBuild(&fx,0),"updater retry delivery fixture");
        CdStreamWriteVideoSector(fx.img,1u,0u,1u,0x1234u,0);
        func_80073C94();B54KR_SeedGpuStatic();  /* 121270 clear-rect dispatch */
        PE_Disc_SetActive(fx.disc);CdDeviceSeed();
        ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,
            "updater retry delivery startup");
        for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u &&
            !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
        ASSERT(PE_LoadU32(0x8009B574u)==1u,"updater retry delivery SDK ready");
        MOVUPD_SeedSuccess(k);
        CdStreamSeedRegisters();
        PE_StoreU16(0x80150000u,0u);              /* no ready record */
        PE_StoreU8(0x800B0DBAu,2u);
        PE_StoreU16(0x800B0DBCu,65535u);
        PE_StoreU8(0x801223F5u,0u);
        PE_StoreU8(0x801223F8u,0u);
        PE_StoreU8(0x801228FCu,1u);
        PE_StoreU32(0x800A3490u,0x00000200u);
        PE_StoreU8(0x800A3493u,0x77u);
        PE_StoreU32(0x800A3494u,0x12345678u);
        PE_StoreU32(0x800A8020u,0u);
        ASSERT(func_80122040()==1 && !PE_Port_ShouldStop() &&
            !g_stub_order_count,"updater retry delivery stopped");
        ASSERT(PE_LoadU32(0x80122414u)==0x11010200u,
            "updater retry delivery stream position");
        ASSERT(PE_LoadU32(0x800A3494u)==0x1234u,
            "updater retry delivery record promote");
        ASSERT(PE_LoadU8(0x800B0DBAu)==2u &&
            PE_LoadU16(0x800B0DBCu)==0u &&
            PE_LoadU8(0x801228D4u)==(uint8_t)(k^1u) &&
            PE_LoadU8(0x801228FCu)==0u,"updater retry delivery state");
        FxFree(&fx);
    }
    /* Abort helper 22354: decrement, sound stop, MDEC unregister,
     * stream teardown, then Pause. Device disabled: the HOST_ADAPTED
     * Pause arm (fmv lane, pe_str_feed) completes without a controller
     * round trip; the pre-Pause effects are checked. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    PE_StoreU8(0x800B0DBAu,2u);
    PE_StoreU32(0x8009D2C0u,0u);                  /* D2C0 bit 1 clear */
    PE_StoreU32(0x8009B27Cu,0x1F801800u);         /* CD pointer table for 7B964 */
    PE_StoreU32(0x8009B280u,0x1F801801u);
    PE_StoreU32(0x8009B284u,0x1F801802u);
    PE_StoreU32(0x8009B288u,0x1F801803u);
    MOVUPD_SeedTeardown();
    func_80122354();
    ASSERT(!PE_Port_ShouldStop(),"abort host Pause completes");
    ASSERT(PE_LoadU8(0x800B0DBAu)==1u,"abort active decrement");
    ASSERT(PE_LoadU8(0x8009D1CAu)==0u && PE_LoadU8(0x8009D1C8u)==0u &&
        PE_LoadU8(0x8009D1CBu)==0u && PE_LoadU8(0x8009D1C9u)==0u,
        "abort sound stop");
    ASSERT(PE_LoadU32(0x8009AFB8u)==0u &&
        PE_LoadU32(0x80120000u)==0xA5A5A500u &&
        PE_LoadU32(0x80120004u)==0xA5A5A500u,"abort stream teardown");
    /* Enabled-device teardown: Pause completes through the real queue
     * (stage 155) and the updater returns 0. The completion flag is set
     * so the wait is skipped; the expiry arm is device independent and
     * already exercised above. */
    for(k=0u;k<2u;k++) {
        DiscFixture fx={0};
        ResetTestState();
        ASSERT(FxBuild(&fx,0),"updater teardown fixture");
        PE_Disc_SetActive(fx.disc);CdDeviceSeed();
        ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,
            "updater teardown startup");
        for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u &&
            !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
        ASSERT(PE_LoadU32(0x8009B574u)==1u,"updater teardown SDK ready");
        MOVUPD_SeedSuccess(k);
        PE_StoreU8(0x800B0DBAu,2u);
        PE_StoreU16(0x800B0DBCu,65535u);
        PE_StoreU8(0x801223F5u,1u);
        PE_StoreU8(0x801223F8u,0u);
        PE_StoreU8(0x801228FCu,1u);
        MOVUPD_SeedTeardown();
        ASSERT(func_80122040()==0 && !PE_Port_ShouldStop(),
            "updater teardown completion");
        ASSERT(PE_LoadU8(0x800B0DBAu)==1u &&
            PE_LoadU16(0x800B0DBCu)==0u &&
            PE_LoadU8(0x801228D4u)==(uint8_t)(k^1u) &&
            PE_LoadU8(0x801228FCu)==0u,"updater teardown state");
        ASSERT(PE_LoadU32(0x800A3608u)==0u,"updater Pause queue retirement");
        ASSERT(PE_LoadU32(0x8009AFB8u)==0u &&
            PE_LoadU32(0x80120000u)==0xA5A5A500u &&
            PE_LoadU32(0x80120004u)==0xA5A5A500u,"updater Pause teardown");
        FxFree(&fx);
    }
    PASS();
}

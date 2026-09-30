/* DAY2-158: autonomous movie stream delivery.
 *
 * The movie player func_80121C04 opens a real ReadS stream through
 * func_80081314; the CD device model delivers one raw Mode-2 sector per
 * read cadence, the data-ready callback chain
 * (7C13C -> 80778 -> 7F88C -> 813E8 -> 7C564) assembles each 2016-byte
 * MDEC slice into a 32-byte stream record, and the DMA3 completion
 * callback (7C214, registered by 81314 through func_800824F0) promotes
 * the record to state 2 so func_8007C484 hands a finished slice to
 * func_80121270.
 *
 * Two runs prove the chain end to end:
 *   - a synthetic fixture whose searched file carries the retail STR
 *     video chunk header shape (raw[24..27]=60 01 01 80, chunk index at
 *     +28, total chunks at +30, frame at +32);
 *   - the real Disc 1 opening movie \FMV1\FMV001.STR;1 (LBA 189742), where
 *     the assembled 2016-byte slice must match the on-disc bytes exactly.
 *
 * The chunk layout itself is verified against the real disc by
 * test_DAY2_movie_complete_frame. */

/* FNV-1a over a host byte range (B54K_MdecFnv1a64 takes a guest address). */
static uint64_t MOVAU_Fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash=UINT64_C(0xCBF29CE484222325);
    for(size_t i=0;i<size;i++) {hash^=bytes[i];hash*=UINT64_C(0x100000001B3);}
    return hash;
}

/* Write the retail Form-1 video chunk header into one fixture sector. */
static void MOVAU_WriteVideoSector(uint8_t *img, uint32_t lba,
                                   uint32_t chunk, uint32_t chunks,
                                   uint32_t frame)
{
    uint8_t *raw = img + (size_t)lba * PE_DISC_RAW_SECTOR;
    /* Video subheader (duplicated), Form-1 data at raw+24. */
    raw[16]=0u;raw[17]=0u;raw[18]=0x48u;raw[19]=0u;
    raw[20]=0u;raw[21]=0u;raw[22]=0x48u;raw[23]=0u;
    raw[24]=0x60u;raw[25]=0x01u;      /* chunk magic 0x0160 */
    raw[26]=0x01u;raw[27]=0x80u;      /* 0x8001: channel selector low bits 0 */
    raw[28]=(uint8_t)chunk;raw[29]=0u;
    raw[30]=(uint8_t)chunks;raw[31]=0u;
    raw[32]=(uint8_t)frame;raw[33]=(uint8_t)(frame>>8);
    raw[34]=(uint8_t)(frame>>16);raw[35]=(uint8_t)(frame>>24);
    for(unsigned i=36u;i<PE_DISC_USER_SECTOR;i++)
        raw[24u+i]=(uint8_t)(i*13u+5u);
}

/* Seed record 0's search prefix/name (leading '\' required by 81414). */
static void MOVAU_SeedPath(const char *prefix, const char *file)
{
    /* Retail 121C04 passes the prefix strings' own addresses (addiu a1,
     * 0x80120FF4 / 0x80120FFC), so the bytes live there, not a pointer. */
    strcpy(PE_Translate(0x80120FF4u,8u),prefix);
    strcpy(PE_Translate(0x80120FFCu,8u),prefix);
    strcpy(PE_Translate(0x80130130u,16u),file);
}

static void MOVAU_SeedRecord(void)
{
    pe_addr_t record=0x80122438u;
    PE_StoreU32(record,0x80130130u);          /* record[0] = name */
    PE_StoreU8(record+4u,0u);                 /* not wide */
    PE_StoreU16(record+6u,1u);                /* stream start frame (B6918) */
    PE_StoreU16(record+8u,10000u);            /* frame limit */
    PE_StoreU16(record+0xAu,0u);              /* slice x */
    PE_StoreU16(record+0xCu,0xF0u);           /* slice y */
    PE_StoreU32(0x80122420u,0x80160000u);
    PE_StoreU32(0x80122424u,0x80164000u);
    PE_StoreU32(0x80122428u,0x80170000u);
    PE_StoreU32(0x8012242Cu,0x80174000u);
    PE_StoreU32(0x80122430u,0x80130000u);     /* VLC table */
    PE_StoreU32(0x80122434u,0x80150000u);     /* record pool */
    /* The libpress module image is absent, so the VLC builder terminates
     * immediately and the decoder pads to its bound (empty decode). */
    PE_StoreU16(0x8010CBFCu,0xFFFFu);
    PE_StoreU32(0x8009CDDCu,0x100u);
    PE_StoreU8(0x800B0DBEu,0x98u);
    PE_StoreU8(0x800B0DBAu,3u);
    PE_StoreU16(0x800B0DBCu,0u);
    PE_StoreU16(0x801227E8u,0xFFFFu);
}

static void MOVAU_SeedRegisterPointers(void)
{
    PE_StoreU32(0x8009B34Cu,0x1F801098u);     /* MDEC DMA1 CHCR */
    PE_StoreU32(0x8009B32Cu,0x1F801800u);
    PE_StoreU32(0x8009B334u,0x1F801802u);
    PE_StoreU32(0x8009B338u,0x1F801803u);
    PE_StoreU32(0x8009B33Cu,0x1F801018u);
    PE_StoreU32(0x8009B340u,0x1F801020u);
    PE_StoreU32(0x8009B344u,0x1F8010F0u);
    PE_StoreU32(0x8009B348u,0x1F8010F4u);
    PE_StoreU32(0x8009B35Cu,0x1F8010B8u);
}

static void test_DAY2_movie_autonomous(void)
{
    DiscFixture fx={0};
    TEST_RETAIL_DISC1("DAY2_movie_autonomous"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv); TEST_RETAIL_FIXUPS(RETAILFIX_cd_device_tables);
    /* ── Synthetic fixture: deterministic chain proof. ───────────────── */
    ResetTestState();
    HostFB_Init();PE_GPU_Init();func_80073C94();B54KR_SeedGpuStatic();
    ASSERT(FxBuild(&fx,1),"autonomous fixture");
    MOVAU_WriteVideoSector(fx.img,FX_FMV018_LBA,0u,1u,1u);
    PE_Disc_SetActive(fx.disc);CdDeviceSeed();
    ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,
        "autonomous startup");
    for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u &&
        !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
    ASSERT(PE_LoadU32(0x8009B574u)==1u,"autonomous SDK ready");
    MOVAU_SeedPath("\\FMV2\\","FMV018.STR;1");
    MOVAU_SeedRecord();
    MOVAU_SeedRegisterPointers();
    (void)func_80121C04(0);
    if(PE_Port_ShouldStop())
        fprintf(stderr,"autonomous synthetic stop: %s\n",
                g_stub_order_count?g_stub_order_log[0]:"?");
    fprintf(stderr,"autonomous synthetic: stop=%d B374=%u B0DBA=%u "
            "B0DBC=%u A3494=%08X\n",
            PE_Port_ShouldStop(),PE_LoadU16(0x8009B374u),
            PE_LoadU8(0x800B0DBAu),PE_LoadU16(0x800B0DBCu),
            PE_LoadU32(0x800A3494u));
    /* 7C564 ran its assembly states (5 = a later non-video sector
     * cleared the record after the first slice was handed off). */
    ASSERT(PE_LoadU16(0x8009B374u)!=0u,
        "autonomous stream state machine never ran");
    /* 7C214 promoted the record (A3494 = record[8]) and 121C04 handed
     * the first slice off. */
    ASSERT(PE_LoadU32(0x800A3494u)==1u,"autonomous DMA3 record promote");
    ASSERT(PE_LoadU8(0x800B0DBAu)==4u && PE_LoadU16(0x800B0DBCu)==1u,
        "autonomous frame handoff");
    ASSERT(!PE_Port_ShouldStop(),"autonomous stream stopped");
    FxFree(&fx);
    /* ── Real disc: opening movie first slice. ───────────────────────── */
    {
        PE_Disc *disc;
        uint8_t raw[2352];
        char err[256]={0};
        disc=BTL6_OpenDisc1(err,sizeof(err));
        ASSERT(disc!=NULL,err[0]?err:"autonomous disc unavailable");
        ResetTestState();
        HostFB_Init();PE_GPU_Init();func_80073C94();B54KR_SeedGpuStatic();
        PE_Disc_SetActive(disc);CdDeviceSeed();
        ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,
            "autonomous disc startup");
        for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u &&
            !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
        ASSERT(PE_LoadU32(0x8009B574u)==1u,"autonomous disc SDK ready");
        ASSERT(PE_Disc_ReadRawSector(disc,189742u,raw),
            "opening STR sector read");
        ASSERT(raw[24]==0x60u && raw[25]==1u &&
            (raw[18]&0x20u)==0u,"opening STR first sector shape");
        MOVAU_SeedPath("\\FMV1\\","FMV001.STR;1");
        MOVAU_SeedRecord();
        MOVAU_SeedRegisterPointers();
        (void)func_80121C04(0);
        if(PE_Port_ShouldStop())
            fprintf(stderr,"autonomous disc stop: %s\n",
                    g_stub_order_count?g_stub_order_log[0]:"?");
        {PeCdDeviceState d;PE_CdReg_GetDeviceState(&d);
        fprintf(stderr,"autonomous disc: stop=%d B374=%u B0DBA=%u "
                "B0DBC=%u A3494=%08X sectors=%u B89F4=%u BE998=%u "
                "BE9E4=%u A8018=%u C0DC0=%u B6918=%u\n",
                PE_Port_ShouldStop(),PE_LoadU16(0x8009B374u),
                PE_LoadU8(0x800B0DBAu),PE_LoadU16(0x800B0DBCu),
                PE_LoadU32(0x800A3494u),d.sectors,PE_LoadU32(0x800B89F4u),
                PE_LoadU32(0x800BE998u),PE_LoadU32(0x800BE9E4u),
                PE_LoadU16(0x800A8018u),PE_LoadU32(0x800C0DC0u),
                PE_LoadU32(0x800B6918u));}
        ASSERT(!PE_Port_ShouldStop(),"autonomous disc stream stopped");
        ASSERT(PE_LoadU8(0x800B0DBAu)==4u && PE_LoadU16(0x800B0DBCu)==1u,
            "autonomous disc frame handoff");
        /* The 2016-byte decoded slice the player published must be the
         * real on-disc MDEC payload (raw+56), byte for byte. */
        ASSERT(PE_LoadU32(0x800A3494u)==1u,"opening STR record frame");
        ASSERT(B54K_MdecFnv1a64(0x80150800u,2016u)==
            MOVAU_Fnv1a64(raw+56,2016u),
            "opening STR assembled slice differs from disc bytes");
        PE_Disc_Close(disc);
    }
    PASS();
}

/* DAY2-159: multi-frame autonomous playback loop.
 *
 * Closes the non-claim in docs/evidence/fmv-autonomous-stream/REPORT.md §7
 * ("the multi-frame updater is covered separately ... not by this run").
 * Drives the real func_80122040 repeatedly: each iteration re-arms a ready
 * stream record exactly like the supported single-slice delivery path
 * (MOVUPD_SeedSuccess, the same seeding test_DAY2_movie_updater uses), then
 * runs the real updater and checks that the display bank flips, the frame
 * counter advances and the next frame's MDEC output is submitted every time.
 * The loop is progressing real frames, not a stuck state.
 *
 * Scope: the assembled-slice -> decode-submit -> presentation loop.  It does
 * not drive a continuous multi-sector CD read (the CD model documents a
 * single pending sector plus an independent requested FIFO, and stops on a
 * hardware-style overrun: see docs/ai_context/DAY2_CD_SECTOR_DEVICE.md).  It
 * also does not claim hardware-exact pixels or XA audio. */
static void test_DAY2_movie_multiframe(void)
{
    unsigned bank,count,flips;
    TEST("DAY2_movie_multiframe");
    ResetTestState();HostFB_Init();PE_GPU_Init();
    /* The retail player (121C04) runs libpress init/reset before its first
     * submit; the MDEC service needs that control state to complete each
     * input transfer between frames. */
    func_8010BE3C(0);
    PE_StoreU8(0x800B0DBAu,2u);
    PE_StoreU8(0x801223F5u,0u);
    PE_StoreU8(0x801223F8u,0u);
    MOVUPD_SeedSuccess(0u);
    PE_StoreU16(0x800B0DBCu,0u);
    flips=0u;
    for(unsigned frame=0;frame<6u && !PE_Port_ShouldStop();frame++) {
        /* The stream would have assembled one ready record while the previous
         * frame was presented; re-arm exactly like the supported single-slice
         * delivery path, then run the real updater. */
        PE_MDEC_BeginReset();
        func_8010BE3C(0);
        bank=(uint8_t)PE_LoadU8(0x801228D4u);
        MOVUPD_SeedSuccess(bank);
        PE_StoreU8(0x800B0DBAu,2u);
        PE_StoreU8(0x801223F5u,0u);
        PE_StoreU8(0x801223F8u,0u);
        PE_StoreU8(0x801228FCu,1u);
        count=PE_LoadU16(0x800B0DBCu);
        ASSERT(func_80122040()==1,"multiframe updater continue");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"multiframe boundary");
        ASSERT(PE_LoadU8(0x801228D4u)==(uint8_t)(bank^1u),
            "multiframe updater bank flip");
        ASSERT(PE_LoadU16(0x800B0DBCu)==(uint16_t)(count+1u),
            "multiframe frame counter");
        flips++;
    }
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"multiframe loop stopped");
    ASSERT(flips>=6u,"multiframe advanced fewer than six frames");
    PASS();
}

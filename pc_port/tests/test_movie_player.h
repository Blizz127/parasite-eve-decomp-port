/* DAY2-157: complete movie player 121C04.
 * Original control-flow authority: pe_movie_player_oracle.py (20 graphs:
 * 4 early returns and 16 full setup/search/decode graphs with explicit
 * SDK/search/frame providers). The native test executes the real
 * SDK/disc implementations. The disabled-device run stops at the recorded
 * movie_player_search_wait boundary. The enabled-device runs really
 * search the fixture ISO and complete Setloc through the stage155 queue;
 * three arms pin the stream delivery frontier:
 *   - the searched file does not carry a Form-1 video chunk -> func_8007C564
 *     really DMA3s the sector into the pool record and then drops it (the
 *     assembled header's frame word never equals the stream-start word
 *     D_800B6918), so the decode retry stops at the recorded movie_retry_wait
 *     boundary with no slice ever fabricated;
 *   - the searched file carries the retail chunk header whose frame word
 *     equals the record's stream-start word -> the physical stream delivers
 *     the slice, 121270 hands it to the player and the first frame is
 *     assembled (this is the frontier DAY2_MOVIE_UPDATER.md named). */

static void MOVPLY_SeedRecord(unsigned id,unsigned wide)
{
    pe_addr_t record=0x80122438u+id*20u;
    /* Retail 121C04 strcats the prefix strings AT 0x80120FF4/0x80120FFC
     * (addiu a1), not through a pointer stored there. */
    PE_StoreU8(0x80120FF4u,'\\');PE_StoreU8(0x80120FF5u,0u);   /* prefix "\\" */
    PE_StoreU8(0x80120FFCu,'\\');PE_StoreU8(0x80120FFDu,0u);
    strcpy(PE_Translate(0x80130130u,16u),"PE.IMG;1");
    PE_StoreU32(0x80122438u+id*20u,0x80130130u);   /* record[0] is the name */
    PE_StoreU8(record+4u,(uint8_t)wide);
    /* record+6 is the stream-start frame word the player copies into
     * D_800B6918 through func_8007C304(1, start, -1, 0, 0). The stream
     * assembly in func_8007C564 drops any record whose sector frame word
     * (record+8 of the assembled header) differs from it, so a delivering
     * fixture must carry this exact frame value in the sector. */
    PE_StoreU16(record+6u,0x1234u);                /* stream start frame */
    PE_StoreU16(record+8u,10000u);                 /* frame limit */
    PE_StoreU16(0x801227E8u,0xFFFFu);              /* last delivered frame */
    PE_StoreU16(record+0xAu,0x5678u);              /* slice x */
    PE_StoreU16(record+0xCu,0x300u);               /* slice y */
    PE_StoreU32(0x80122420u,0x80160000u);          /* RLE arenas */
    PE_StoreU32(0x80122424u,0x80164000u);
    PE_StoreU32(0x80122428u,0x80170000u);          /* output buffers */
    PE_StoreU32(0x8012242Cu,0x80174000u);
    PE_StoreU32(0x80122430u,0x80130000u);          /* VLC table */
    PE_StoreU32(0x80122434u,0x80150000u);          /* record pool */
    /* The player's BD4C RLE decode reads the libpress module source at
     * 0x8010CBFC; the module image is absent from this fixture, so seed
     * the FF FF terminator for a deterministic empty decode (the real
     * table build is covered by DAY2_movie_complete_frame). */
    PE_StoreU16(0x8010CBFCu,0xFFFFu);
    PE_StoreU32(0x8009CDDCu,0x100u);               /* display bank low byte 0 */
    PE_StoreU8(0x800B0DBEu,0x98u);
    PE_StoreU8(0x800B0DBAu,3u);
    PE_StoreU16(0x800B0DBCu,0u);
}

static void test_DAY2_movie_player(void)
{
    PeMdecState mdec;
    unsigned wide,k;

    TEST_RETAIL_DISC1("DAY2_movie_player"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv); TEST_RETAIL_FIXUPS(RETAILFIX_cd_device_tables);
    /* Early returns: halfword id >= 0x2F returns 0 without touching B0DBF. */
    for(k=0u;k<4u;k++) {
        unsigned id=k==0u?0x2Fu:k==1u?0x30u:k==2u?0x8000012Fu:0xFFFFu;
        ResetTestState();HostFB_Init();PE_GPU_Init();
        PE_StoreU8(0x800B0DBFu,0xA5u);
        ASSERT(func_80121C04((int32_t)id)==0,"player early return");
        ASSERT(PE_LoadU8(0x800B0DBFu)==0xA5u &&
            !PE_Port_ShouldStop()&&!g_stub_order_count,"player early state");
    }
    /* Disabled device: the search readiness loop keeps the recorded
     * boundary visible before any search. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    MOVPLY_SeedRecord(0u,1u);
    ASSERT(func_80121C04(0)==0 && PE_Port_ShouldStop(),"player search stop");
    ASSERT(g_stub_order_count==1u &&
        strcmp(g_stub_order_log[0],"movie_player_search_wait")==0,
        "player search boundary identity");
    /* The setup prefix still ran: both display envs, B0DBF/B0DBB and
     * the record pointer are in place. */
    ASSERT(PE_LoadU8(0x800B0DBFu)==0u && PE_LoadU8(0x800B0DBBu)==1u &&
        PE_LoadU32(0x801227E4u)==0x80122438u,"player setup prefix state");
    /* Enabled device: real ISO search, Setloc and ReadS issue; then the
     * decode retry stops at the recorded boundary because the searched
     * fixture file carries no Form-1 video chunk: 7C564 refuses every
     * sector (assembly status 5) and clears the record, so no slice may be
     * fabricated. */
    {
        DiscFixture fx={0};
        ResetTestState();
        ASSERT(FxBuild(&fx,0),"player fixture");
        PE_Disc_SetActive(fx.disc);CdDeviceSeed();
        ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,
            "player startup");
        for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u &&
            !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
        ASSERT(PE_LoadU32(0x8009B574u)==1u,"player SDK ready");
        MOVPLY_SeedRecord(0u,1u);
        CdStreamSeedRegisters();
        ASSERT(func_80121C04(0)==0 && PE_Port_ShouldStop(),
            "player stream boundary stop");
        /* Exactly the retry boundary: the command-poll event dispatch
         * (func_8007B010's D_8009AFB8 arm) reached the native 80080778 leaf
         * and its data-ready chain, so no indirect-callback boundary fired. */
        ASSERT(g_stub_order_count==1u &&
            strcmp(g_stub_order_log[0],"movie_retry_wait")==0 &&
            CountOrderLog("func_8007B010_afb8_callback")==0u &&
            CountOrderLog("func_8007B558_afb8_callback")==0u &&
            CountOrderLog("CD_device_data_underflow")==0u,
            "player stream boundary identity");
        /* The search really resolved the fixture file and the start
         * Setloc really completed through the queue. */
        ASSERT(PE_LoadU32(0x801223FCu)!=0u && PE_LoadU32(0x80122414u)!=0u,
            "player search result missing");
        /* 81314 installed the streaming callbacks before the device
         * boundary. */
        ASSERT(PE_LoadU32(0x800B8AB4u)==0x800813E8u &&
            func_800824F0(0u)==0x8007C214u,
            "player stream callbacks");
        /* The assembled record was really DMA3'd into the pool, and then
         * dropped by the stream-start filter: the fixture sector's frame
         * word (record+8 of the assembled header) never equals the record's
         * stream-start word D_800B6918, so func_8007C564 clears the record
         * and no state-2 slot is ever promoted. The 12-byte location prefix
         * of that sector reached the FIFO (A34A0 holds the pool record). */
        ASSERT(PE_LoadU32(0x800A34A0u)==0x80150000u &&
            PE_LoadU16(0x80150000u)==0u && PE_LoadU32(0x800B89F4u)==0u,
            "player record dropped at the stream-start filter");
        ASSERT(PE_LoadU32(0x800BE998u)==0u && PE_LoadU32(0x800B0CD0u)==0u,
            "player record index untouched");
        ASSERT(PE_LoadU8(0x800B0DBAu)==3u && PE_LoadU16(0x800B0DBCu)==0u &&
            PE_LoadU32(0x800A3494u)==0u,"player no fabricated slice");
        ASSERT(PE_LoadU8(0x801228D4u)==0u && PE_LoadU8(0x801228E0u)==0u &&
            PE_LoadU16(0x801228E2u)==0x5678u &&
            PE_LoadU16(0x801228E4u)==0x3F0u &&
            PE_LoadU16(0x801228F8u)==0x18u,"player slice geometry");
        FxFree(&fx);
    }
    /* Enabled device + a real Form-1 video chunk in the searched file whose
     * frame word equals the record's stream-start word: the physical stream
     * delivers the slice, 7C214 promotes the record and the player assembles
     * the first frame instead of restarting. This is the delivery frontier
     * DAY2_MOVIE_UPDATER.md named (previously unexercised: the arm above
     * only ever proved the refusal path). */
    {
        DiscFixture fx={0};
        unsigned uploads_before;
        ResetTestState();
        ASSERT(FxBuild(&fx,0),"player delivery fixture");
        /* func_80081414 resolves "\\PE.IMG;1" to FX_PEIMG_LBA; the stream
         * Setloc/ReadS therefore starts on that sector. */
        CdStreamWriteVideoSector(fx.img,FX_PEIMG_LBA,0u,1u,0x1234u,1);
        func_80073C94();B54KR_SeedGpuStatic();  /* 121270 clear-rect dispatch */
        PE_Disc_SetActive(fx.disc);CdDeviceSeed();
        ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,
            "player delivery startup");
        for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u &&
            !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
        ASSERT(PE_LoadU32(0x8009B574u)==1u,"player delivery SDK ready");
        MOVPLY_SeedRecord(0u,1u);
        CdStreamSeedRegisters();
        PE_MDEC_GetState(&mdec);
        uploads_before=mdec.upload_count;
        ASSERT(func_80121C04(0)==0 && !PE_Port_ShouldStop(),
            "player delivered slice stopped");
        /* 7C214 published the assembled record's frame word and the record
         * was promoted through 7C484 into the player. */
        ASSERT(PE_LoadU32(0x800A3494u)==0x1234u,"player slice frame");
        ASSERT(PE_LoadU8(0x800B0DBAu)==4u && PE_LoadU16(0x800B0DBCu)==1u,
            "player frame handoff");
        ASSERT(PE_LoadU8(0x801223F5u)==0u && PE_LoadU8(0x801228D4u)==1u,
            "player frame state");
        ASSERT(PE_LoadU32(0x800B8AB4u)==0x800813E8u &&
            func_800824F0(0u)==0x8007C214u,"player delivery callbacks");
        PE_MDEC_GetState(&mdec);
        ASSERT(mdec.upload_count>=uploads_before&&mdec.upload_count>=1u,
            "player libpress tables");
        FxFree(&fx);
    }
    (void)wide;
    PASS();
}

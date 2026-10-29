/* Real room-entry witnesses: existing native helpers must be reached by the
 * production destination path, rather than only by direct helper tests. */
static void test_ROOMENTRY_default_lighting(void)
{
    PE_Disc *disc;
    char err[256];
    unsigned i;
    static const uint16_t direction[9] = {
        0x7FFFu, 0, 0, 0, 0x7FFFu, 0, 0, 0, 0x7FFFu
    };
    static const uint16_t colour[9] = {
        255, 128, 96, 255, 128, 96, 255, 128, 96
    };

    TEST_RETAIL_DISC1("ROOMENTRY_default_lighting");
    ResetTestState();
    for (i = 0; i < 0x50u; ++i)
        PE_StoreU8(0x800BEA40u + i, 0xA5u);
    PE_GTE_SetBK(1, 2, 3);
    disc = pe_btl124_open_m0005i(err, sizeof(err));
    ASSERT(disc != NULL, err[0] ? err : "open m0005i");
    PE_Disc_SetActive(NULL);
    PE_Disc_Close(disc);
    for (i = 0; i < 9; ++i) {
        ASSERT(PE_LoadU16(0x800BEA40u + 2u*i) == direction[i],
               "retail clamps direction axes to signed 0x7FFF");
        ASSERT(PE_LoadU16(0x800BEA60u + 2u*i) == colour[i],
               "retail fills three light-colour columns");
    }
    ASSERT(PE_GTE_ReadCtrl(13) == 0x280u && PE_GTE_ReadCtrl(14) == 0x280u
           && PE_GTE_ReadCtrl(15) == 0x280u, "default background colour");
    for (i = 0; i < 5; ++i)
        ASSERT(PE_GTE_ReadCtrl(16u+i) ==
               (i == 4 ? (uint32_t)(int32_t)(int16_t)PE_LoadU16(0x800BEA70u)
                       : PE_LoadU32(0x800BEA60u + 4u*i)), "LCM published to GTE");
    ASSERT(PE_LoadU32(0x800BEA80u) == 0xA5FFFFFFu
           && PE_LoadU32(0x800BEA84u) == 0xA5808080u
           && PE_LoadU32(0x800BEA88u) == 0xA5606060u,
           "colour bytes updated with padding preserved");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE, "no boundary");
    PASS();
}

static void test_ROOMENTRY_upload_selected_bank(void)
{
    PE_Disc *disc;
    char err[256];

    TEST_RETAIL_DISC1("ROOMENTRY_upload_selected_bank");
    ResetTestState();
    /* A different bank forces 6BD68's CD-read/TIM-upload arm. The selected
     * bank comes from the real room header, never from a patched asset. */
    PE_StoreU8(0x800B0CE1u, 0x7Fu);
    disc = pe_btl124_open_m0005i(err, sizeof(err));
    ASSERT(disc != NULL, err[0] ? err : "open m0005i");
    PE_Disc_SetActive(NULL);
    PE_Disc_Close(disc);
    ASSERT(PE_LoadU8(0x800B0CE0u) != 0x7Fu, "fixture selects another bank");
    ASSERT(PE_LoadU8(0x800B0CE1u) == PE_LoadU8(0x800B0CE0u),
           "native upload completed and cached the room bank");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE, "no boundary");
    PASS();
}

static void test_ROOMENTRY_upload_cached_noop(void)
{
    int before, after;
    TEST("ROOMENTRY_upload_cached_noop");
    ResetTestState();
    HostFB_Init();
    PE_StoreU8(0x800B0CE0u, 0xFFu);
    PE_StoreU8(0x800B0CE1u, 0xFFu);
    PE_StoreU32(0x800B0E6Cu, 0xDEADBEEFu); /* no dereference on cached arm */
    HostFB_GetState(NULL, &before, NULL, NULL);
    ASSERT(func_8006BD68() == 0, "cached bank returns zero");
    HostFB_GetState(NULL, &after, NULL, NULL);
    ASSERT(after == before && PE_LoadU32(0x800B0E6Cu) == 0xDEADBEEFu,
           "cached arm issues neither sync nor upload");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE, "no boundary");
    PASS();
}

static void test_ROOMENTRY_native_bank_pixels(void)
{
    PE_Disc *disc;
    char err[256];
    pe_addr_t tim, image;
    uint32_t x, y, width, height, i, j;
    uint16_t pixel;

    TEST_RETAIL_DISC1("ROOMENTRY_native_bank_pixels");
    ResetTestState();
    disc = pe_btl124_open_m0005i(err, sizeof(err));
    ASSERT(disc != NULL, err[0] ? err : "open m0005i");
    PE_GPU_Init();
    B54KR_SeedGpuStatic(); /* reproduce GPU/DMA bring-up after the reset */
    PE_StoreU8(0x800B0CE1u, PE_LoadU8(0x800B0CE0u) ^ 0x80u);
    ASSERT(func_8006BD68() == 0, "native bank read and upload");
    ASSERT(func_80074DC0(0) == 0, "native GPU queue drained");
    tim = PE_LoadU32(0x800B0E6Cu);
    ASSERT(PE_LoadU32(tim) == 0x10u, "real selected-bank payload is TIM");
    image = tim + 8u;
    if (PE_LoadU32(tim + 4u) & 8u)
        image += PE_LoadU32(image);
    x = PE_LoadU16(image + 4u);
    y = PE_LoadU16(image + 6u);
    width = PE_LoadU16(image + 8u);
    height = PE_LoadU16(image + 10u);
    ASSERT(width > 0 && height > 0 && x + width <= 1024u && y + height <= 512u,
           "real TIM image has bounded nonempty dimensions");
    PE_Disc_SetActive(NULL);
    PE_Disc_Close(disc);
    for (j = 0; j < height; ++j)
        for (i = 0; i < width; ++i) {
            int same = PE_GPU_ReadVRAM(x+i, y+j, &pixel)
                       && pixel == PE_LoadU16(image + 12u + 2u*(j*width+i));
            snprintf(err, sizeof(err), "TIM image word (%u,%u) expected %04X, VRAM %04X",
                     x+i, y+j, PE_LoadU16(image + 12u + 2u*(j*width+i)), pixel);
            ASSERT(same, err);
        }
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE, "no boundary");
    PASS();
}

/* F074 suffix, retail 3F1E0..3F208: after 68B94 the room-entry path runs
 * SetGeomScreen(D_800B8A18), then sets D_800BCFEE bit 0x40 (the sb sits in
 * 6BE4C's delay slot), which arms 3F3C4's first-frame fade. 68B94 has just
 * set the same H unless 677FC failed, so only the bit is witnessed here. */
static void test_ROOMENTRY_fade_arm(void)
{
    PE_Disc *disc;
    char err[256];

    TEST_RETAIL_DISC1("ROOMENTRY_fade_arm");
    ResetTestState();
    PE_StoreU8(0x800BCFEEu, 0x01u);       /* bit 0x40 clear on entry */
    disc = pe_btl124_open_m0005i(err, sizeof(err));
    ASSERT(disc != NULL, err[0] ? err : "open m0005i");
    PE_Disc_SetActive(NULL);
    PE_Disc_Close(disc);
    ASSERT((PE_LoadU8(0x800BCFEEu) & 0x40u) != 0u,
           "room entry arms the first-frame fade");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE, "no boundary");
    PASS();
}

/* The consumer of that bit: 3F3C4's first-frame check (retail
 * 3F598..3F5E4) starts 66C7C(0xF) when the window is fully covered.
 * Before the F074 suffix no reachable port path set the bit. */
static void test_ROOMENTRY_first_frame_fade(void)
{
    char msg[128];
    unsigned arm;

    TEST_RETAIL_DISC1("ROOMENTRY_first_frame_fade"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    for (arm = 0; arm < 2; ++arm) {
        ResetTestState();
        FT1_SeedOtcJtb();
        B54KR_SeedGpuStatic();                /* retail libgpu workers */
        pe_btl38_plant_fade_slot();
        PE_StoreU32(0x800B0CD8u, 0x40002000u);
        PE_StoreU32(0x8009CDA4u, 0u);         /* first field frame */
        PE_StoreU16(0x800BCFE8u, 0xFFu);      /* exit fade finished: */
        PE_StoreU16(0x800BCFEAu, 0xFFu);      /* window fully covered */
        PE_StoreU16(0x800BCFECu, 0xFFu);
        PE_StoreU8(0x800BCFEEu, arm ? 0x41u : 0x01u);
        Test_SeedDigitalController();
        func_8003F3C4();
        if (arm) {
            ASSERT(PE_LoadU8(0x800BCFEEu) == 6u && PE_LoadU16(0x800BCFF6u) == 15u
                   && PE_LoadU16(0x800BCFF8u) == 0u,
                   "armed first frame starts the 15-tick fade-in");
            ASSERT(PE_LoadU16(0x800BCFE8u) == 0u && PE_LoadU16(0x800BCFEAu) == 0u
                   && PE_LoadU16(0x800BCFECu) == 0u
                   && PE_LoadU16(0x800BCFF0u) == 0xFFu && PE_LoadU16(0x800BCFF2u) == 0xFFu
                   && PE_LoadU16(0x800BCFF4u) == 0xFFu,
                   "window cleared after its snapshot");
        } else {
            ASSERT(PE_LoadU8(0x800BCFEEu) == 1u && PE_LoadU16(0x800BCFE8u) == 0xFFu,
                   "without bit 0x40 the covered window holds");
        }
        ASSERT(PE_LoadU32(0x8009CDA4u) == 1u, "one field frame ran");
        snprintf(msg, sizeof(msg), "no boundary (arm %u: %s)", arm,
                 PE_Port_StopReasonName(PE_Port_GetStopReason()));
        ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE, msg);
    }
    PASS();
}

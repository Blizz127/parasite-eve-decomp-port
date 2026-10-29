/* The ending credits (ovl_0457, PE.IMG [0x457,0x4FC) -> 0x80120D00) as native
 * code: func_8001220C reaches the credits main loop 0x801235DC through the
 * guest-code registry, and the credits' libpress calls into ovl_039F
 * (PE.IMG [0x39F,0x3C5) -> 0x8010BCF8) run on host replacements
 * (platform/pe_libpress.c) and the func_8010C89C hand port.
 *
 * The retail cases copy the two overlays from the owner's PE.IMG into guest
 * RAM, as func_8006F044 does before the call, and run the retail bytes in the
 * TEST-ONLY R3000A oracle (tests/pe_akao_interp_oracle.c) next to the native
 * code.  No retail byte is in git. */
#ifndef TEST_CREDITS_NATIVE_H
#define TEST_CREDITS_NATIVE_H
#include "pe_guestcode.h"
#include "pe_mdec.h"
#include "pe_audio_driver.h"
#include "pe_spu.h"

#define CREDITS_LIBPRESS_VMA 0x8010BCF8u
#define CREDITS_OVL_VMA      0x80120D00u
/* func_8012403C: D_80011610 + ((D_80093168 - D_80093166) << 11) + 0x38800 /
 * + 0x43000, with D_80011610 = 0x80120D00 and the pair (0x457, 0x4FC). */
#define CREDITS_VLC_OUT      (CREDITS_OVL_VMA + 0x52800u + 0x38800u)
#define CREDITS_VLC_TABLE    (CREDITS_OVL_VMA + 0x52800u + 0x43000u)
#define CREDITS_PICTURES     0x80125BA4u   /* D_80125BA4: 31 BS picture pointers */
#define CREDITS_VLC_BYTES    (CREDITS_VLC_TABLE - CREDITS_VLC_OUT)   /* 0xA800 */

static int credits_load_sectors(uint32_t first, uint32_t end, pe_addr_t dst)
{
    char err[160];
    uint32_t bytes = (end - first) * 0x800u;
    uint8_t *ram = (uint8_t *)PE_Translate(dst, bytes);
    if (!ram) return -1;
    return PE_DiscCache_FileRead(PE_DISC_SRC_PEIMG, first * 0x800u, ram, bytes, err, sizeof err);
}

static int credits_load_overlays(void)
{
    if (credits_load_sectors(0x39Fu, 0x3C5u, CREDITS_LIBPRESS_VMA) != 0) return -1;
    return credits_load_sectors(0x457u, 0x4FCu, CREDITS_OVL_VMA);
}

/* DecDCTvlcSize / DecDCTBufSize as the retail bytes define them
 * (func_8010C86C: addi at,a0,-1; blez -> 0xFFFFFF, else a0 << 1; the old
 * bound is returned from the delay slot). */
static void test_credits_libpress_replacements(void)
{
    TEST("credits_libpress_vlcsize_bufsize_outsync");
    ResetTestState();
    PE_StoreU32(0x8011EB8Cu, 0x00FFFFFFu);
    ASSERT(PE_DecDCTvlcSize_Guest(0x450u) == 0x00FFFFFF && PE_LoadU32(0x8011EB8Cu) == 0x8A0u,
           "vlcSize(0x450): old bound returned, new bound size<<1");
    ASSERT(PE_DecDCTvlcSize_Guest(2u) == 0x8A0 && PE_LoadU32(0x8011EB8Cu) == 4u, "vlcSize(2)");
    ASSERT(PE_DecDCTvlcSize_Guest(1u) == 4 && PE_LoadU32(0x8011EB8Cu) == 0x00FFFFFFu,
           "vlcSize(1): no bound");
    ASSERT(PE_DecDCTvlcSize_Guest(0u) == 0x00FFFFFF && PE_LoadU32(0x8011EB8Cu) == 0x00FFFFFFu,
           "vlcSize(0): no bound");
    ASSERT(PE_DecDCTvlcSize_Guest(0xFFFFFFFFu) == 0x00FFFFFF &&
           PE_LoadU32(0x8011EB8Cu) == 0x00FFFFFFu, "vlcSize(-1): signed compare");
    PE_StoreU32(0x80100000u, 0x38001140u);
    ASSERT(PE_DecDCTBufSize_Guest(0x80100000u) == 0x1140u, "BufSize: first halfword");
    /* DecDCToutSync(1) reads MDEC1 status bit 24 = the command's bit 26. */
    PE_MDEC_Init();
    PE_MDEC_BeginReset();
    ASSERT(PE_DecDCToutSync_Guest(1) == 0, "outSync(1) after reset (status 0x80040000)");
    ASSERT(PE_DecDCToutSync_Guest(0) == 0, "outSync(0) with DMA1 idle");
    PE_StoreU32(0x80100000u, 0x3C000020u);
    ASSERT(PE_MDEC_SubmitInputTable(0x80100000u, 0x20u) == 0, "submit signed-output command");
    ASSERT(PE_DecDCToutSync_Guest(1) == 1, "outSync(1) = CMD.26 set");
    PE_MDEC_BeginReset();
    PE_StoreU32(0x80100000u, 0x38000020u);
    ASSERT(PE_MDEC_SubmitInputTable(0x80100000u, 0x20u) == 0, "submit unsigned-output command");
    ASSERT(PE_DecDCToutSync_Guest(1) == 0, "outSync(1) = CMD.26 clear");
    PE_MDEC_Init();
    PASS();
}

/* The two pure replacements against the retail ovl_039F bytes. */
static void test_credits_libpress_retail_oracle(void)
{
    static const uint32_t sizes[] = {0u, 1u, 2u, 3u, 0x10u, 0x450u, 0x3FFFu, 0x7FFFu,
                                     0x7FFFFFFFu, 0x80000001u, 0xFFFFFFFEu, 0xFFFFFFFFu};
    unsigned i;
    TEST_RETAIL_DISC1("credits_libpress_vlcsize_bufsize_match_retail");
    ResetTestState();
    ASSERT(credits_load_overlays() == 0, "ovl_039F/ovl_0457 from PE.IMG");
    for (i = 0; i < sizeof sizes / sizeof sizes[0] + 64u; i++) {
        uint32_t size = i < sizeof sizes / sizeof sizes[0] ? sizes[i] : (i * 0x9E3779B9u) >> (i & 15u);
        uint32_t seed = 0xA5A50000u ^ (i * 0x01000193u), rv, nv, rb;
        int ok = 0;
        if (size == 0x80000000u) continue;   /* addi overflow trap */
        PE_StoreU32(0x8011EB8Cu, seed);
        rv = PE_AudioDriver_CallGuest(0x8010C86Cu, size, 0u, 0u, 0u, &ok);
        ASSERT(ok, "retail func_8010C86C ran");
        rb = PE_LoadU32(0x8011EB8Cu);
        PE_StoreU32(0x8011EB8Cu, seed);
        nv = (uint32_t)PE_DecDCTvlcSize_Guest(size);
        ASSERT(nv == rv && PE_LoadU32(0x8011EB8Cu) == rb, "vlcSize matches retail");
    }
    for (i = 0; i < 31u; i++) {
        pe_addr_t bs = PE_LoadU32(CREDITS_PICTURES + i * 4u);
        int ok = 0;
        uint32_t rv = PE_AudioDriver_CallGuest(0x8010BF94u, bs, 0u, 0u, 0u, &ok);
        ASSERT(ok && rv == PE_DecDCTBufSize_Guest(bs), "BufSize matches retail");
    }
    PASS();
}

/* func_8001220C reaches 0x801235DC with the reloaded ovl_0457 bytes there:
 * the registry must bind the generated credits loop, and every credits op
 * (the D_80125B4C / D_80125B68 tables it calls through) must be native.
 * Other bytes at the address stay a loud boundary. */
static void test_credits_registry_dispatch(void)
{
    static const pe_addr_t tables[2] = {0x80125B4Cu, 0x80125B68u};
    const PeGuestCodeEntry *e;
    unsigned t, k;
    TEST_RETAIL_DISC1("credits_main_loop_resolves_native");
    ResetTestState();
    PE_Overlay_ResetResidency();
    ASSERT(credits_load_overlays() == 0, "ovl_039F/ovl_0457 from PE.IMG");
    e = PE_GuestCode_Resolve(0x801235DCu);
    ASSERT(e && e->symbol && strcmp(e->symbol, "ovl_0457__func_801235DC") == 0,
           "0x801235DC resolves to the generated ovl_0457 credits loop");
    for (t = 0; t < 2u; t++)
        for (k = 0; k < 7u; k++) {
            pe_addr_t fn = PE_LoadU32(tables[t] + k * 4u);
            if (!fn) continue;
            e = PE_GuestCode_Resolve(fn);
            ASSERT(e && e->symbol && strncmp(e->symbol, "ovl_0457__", 10) == 0,
                   "every credits op resolves to a generated ovl_0457 TU");
        }
    PE_StoreU8(0x801235DCu, (uint8_t)(PE_LoadU8(0x801235DCu) ^ 0xFFu));
    ASSERT(PE_GuestCode_Resolve(0x801235DCu) == NULL, "other bytes at 0x801235DC do not resolve");
    PASS();
}

/* The credits decode every picture with a real VLC bound (DecDCTvlcSize of
 * BufSize >> 2, or >> 4 in func_80125094) and resume with func_8010C89C(0,
 * 0, table) until it returns 0.  Retail bytes in the oracle and the hand port
 * must leave identical guest RAM and the same return sequence; an unbounded
 * native decode must produce the same output stream. */
static uint8_t *g_credits_snap, *g_credits_ref, *g_credits_unbounded;

static unsigned credits_decode(int retail, pe_addr_t bs, unsigned shift, uint32_t *rets)
{
    unsigned n = 0;
    int ok = 1;
    uint32_t r;
    if (retail) {
        uint32_t size = PE_AudioDriver_CallGuest(0x8010BF94u, bs, 0u, 0u, 0u, &ok);
        (void)PE_AudioDriver_CallGuest(0x8010C86Cu, shift ? size >> shift : 0u, 0u, 0u, 0u, &ok);
        r = PE_AudioDriver_CallGuest(0x8010C89Cu, bs, CREDITS_VLC_OUT, CREDITS_VLC_TABLE, 0u, &ok);
    } else {
        uint32_t size = PE_DecDCTBufSize_Guest(bs);
        (void)PE_DecDCTvlcSize_Guest(shift ? size >> shift : 0u);
        r = (uint32_t)func_8010C89C(bs, CREDITS_VLC_OUT, CREDITS_VLC_TABLE, 0u);
    }
    for (;;) {
        if (!ok) return 0u;
        if (n < 4096u) rets[n] = r;
        n++;
        if (r == 0u || n >= 4096u) break;
        r = retail ? PE_AudioDriver_CallGuest(0x8010C89Cu, 0u, 0u, CREDITS_VLC_TABLE, 0u, &ok)
                   : (uint32_t)func_8010C89C(0u, 0u, CREDITS_VLC_TABLE, 0u);
    }
    return n;
}

static void test_credits_vlc_bounded_resume_oracle(void)
{
    static uint32_t rret[4096], nret[4096];
    uint8_t *ram = (uint8_t *)PE_Translate(0x80000000u, PE_RAM_SIZE);
    unsigned i, s, resumes = 0;
    int ok = 0;
    TEST_RETAIL_DISC1("credits_vlc_decode_bounded_resume_matches_retail");
    ResetTestState();
    ASSERT(credits_load_overlays() == 0, "ovl_039F/ovl_0457 from PE.IMG");
    if (!g_credits_snap) g_credits_snap = (uint8_t *)malloc(PE_RAM_SIZE);
    if (!g_credits_ref) g_credits_ref = (uint8_t *)malloc(PE_RAM_SIZE);
    if (!g_credits_unbounded) g_credits_unbounded = (uint8_t *)malloc(CREDITS_VLC_BYTES);
    ASSERT(g_credits_snap && g_credits_ref && g_credits_unbounded, "snapshot buffers");
    /* func_8012489C: func_8010BD4C builds the VLC table (retail bytes). */
    (void)PE_AudioDriver_CallGuest(0x8010BD4Cu, CREDITS_VLC_TABLE, 0u, 0u, 0u, &ok);
    ASSERT(ok, "retail VLC table build");
    for (i = 0; i < 31u; i++) {
        pe_addr_t bs = PE_LoadU32(CREDITS_PICTURES + i * 4u);
        for (s = 0; s < 3u; s++) {
            static const unsigned shifts[3] = {0u, 2u, 4u};
            unsigned rn, nn, k;
            memcpy(g_credits_snap, ram, PE_RAM_SIZE);
            rn = credits_decode(1, bs, shifts[s], rret);
            ASSERT(rn > 0u && rn < 4096u, "retail decode ran to the pad exit");
            memcpy(g_credits_ref, ram, PE_RAM_SIZE);
            memcpy(ram, g_credits_snap, PE_RAM_SIZE);
            nn = credits_decode(0, bs, shifts[s], nret);
            ASSERT(nn == rn, "same number of bounded calls as retail");
            for (k = 0; k < rn; k++)
                ASSERT(nret[k] == rret[k], "same return sequence as retail");
            if (memcmp(g_credits_ref, ram, PE_RAM_SIZE) != 0) {
                for (k = 0; k < PE_RAM_SIZE && g_credits_ref[k] == ram[k]; k++) {}
                printf("[credits] picture %u shift %u: first RAM difference 0x%08X retail %02X native %02X\n",
                       i, shifts[s], 0x80000000u + k, g_credits_ref[k], ram[k]);
                ASSERT(0, "guest RAM after the decode matches retail");
            }
            if (s == 0u)
                memcpy(g_credits_unbounded, ram + (CREDITS_VLC_OUT & 0x1FFFFFu), CREDITS_VLC_BYTES);
            else {
                resumes += rn - 1u;
                ASSERT(memcmp(g_credits_unbounded, ram + (CREDITS_VLC_OUT & 0x1FFFFFu), CREDITS_VLC_BYTES) == 0,
                       "bounded+resume output equals the unbounded decode");
            }
            memcpy(ram, g_credits_snap, PE_RAM_SIZE);
        }
    }
    printf("[credits] 31 pictures x 3 bounds, %u resumed calls, retail == native ", resumes);
    ASSERT(resumes > 0u, "the bound actually interrupted a decode");
    PASS();
}

/* func_8001220C's credits arm refuses to enter in the cases the loop cannot
 * finish, each on a named boundary.  Modes 0 and 0x20000 without
 * \XASTREAM\CREDITS.XA;1 on the active disc (none here) are
 * credits_xa_file_absent, because func_8012562C would retry its search
 * forever.  The pad-code mode (bit 16: 0x10000, and the 0x12000 the pad code
 * sets) opens no file but still needs the drive, so with no disc it is
 * credits_cd_attach.  Nothing of the credits runs: the op-table pointer the
 * loop stores first keeps its sentinel, the libcd queue is untouched, and the
 * device stays detached. */
static void test_credits_entry_gates(void)
{
    static const uint32_t modes[] = { 0u, 0x10000u, 0x12000u, 0x20000u };
    unsigned m;
    TEST("credits_entry_gates");
    ASSERT(PE_Cd_XaServiceAvailable() == 1, "the attached device serves XA streaming");
    for (m = 0; m < 4u; m++) {
        const char *want = (modes[m] & 0x10000u) ? "credits_cd_attach"
                                                 : "credits_xa_file_absent";
        ResetTestState();
        PE_CdReg_Reset();
        PE_Disc_SetActive(NULL);
        PE_StoreU32(0x8009D1A0u, modes[m]);
        PE_StoreU32(0x80172CD0u, 0xC0DEC0DEu);
        PE_StoreU32(0x8009B53Cu, 0x55u);
        ASSERT(PE_Main_EnterCredits() == 0, "credits arm reports not run");
        ASSERT(PE_Port_ShouldStop() &&
               PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
               "loud unresolved-boundary stop");
        ASSERT(g_stub_order_count == 1 && strcmp(g_stub_order_log[0], want) == 0,
               "credits gate boundary identity");
        ASSERT(g_bootstrap_arg_call_count == 1 &&
               g_bootstrap_arg_calls[0].arg0 == (modes[m] & 0x30000u),
               "boundary records the credits mode bits");
        ASSERT(PE_LoadU32(0x80172CD0u) == 0xC0DEC0DEu, "credits loop not entered");
        ASSERT(PE_LoadU32(0x8009B53Cu) == 0x55u && PE_LoadU32(0x800A3608u) == 0u,
               "libcd queue untouched");
        ASSERT(!PE_CdReg_DeviceEnabled(), "device not attached");
    }
    PASS();
}

/* Why the arm attaches the device.  With the device disabled (production's
 * host-mode provider) a queued command never completes.  func_80125744 starts with CdlSetmode 0xC8
 * through func_8007EE84(14, mode, 0, -1) and polls func_8007F418 in a
 * for(;;) that only a 2 (or a retry on another nonzero) leaves; here every
 * poll reads 0 (pending) and nothing requests a stop.  The kick latches lane
 * D_8009B574 = 2 and only the IRQ chain clears it, so every later
 * func_8007F72C() == 1 wait (func_801255A4's drain among them) would spin as
 * well.  The completion ring stays empty, so func_80125894's
 * func_8007F418(0, 0) reads 6, never 2. */
static void test_credits_xa_commands_never_complete_without_device(void)
{
    const pe_addr_t mode = 0x801F0000u, resp = 0x801F0010u;
    PE_Disc *disc;
    char err[256];
    uint32_t seq;
    unsigned i;
    TEST_RETAIL_DISC1("credits_xa_commands_never_complete_without_device");
    ResetTestState();
    PE_CdReg_Reset();
    err[0] = 0;
    disc = BTL6_OpenDisc1(err, sizeof(err));
    ASSERT(disc != NULL, err[0] ? err : "PE_Disc_Open failed");
    PE_Disc_SetActive(disc);
    ASSERT(PE_GuestImage_LoadExe(disc, err, sizeof(err)) == 0, err);
    ASSERT(PE_Globals_AdoptRetailImage() == 0, "retail startup globals");
    ASSERT(!PE_CdReg_DeviceEnabled(), "production provider: device disabled");
    ASSERT(func_8007EC14() == 1 && func_8007ED58() == 1, "CdInit and reset");
    ASSERT(func_8007F72C() == 1 && func_8007F778() == 0, "drive idle, queue empty");
    PE_StoreU8(mode, 200u);
    seq = func_8007EE84(14u, mode, 0u, 0xFFFFFFFFu);
    ASSERT(seq != 0u && !PE_Port_ShouldStop() && PE_LoadU32(0x800A3608u) == 1u,
           "CdlSetmode queued and kicked");
    for (i = 0; i < 100000u; i++)
        if (func_8007F418(seq, resp) != 0) break;
    ASSERT(i == 100000u && !PE_Port_ShouldStop(),
           "CdlSetmode stays pending for 100000 polls with no stop");
    ASSERT(PE_LoadU32(0x8009B574u) == 2u && func_8007F72C() == 2,
           "lane stays 2, so later idle waits cannot pass");
    ASSERT(func_8007F418(0u, 0u) == 6, "empty completion ring: the channel-switch wait reads 6");
    PE_Disc_SetActive(NULL);
    PE_Disc_Close(disc);
    PASS();
}

/* The ending credits' CD-XA path on the register-level device: the retail
 * EXE image, ovl_039F/ovl_0457 from PE.IMG, and a synthetic Disc-2-like
 * image holding \XASTREAM\CREDITS.XA;1 laid out as the real file is
 * (file 1; channels 0 and 1 one sector in 8 with submode 64h, EOF-flagged
 * last ones; null sectors between).  No retail byte is in the fixture. */
#define CXA_SECTORS      512u
#define CXA_FILE_LBA     64u
#define CXA_FILE_SECTORS 48u
int32_t ovl_0457__func_8012562C(void);
int32_t ovl_0457__func_80125744(void);
int32_t ovl_0457__func_80125894(uint8_t a_a0);
int ovl_0457__func_801255A4(void);
void ovl_0457__func_801235DC(void);
void func_800870F0(unsigned int a0);
void func_8006F044(void);

/* total_sectors >= CXA_SECTORS: sectors past CXA_SECTORS are left untouched
 * (calloc'd zero pages, never written) so a long fixture costs no memory; the
 * drive delivers them as zero data sectors. */
static PE_Disc *CxaBuildDiscSized(uint8_t **out, uint32_t total_sectors, int with_file)
{
    uint8_t *img = calloc(total_sectors, PE_DISC_RAW_SECTOR), *p;
    uint32_t i;
    int n;
    if (!img) return NULL;
    for (i = 0; i < CXA_SECTORS; i++) {
        uint8_t *raw = img + (size_t)i * PE_DISC_RAW_SECTOR;
        memset(raw + 1, 0xFF, 10);
        raw[15] = 2u;
    }
    p = FxUser(img, 16);
    p[0] = 1; memcpy(p + 1, "CD001", 5); p[6] = 1;
    memcpy(p + 40, "PARASITE_EVE                    ", 32);
    FxPutDirRec(p + 156, 20, 2048, 0x02, "\0", 1);
    p = FxUser(img, 20);
    n = FxPutDirRec(p, 20, 2048, 0x02, "\0", 1);
    n += FxPutDirRec(p + n, 20, 2048, 0x02, "\1", 1);
    FxPutDirRec(p + n, 21, 2048, 0x02, "XASTREAM", 8);
    p = FxUser(img, 21);
    n = FxPutDirRec(p, 21, 2048, 0x02, "\0", 1);
    n += FxPutDirRec(p + n, 20, 2048, 0x02, "\1", 1);
    if (with_file)
        FxPutDirRec(p + n, CXA_FILE_LBA, CXA_FILE_SECTORS * 2048u, 0x00, "CREDITS.XA;1", 12);
    for (i = 0; i < CXA_FILE_SECTORS; i++) {
        uint8_t *raw = img + (size_t)(CXA_FILE_LBA + i) * PE_DISC_RAW_SECTOR;
        uint8_t slot = (uint8_t)(i & 7u), audio = slot < 2u;
        uint8_t sm = audio ? (i + 8u >= CXA_FILE_SECTORS ? 0xE4u : 0x64u) : 0x00u;
        raw[16] = raw[20] = 1u;
        raw[17] = raw[21] = audio ? slot : 0u;
        raw[18] = raw[22] = sm;
        raw[19] = raw[23] = audio ? 1u : 0u;
    }
    *out = img;
    return PE_Disc_OpenMemory(img, (size_t)total_sectors * PE_DISC_RAW_SECTOR);
}

static PE_Disc *CxaBuildDisc(uint8_t **out)
{
    return CxaBuildDiscSized(out, CXA_SECTORS, 1);
}

static void test_credits_xa_device_path(void)
{
    PE_Disc *d1, *cx;
    uint8_t *img = NULL;
    char err[256];
    PeCdDeviceState dev;
    unsigned t;
    TEST_RETAIL_DISC1("credits_xa_device_path");
    ResetTestState();
    PE_CdReg_Reset();
    err[0] = 0;
    d1 = BTL6_OpenDisc1(err, sizeof(err));
    ASSERT(d1 != NULL, err[0] ? err : "PE_Disc_Open failed");
    PE_Disc_SetActive(d1);
    ASSERT(PE_GuestImage_LoadExe(d1, err, sizeof(err)) == 0, err);
    ASSERT(PE_Globals_AdoptRetailImage() == 0, "retail startup globals");
    ASSERT(credits_load_overlays() == 0, "ovl_039F/ovl_0457 from PE.IMG");
    cx = CxaBuildDisc(&img);
    ASSERT(cx != NULL, "credits disc fixture");
    PE_Disc_SetActive(cx);
    HostFB_Init(); PE_GPU_Init(); PE_Callback_Init();
    ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14() == 1 && !PE_Port_ShouldStop(),
           "CdInit on the device");
    for (t = 0; t < 20u && PE_LoadU32(0x8009B574u) != 1u && !PE_Port_ShouldStop(); t++)
        HostFB_VSync(0);
    ASSERT(PE_LoadU32(0x8009B574u) == 1u && !PE_Port_ShouldStop(), "drive ready");
    ASSERT(ovl_0457__func_8012562C() == 0 && !PE_Port_ShouldStop(), "func_8012562C: CdSearchFile");
    ASSERT(PE_LoadU32(0x80172CE8u) == CXA_FILE_LBA &&
           PE_LoadU32(0x80172CECu) == CXA_FILE_LBA + CXA_FILE_SECTORS, "credits start and end LBA");
    ASSERT(ovl_0457__func_80125744() == 0 && !PE_Port_ShouldStop(),
           "func_80125744: Setmode, Setfilter and ReadS complete through the real queue");
    PE_CdReg_GetDeviceState(&dev);
    ASSERT(dev.reading && dev.mode == 0xC8u, "ReadS running in mode 0xC8");
    for (t = 0; t < 600u && !PE_LoadU32(0x80172CE4u) && !PE_Port_ShouldStop(); t++)
        HostFB_VSync(0);
    PE_CdReg_GetDeviceState(&dev);
    ASSERT(!PE_Port_ShouldStop(), "no stop while the XA stream plays");
    ASSERT(PE_LoadU32(0x80172CE4u) == 1u, "GetlocP positions carried the credits past the file end");
    ASSERT((int32_t)PE_LoadU32(0x80172CF0u) > (int32_t)(CXA_FILE_LBA + CXA_FILE_SECTORS),
           "position past the end");
    ASSERT(dev.xa_sectors >= CXA_FILE_SECTORS / 4u, "every audio sector went to the SPU");
    ASSERT(ovl_0457__func_80125894(1u) == 0 && !PE_Port_ShouldStop(), "channel switch to 1");
    ASSERT(ovl_0457__func_801255A4() == 0 && !PE_Port_ShouldStop(), "func_801255A4 teardown");
    PE_CdReg_GetDeviceState(&dev);
    ASSERT(!dev.reading && PE_LoadU32(0x8009B574u) == 1u && PE_LoadU32(0x800A3608u) == 0u,
           "after teardown the drive is idle and the queue empty");
    PE_CdReg_Reset();
    PE_Disc_SetActive(NULL);
    PE_Disc_Close(cx);
    PE_Disc_Close(d1);
    free(img);
    PASS();
}

/* The credits CD steps from production's entry state.  Production boots libds
 * in host mode: func_8003E610 runs ResetCallback and the host DsInit
 * (D_8009B554 = 1), and func_8006A5BC's CdReset leaves lane 1.  The overlay's
 * own DsInit is then a no-op, so PE_Cd_AttachDevice must supply the device
 * branch's two hooks, and PE_Cd_DetachDevice must take both off again.
 * Afterwards the IRQ 2 slot, VSync slot 0, I_MASK and the registered-source
 * mask must equal their pre-attach values.  A host-mode read must still
 * complete. */
static void test_credits_attach_from_host_boot(void)
{
    PE_Disc *d1, *cx;
    uint8_t *img = NULL;
    char err[256];
    PeCdDeviceState dev;
    PeCdAttach at;
    uint32_t irq2, vsync0;
    uint16_t mask, registered;
    unsigned t;
    TEST_RETAIL_DISC1("credits_attach_from_host_boot");
    ResetTestState();
    PE_CdReg_Reset();
    err[0] = 0;
    d1 = BTL6_OpenDisc1(err, sizeof(err));
    ASSERT(d1 != NULL, err[0] ? err : "PE_Disc_Open failed");
    PE_Disc_SetActive(d1);
    ASSERT(PE_GuestImage_LoadExe(d1, err, sizeof(err)) == 0, err);
    ASSERT(PE_Globals_AdoptRetailImage() == 0, "retail startup globals");
    ASSERT(credits_load_overlays() == 0, "ovl_039F/ovl_0457 from PE.IMG");
    cx = CxaBuildDisc(&img);
    ASSERT(cx != NULL, "credits disc fixture");
    PE_Disc_SetActive(cx);
    HostFB_Init(); PE_GPU_Init(); PE_Callback_Init();
    func_80073C94();
    ASSERT(!PE_CdReg_DeviceEnabled() && func_8007EC14() == 1 &&
           PE_LoadU32(0x8009B554u) == 1u && func_8007ED58() == 1 &&
           func_8007F72C() == 1 && !PE_Port_ShouldStop(),
           "host-mode boot: DsInit host branch, CdReset, lane 1");
    irq2 = PE_LoadU32(0x800945E8u + 8u);
    vsync0 = PE_Callback_GetSlot(0u);
    mask = PE_IRQ_GetMask();
    registered = PE_LoadU16(0x80094614u);
    ASSERT(irq2 == 0u && vsync0 == 0u, "host boot installs neither device hook");
    ASSERT(PE_Cd_AttachDevice(&at) && PE_CdReg_DeviceEnabled() &&
           PE_LoadU32(0x800945E8u + 8u) == 0x8007C13Cu &&
           PE_Callback_GetSlot(0u) == 0x8007FE24u && (PE_IRQ_GetMask() & 4u),
           "attach installs InterruptCallback(2) and VSyncCallbacks(0)");
    {
        PeCdAttach again;
        ASSERT(!PE_Cd_AttachDevice(&again) && !again.attached, "attach refuses twice");
    }
    ASSERT(ovl_0457__func_8012562C() == 0 && !PE_Port_ShouldStop(),
           "func_8012562C: DsInit no-op, CdSearchFile");
    ASSERT(PE_LoadU32(0x80172CE8u) == CXA_FILE_LBA, "credits start LBA");
    ASSERT(ovl_0457__func_80125744() == 0 && !PE_Port_ShouldStop(),
           "func_80125744 through the attached drive");
    for (t = 0; t < 600u && !PE_LoadU32(0x80172CE4u) && !PE_Port_ShouldStop(); t++)
        HostFB_VSync(0);
    PE_CdReg_GetDeviceState(&dev);
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x80172CE4u) == 1u &&
           dev.xa_sectors >= CXA_FILE_SECTORS / 4u, "XA stream played to its end");
    ASSERT(ovl_0457__func_801255A4() == 0 && !PE_Port_ShouldStop(), "teardown");
    {
        /* The game's CdMix (func_800870F0 -> func_8007A88C) through the
         * attached drive; the latch must survive the detach, because the
         * host movie feed reads it with the device off. */
        uint8_t mix[4], kept[4];
        func_800870F0(144u);
        PE_CdReg_GetAudioVolumes(mix);
        ASSERT(!PE_Port_ShouldStop() && (mix[0] | mix[1] | mix[2] | mix[3]),
               "CdMix applied through the attached drive");
        PE_Cd_DetachDevice(&at);
        PE_CdReg_GetAudioVolumes(kept);
        ASSERT(memcmp(mix, kept, 4) == 0, "detach keeps the CD audio mix");
    }
    ASSERT(!PE_CdReg_DeviceEnabled(), "detach disables the device");
    ASSERT(PE_LoadU32(0x800945E8u + 8u) == irq2 && PE_Callback_GetSlot(0u) == vsync0 &&
           PE_IRQ_GetMask() == mask && PE_LoadU16(0x80094614u) == registered,
           "detach restores the IRQ 2 slot, VSync slot 0 and both masks");
    ASSERT(func_8006E6D4(16, 0, 0x801E0000u, 1) == 1 && !PE_Port_ShouldStop() &&
           memcmp(PE_Translate(0x801E0001u, 5u), "CD001", 5) == 0,
           "a host-mode read completes after detach");
    PE_CdReg_Reset();
    PE_Disc_SetActive(NULL);
    PE_Disc_Close(cx);
    PE_Disc_Close(d1);
    free(img);
    PASS();
}

/* Bounded credits runs from the production entry state: host-mode libds boot
 * (ResetCallback, host DsInit, CdReset), the pad records and the boot input
 * config (func_8003E974), no button held, and an 80000-sector fixture.  Each
 * runs in a forked child under alarm(): the generated loops have no stop test,
 * so a hang must fail the test, not the suite.  A test VBlank hook in VSync
 * slot 7 (the game uses 0 and 4) counts VBlanks and injects the stop or the
 * pad press a case needs.
 *
 * The scroll is 744 lines of 18 px at 1 px per loop iteration after a 224 px
 * lead-in (func_80123E08, the 0x80125C88 line table): about 13600 iterations.
 * The 2x ReadS keeps running past the 48-sector file until the teardown,
 * 2.5 sectors per VBlank, so the disc must outlast that.  Only the first 512
 * sectors are written. */
#define CXA_LOOP_SECTORS 80000u
#define CXA_HOOK_ADDR    0x8000FF00u
enum { CXA_MODE0, CXA_MODE20000, CXA_STOP_IN_LOOP, CXA_QUIT_IN_LOOP,
       CXA_STOP_LANE_WAIT, CXA_STOP_TEARDOWN, CXA_STOP_SEARCH, CXA_AFTER_F044,
       CXA_PAD_CODE, CXA_MODE20000_HOSTPAD, CXA_MODE20000_LEADOUT };
typedef struct {
    uint32_t setup, ret, escaped, stopped, reason, attached_after, slots_ok;
    uint32_t ce0, ce4, iterations, presented, xa, lane, queue, d1a0;
    uint32_t press_iter, pressed_at, vblanks;
    uint32_t dssync0, setfilter_seen, leadout_seen, int4_consumed;
} CxaRun;
static unsigned g_cxa_vblanks, g_cxa_stop_at, g_cxa_quit_after, g_cxa_quit_polls;
static unsigned g_cxa_press_iter, g_cxa_pressed_at, g_cxa_setfilter;
static uint16_t g_cxa_press_pad;
static unsigned g_cxa_leadout_seen, g_cxa_int4_consumed;
static PeCdAttach g_cxa_at;
static unsigned g_cxa_host_press_iter;    /* CXA_MODE20000_HOSTPAD only */

/* A host pad source (window or route script in production) that starts
 * pressing at the given loop iteration.  Nothing else writes the pad buffer
 * in this scenario, so the press reaches func_8003EB04 only if production
 * refills 0x800BE9A2 inside the credits loop. */
static uint16_t CxaHostPad(void)
{
    uint32_t it = PE_LoadU32(0x80172CD4u);      /* the loop counts from 420 */
    if (!g_cxa_host_press_iter || it < 420u || it - 420u < g_cxa_host_press_iter)
        return 0xFFFFu;
    if (!g_cxa_pressed_at) g_cxa_pressed_at = it - 420u;
    return g_cxa_press_pad;
}

static void CxaVBlankHook(void)
{
    g_cxa_vblanks++;
    /* Native func_8007F88C publishes DataEnd into the libds event slot. */
    if(PE_LoadU8(0x800A3534u)==4u) g_cxa_int4_consumed=1u;
    if (PE_CdReg_DeviceEnabled()) {
        PeCdDeviceState d;
        PE_CdReg_GetDeviceState(&d);
        if(d.next_lba==512u && !d.reading && d.motor_off) g_cxa_leadout_seen=1u;
        for (unsigned i = 0; i < 16u && i < d.commands; i++)
            if (d.command_log[i] == 13u) g_cxa_setfilter = 1u;
    }
    if (g_cxa_stop_at && g_cxa_vblanks == g_cxa_stop_at)
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    if (g_cxa_press_iter && !g_cxa_pressed_at) {
        uint32_t it = PE_LoadU32(0x80172CD4u);      /* the loop counts from 420 */
        if (it >= 420u && it - 420u >= g_cxa_press_iter) {
            PE_StoreU16(0x800BE9A2u, g_cxa_press_pad);    /* held from now on */
            g_cxa_pressed_at = it - 420u;
        }
    }
}

static int CxaQuitPoll(void)
{
    return g_cxa_quit_after && ++g_cxa_quit_polls >= g_cxa_quit_after;
}

/* The pad bits (active low) that make D_8009D26C carry one of the given
 * bits under the boot config, inverting func_8003EB04's mapping
 * (m = ~pad & ~0x6000, with buttons 0x2000/0x4000 swapped). */
static int CxaPressFor(const unsigned *bits, unsigned n, uint16_t *pad)
{
    unsigned k;
    for (k = 0; k < n; k++) {
        uint32_t want = PE_LoadU32(0x800A76F0u + bits[k] * 4u), press;
        if (!want) continue;
        press = want & ~0x6000u;
        if (want & 0x4000u) press |= 0x2000u;
        if (want & 0x2000u) press |= 0x4000u;
        *pad = (uint16_t)~press;
        return 1;
    }
    return 0;
}

static int CxaExitPress(uint16_t *pad)
{
    static const unsigned bits[] = { 2u, 28u, 29u, 30u, 31u };
    return CxaPressFor(bits, 5u, pad);
}

static void CxaChild(int scenario, CxaRun *r)
{
    PE_Disc *d1, *cx;
    uint8_t *img = NULL;
    char err[256];
    uint32_t irq2, vsync0;
    uint16_t mask, registered;
    int vs, ds, pr, mk;
    jmp_buf escape;
    memset(r, 0, sizeof *r);
    ResetTestState();
    PE_CdReg_Reset();
    err[0] = 0;
    if (!(d1 = BTL6_OpenDisc1(err, sizeof(err)))) return;
    PE_Disc_SetActive(d1);
    if (PE_GuestImage_LoadExe(d1, err, sizeof(err)) != 0 ||
        PE_Globals_AdoptRetailImage() != 0 || credits_load_overlays() != 0) return;
    if (!(cx = CxaBuildDiscSized(&img, scenario == CXA_MODE20000_LEADOUT ? 512u : CXA_LOOP_SECTORS,
                                  scenario != CXA_STOP_SEARCH))) return;
    if (scenario != CXA_AFTER_F044 && scenario != CXA_PAD_CODE) PE_Disc_SetActive(cx);
    HostFB_Init(); PE_GPU_Init(); PE_Callback_Init();
    PE_Spu_SetActive(1);                        /* the SPU decodes routed XA */
    func_80073C94();
    Test_SeedDigitalController();               /* boot's pad init: 844E4/84B44 */
    (void)func_8003E974();                      /* boot's input config */
    PE_StoreU16(0x800BE9A2u, 0xFFFFu);          /* no button held */
    if (func_8007EC14() != 1 || func_8007ED58() != 1 || func_8007F72C() != 1) return;
    if (scenario == CXA_AFTER_F044 || scenario == CXA_PAD_CODE) {
        /* main's arm runs func_8006F044 first: the host-mode reload of
         * ovl_039F and ovl_0457 from PE.IMG (Disc 1 here; the fixture has
         * no PE.IMG), mounted as func_800698D4 mounts it. */
        uint32_t lba, size;
        if (!PE_Disc_FindFile(d1, "\\PE.IMG;1", &lba, &size, NULL, 0)) return;
        PE_StoreU32(0x800B0DD8u, lba);
        func_8006F044();
        if (PE_Port_ShouldStop()) return;
        PE_Disc_SetActive(cx);
        r->dssync0 = (uint32_t)func_8007F418(0u, 0u);
    }
    irq2 = PE_LoadU32(0x800945E8u + 8u);
    vsync0 = PE_Callback_GetSlot(0u);
    mask = PE_IRQ_GetMask();
    registered = PE_LoadU16(0x80094614u);
    if (PE_Callback_Bind(CXA_HOOK_ADDR, CxaVBlankHook) != 0) return;
    (void)func_80073D58(7u, CXA_HOOK_ADDR);
    PE_StoreU32(0x8009D1A0u, PE_LoadU32(0x8009D1A0u) & ~0x30000u);
    r->setup = 1;
    switch (scenario) {
    case CXA_MODE20000:
        PE_StoreU32(0x8009D1A0u, PE_LoadU32(0x8009D1A0u) | 0x20000u);
        if (!CxaExitPress(&g_cxa_press_pad)) { r->setup = 0; return; }
        g_cxa_press_iter = 14000u;          /* inside state 2, after the scroll */
        r->ret = (uint32_t)PE_Main_EnterCredits();
        break;
    case CXA_MODE20000_LEADOUT:
    case CXA_MODE20000_HOSTPAD:
        PE_StoreU32(0x8009D1A0u, PE_LoadU32(0x8009D1A0u) | 0x20000u);
        if (!CxaExitPress(&g_cxa_press_pad)) { r->setup = 0; return; }
        g_cxa_host_press_iter = 14000u;     /* the hook stays silent */
        PE_Port_SetPadSource(CxaHostPad);
        r->ret = (uint32_t)PE_Main_EnterCredits();
        r->press_iter = g_cxa_host_press_iter;
        break;
    case CXA_PAD_CODE: {
        static const unsigned channel0[] = { 26u };
        PE_StoreU32(0x8009D1A0u, PE_LoadU32(0x8009D1A0u) | 0x12000u);
        if (!CxaPressFor(channel0, 1u, &g_cxa_press_pad)) { r->setup = 0; return; }
        g_cxa_press_iter = 2000u;           /* mid-scroll: channel switch to 0 */
        r->ret = (uint32_t)PE_Main_EnterCredits();
        break;
    }
    case CXA_STOP_IN_LOOP:
        g_cxa_stop_at = 600u;
        r->ret = (uint32_t)PE_Main_EnterCredits();
        break;
    case CXA_QUIT_IN_LOOP:
        g_cxa_quit_after = 200000u;
        PE_Port_SetQuitPoll(CxaQuitPoll);
        r->ret = (uint32_t)PE_Main_EnterCredits();
        break;
    case CXA_MODE0:
    case CXA_AFTER_F044:
        r->ret = (uint32_t)PE_Main_EnterCredits();
        break;
    default:
        /* One CD wait class with the stop already requested (or requested by
         * the hook while it retries), the escape armed directly. */
        if (!PE_Cd_AttachDevice(&g_cxa_at)) { r->setup = 0; return; }
        if (setjmp(escape) == 0) {
            PE_Port_EscapeArm(&escape);
            if (scenario == CXA_STOP_LANE_WAIT) {
                PE_StoreU8(0x801E0000u, 0xC8u);
                (void)func_8007EE84(14u, 0x801E0000u, 0u, 0xFFFFFFFFu);
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                (void)ovl_0457__func_80125744();
            } else if (scenario == CXA_STOP_TEARDOWN) {
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                (void)ovl_0457__func_801255A4();
            } else {
                g_cxa_stop_at = 5u;             /* the file is absent: it retries */
                (void)ovl_0457__func_8012562C();
            }
            r->escaped = 0;
        } else {
            r->escaped = 1;
        }
        PE_Port_EscapeDisarm();
        PE_Cd_DetachDevice(&g_cxa_at);
        break;
    }
    r->attached_after = (uint32_t)PE_CdReg_DeviceEnabled();
    r->stopped = (uint32_t)PE_Port_ShouldStop();
    r->reason = (uint32_t)PE_Port_GetStopReason();
    r->slots_ok = PE_LoadU32(0x800945E8u + 8u) == irq2 && PE_Callback_GetSlot(0u) == vsync0 &&
                  PE_IRQ_GetMask() == mask && PE_LoadU16(0x80094614u) == registered;
    r->ce0 = PE_LoadU32(0x80172CE0u);
    r->ce4 = PE_LoadU32(0x80172CE4u);
    r->iterations = PE_LoadU32(0x80172CD4u) - 420u;
    r->xa = (uint32_t)PE_Spu_XaSectors();
    r->lane = PE_LoadU32(0x8009B574u);
    r->queue = PE_LoadU32(0x800A3608u);
    r->d1a0 = PE_LoadU32(0x8009D1A0u);
    r->press_iter = g_cxa_host_press_iter ? g_cxa_host_press_iter : g_cxa_press_iter;
    r->pressed_at = g_cxa_pressed_at;
    r->vblanks = g_cxa_vblanks;
    r->setfilter_seen = g_cxa_setfilter;
    r->leadout_seen = g_cxa_leadout_seen;
    r->int4_consumed = g_cxa_int4_consumed;
    HostFB_GetState(&vs, &ds, &pr, &mk);
    r->presented = (uint32_t)pr;
}

/* 1 when the child ran to completion inside the bound and wrote its result. */
static int CxaFork(int scenario, CxaRun *r, unsigned bound)
{
    int fds[2], status = 0;
    pid_t pid;
    ssize_t got;
    memset(r, 0, sizeof *r);
    if (pipe(fds) != 0) return 0;
    fflush(NULL);
    pid = fork();
    if (pid == 0) {
        CxaRun mine;
        close(fds[0]);
        alarm(bound);
        CxaChild(scenario, &mine);
        _exit(write(fds[1], &mine, sizeof mine) == (ssize_t)sizeof mine ? 0 : 3);
    }
    close(fds[1]);
    if (pid < 0) { close(fds[0]); return 0; }
    got = read(fds[0], r, sizeof *r);
    close(fds[0]);
    if (waitpid(pid, &status, 0) != pid) return 0;
    return !WIFSIGNALED(status) && WIFEXITED(status) && WEXITSTATUS(status) == 0 &&
           got == (ssize_t)sizeof *r;
}

/* Mode 0 (the Disc 2 ending) through main's own entry, PE_Main_EnterCredits:
 * the file check, the attach, the whole loop, the detach.  It must leave
 * through D_80172CE0 = D_80172CE4 (set once a GetlocP position passes the
 * file end) after func_80123950 reports the scroll done, with the teardown
 * leaving the drive idle, both hooks and masks restored and bit 16 clear. */
static void test_credits_main_loop_runs_to_exit(void)
{
    CxaRun r;
    TEST_RETAIL_DISC1("credits_main_loop_runs_to_exit");
    ASSERT(CxaFork(CXA_MODE0, &r, 240u), "the credits loop did not exit within 240 s");
    fprintf(stderr, "[credits] mode 0: %u iterations, %u presents, %u XA sectors decoded\n",
            r.iterations, r.presented, r.xa);
    ASSERT(r.setup && r.ret == 1u && !r.stopped, "entry, attach and the loop ran without a stop");
    ASSERT(r.ce0 == 1u && r.ce4 == 1u, "the loop left through D_80172CE0 = D_80172CE4 = 1");
    ASSERT(r.iterations >= 13000u && r.presented >= r.iterations, "the whole scroll ran");
    ASSERT(!r.attached_after && r.slots_ok, "detached; IRQ 2, VSync 0 and both masks restored");
    ASSERT(r.lane == 1u && r.queue == 0u, "the teardown left the drive idle and the queue empty");
    ASSERT(!(r.d1a0 & 0x10000u), "bit 16 of D_8009D1A0 clear on exit");
    ASSERT(r.xa >= CXA_FILE_SECTORS / 16u, "the XA channel was decoded");
    PASS();
}

/* The production order: main's arm runs func_8006F044 (stop all sound, then
 * reload ovl_039F and ovl_0457 through host reads) and then
 * PE_Main_EnterCredits.  Whatever func_8006F044 leaves in libds must still
 * let the attach accept (lane 1, queue empty, D_8009B554 = 1), or production
 * would stop on credits_cd_attach and never run the credits. */
static void test_credits_after_overlay_reload(void)
{
    CxaRun r;
    TEST_RETAIL_DISC1("credits_after_overlay_reload");
    ASSERT(CxaFork(CXA_AFTER_F044, &r, 240u), "func_8006F044 then the credits did not finish");
    ASSERT(r.setup, "func_8006F044 ran on Disc 1 without a stop");
    ASSERT(r.ret == 1u && !r.stopped && r.ce0 == 1u && r.iterations >= 13000u,
           "PE_Main_EnterCredits attached and ran the whole loop");
    ASSERT(!r.attached_after && r.slots_ok && r.lane == 1u && r.queue == 0u,
           "detached, hooks restored, drive idle");
    PASS();
}

/* Mode 0x20000 (bit 17): channel 1 (func_80125894(1)), the fade states 1-3 and
 * the state-2 wait for D_80172CE4 and a held exit button.  No button is held
 * until the hook presses the one the boot config maps to an exit bit, at
 * iteration 14000, past the scroll; the loop must wait for it and then leave. */
static void test_credits_mode_20000_waits_for_press(void)
{
    CxaRun r;
    TEST_RETAIL_DISC1("credits_mode_20000_waits_for_press");
    ASSERT(CxaFork(CXA_MODE20000, &r, 240u), "the credits loop did not exit within 240 s");
    fprintf(stderr, "[credits] mode 0x20000: %u iterations, pressed at %u, %u XA sectors decoded\n",
            r.iterations, r.pressed_at, r.xa);
    ASSERT(r.setup && r.ret == 1u && !r.stopped, "entry and the loop ran without a stop");
    ASSERT(r.pressed_at >= r.press_iter && r.pressed_at < r.press_iter + 2u &&
           r.iterations > r.pressed_at + 60u && r.iterations < r.pressed_at + 80u,
           "state 2 waited for the press, then the 64-step fade-out ran");
    ASSERT(r.ce0 == 1u && r.ce4 == 1u, "left through D_80172CE0 with the stream ended");
    ASSERT(!r.attached_after && r.slots_ok && r.lane == 1u && r.queue == 0u,
           "detached, hooks restored, drive idle");
    ASSERT(r.xa >= CXA_FILE_SECTORS / 16u, "the channel-1 XA was decoded");
    PASS();
}

/* The same EX-ending wait, with the press coming from a host pad source (the
 * window or a route script) instead of a direct store into the pad buffer.
 * Retail's VSync interrupt refills that buffer every VBlank; the credits loop
 * has no host fill point of its own, so without the credits pad refill in
 * HostFB_VSync the press never reaches func_8003EB04 and state 2 waits
 * forever (the EX ending soft-locks). */
static void test_credits_mode_20000_host_pad_press(void)
{
    CxaRun r;
    TEST_RETAIL_DISC1("credits_mode_20000_host_pad_press");
    ASSERT(CxaFork(CXA_MODE20000_HOSTPAD, &r, 120u),
           "the credits loop did not exit within 120 s (the host pad never reached the loop)");
    fprintf(stderr, "[credits] mode 0x20000, host pad: %u iterations, pressed at %u\n",
            r.iterations, r.pressed_at);
    ASSERT(r.setup && r.ret == 1u && !r.stopped, "entry and the loop ran without a stop");
    ASSERT(r.pressed_at >= r.press_iter && r.pressed_at < r.press_iter + 2u &&
           r.iterations > r.pressed_at + 60u && r.iterations < r.pressed_at + 80u,
           "state 2 waited for the host press, then the 64-step fade-out ran");
    ASSERT(r.ce0 == 1u && r.ce4 == 1u, "left through D_80172CE0 with the stream ended");
    ASSERT(!r.attached_after && r.slots_ok && r.lane == 1u && r.queue == 0u,
           "detached, hooks restored, drive idle");
    PASS();
}

/* A short validated disc reaches lead-out long before the EX wait receives
 * its host pad press. The production callback/teardown must still complete. */
static void test_credits_mode_20000_leadout_host_pad(void)
{
    CxaRun r;
    TEST_RETAIL_DISC1("credits_mode_20000_leadout_host_pad");
    ASSERT(CxaFork(CXA_MODE20000_LEADOUT, &r, 240u), "EX credits did not complete");
    fprintf(stderr, "[credits] lead-out host pad: %u iterations, pressed %u, stop %u\n",
            r.iterations, r.pressed_at, r.reason);
    ASSERT(r.setup && r.ret==1u && !r.stopped && r.leadout_seen && r.int4_consumed, "EX credits consumes INT4 and survives observed lead-out");
    ASSERT(r.pressed_at>=14000u && r.iterations>r.pressed_at+60u &&
           r.iterations<r.pressed_at+80u, "EX waited for host pad then faded out");
    ASSERT(r.ce0==1u && r.ce4==1u && !r.attached_after && r.slots_ok &&
           r.lane==1u && r.queue==0u, "stream completed and teardown restored production state");
    PASS();
}

/* The pad-code mode (0x12000) after production's func_8006F044: the two host
 * reloads leave the CdlComplete of each read's closing CdlPause in the libds
 * ring, so DsSync(0) reads 2 as on hardware.  Then the second op table runs
 * the scroll; a press of the button the boot config maps to D_8009D1F4 bit 26
 * (iteration 2000) runs func_80125894(0), whose wait for DsSync(0) == 2
 * returns and whose CdlSetfilter reaches the attached drive.  Bit 16 makes
 * state 4 leave at once after the scroll. */
static void test_credits_pad_code_mode(void)
{
    CxaRun r;
    TEST_RETAIL_DISC1("credits_pad_code_mode");
    ASSERT(CxaFork(CXA_PAD_CODE, &r, 240u), "the pad-code credits did not finish");
    fprintf(stderr, "[credits] pad code: DsSync(0)=%u, %u iterations, pressed at %u, Setfilter %u\n",
            r.dssync0, r.iterations, r.pressed_at, r.setfilter_seen);
    ASSERT(r.setup && r.dssync0 == 2u, "after the host reloads DsSync(0) reads CdlComplete");
    ASSERT(r.ret == 1u && !r.stopped && r.ce0 == 1u && r.iterations >= 13000u,
           "the second op table ran the whole scroll and left");
    ASSERT(r.pressed_at >= r.press_iter && r.setfilter_seen,
           "the channel switch reached the drive as CdlSetfilter");
    ASSERT(!r.attached_after && r.slots_ok && r.lane == 1u && r.queue == 0u,
           "detached, hooks restored, drive idle");
    PASS();
}

/* A stop inside the credits must come back to the entry, not spin: once from
 * the loop body (a boundary at VBlank 600) and once as a host quit through the
 * quit poll.  Each must return 0 with the device detached and the hooks and
 * masks as before the attach. */
static void test_credits_escape_from_loop(void)
{
    CxaRun r;
    TEST_RETAIL_DISC1("credits_escape_from_loop");
    ASSERT(CxaFork(CXA_STOP_IN_LOOP, &r, 120u), "a stop in the loop did not come back");
    ASSERT(r.setup && r.ret == 0u && r.stopped && r.reason == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "boundary stop returns 0 from PE_Main_EnterCredits");
    ASSERT(r.iterations < 13000u && !r.attached_after && r.slots_ok,
           "left mid-scroll; detached and restored");
    ASSERT(CxaFork(CXA_QUIT_IN_LOOP, &r, 120u), "a host quit in the loop did not come back");
    ASSERT(r.setup && r.ret == 0u && r.stopped && r.reason == PE_PORT_STOP_HOST_QUIT,
           "host quit returns 0 from PE_Main_EnterCredits");
    ASSERT(!r.attached_after && r.slots_ok, "detached and restored after the quit");
    PASS();
}

/* The CD wait classes in the credits helpers, each with a stop: the empty-body
 * lane wait in func_80125744 (a queued Setmode keeps DsQueueLen nonzero), the
 * func_801255A4 teardown's `while (func_80080DC4(9, 0, 0) == 0)`, and
 * func_8012562C's CdSearchFile retry on a disc without the file. */
static void test_credits_escape_from_cd_waits(void)
{
    static const struct { int scenario; const char *what; } cases[] = {
        { CXA_STOP_LANE_WAIT, "func_80125744 lane wait" },
        { CXA_STOP_TEARDOWN,  "func_801255A4 Pause wait" },
        { CXA_STOP_SEARCH,    "func_8012562C search retry" },
    };
    CxaRun r;
    unsigned k;
    TEST_RETAIL_DISC1("credits_escape_from_cd_waits");
    for (k = 0; k < 3u; k++) {
        if (!CxaFork(cases[k].scenario, &r, 60u)) {
            fprintf(stderr, "[credits] no escape: %s\n", cases[k].what);
            FAIL("a CD wait under a stop reached no escape point");
            return;
        }
        ASSERT(r.setup && r.escaped && r.stopped, "the escape was taken");
        ASSERT(!r.attached_after && r.slots_ok, "detached and restored after the escape");
    }
    PASS();
}

static void test_credits_native_all(void)
{
    test_credits_libpress_replacements();
    test_credits_libpress_retail_oracle();
    test_credits_registry_dispatch();
    test_credits_vlc_bounded_resume_oracle();
    test_credits_entry_gates();
    test_credits_xa_commands_never_complete_without_device();
    test_credits_xa_device_path();
    test_credits_attach_from_host_boot();
    test_credits_main_loop_runs_to_exit();
    test_credits_after_overlay_reload();
    test_credits_mode_20000_waits_for_press();
    test_credits_mode_20000_host_pad_press();
    test_credits_mode_20000_leadout_host_pad();
    test_credits_pad_code_mode();
    test_credits_escape_from_loop();
    test_credits_escape_from_cd_waits();
}
#endif

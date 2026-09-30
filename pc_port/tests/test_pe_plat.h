/*
 * Step P0 — pe_plat interface adapters (docs/ARCHITECTURE-PORT.md).
 *
 * One small test per adapter over its in-house backend.  The adapters are
 * compiled into this test TU directly (plat_mods/plat_cheats/plat_timing/
 * plat_audio are also in PLATFORM_SRCS; the test TU's definitions satisfy
 * the archive references, so those members are not pulled twice).  The host-bound
 * sinks (plat_audio_host.c, plat_input_host_pad.c) need host_audio.c /
 * host_pad.c and are not linked here.  All data is synthetic.
 */
#include "platform/plat_renderer.c"
#include "platform/plat_audio.c"
#include "platform/plat_input.c"
#include "platform/plat_storage.c"
#include "platform/plat_timing.c"
#include "platform/plat_mods.c"

static void test_PLAT_renderer_over_pe_gpu(void)
{
    static const uint16_t img[8] = {1, 2, 3, 4, 0x7FFF, 0x8000, 0x1234, 0x0421};
    uint16_t back[8];
    uint16_t px = 0;
    PePlatRect r = {64, 32, 4, 2}, bad = {1020, 0, 8, 1}, fill = {16, 100, 16, 4};
    PePlatDrawEnv de = {{0, 0, 1024, 512}, 0, 0};
    PePlatDispEnv disp = {0, 256};
    PePlatPrimitive tile, tri;
    PePlatRendererCaps caps;
    PeGpuState st;
    uint64_t polys;
    int i;

    TEST("PLAT_renderer_over_pe_gpu");
    pe_plat_renderer_init();
    pe_plat_renderer_caps(&caps);
    ASSERT(caps.internal_scale == 1 && caps.backend_name, "renderer caps");

    ASSERT(pe_plat_renderer_load_image(&r, img), "load_image accepted");
    ASSERT(pe_plat_renderer_store_image(&r, back), "store_image accepted");
    for (i = 0; i < 8; i++) ASSERT(back[i] == img[i], "VRAM round trip");
    ASSERT(!pe_plat_renderer_load_image(&bad, img), "out-of-VRAM rect rejected");

    ASSERT(pe_plat_renderer_move_image(&r, 128, 40), "move_image accepted");
    ASSERT(pe_plat_renderer_read_pixel(131, 41, &px) && px == 0x0421, "move_image copied");
    PE_GPU_GetState(&st);
    ASSERT(st.gp1_dma_direction == 0u, "move_image restores the transfer direction");

    ASSERT(pe_plat_renderer_clear_image(&fill, (PePlatColor){255, 0, 0}), "clear accepted");
    ASSERT(pe_plat_renderer_read_pixel(31, 103, &px) && px == 0x001F, "clear fills 15-bit red");

    ASSERT(pe_plat_renderer_put_draw_env(&de), "draw env accepted");
    ASSERT(pe_plat_renderer_put_disp_env(&disp), "disp env accepted");
    PE_GPU_GetState(&st);
    ASSERT(st.drawing_area_bottom_right == (0xE4000000u | (511u << 10) | 1023u), "draw area encoded");
    ASSERT(st.display_start == (256u << 10), "display start encoded");

    memset(&tile, 0, sizeof(tile));
    tile.kind = PE_PLAT_PRIM_TILE;
    tile.v[0].x = 200; tile.v[0].y = 200; tile.w = 4; tile.h = 4;
    tile.c[0].g = 255;
    ASSERT(pe_plat_renderer_draw(&tile), "tile drawn");
    ASSERT(pe_plat_renderer_read_pixel(201, 201, &px) && px == 0x03E0, "tile pixel is 15-bit green");

    memset(&tri, 0, sizeof(tri));
    tri.kind = PE_PLAT_PRIM_TRIANGLE;
    tri.flags = PE_PLAT_PRIM_GOURAUD;
    tri.v[0].x = 300; tri.v[0].y = 300; tri.v[1].x = 340; tri.v[1].y = 300;
    tri.v[2].x = 300; tri.v[2].y = 340;
    tri.c[0].r = tri.c[1].g = tri.c[2].b = 255;
    PE_GPU_GetState(&st); polys = st.polygon_count;
    ASSERT(pe_plat_renderer_draw(&tri), "gouraud triangle drawn");
    PE_GPU_GetState(&st);
    ASSERT(st.polygon_count == polys + 1u && st.gp0_state == PE_GPU_GP0_IDLE,
           "triangle is one complete packet");
    tri.kind = PE_PLAT_PRIM_LINE; tri.flags = PE_PLAT_PRIM_TEXTURED;
    ASSERT(!pe_plat_renderer_draw(&tri), "textured line rejected");
    ASSERT(pe_plat_renderer_draw_sync() == 0, "draw_sync idle");

    pe_plat_renderer_init();
    PASS();
}

typedef struct { int calls, frames, ff, closed; } PlatTestSink;
static void plat_test_submit(void *ctx, const int16_t *s, int n)
{ PlatTestSink *k = (PlatTestSink *)ctx; (void)s; k->calls++; k->frames += n; }
static void plat_test_ff(void *ctx, int on) { ((PlatTestSink *)ctx)->ff = on; }
static void plat_test_close(void *ctx) { ((PlatTestSink *)ctx)->closed++; }

static void test_PLAT_audio_over_pe_spu(void)
{
    PlatTestSink sink = {0, 0, 0, 0};
    PePlatAudioOutput out = {&sink, plat_test_submit, plat_test_ff, plat_test_close, "test"};
    PePlatAudioStats a, b;
    uint16_t regs[4];
    uint8_t ram_save[4], wr[4] = {0x11, 0x22, 0x33, 0x44}, rd[4];
    int i;

    TEST("PLAT_audio_over_pe_spu");
    for (i = 0; i < 4; i++) regs[i] = PE_SpuRegister_LoadU16(0x188u + 2u * (uint32_t)i);
    ASSERT(pe_plat_audio_transfer_read(0x7FFFEu, ram_save, 4), "save sample RAM");
    PE_Spu_SetActive(0);
    ASSERT(pe_plat_audio_init() && PE_Spu_Active(), "init activates the mixer");
    pe_plat_audio_reset();
    ASSERT(pe_plat_audio_sample_rate() == 44100, "44.1 kHz");

    pe_plat_audio_stats(&a);
    pe_plat_audio_voice_key_on(0x000001u);
    pe_plat_audio_voice_key_on(0x010000u);     /* voice 16: high register */
    pe_plat_audio_voice_key_off(0x000001u);
    pe_plat_audio_stats(&b);
    ASSERT(b.key_on_events == a.key_on_events + 2u, "key-on reaches the synthesizer");
    ASSERT(b.key_off_events == a.key_off_events + 1u, "key-off reaches the synthesizer");

    ASSERT(pe_plat_audio_transfer_write(0x7FFFEu, wr, 4), "sample upload");
    ASSERT(pe_plat_audio_transfer_read(0x7FFFEu, rd, 4) && !memcmp(rd, wr, 4), "upload wraps and reads back");
    ASSERT(PE_SpuRam_Data()[0] == 0x33 && PE_SpuRam_Data()[1] == 0x44, "wrap lands at address 0");

    pe_plat_audio_voice_key_off(0xFFFFFFu);
    pe_plat_audio_set_output(&out);
    pe_plat_audio_stats(&a);
    pe_plat_audio_tick(600);
    pe_plat_audio_stats(&b);
    ASSERT(sink.frames == 600 && sink.calls == 2, "tick feeds the output in chunks");
    ASSERT(b.mixed_frames == a.mixed_frames + 600u, "tick mixes emulated frames");
    pe_plat_audio_set_fast_forward(1);
    ASSERT(sink.ff == 1, "fast-forward forwarded to the output");

    pe_plat_audio_shutdown();
    ASSERT(sink.closed == 1 && !PE_Spu_Active(), "shutdown closes output and restores state");
    pe_plat_audio_transfer_write(0x7FFFEu, ram_save, 4);
    for (i = 0; i < 4; i++) PE_SpuRegister_StoreU16(0x188u + 2u * (uint32_t)i, regs[i]);
    PE_Spu_Reset();
    PASS();
}

typedef struct { int polls, closed; } PlatTestPad;
static void plat_test_pad_poll(void *ctx) { ((PlatTestPad *)ctx)->polls++; }
static uint16_t plat_test_pad_held(void *ctx, int port)
{ (void)ctx; return port == 0 ? (uint16_t)(PE_PLAT_BTN_CROSS | PE_PLAT_BTN_UP) : 0u; }
static uint8_t plat_test_pad_hot(void *ctx) { (void)ctx; return PE_PLAT_HOTKEY_FAST_FORWARD_HOLD; }
static void plat_test_pad_close(void *ctx) { ((PlatTestPad *)ctx)->closed++; }

static void test_PLAT_input_dispatch(void)
{
    PlatTestPad pad = {0, 0};
    PePlatInputBackend be = {&pad, plat_test_pad_poll, plat_test_pad_held,
                             plat_test_pad_hot, plat_test_pad_close, "test"};
    PePlatInputState s;

    TEST("PLAT_input_dispatch");
    pe_plat_input_init();
    pe_plat_input_set_backend(NULL);
    pe_plat_input_state(0, &s);
    ASSERT(!s.connected && !s.buttons, "no backend: nothing held");

    pe_plat_input_set_backend(&be);
    pe_plat_input_poll();
    ASSERT(pad.polls == 1, "poll reaches the backend");
    pe_plat_input_state(0, &s);
    ASSERT(s.connected && s.buttons == (PE_PLAT_BTN_CROSS | PE_PLAT_BTN_UP), "physical buttons");
    ASSERT(s.hotkeys == PE_PLAT_HOTKEY_FAST_FORWARD_HOLD, "hotkeys kept apart from buttons");

    ASSERT(pe_plat_input_bind(PE_PLAT_BTN_CROSS, PE_PLAT_BTN_CIRCLE), "rebind accepted");
    ASSERT(!pe_plat_input_bind(PE_PLAT_BTN_CROSS | PE_PLAT_BTN_UP, PE_PLAT_BTN_CIRCLE), "multi-bit bind rejected");
    pe_plat_input_inject(0, PE_PLAT_BTN_START);
    pe_plat_input_state(0, &s);
    ASSERT(s.buttons == (PE_PLAT_BTN_CIRCLE | PE_PLAT_BTN_UP | PE_PLAT_BTN_START), "rebinding + injection");
    ASSERT(pe_plat_input_pad_word(PE_PLAT_BTN_START) == 0xFFF7u, "active-low pad word");
    pe_plat_input_state(1, &s);
    ASSERT(!s.buttons && !s.hotkeys, "port 2 idle");

    pe_plat_input_shutdown();
    ASSERT(pad.closed == 1, "shutdown closes the backend");
    pe_plat_input_init();
    PASS();
}

static int plat_test_dirrec(uint8_t *p, uint32_t extent, uint32_t size, uint8_t flags,
                            const char *id, int id_len)
{
    int len = (33 + id_len + 1) & ~1, k;
    memset(p, 0, (size_t)len);
    p[0] = (uint8_t)len;
    for (k = 0; k < 4; k++) {
        p[2 + k] = (uint8_t)(extent >> (8 * k)); p[9 - k] = (uint8_t)(extent >> (8 * k));
        p[10 + k] = (uint8_t)(size >> (8 * k)); p[17 - k] = (uint8_t)(size >> (8 * k));
    }
    p[25] = flags;
    p[32] = (uint8_t)id_len;
    memcpy(p + 33, id, (size_t)id_len);
    return len;
}

static void test_PLAT_storage_over_pe_disc(void)
{
    enum { SECTORS = 26 };
    uint8_t *img, *sec, raw[PE_PLAT_SECTOR_RAW], buf[8];
    PE_Disc *before = PE_Disc_GetActive();
    PePlatFileInfo fi;
    char vol[40];
    uint32_t i;
    int n;
#define PLAT_USER(l) (img + (size_t)(l) * PE_DISC_RAW_SECTOR + PE_DISC_USER_OFFSET)

    TEST("PLAT_storage_over_pe_disc");
    img = calloc(SECTORS, PE_DISC_RAW_SECTOR);
    sec = malloc(2 * PE_PLAT_SECTOR_USER);
    if (!img || !sec) { free(img); free(sec); FAIL("alloc"); return; }
    for (i = 0; i < SECTORS; i++) {
        uint8_t *r = img + (size_t)i * PE_DISC_RAW_SECTOR;
        memset(r + 1, 0xFF, 10);
        r[15] = 0x02;
    }
    PLAT_USER(16)[0] = 1; memcpy(PLAT_USER(16) + 1, "CD001", 5); PLAT_USER(16)[6] = 1;
    memcpy(PLAT_USER(16) + 40, "PLAT_TEST                       ", 32);
    plat_test_dirrec(PLAT_USER(16) + 156, 20, 2048, 0x02, "\0", 1);
    n = plat_test_dirrec(PLAT_USER(20), 20, 2048, 0x02, "\0", 1);
    n += plat_test_dirrec(PLAT_USER(20) + n, 20, 2048, 0x02, "\1", 1);
    n += plat_test_dirrec(PLAT_USER(20) + n, 21, 16, 0x00, "SLUS_006.62;1", 13);
    plat_test_dirrec(PLAT_USER(20) + n, 22, 3000, 0x00, "DATA.BIN;1", 10);
    for (i = 0; i < 3000u; i++) PLAT_USER(22 + i / 2048u)[i % 2048u] = (uint8_t)(i * 13u + 1u);

    ASSERT(pe_plat_storage_disc_open_memory(img, (size_t)SECTORS * PE_DISC_RAW_SECTOR), "fixture accepted");
    ASSERT(pe_plat_storage_disc_present() && pe_plat_storage_sector_count() == SECTORS, "geometry");
    ASSERT(pe_plat_storage_disc_number() == 1, "disc 1 identified");
    ASSERT(pe_plat_storage_volume_id(vol, sizeof(vol)) && !strcmp(vol, "PLAT_TEST"), "volume id");
    ASSERT(pe_plat_storage_find_file("\\DATA.BIN;1", &fi) && fi.lba == 22 && fi.size == 3000, "file lookup");
    ASSERT(!pe_plat_storage_find_file("\\NOPE.BIN;1", &fi), "missing file");
    ASSERT(pe_plat_storage_read_sectors(22, 2, sec) && sec[2047] == (uint8_t)(2047u * 13u + 1u) &&
           sec[2048] == (uint8_t)(2048u * 13u + 1u), "multi-sector read");
    ASSERT(!pe_plat_storage_read_sectors(SECTORS - 1, 2, sec), "read past end rejected");
    ASSERT(pe_plat_storage_read(22, 2046, buf, 4) && buf[2] == (uint8_t)(2048u * 13u + 1u), "byte read crosses sectors");
    ASSERT(pe_plat_storage_read_raw_sector(16, raw) && raw[15] == 0x02, "raw sector");
    pe_plat_storage_disc_close();
    ASSERT(PE_Disc_GetActive() == before, "close restores the previous disc");
#undef PLAT_USER
    free(img); free(sec);
    PASS();
}

static void test_PLAT_timing_adapter(void)
{
    uint64_t d1, d2;
    TEST("PLAT_timing_adapter");
    pe_plat_timing_init();
    d1 = pe_plat_timing_next_frame_deadline(1000u);
    d2 = pe_plat_timing_next_frame_deadline(1000u);
    ASSERT(d1 == 1000u + 16683333u && d2 == d1 + 16683333u, "NTSC frame deadlines");
    ASSERT(pe_plat_timing_frame_rate_millihz() == 59940u, "frame rate");
    pe_plat_timing_set_fast_forward(1);
    ASSERT(pe_plat_timing_fast_forward() == 1, "fast-forward flag");
    pe_plat_timing_set_fast_forward(0);
    ASSERT(pe_plat_timing_vblank_count() == PE_GPU_VSyncQuery(), "vblank count");

    ASSERT(!pe_plat_timing_timeout_start(0, 0), "zero timeout rejected");
    ASSERT(pe_plat_timing_timeout_start(100, 0), "timeout armed");
    pe_plat_timing_advance_cycles(99);
    ASSERT(!pe_plat_timing_timeout_elapsed(), "not yet elapsed");
    pe_plat_timing_advance_cycles(1);
    ASSERT(pe_plat_timing_timeout_elapsed() && pe_plat_timing_timeout_elapsed(), "elapsed and sticky");
    ASSERT(pe_plat_timing_timeout_start(10, 1), "div8 timeout armed");
    pe_plat_timing_advance_cycles(79);
    ASSERT(!pe_plat_timing_timeout_elapsed(), "div8 not yet");
    pe_plat_timing_advance_cycles(1);
    ASSERT(pe_plat_timing_timeout_elapsed(), "div8 elapsed at 80 cycles");
    PE_Rcnt2_Reset();
    pe_plat_timing_init();
    PASS();
}

static int plat_test_log[8], plat_test_log_n;
static void plat_test_hook_a(const PePlatModEventData *e, void *u)
{ (void)u; if (plat_test_log_n < 8) plat_test_log[plat_test_log_n++] = 100 + (int)e->arg0; }
static void plat_test_hook_b(const PePlatModEventData *e, void *u)
{ if (plat_test_log_n < 8) plat_test_log[plat_test_log_n++] = 200 + (int)e->arg0 + *(int *)u; }

static void test_PLAT_mods_hooks(void)
{
    PePlatModManifest m;
    int extra = 1, ha, hb;
    TEST("PLAT_mods_hooks");
    pe_plat_mods_reset();
    ASSERT(pe_plat_mods_emit(PE_PLAT_EVENT_FRAME_TICK, 0, 0) == 0, "no hooks: emit is a no-op");
    ha = pe_plat_mods_subscribe(PE_PLAT_EVENT_ROOM_ENTER, plat_test_hook_a, NULL);
    hb = pe_plat_mods_subscribe(PE_PLAT_EVENT_ROOM_ENTER, plat_test_hook_b, &extra);
    ASSERT(ha > 0 && hb > ha, "handles");
    ASSERT(!pe_plat_mods_subscribe(PE_PLAT_EVENT_COUNT, plat_test_hook_a, NULL), "bad event rejected");
    ASSERT(pe_plat_mods_hook_count(PE_PLAT_EVENT_ROOM_ENTER) == 2, "hook count");
    plat_test_log_n = 0;
    ASSERT(pe_plat_mods_emit(PE_PLAT_EVENT_ROOM_ENTER, 5, 0) == 2, "both hooks ran");
    ASSERT(plat_test_log_n == 2 && plat_test_log[0] == 105 && plat_test_log[1] == 206, "subscription order and args");
    ASSERT(pe_plat_mods_unsubscribe(ha) && !pe_plat_mods_unsubscribe(ha), "unsubscribe once");
    ASSERT(pe_plat_mods_emit(PE_PLAT_EVENT_ROOM_ENTER, 1, 0) == 1, "one hook left");
    ASSERT(!strcmp(pe_plat_mods_event_name(PE_PLAT_EVENT_ROOM_ENTER), "room_enter"), "event name");

    ASSERT(pe_plat_mods_parse_manifest("# demo\nname = Test Mod \nversion=1.2\n load_order = 7\nabi = 1\n", &m),
           "manifest parsed");
    ASSERT(!strcmp(m.name, "Test Mod") && !strcmp(m.version, "1.2") && m.load_order == 7, "manifest fields");
    ASSERT(!pe_plat_mods_parse_manifest("name = X\nabi = 99\n", &m), "incompatible ABI rejected");
    ASSERT(!pe_plat_mods_parse_manifest("version = 1\n", &m), "nameless manifest rejected");
    pe_plat_mods_reset();
    PASS();
}

static void test_PLAT_all(void)
{
    test_PLAT_renderer_over_pe_gpu();
    test_PLAT_audio_over_pe_spu();
    test_PLAT_input_dispatch();
    test_PLAT_storage_over_pe_disc();
    test_PLAT_timing_adapter();
    test_PLAT_mods_hooks();
}

/* Opt-in presenter enhancements: DISPLAY FILTER (scanlines / CRT mask) and
 * WINDOW FIT (keep aspect / integer scale / stretch).
 * pe_plat/enhance_host.h, platform/plat_enhance.c, platform/host_filter_fit.h,
 * platform/host_filter_crt.h.
 *
 * The presenter (host_window.c) needs a display, so what runs here is every
 * pure piece it calls, over synthetic frames, with golden FNV-1a hashes of the
 * BGRA window buffers.  The ctest `enh-window` runs the real HostWindow_Blit
 * against a fake X sink.  The hashes pin the integer arithmetic: they are the
 * same on every compiler. */
#include "pe_plat/enhance_host.h"
#include "host_filter_crt.h"
#include <unistd.h>

#define ENHX_FW 320
#define ENHX_FH 240

static uint64_t enhx_fnv(const uint8_t *p, size_t n)
{
    uint64_t h = 1469598103934665603ull;
    while (n--) { h ^= *p++; h *= 1099511628211ull; }
    return h;
}

/* Colour bars over a ramp over a checkerboard with an orange rule. */
static void enhx_frame(uint8_t *rgb, int w, int h)
{
    static const uint8_t bars[8][3] = {{255, 255, 255}, {255, 255, 0}, {0, 255, 255}, {0, 255, 0},
                                       {255, 0, 255}, {255, 0, 0}, {0, 0, 255}, {32, 32, 32}};
    for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) {
        uint8_t *p = rgb + ((size_t)y * (size_t)w + (size_t)x) * 3u;
        if (y < h / 2) { int b = x * 8 / w; p[0] = bars[b][0]; p[1] = bars[b][1]; p[2] = bars[b][2]; }
        else if (y < h * 3 / 4) { p[0] = p[1] = p[2] = (uint8_t)(x * 255 / (w - 1)); }
        else {
            p[0] = p[1] = p[2] = (((x / 4 + y / 4) & 1) ? 220 : 40);
            if ((x % 40) < 2) { p[0] = 255; p[1] = 128; p[2] = 0; }
        }
    }
}

/* The window buffer the retail path produces: HostWindow_ScaleBGRA. */
static uint8_t *enhx_retail_window(const uint8_t *rgb, int fb_w, int fb_h, int win_w, int win_h)
{
    uint8_t *out = (uint8_t *)malloc((size_t)win_w * (size_t)win_h * 4u);
    if (out) HostWindow_ScaleBGRA(out, win_w, win_h, rgb, fb_w, fb_h);
    return out;
}

/* One golden check: prints the value it computed when it differs. */
#ifndef ENHX_HASH
#define ENHX_HASH(buf, bytes, want, what) do { \
    uint64_t got_ = enhx_fnv((buf), (bytes)); \
    if (got_ != (want)) { \
        static char msg_[160]; \
        snprintf(msg_, sizeof msg_, "%s: got %016llx want %016llx", (what), (unsigned long long)got_, \
                 (unsigned long long)(want)); \
        FAIL(msg_); return; \
    } \
} while (0)
#endif

static void test_ENHX_display_registry(void)
{
    char label[64];
    int f, t;
    TEST("ENHX_display_registry");
    enh_isolate();
    unsetenv("PE_DISPLAY_FILTER"); unsetenv("PE_DISPLAY_FIT"); pe_enh_reset();
    f = pe_enh_option_index("display_filter"); t = pe_enh_option_index("display_fit");
    ASSERT(f >= 0 && t >= 0, "display_filter and display_fit are registered");
    {
        const PeEnhOption *of = pe_enh_option(f), *ot = pe_enh_option(t);
        ASSERT(of->category == PE_ENH_CAT_GRAPHICS && ot->category == PE_ENH_CAT_GRAPHICS &&
               of->format == PE_ENH_FMT_NAMED && ot->format == PE_ENH_FMT_NAMED &&
               of->nvalues == 3 && ot->nvalues == 3 && of->values[0] == 0 && ot->values[0] == 0 &&
               of->names && ot->names, "named graphics rows, three values, retail first");
        ASSERT(strcmp(of->env, "PE_DISPLAY_FILTER") == 0 && strcmp(ot->env, "PE_DISPLAY_FIT") == 0 &&
               of->credit && ot->credit && strcmp(of->credit, "the owner") == 0 &&
               strcmp(ot->credit, "the owner") == 0, "env overrides and credit");
    }
    ASSERT(pe_enh_category_option(PE_ENH_CAT_GRAPHICS, 5) == pe_enh_option_index("subpixel_geometry") &&
           pe_enh_category_option(PE_ENH_CAT_GRAPHICS, 6) == f && pe_enh_category_option(PE_ENH_CAT_GRAPHICS, 7) == t &&
           pe_enh_category_option(PE_ENH_CAT_GRAPHICS, 9) == -1, "rows 7 and 8 of the Graphics page, after the existing six");
    ASSERT((int)PE_ENH_FILTER_OFF == (int)HOST_CRT_OFF && (int)PE_ENH_FILTER_SCANLINES == (int)HOST_CRT_SCANLINES &&
           (int)PE_ENH_FILTER_CRT == (int)HOST_CRT_MASK && (int)PE_ENH_FIT_KEEP == (int)HOST_FIT_KEEP &&
           (int)PE_ENH_FIT_INTEGER == (int)HOST_FIT_INTEGER && (int)PE_ENH_FIT_STRETCH == (int)HOST_FIT_STRETCH,
           "registry values are the presenter's enums");
    /* defaults are retail, and a query writes nothing */
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_OFF && pe_enh_display_fit() == PE_ENH_FIT_KEEP &&
           pe_enh_option_is_retail(f) && pe_enh_option_is_retail(t), "both default to retail");
    ASSERT(access(enh_conf, F_OK) != 0, "queries create no config file");
    pe_enh_option_label(f, label, sizeof label);
    ASSERT(strcmp(label, "DISPLAY FILTER: OFF (RETAIL)") == 0, "filter label names the retail value");
    pe_enh_option_label(t, label, sizeof label);
    ASSERT(strcmp(label, "WINDOW FIT: KEEP ASPECT (RETAIL)") == 0, "fit label names the retail value");
    pe_enh_option_cycle(f);
    pe_enh_option_label(f, label, sizeof label);
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_SCANLINES && strcmp(label, "DISPLAY FILTER: SCANLINES") == 0,
           "A cycles to SCANLINES");
    pe_enh_option_cycle(f);
    pe_enh_option_label(f, label, sizeof label);
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_CRT && strcmp(label, "DISPLAY FILTER: CRT (SCANLINES + MASK)") == 0,
           "then CRT (SCANLINES + MASK)");
    pe_enh_option_cycle(f);
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_OFF, "and wraps to OFF");
    pe_enh_option_cycle(t);
    pe_enh_option_label(t, label, sizeof label);
    ASSERT(pe_enh_display_fit() == PE_ENH_FIT_INTEGER && strcmp(label, "WINDOW FIT: INTEGER SCALE") == 0,
           "A cycles the fit to INTEGER SCALE");
    pe_enh_option_cycle(t);
    pe_enh_option_label(t, label, sizeof label);
    ASSERT(pe_enh_display_fit() == PE_ENH_FIT_STRETCH && strcmp(label, "WINDOW FIT: STRETCH") == 0, "then STRETCH");
    pe_enh_option_cycle(t);
    ASSERT(pe_enh_display_fit() == PE_ENH_FIT_KEEP, "and wraps to KEEP ASPECT");
    pe_enh_set_display_filter(9); pe_enh_set_display_fit(9);
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_CRT && pe_enh_display_fit() == PE_ENH_FIT_STRETCH,
           "set snaps to the largest listed value");
    pe_enh_set_display_filter(-3); pe_enh_set_display_fit(-3);
    ASSERT(pe_enh_display_filter() == 0 && pe_enh_display_fit() == 0, "below the list snaps to retail");
    /* persistence in enhancements.conf */
    pe_enh_set_display_filter(PE_ENH_FILTER_CRT); pe_enh_set_display_fit(PE_ENH_FIT_INTEGER);
    {
        char text[1024] = {0};
        FILE *fp = fopen(enh_conf, "r");
        ASSERT(fp != NULL, "the menu change wrote the config");
        size_t n_ = fread(text, 1, sizeof text - 1, fp);
        text[n_] = 0; fclose(fp);
        ASSERT(strstr(text, "display_filter=2\n") && strstr(text, "display_fit=1\n"), "config lines display_filter=2 display_fit=1");
    }
    pe_enh_reset();
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_CRT && pe_enh_display_fit() == PE_ENH_FIT_INTEGER,
           "both persist across a reload");
    setenv("PE_DISPLAY_FILTER", "1", 1); setenv("PE_DISPLAY_FIT", "2", 1); pe_enh_reset();
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_SCANLINES && pe_enh_display_fit() == PE_ENH_FIT_STRETCH,
           "env overrides apply");
    unsetenv("PE_DISPLAY_FILTER"); unsetenv("PE_DISPLAY_FIT"); pe_enh_reset();
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_CRT && pe_enh_display_fit() == PE_ENH_FIT_INTEGER,
           "env overrides are not written back");
    setenv("PE_DISPLAY_FILTER", "loud", 1); setenv("PE_DISPLAY_FIT", "", 1); pe_enh_reset();
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_OFF && pe_enh_display_fit() == PE_ENH_FIT_INTEGER,
           "an unparsable override means retail; an empty one is ignored");
    unsetenv("PE_DISPLAY_FILTER"); unsetenv("PE_DISPLAY_FIT");
    unlink(enh_conf); pe_enh_reset();
    PASS();
}

/* With both options at their defaults the presenter's pure pieces give the
 * retail bytes: the geometry is the retail letterbox, the rectangle scaler
 * equals HostWindow_ScaleBGRA, the filter is a no-op. */
static void test_ENHX_display_defaults_identity(void)
{
    static uint8_t frame[ENHX_FW * ENHX_FH * 3], frame2[2 * ENHX_FW * 2 * ENHX_FH * 3];
    static const int win[][2] = {{320, 240}, {640, 480}, {960, 720}, {1280, 800}, {1920, 1080}, {1000, 300},
                                 {300, 1000}, {333, 777}, {1, 1}, {7, 5}, {319, 239}, {321, 241}};
    TEST("ENHX_display_defaults_identity");
    enh_isolate();
    unsetenv("PE_DISPLAY_FILTER"); unsetenv("PE_DISPLAY_FIT"); pe_enh_reset();
    ASSERT(pe_enh_display_filter() == HOST_CRT_OFF && pe_enh_display_fit() == HOST_FIT_KEEP, "defaults");
    enhx_frame(frame, ENHX_FW, ENHX_FH);
    enhx_frame(frame2, 2 * ENHX_FW, 2 * ENHX_FH);
    for (unsigned i = 0; i < sizeof win / sizeof win[0]; i++) {
        for (int pass = 0; pass < 2; pass++) {
            const uint8_t *src = pass ? frame2 : frame;
            int fw = pass ? 2 * ENHX_FW : ENHX_FW, fh = pass ? 2 * ENHX_FH : ENHX_FH;
            int ww = win[i][0], wh = win[i][1];
            size_t bytes = (size_t)ww * (size_t)wh * 4u;
            uint8_t *want = enhx_retail_window(src, fw, fh, ww, wh), *got = (uint8_t *)malloc(bytes);
            HostFilterPlan p;
            HostFilterRect keep;
            ASSERT(want && got, "alloc");
            HostWindow_FitRect(ww, wh, fw, fh, &keep.x0, &keep.y0, &keep.w, &keep.h);
            HostFilter_Plan(&p, pe_enh_display_fit(), ww, wh, fw, fh, ENHX_FW, ENHX_FH);
            ASSERT(p.direct && HostFilter_RectEq(&p.rect, &keep) && p.bw == ww && p.bh == wh,
                   "KEEP ASPECT plans the retail letterbox, direct");
            memset(got, 0xA5, bytes);
            HostFilter_ScaleNearestRect(got, ww, wh, src, fw, fh, &p.rect);
            ASSERT(memcmp(want, got, bytes) == 0, "the rectangle scaler equals HostWindow_ScaleBGRA byte for byte");
            HostFilter_ApplyCrt(got, ww, wh, &p.rect, ENHX_FH, pe_enh_display_filter());
            ASSERT(memcmp(want, got, bytes) == 0, "filter OFF leaves every byte alone");
            free(want); free(got);
        }
    }
    {   /* the retail window buffers themselves, pinned (they are HostWindow_ScaleBGRA's) */
        uint8_t *a = enhx_retail_window(frame, ENHX_FW, ENHX_FH, 960, 720);
        uint8_t *b = enhx_retail_window(frame, ENHX_FW, ENHX_FH, 1280, 800);
        ASSERT(a && b, "alloc");
        ENHX_HASH(a, 960u * 720u * 4u, 0xb261cf5565154583ull, "retail 960x720");
        ENHX_HASH(b, 1280u * 800u * 4u, 0xd9c2f223107bea03ull, "retail 1280x800");
        free(a); free(b);
    }
    ASSERT(access(enh_conf, F_OK) != 0, "no config file was created");
    PASS();
}

static void test_ENHX_fit_geometry(void)
{
    HostFilterRect r;
    HostFilterPlan p;
    TEST("ENHX_fit_geometry");
    enh_isolate();
    /* INTEGER: the largest whole multiple of 320x240 that fits, centred */
    HostFilter_FitRect(HOST_FIT_INTEGER, 1920, 1080, 320, 240, 320, 240, &r);
    ASSERT(r.x0 == 320 && r.y0 == 60 && r.w == 1280 && r.h == 960, "1920x1080 -> 4x, 1280x960 at (320,60)");
    HostFilter_FitRect(HOST_FIT_INTEGER, 1280, 800, 320, 240, 320, 240, &r);
    ASSERT(r.x0 == 160 && r.y0 == 40 && r.w == 960 && r.h == 720, "1280x800 -> 3x, 960x720 at (160,40)");
    HostFilter_FitRect(HOST_FIT_INTEGER, 1000, 300, 320, 240, 320, 240, &r);
    ASSERT(r.x0 == 340 && r.y0 == 30 && r.w == 320 && r.h == 240, "a short window gives 1x");
    HostFilter_FitRect(HOST_FIT_INTEGER, 300, 200, 320, 240, 320, 240, &r);
    {
        HostFilterRect keep;
        HostWindow_FitRect(300, 200, 320, 240, &keep.x0, &keep.y0, &keep.w, &keep.h);
        ASSERT(HostFilter_RectEq(&r, &keep) && r.w == 266 && r.h == 200, "a window below 1x falls back to the aspect fit");
    }
    HostFilter_FitRect(HOST_FIT_INTEGER, 1920, 1080, 640, 480, 320, 240, &r);
    ASSERT(r.w == 1280 && r.h == 960, "a 2x presentation frame still scales by whole multiples of the native frame");
    HostFilter_FitRect(HOST_FIT_STRETCH, 1920, 1080, 320, 240, 320, 240, &r);
    ASSERT(r.x0 == 0 && r.y0 == 0 && r.w == 1920 && r.h == 1080, "STRETCH fills the window");
    HostFilter_FitRect(HOST_FIT_KEEP, 1920, 1080, 320, 240, 320, 240, &r);
    ASSERT(r.x0 == 240 && r.y0 == 0 && r.w == 1440 && r.h == 1080, "KEEP is the retail letterbox");
    /* plans */
    HostFilter_Plan(&p, HOST_FIT_INTEGER, 960, 720, 320, 240, 320, 240);
    ASSERT(p.direct && p.rect.w == 960 && p.bw == 960 && p.bh == 720, "an exact multiple is the retail letterbox: direct");
    HostFilter_Plan(&p, HOST_FIT_INTEGER, 1920, 1080, 320, 240, 320, 240);
    ASSERT(!p.direct && p.rect.w == 1280 && p.bw == 1280 && p.bh == 960, "INTEGER at 1080p: a 1280x960 box, copied");
    HostFilter_Plan(&p, HOST_FIT_STRETCH, 1920, 1080, 320, 240, 320, 240);
    ASSERT(!p.direct && p.rect.w == 1920 && p.rect.h == 1080 && p.bw == 1440 && p.bh == 1080,
           "STRETCH at 16:9: the scaler box is the aspect fit, stretched into the window");
    HostFilter_Plan(&p, HOST_FIT_STRETCH, 1280, 960, 320, 240, 320, 240);
    ASSERT(p.direct, "STRETCH in a 4:3 window is the retail letterbox: direct");
    HostFilter_Plan(&p, HOST_FIT_STRETCH, 1280, 800, 320, 240, 320, 240);
    ASSERT(!p.direct && p.bw == 1064 && p.bh == 798 && p.rect.w == 1280 && p.rect.h == 800,
           "STRETCH at 16:10: the box is the largest exact 4:3 size inside the aspect fit (1066x800 rounds to a row off 4:3)");
    /* A scaler's box must be filled exactly by the scaler's own letterbox, whatever the window: no black edge for
     * a stretch to enlarge.  Swept over many window sizes, both frame sizes, both non-retail fits. */
    for (int frame = 0; frame < 2; frame++) for (int fit = HOST_FIT_INTEGER; fit <= HOST_FIT_STRETCH; fit++)
        for (int ww = 100; ww <= 2600; ww += 37) for (int wh = 60; wh <= 1500; wh += 29) {
            int fw = frame ? 640 : 320, fh = frame ? 480 : 240;
            HostFilterRect f;
            HostFilter_Plan(&p, fit, ww, wh, fw, fh, 320, 240);
            if (p.direct) continue;
            HostWindow_FitRect(p.bw, p.bh, fw, fh, &f.x0, &f.y0, &f.w, &f.h);
            ASSERT(f.x0 == 0 && f.y0 == 0 && f.w == p.bw && f.h == p.bh, "a scaler fills its box exactly (no bar)");
            ASSERT(p.bw <= ww && p.bh <= wh && p.rect.x0 >= 0 && p.rect.y0 >= 0 &&
                   p.rect.x0 + p.rect.w <= ww && p.rect.y0 + p.rect.h <= wh, "box and rectangle lie inside the window");
            if (fit == HOST_FIT_STRETCH)
                ASSERT(p.rect.x0 == 0 && p.rect.y0 == 0 && p.rect.w == ww && p.rect.h == wh, "STRETCH fills the window");
            else
                ASSERT(p.bw == p.rect.w && p.bh == p.rect.h, "INTEGER copies the box");
        }
    PASS();
}

/* Row gains and golden window buffers of the CRT filter. */
static void test_ENHX_crt_golden(void)
{
    static uint8_t frame[ENHX_FW * ENHX_FH * 3], gray[ENHX_FW * ENHX_FH * 3];
    static const struct { int w, h, mode; uint64_t want; } golden[] = {
        {960, 720, HOST_CRT_SCANLINES, 0x5dbe04e62290f1f3ull}, {960, 720, HOST_CRT_MASK, 0x89fcc30396670753ull},
        {1280, 800, HOST_CRT_SCANLINES, 0xdda047e573dedf73ull}, {1280, 800, HOST_CRT_MASK, 0xb66455b94b46f4ebull},
        {1920, 1080, HOST_CRT_SCANLINES, 0x59736c9a440eb883ull}, {1920, 1080, HOST_CRT_MASK, 0x474206413d9ff95bull},
        {640, 480, HOST_CRT_SCANLINES, 0x7700b643e8994503ull}, {640, 360, HOST_CRT_MASK, 0x47ee01194eaef893ull}};
    TEST("ENHX_crt_golden");
    enh_isolate();
    enhx_frame(frame, ENHX_FW, ENHX_FH);
    /* the beam profile: 3 and 4 window rows per source line */
    {
        int g3[6] = {206, 320, 206, 206, 320, 206}, g4[8] = {176, 304, 304, 176, 176, 304, 304, 176};
        for (int y = 0; y < 6; y++) ASSERT(HostFilter_CrtRowGain(y, 720, 240) == g3[y], "3 rows per line: dim, bright, dim");
        for (int y = 0; y < 8; y++) ASSERT(HostFilter_CrtRowGain(y, 960, 240) == g4[y], "4 rows per line: dim, bright, bright, dim");
        for (int h = 480; h <= 2160; h += 37) for (int y = 0; y < h; y++) {
            int g = HostFilter_CrtRowGain(y, h, 240);
            ASSERT(g >= 64 && g <= HOST_CRT_SCAN_GAIN, "the row gain stays between the gap and the beam centre");
        }
        for (int y = 0; y < 720; y++)
            ASSERT(HostFilter_CrtRowGain(y, 720, 240) == HostFilter_CrtRowGain(719 - y, 720, 240), "the profile is symmetric");
    }
    for (unsigned i = 0; i < sizeof golden / sizeof golden[0]; i++) {
        int ww = golden[i].w, wh = golden[i].h;
        size_t bytes = (size_t)ww * (size_t)wh * 4u;
        uint8_t *buf = enhx_retail_window(frame, ENHX_FW, ENHX_FH, ww, wh), *again = (uint8_t *)malloc(bytes);
        HostFilterPlan p;
        char what[64];
        ASSERT(buf && again, "alloc");
        HostFilter_Plan(&p, HOST_FIT_KEEP, ww, wh, ENHX_FW, ENHX_FH, ENHX_FW, ENHX_FH);
        memcpy(again, buf, bytes);
        HostFilter_ApplyCrt(buf, ww, wh, &p.rect, ENHX_FH, golden[i].mode);
        HostFilter_ApplyCrt(again, ww, wh, &p.rect, ENHX_FH, golden[i].mode);
        ASSERT(memcmp(buf, again, bytes) == 0, "deterministic");
        snprintf(what, sizeof what, "crt %dx%d mode %d", ww, wh, golden[i].mode);
        ENHX_HASH(buf, bytes, golden[i].want, what);
        free(buf); free(again);
    }
    /* bars stay black, and a window too short for a gap skips the scanlines but not the mask */
    {
        int ww = 1280, wh = 800;
        size_t bytes = (size_t)ww * (size_t)wh * 4u;
        uint8_t *buf = enhx_retail_window(frame, ENHX_FW, ENHX_FH, ww, wh);
        HostFilterPlan p;
        ASSERT(buf != NULL, "alloc");
        HostFilter_Plan(&p, HOST_FIT_KEEP, ww, wh, ENHX_FW, ENHX_FH, ENHX_FW, ENHX_FH);
        HostFilter_ApplyCrt(buf, ww, wh, &p.rect, ENHX_FH, HOST_CRT_MASK);
        for (int y = 0; y < wh; y++) for (int x = 0; x < p.rect.x0; x++)
            ASSERT(*(const uint32_t *)(buf + ((size_t)y * (size_t)ww + (size_t)x) * 4u) == 0u, "the pillarbox stays black");
        free(buf); (void)bytes;
        buf = enhx_retail_window(frame, ENHX_FW, ENHX_FH, 640, 360);
        {
            uint8_t *ref = enhx_retail_window(frame, ENHX_FW, ENHX_FH, 640, 360);
            HostFilter_Plan(&p, HOST_FIT_KEEP, 640, 360, ENHX_FW, ENHX_FH, ENHX_FW, ENHX_FH);
            HostFilter_ApplyCrt(buf, 640, 360, &p.rect, ENHX_FH, HOST_CRT_SCANLINES);
            ASSERT(memcmp(buf, ref, 640u * 360u * 4u) == 0, "under 2 window rows per line the scanlines are skipped");
            HostFilter_ApplyCrt(buf, 640, 360, &p.rect, ENHX_FH, HOST_CRT_MASK);
            ASSERT(memcmp(buf, ref, 640u * 360u * 4u) != 0, "but the mask still applies");
            free(ref);
        }
        free(buf);
    }
    /* the documented formula on a flat grey frame: byte = (128 * rowgain * maskgain) with the filter's own rounding */
    memset(gray, 128, sizeof gray);
    {
        int ww = 960, wh = 720;
        uint8_t *buf = enhx_retail_window(gray, ENHX_FW, ENHX_FH, ww, wh);
        HostFilterPlan p;
        ASSERT(buf != NULL, "alloc");
        HostFilter_Plan(&p, HOST_FIT_KEEP, ww, wh, ENHX_FW, ENHX_FH, ENHX_FW, ENHX_FH);
        HostFilter_ApplyCrt(buf, ww, wh, &p.rect, ENHX_FH, HOST_CRT_MASK);
        for (int y = 0; y < 6; y++) for (int x = 0; x < 6; x++) {
            const uint8_t *px = buf + ((size_t)y * (size_t)ww + (size_t)x) * 4u;
            int rg = HostFilter_CrtRowGain(y, wh, ENHX_FH);
            for (int byte = 0; byte < 3; byte++) {
                int mg = HostFilter_CrtMaskGain(x % 3, 2 - byte);
                int g = (rg * mg + 128) >> 8, v = (128 * g + 128) >> 8;
                ASSERT(px[byte] == (uint8_t)(v > 255 ? 255 : v) && px[3] == 0, "mask x scanline gain on a flat grey");
            }
        }
        /* the lit channel of each column: red at x%3==0 (byte 2), green at 1 (byte 1), blue at 2 (byte 0) */
        {
            const uint8_t *r0 = buf + 1u * (size_t)ww * 4u;               /* row 1: the beam centre */
            ASSERT(r0[2] > r0[1] && r0[2] > r0[0], "column 0 is red");
            ASSERT(r0[4 + 1] > r0[4 + 2] && r0[4 + 1] > r0[4 + 0], "column 1 is green");
            ASSERT(r0[8 + 0] > r0[8 + 1] && r0[8 + 0] > r0[8 + 2], "column 2 is blue");
        }
        free(buf);
    }
    PASS();
}

/* Golden window buffers of the fit modes, and the structure of an integer scale. */
static void test_ENHX_fit_golden(void)
{
    static uint8_t frame[ENHX_FW * ENHX_FH * 3];
    static const struct { int w, h, fit; uint64_t want; } golden[] = {
        {1920, 1080, HOST_FIT_INTEGER, 0x60eed65737253d83ull}, {1280, 800, HOST_FIT_INTEGER, 0xedcbf4bce10a5d83ull},
        {1280, 800, HOST_FIT_STRETCH, 0x3e9bb815b457c683ull}, {1920, 1080, HOST_FIT_STRETCH, 0x7f72a27a77ca1723ull}};
    TEST("ENHX_fit_golden");
    enh_isolate();
    enhx_frame(frame, ENHX_FW, ENHX_FH);
    for (unsigned i = 0; i < sizeof golden / sizeof golden[0]; i++) {
        int ww = golden[i].w, wh = golden[i].h;
        size_t bytes = (size_t)ww * (size_t)wh * 4u;
        uint8_t *buf = (uint8_t *)malloc(bytes);
        HostFilterPlan p;
        char what[64];
        ASSERT(buf != NULL, "alloc");
        HostFilter_Plan(&p, golden[i].fit, ww, wh, ENHX_FW, ENHX_FH, ENHX_FW, ENHX_FH);
        memset(buf, 0x77, bytes);
        HostFilter_ScaleNearestRect(buf, ww, wh, frame, ENHX_FW, ENHX_FH, &p.rect);
        snprintf(what, sizeof what, "fit %d at %dx%d", golden[i].fit, ww, wh);
        ENHX_HASH(buf, bytes, golden[i].want, what);
        free(buf);
    }
    /* an integer scale is whole k x k blocks of the source pixel, centred, black around */
    for (int k = 2; k <= 4; k++) {
        int ww = ENHX_FW * k + 37, wh = ENHX_FH * k + 11;
        uint8_t *buf = (uint8_t *)malloc((size_t)ww * (size_t)wh * 4u);
        HostFilterPlan p;
        ASSERT(buf != NULL, "alloc");
        HostFilter_Plan(&p, HOST_FIT_INTEGER, ww, wh, ENHX_FW, ENHX_FH, ENHX_FW, ENHX_FH);
        ASSERT(p.rect.w == ENHX_FW * k && p.rect.h == ENHX_FH * k && p.rect.x0 == 18 && p.rect.y0 == 5, "k x k, centred");
        HostFilter_ScaleNearestRect(buf, ww, wh, frame, ENHX_FW, ENHX_FH, &p.rect);
        for (int sy = 0; sy < ENHX_FH; sy += 7) for (int sx = 0; sx < ENHX_FW; sx += 5) {
            const uint8_t *s = frame + ((size_t)sy * ENHX_FW + (size_t)sx) * 3u;
            for (int dy = 0; dy < k; dy++) for (int dx = 0; dx < k; dx++) {
                const uint8_t *d = buf + ((size_t)(p.rect.y0 + sy * k + dy) * (size_t)ww + (size_t)(p.rect.x0 + sx * k + dx)) * 4u;
                ASSERT(d[0] == s[2] && d[1] == s[1] && d[2] == s[0] && d[3] == 0, "each source pixel is a k x k block");
            }
        }
        ASSERT(*(const uint32_t *)buf == 0u && *(const uint32_t *)(buf + ((size_t)(wh - 1) * (size_t)ww + (size_t)(ww - 1)) * 4u) == 0u,
               "black around the frame");
        free(buf);
    }
    /* Place: a box the size of the rectangle is copied, any other size is stretched smoothly */
    {
        enum { BW = 24, BH = 18 };
        uint8_t box[BW * BH * 4], *win = (uint8_t *)malloc(64u * 48u * 4u);
        HostFilterRect r = {8, 6, 48, 36};
        ASSERT(win != NULL, "alloc");
        for (int y = 0; y < BH; y++) for (int x = 0; x < BW; x++) {
            uint8_t *q = box + ((size_t)y * BW + (size_t)x) * 4u;
            q[0] = (uint8_t)(x * 10); q[1] = (uint8_t)(y * 13); q[2] = (uint8_t)(x * 3 + y * 5); q[3] = 0;
        }
        {   /* copy */
            HostFilterRect rc = {8, 6, BW, BH};
            memset(win, 0x33, 64u * 48u * 4u);
            HostFilter_Place(win, 64, 48, box, BW, BH, &rc);
            for (int y = 0; y < BH; y++)
                ASSERT(memcmp(win + ((size_t)(y + 6) * 64u + 8u) * 4u, box + (size_t)y * BW * 4u, BW * 4u) == 0, "an equal-size box is copied");
            ASSERT(*(const uint32_t *)win == 0u && *(const uint32_t *)(win + (size_t)(6 + BH) * 64u * 4u) == 0u, "the rest of the window is black");
        }
        memset(win, 0x33, 64u * 48u * 4u);
        HostFilter_Place(win, 64, 48, box, BW, BH, &r);
        ENHX_HASH(win, 64u * 48u * 4u, 0xf10fbcf95a51ca52ull, "place 24x18 -> 48x36 in 64x48");
        {   /* an axis within 1 % of the rectangle is mapped by nearest index (no blending): every output pixel is a box pixel */
            enum { SBW = 200, SBH = 150, SRW = 201, SRH = 151 };
            static uint8_t sbox[SBW * SBH * 4], swin[SRW * SRH * 4];
            HostFilterRect sr = {0, 0, SRW, SRH};
            uint32_t lcg = 777u;
            for (int i = 0; i < SBW * SBH * 4; i++) { lcg = lcg * 1664525u + 1013904223u; sbox[i] = (i & 3) == 3 ? 0 : (uint8_t)(lcg >> 24); }
            HostFilter_Place(swin, SRW, SRH, sbox, SBW, SBH, &sr);
            for (int y = 0; y < SRH; y++) for (int x = 0; x < SRW; x++) {
                int bx = (int)(((long long)(2 * x + 1) * SBW) / (2 * SRW)), by = (int)(((long long)(2 * y + 1) * SBH) / (2 * SRH));
                ASSERT(memcmp(swin + ((size_t)y * SRW + (size_t)x) * 4u, sbox + ((size_t)by * SBW + (size_t)bx) * 4u, 4u) == 0,
                       "within 1 %: nearest index, every output pixel is a box pixel");
            }
        }
        {   /* the corners of the stretch are the box's corners; a constant box stays constant */
            const uint8_t *tl = win + ((size_t)6 * 64u + 8u) * 4u, *br = win + ((size_t)(6 + 35) * 64u + (size_t)(8 + 47)) * 4u;
            ASSERT(memcmp(tl, box, 3) == 0 && memcmp(br, box + ((size_t)(BH - 1) * BW + (BW - 1)) * 4u, 3) == 0, "corners are kept");
            memset(box, 90, sizeof box);
            HostFilter_Place(win, 64, 48, box, BW, BH, &r);
            for (int y = 6; y < 6 + 36; y++) for (int x = 8; x < 8 + 48; x++)
                ASSERT(win[((size_t)y * 64u + (size_t)x) * 4u + 1u] == 90, "a flat box stays flat");
        }
        free(win);
    }
    PASS();
}

/* OPTIONS > GRAPHICS rows 7 and 8 in the dev menu. */
static void test_ENHX_display_dev_menu(void)
{
    static uint8_t fb[ENHX_FW * ENHX_FH * 3];
    char label[96];
    TEST("ENHX_display_dev_menu");
    enh_isolate();
    unsetenv("PE_DISPLAY_FILTER"); unsetenv("PE_DISPLAY_FIT"); pe_enh_reset();
    setenv("PE_DEV_MENU", "1", 1);
    pe_plat_cheats_reset();
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    for (int i = 0; i < 6; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* OPTIONS (row 7) */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> OPTIONS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                              /* GRAPHICS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> GRAPHICS */
    for (int i = 0; i < 6; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* row 7: DISPLAY FILTER */
    ASSERT(pe_plat_cheats_overlay(fb, ENHX_FW, ENHX_FH) != fb, "graphics page draws");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_SCANLINES && pe_enh_display_fit() == PE_ENH_FIT_KEEP, "row 7 cycles the display filter");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_CRT, "to CRT");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_display_filter() == PE_ENH_FILTER_OFF, "and back to OFF");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                              /* row 8: WINDOW FIT */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_display_fit() == PE_ENH_FIT_INTEGER && pe_enh_display_filter() == PE_ENH_FILTER_OFF, "row 8 cycles the window fit");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_display_fit() == PE_ENH_FIT_STRETCH, "to STRETCH");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_display_fit() == PE_ENH_FIT_KEEP, "and back to KEEP ASPECT");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);  /* row 9 is BRIGHTNESS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                              /* wraps to row 1: DISPLAY SCALER */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_display_scaler() == PE_ENH_SCALER_SHARP_BILINEAR, "the page wraps from row 9 to row 1");
    pe_enh_set_display_scaler(PE_ENH_SCALER_NEAREST);
    for (int n = 0; n < pe_enh_category_count(PE_ENH_CAT_GRAPHICS); n++) {
        pe_enh_option_label(pe_enh_category_option(PE_ENH_CAT_GRAPHICS, n), label, sizeof label);
        ASSERT(strlen(label) + 2 <= 50, "every graphics row fits the 320-pixel overlay (6 px per character)");
    }
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_BACK, 1);                              /* -> OPTIONS row 2 */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_UP, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> ENHANCEMENTS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_move_speed_pct() == 125, "BACK returns to OPTIONS, whose row 1 is still ENHANCEMENTS");
    pe_enh_set_move_speed_pct(100);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    unsetenv("PE_DEV_MENU");
    pe_plat_cheats_reset();
    unlink(enh_conf); pe_enh_reset();
    PASS();
}

static void test_ENHX_display_all(void)
{
    test_ENHX_display_registry();
    test_ENHX_display_defaults_identity();
    test_ENHX_fit_geometry();
    test_ENHX_crt_golden();
    test_ENHX_fit_golden();
    test_ENHX_display_dev_menu();
}

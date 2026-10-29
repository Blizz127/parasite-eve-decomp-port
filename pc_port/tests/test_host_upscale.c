/* Unit test for the DISPLAY SCALER filters (platform/host_upscale.h):
 *  - mode NEAREST is byte-identical to HostWindow_ScaleBGRA (the retail
 *    default adds no new code path), at several window sizes and thread
 *    counts;
 *  - SHARP BILINEAR and FSR 1 keep the letterbox geometry and black bars,
 *    reproduce a flat frame exactly, SHARP BILINEAR never leaves the
 *    source's value range and equals nearest at integer ratios, FSR 1
 *    tracks nearest closely (RCAS overshoot is bounded by design), and both
 *    are deterministic.
 * No X11/Win32, no game runtime.
 *
 * Capture mode (before/after at the same frame, for evidence):
 *   pe-upscale-tests --demo in.ppm out_prefix WIDTH HEIGHT
 * reads a binary P6 PPM (a port --screenshot), writes
 * out_prefix-nearest.ppm, -sharp.ppm, -fsr1.ppm at WIDTH x HEIGHT and
 * prints the per-mode wall time. */
#include <stdatomic.h>
#define HostWindow_ScaleBGRA test_nearest_kernel
#include "../platform/host_window_scale.h"
#undef HostWindow_ScaleBGRA
static atomic_uint nearest_calls;
static void HostWindow_ScaleBGRA(uint8_t *out, int ww, int wh,
                                const uint8_t *rgb, int fw, int fh)
{
    atomic_fetch_add(&nearest_calls, 1);
    test_nearest_kernel(out, ww, wh, rgb, fw, fh);
}
static atomic_uint injected_failures;
static int injected_failure(int point)
{
    unsigned mask = 1u << point;
    return (atomic_fetch_and(&injected_failures, ~mask) & mask) != 0;
}
#define HOST_UPSCALE_FAIL(point) injected_failure(point)
#include "../platform/host_upscale.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int g_fail;
#define CHECK(cond, ...) do { \
        if (!(cond)) { fprintf(stderr, "FAIL: " __VA_ARGS__); fputc('\n', stderr); g_fail = 1; } \
    } while (0)

static uint8_t *make_source(int fb_w, int fb_h, unsigned seed)
{
    uint8_t *rgb = malloc((size_t)fb_w * fb_h * 3u);
    unsigned s = seed ? seed : 1;
    for (int y = 0; y < fb_h; y++)
        for (int x = 0; x < fb_w; x++) {
            uint8_t *p = rgb + ((size_t)y * fb_w + x) * 3u;
            /* gradients plus a checker and a noise band: edges, flats, dither */
            s = s * 1664525u + 1013904223u;
            p[0] = (uint8_t)(x * 255 / (fb_w - 1));
            p[1] = (uint8_t)(((x / 8) ^ (y / 8)) & 1 ? 220 : 30);
            p[2] = (uint8_t)(y < fb_h / 3 ? (int)(s >> 24) : y * 255 / (fb_h - 1));
        }
    return rgb;
}

static int is_black(const uint8_t *out, int width, int x, int y)
{
    const uint8_t *p = out + ((size_t)y * width + x) * 4u;
    return p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0;
}

static void check_geometry(const char *name, const uint8_t *out, int ww, int wh, int fb_w, int fb_h)
{
    int x0, y0, w, h;
    HostWindow_FitRect(ww, wh, fb_w, fb_h, &x0, &y0, &w, &h);
    if (x0 > 0) CHECK(is_black(out, ww, 0, wh / 2) && is_black(out, ww, x0 - 1, wh / 2), "%s: left bar", name);
    if (x0 + w < ww) CHECK(is_black(out, ww, ww - 1, wh / 2) && is_black(out, ww, x0 + w, wh / 2), "%s: right bar", name);
    if (y0 > 0) CHECK(is_black(out, ww, ww / 2, 0) && is_black(out, ww, ww / 2, y0 - 1), "%s: top bar", name);
    if (y0 + h < wh) CHECK(is_black(out, ww, ww / 2, wh - 1) && is_black(out, ww, ww / 2, y0 + h), "%s: bottom bar", name);
    /* the content rectangle is written (alpha byte is 0 everywhere, so test a colour) */
    {
        int any = 0;
        for (int y = y0; y < y0 + h && !any; y++)
            for (int x = x0; x < x0 + w; x++)
                if (!is_black(out, ww, x, y)) { any = 1; break; }
        CHECK(any, "%s: content rectangle is not all black", name);
    }
}

static void check_range(const char *name, const uint8_t *out, int ww, int wh, const uint8_t *rgb, int fb_w, int fb_h)
{
    uint8_t lo[3] = {255, 255, 255}, hi[3] = {0, 0, 0};
    int x0, y0, w, h;
    for (size_t i = 0; i < (size_t)fb_w * fb_h; i++)
        for (int c = 0; c < 3; c++) {
            if (rgb[i * 3 + c] < lo[c]) lo[c] = rgb[i * 3 + c];
            if (rgb[i * 3 + c] > hi[c]) hi[c] = rgb[i * 3 + c];
        }
    HostWindow_FitRect(ww, wh, fb_w, fb_h, &x0, &y0, &w, &h);
    for (int y = y0; y < y0 + h; y++)
        for (int x = x0; x < x0 + w; x++) {
            const uint8_t *p = out + ((size_t)y * ww + x) * 4u;
            for (int c = 0; c < 3; c++)
                if (p[2 - c] < lo[c] || p[2 - c] > hi[c]) {
                    CHECK(0, "%s: pixel (%d,%d) channel %d = %d outside source range [%d,%d]",
                          name, x, y, c, p[2 - c], lo[c], hi[c]);
                    return;
                }
        }
}

/* A native text-like white stroke at handheld output scale must not spread
 * each vertical edge across four output columns (the old direct 5x pass).
 * This checks the visible transition width rather than the cache layout. */
static void run_text_edges(void)
{
    uint8_t rgb[16 * 12 * 3] = {0}, out[80 * 60 * 4];
    for (int y = 4; y < 8; y++)
        for (int x = 4; x < 12; x++)
            memset(rgb + (y * 16 + x) * 3, 255, 3);
    HostUpscale_Run(HOST_UPSCALE_FSR1, out, 80, 60, rgb, 16, 12);
    int transitions = 0;
    for (int x = 0; x < 80; x++) {
        uint8_t v = out[(30 * 80 + x) * 4];
        if (v > 0 && v < 255) transitions++;
    }
    CHECK(transitions <= 4, "5x text stroke has at most two transitional columns per edge (got %d)", transitions);
    CHECK(out[(30 * 80 + 40) * 4] == 255, "text stroke centre stays white");
    CHECK(out[(30 * 80 + 10) * 4] == 0, "text background stays black");
}

static void run_cases(void)
{
    static const int sizes[][2] = {{640, 480}, {1280, 800}, {1920, 1080}, {960, 720}, {300, 200}, {1281, 721}};
    const int fb_w = 320, fb_h = 240;
    uint8_t *rgb = make_source(fb_w, fb_h, 7);
    uint8_t *flat = malloc((size_t)fb_w * fb_h * 3u);
    for (size_t i = 0; i < (size_t)fb_w * fb_h; i++) { flat[i * 3] = 90; flat[i * 3 + 1] = 140; flat[i * 3 + 2] = 200; }
    for (size_t k = 0; k < sizeof sizes / sizeof sizes[0]; k++) {
        int ww = sizes[k][0], wh = sizes[k][1];
        size_t bytes = (size_t)ww * wh * 4u;
        uint8_t *ref = calloc(bytes, 1), *out = calloc(bytes, 1), *again = calloc(bytes, 1);
        char name[64];
        HostWindow_ScaleBGRA(ref, ww, wh, rgb, fb_w, fb_h);
        /* NEAREST == retail scaler */
        memset(out, 0xAA, bytes);
        CHECK(HostUpscale_Run(HOST_UPSCALE_NEAREST, out, ww, wh, rgb, fb_w, fb_h) == HOST_UPSCALE_NEAREST, "nearest mode id");
        CHECK(memcmp(out, ref, bytes) == 0, "%dx%d: NEAREST is byte-identical to HostWindow_ScaleBGRA", ww, wh);
        for (int mode = HOST_UPSCALE_SHARP_BILINEAR; mode <= HOST_UPSCALE_FSR1; mode++) {
            snprintf(name, sizeof name, "%s %dx%d", mode == HOST_UPSCALE_FSR1 ? "FSR1" : "SHARP", ww, wh);
            memset(out, 0xAA, bytes);
            int expected_mode = mode;
#ifndef PE_UPSCALE_THREADS
            if (mode == HOST_UPSCALE_FSR1) expected_mode = HOST_UPSCALE_SHARP_BILINEAR;
#endif
            CHECK(HostUpscale_Run(mode, out, ww, wh, rgb, fb_w, fb_h) == expected_mode,
                  "%s: actual supported mode returned", name);
            check_geometry(name, out, ww, wh, fb_w, fb_h);
            if (mode == HOST_UPSCALE_SHARP_BILINEAR) check_range(name, out, ww, wh, rgb, fb_w, fb_h);
            else {
                /* RCAS is a sharpener, so local overshoot is by design (bounded
                 * so nothing clips); the frame as a whole must still track
                 * the nearest result closely. */
                int x0, y0, w, h; double sum = 0; size_t n = 0;
                HostWindow_FitRect(ww, wh, fb_w, fb_h, &x0, &y0, &w, &h);
                for (int y = y0; y < y0 + h; y++)
                    for (int x = x0; x < x0 + w; x++)
                        for (int c = 0; c < 3; c++, n++)
                            sum += abs((int)out[((size_t)y * ww + x) * 4u + c] - (int)ref[((size_t)y * ww + x) * 4u + c]);
                CHECK(n && sum / n < 24.0, "%s: mean abs difference to nearest is %.1f levels", name, n ? sum / n : 0.0);
            }
            /* deterministic: a second run is identical */
            memset(again, 0x55, bytes);
            HostUpscale_Run(mode, again, ww, wh, rgb, fb_w, fb_h);
            CHECK(memcmp(out, again, bytes) == 0, "%s: deterministic", name);
            /* a flat frame stays exactly flat (no ringing, no bars inside) */
            HostUpscale_Run(mode, out, ww, wh, flat, fb_w, fb_h);
            {
                int x0, y0, w, h, ok = 1;
                HostWindow_FitRect(ww, wh, fb_w, fb_h, &x0, &y0, &w, &h);
                for (int y = y0; y < y0 + h && ok; y++)
                    for (int x = x0; x < x0 + w; x++) {
                        const uint8_t *p = out + ((size_t)y * ww + x) * 4u;
                        if (p[0] != 200 || p[1] != 140 || p[2] != 90) { ok = 0; break; }
                    }
                CHECK(ok, "%s: flat frame reproduced exactly", name);
            }
        }
        /* integer ratio: sharp bilinear equals nearest (pre-scale only) */
        if (ww == 640 && wh == 480) {
            HostUpscale_Run(HOST_UPSCALE_SHARP_BILINEAR, out, ww, wh, rgb, fb_w, fb_h);
            CHECK(memcmp(out, ref, bytes) == 0, "640x480: SHARP BILINEAR at 2x equals nearest");
        }
        free(ref); free(out); free(again);
    }
#ifdef PE_UPSCALE_THREADS
    /* presenter-thread path: the asynchronous result equals the synchronous one,
     * a newer submission replaces a pending one, and a stale size is discarded */
    {
        const int ww = 1280, wh = 800;
        size_t bytes = (size_t)ww * wh * 4u;
        uint8_t *sync = calloc(bytes, 1), *async = calloc(bytes, 1);
        uint8_t *rgb2 = make_source(fb_w, fb_h, 99);
        HostUpscale_Run(HOST_UPSCALE_FSR1, sync, ww, wh, rgb2, fb_w, fb_h);
        CHECK(HostUpscale_SubmitAsync(HOST_UPSCALE_FSR1, rgb, fb_w, fb_h, ww, wh), "async submit 1");
        CHECK(HostUpscale_SubmitAsync(HOST_UPSCALE_FSR1, rgb2, fb_w, fb_h, ww, wh), "async submit 2 (replaces or follows 1)");
        HostUpscale_Drain();
        CHECK(HostUpscale_TakeReady(async, ww, wh), "a finished frame is available after drain");
        CHECK(memcmp(sync, async, bytes) == 0, "async FSR1 result equals the synchronous result for the newest frame");
        CHECK(!HostUpscale_TakeReady(async, ww, wh), "a frame is taken once");
        CHECK(HostUpscale_SubmitAsync(HOST_UPSCALE_SHARP_BILINEAR, rgb2, fb_w, fb_h, 640, 480), "async submit at another size");
        HostUpscale_Drain();
        CHECK(!HostUpscale_TakeReady(async, ww, wh), "a finished frame of another size is discarded");
        free(sync); free(async); free(rgb2);
    }
#endif
    free(rgb); free(flat);
}

/* Accepted static frames must survive transient allocation failures without
 * requiring the source pixels to change. These cases also exercise cleanup
 * after each stage of failed pthread startup. */
static void run_async_failures(void)
{
#ifdef PE_UPSCALE_THREADS
    uint8_t *rgb = make_source(8, 6, 42);
    uint8_t out[192 * 144 * 4], ref[192 * 144 * 4];
    HostUpscale_Shutdown();
    for (int point = HU_FAIL_ASYNC_MUTEX; point <= HU_FAIL_ASYNC_START; point++) {
        atomic_store(&injected_failures, 1u << point);
        CHECK(!HostUpscale_AsyncAvailable(), "startup stage %d fails safely", point);
        CHECK(!HostUpscale_AsyncAvailable(), "startup failure remains unavailable");
        CHECK(!HostUpscale_SubmitAsync(HOST_UPSCALE_FSR1, rgb, 8, 6, 40, 30),
              "unavailable startup rejects FSR instead of synchronous execution");
        HostUpscale_Shutdown();
    }
    CHECK(HostUpscale_AsyncAvailable(), "startup recovers after shutdown/reset");
    for (int point = HU_FAIL_ASYNC_PENDING; point <= HU_FAIL_FSR_ROW_FRAC; point++) {
        if (point == HU_FAIL_SHARP_TABLE) continue;
        /* Grow every cache, including the integer pre-scale source plane. */
        int ww = point < HU_FAIL_ASYNC_FILTER ? 40 : 64 + 16 * (point - HU_FAIL_ASYNC_FILTER);
        int wh = ww * 3 / 4;
        unsigned nearest_before = atomic_load(&nearest_calls);
        atomic_store(&injected_failures, 1u << point);
        int accepted = HostUpscale_SubmitAsync(HOST_UPSCALE_FSR1, rgb, 8, 6, ww, wh);
        CHECK(accepted == (point != HU_FAIL_ASYNC_PENDING), "allocation stage %d acceptance", point);
        HostUpscale_Drain();
        CHECK(atomic_load(&nearest_calls) == nearest_before,
              "failed async stage %d never uses game-thread nearest cache", point);
        CHECK(!hu_async.pend_new && !hu_async.inflight, "failed stage %d clears busy state", point);
        CHECK(HostUpscale_TakeFailure(), "failed stage %d requests retry", point);
        CHECK(!HostUpscale_TakeFailure(), "failure notification consumed once");
        CHECK(!HostUpscale_TakeReadyFor(out, HOST_UPSCALE_FSR1, ww, wh),
              "failed/degraded stage %d publishes no completed frame", point);
        CHECK(atomic_load(&injected_failures) == 0, "allocation failure point %d reached", point);
        CHECK(HostUpscale_SubmitAsync(HOST_UPSCALE_FSR1, rgb, 8, 6, ww, wh),
              "same static frame resubmitted after stage %d failure", point);
        HostUpscale_Drain();
        CHECK(HostUpscale_TakeReadyFor(out, HOST_UPSCALE_FSR1, ww, wh), "static retry produces ready frame");
        /* Synchronous reference is safe after stopping the coordinator. */
        HostUpscale_Shutdown();
        CHECK(HostUpscale_Run(HOST_UPSCALE_FSR1, ref, ww, wh, rgb, 8, 6) == HOST_UPSCALE_FSR1,
              "reference keeps requested filter");
        CHECK(memcmp(out, ref, (size_t)ww * wh * 4u) == 0, "unchanged static retry equals reference");
        CHECK(HostUpscale_AsyncAvailable(), "restart after verification");
    }
    CHECK(HostUpscale_SubmitAsync(HOST_UPSCALE_SHARP_BILINEAR, rgb, 8, 6, 40, 30), "queue old filter");
    HostUpscale_Drain();
    CHECK(!HostUpscale_TakeReadyFor(out, HOST_UPSCALE_FSR1, 40, 30), "different mode result discarded");
    CHECK(!HostUpscale_TakeReady(out, 40, 30), "stale mode result consumed once");
    HostUpscale_Shutdown();
    free(rgb);
#else
    CHECK(!HostUpscale_AsyncAvailable(), "no-threads build reports unavailable");
    CHECK(!HostUpscale_TakeFailure(), "no-threads failure stub");
    HostUpscale_Shutdown();
#endif
}

/* ── capture mode ───────────────────────────────────────────────────── */
static uint8_t *read_ppm(const char *path, int *w, int *h)
{
    FILE *f = fopen(path, "rb");
    int maxv;
    uint8_t *rgb;
    if (!f) { perror(path); return NULL; }
    if (fscanf(f, "P6 %d %d %d", w, h, &maxv) != 3 || maxv != 255 || fgetc(f) == EOF) { fclose(f); fprintf(stderr, "%s: not a P6 PPM\n", path); return NULL; }
    rgb = malloc((size_t)*w * *h * 3u);
    if (fread(rgb, 3u, (size_t)*w * *h, f) != (size_t)*w * *h) { fclose(f); free(rgb); fprintf(stderr, "%s: short read\n", path); return NULL; }
    fclose(f);
    return rgb;
}

static int write_ppm(const char *path, const uint8_t *bgra, int w, int h)
{
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); return -1; }
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (size_t i = 0; i < (size_t)w * h; i++) {
        uint8_t px[3] = {bgra[i * 4 + 2], bgra[i * 4 + 1], bgra[i * 4]};
        fwrite(px, 1, 3, f);
    }
    fclose(f);
    return 0;
}

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}

static int demo(int argc, char **argv)
{
    static const char *const names[] = {"nearest", "sharp", "fsr1"};
    int fb_w, fb_h, ww, wh;
    uint8_t *rgb, *out;
    char path[1024];
    if (argc != 6) { fprintf(stderr, "usage: %s --demo in.ppm out_prefix WIDTH HEIGHT\n", argv[0]); return 2; }
    rgb = read_ppm(argv[2], &fb_w, &fb_h);
    if (!rgb) return 1;
    ww = atoi(argv[4]); wh = atoi(argv[5]);
    if (ww <= 0 || wh <= 0) return 2;
    out = calloc((size_t)ww * wh, 4u);
    for (int mode = 0; mode <= 2; mode++) {
        double t0, best = 1e9;
        for (int rep = 0; rep < 5; rep++) {          /* warm pool, report the best of 5 */
            t0 = now_ms();
            HostUpscale_Run(mode, out, ww, wh, rgb, fb_w, fb_h);
            if (now_ms() - t0 < best) best = now_ms() - t0;
        }
        snprintf(path, sizeof path, "%s-%s.ppm", argv[3], names[mode]);
        if (write_ppm(path, out, ww, wh) != 0) return 1;
        printf("%-8s %dx%d -> %dx%d  %.2f ms (best of 5)  %s\n", names[mode], fb_w, fb_h, ww, wh, best, path);
    }
    free(rgb); free(out);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "--demo") == 0) return demo(argc, argv);
    run_async_failures();
    run_text_edges();
    run_cases();
    HostUpscale_Shutdown();
    if (g_fail) { fprintf(stderr, "upscale filter tests FAILED\n"); return 1; }
    printf("upscale filter tests passed\n");
    return 0;
}

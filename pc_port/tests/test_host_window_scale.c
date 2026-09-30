/* Unit test for the letterbox/pillarbox scaling math used by both the X11
 * (host_window.c) and Win32 (host_window_win32.c) backends, including the
 * --fullscreen path where the window size is whatever the WM/compositor
 * grants (e.g. gamescope forcing the screen size via ConfigureNotify).
 * HostWindow_ScaleBGRA() is declared `static` in the header so this test
 * includes it directly rather than linking against a platform backend;
 * it needs no X11/Win32 and no game runtime. */
#include "../platform/host_window_scale.h"
#include <stdio.h>
#include <stdlib.h>

static int g_fail;

#define CHECK(cond, ...) do { \
        if (!(cond)) { fprintf(stderr, "FAIL: " __VA_ARGS__); fputc('\n', stderr); g_fail = 1; } \
    } while (0)

/* out is BGRA (see HostWindow_ScaleBGRA); a border/black pixel is 0,0,0,0. */
static int is_black(const uint8_t *out, int width, int x, int y) {
    const uint8_t *p = out + ((size_t)y * width + x) * 4u;
    return p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0;
}

/* Fills a source with a distinct byte per channel per pixel, offset by 1
 * so pixel (0,0) is nonzero and distinguishable from a black border
 * pixel, so a sampled destination pixel can be checked against the
 * pixel it should have nearest-neighbor-sampled. */
static uint8_t *make_source(int fb_w, int fb_h) {
    uint8_t *rgb = malloc((size_t)fb_w * fb_h * 3u);
    for (int y = 0; y < fb_h; y++)
        for (int x = 0; x < fb_w; x++) {
            uint8_t *p = rgb + ((size_t)y * fb_w + x) * 3u;
            p[0] = (uint8_t)(x + 1); p[1] = (uint8_t)(y + 1); p[2] = (uint8_t)((x ^ y) + 1);
        }
    return rgb;
}

static void check_case(const char *name, int ww, int wh, int fb_w, int fb_h,
                       int exp_w, int exp_h, int exp_x0, int exp_y0)
{
    uint8_t *out = calloc((size_t)ww * wh, 4u);
    uint8_t *rgb = make_source(fb_w, fb_h);
    HostWindow_ScaleBGRA(out, ww, wh, rgb, fb_w, fb_h);

    /* Aspect ratio is preserved: content rect matches the analytically
     * expected size/position for this window/source combination. */
    CHECK(exp_x0 >= 0 && exp_y0 >= 0 && exp_x0 + exp_w <= ww && exp_y0 + exp_h <= wh,
          "%s: bad expected rect", name);

    /* Border pixels outside the content rect are black (letterbox/pillarbox
     * bars), when a border exists on that axis. */
    if (exp_x0 > 0) CHECK(is_black(out, ww, 0, wh / 2), "%s: left border not black", name);
    if (exp_x0 + exp_w < ww) CHECK(is_black(out, ww, ww - 1, wh / 2), "%s: right border not black", name);
    if (exp_y0 > 0) CHECK(is_black(out, ww, ww / 2, 0), "%s: top border not black", name);
    if (exp_y0 + exp_h < wh) CHECK(is_black(out, ww, ww / 2, wh - 1), "%s: bottom border not black", name);

    /* HostWindow_ScaleBGRA writes dst as {src[2], src[1], src[0], 0} — i.e.
     * out[0]=(sx^sy)+1, out[1]=sy+1, out[2]=sx+1 for the source pixel it
     * nearest-neighbor-sampled (see make_source's +1-offset fill, which
     * keeps (0,0,0,0) reserved for the border). */
    {
        const uint8_t *p0 = out + ((size_t)exp_y0 * ww + exp_x0) * 4u;
        CHECK(p0[3] == 0, "%s: content pixel alpha byte must be 0 (fixed by HostWindow_ScaleBGRA)", name);
        CHECK(p0[0] == 1 && p0[1] == 1 && p0[2] == 1,
              "%s: top-left content pixel BGR=%u,%u,%u want 1,1,1 (source 0,0)", name, p0[0], p0[1], p0[2]);
    }
    /* The last content pixel samples source (sx,sy) via the same
     * centre-sampled nearest neighbour HostWindow_ScaleBGRA uses;
     * that only lands exactly on (fb_w-1,fb_h-1) when the content rect
     * is an exact (integer-ratio) multiple of the source, which is not
     * the case when e.g. downscaling to a non-divisor size. */
    {
        const uint8_t *pn = out + ((size_t)(exp_y0 + exp_h - 1) * ww + (exp_x0 + exp_w - 1)) * 4u;
        int sx = HostWindow_NearestIndex(exp_w - 1, HostWindow_NearestStep(fb_w, exp_w));
        int sy = HostWindow_NearestIndex(exp_h - 1, HostWindow_NearestStep(fb_h, exp_h));
        uint8_t expect_b = (uint8_t)((sx ^ sy) + 1);
        uint8_t expect_g = (uint8_t)(sy + 1);
        uint8_t expect_r = (uint8_t)(sx + 1);
        CHECK(pn[0] == expect_b && pn[1] == expect_g && pn[2] == expect_r,
              "%s: bottom-right content BGR=%u,%u,%u want %u,%u,%u (source %d,%d)",
              name, pn[0], pn[1], pn[2], expect_b, expect_g, expect_r, sx, sy);
    }

    free(out); free(rgb);
    printf("  %s: %dx%d window, %dx%d source -> content %dx%d at (%d,%d): %s\n",
           name, ww, wh, fb_w, fb_h, exp_w, exp_h, exp_x0, exp_y0,
           g_fail ? "FAIL" : "ok");
}

/* An integer scale k maps destination pixel i to source i/k exactly (so
 * --scale N windows show the same pixels as plain truncating division),
 * and every mapping stays inside the source. */
static void check_mapping(void)
{
    static const int sizes[][2] = {{320,960},{320,1600},{240,720},{240,1200},
                                   {320,2133},{240,1600},{320,1066},{320,160}};
    for (size_t n = 0; n < sizeof(sizes) / sizeof(sizes[0]); n++) {
        int src = sizes[n][0], dst = sizes[n][1], k = dst % src == 0 ? dst / src : 0;
        int32_t step = HostWindow_NearestStep(src, dst);
        for (int i = 0; i < dst; i++) {
            int s = HostWindow_NearestIndex(i, step);
            CHECK(s >= 0 && s < src, "map %d->%d: index %d -> %d out of range", src, dst, i, s);
            if (k) CHECK(s == i / k, "map %d->%d: integer scale index %d -> %d want %d", src, dst, i, s, i / k);
        }
    }
    printf("  nearest mapping: in range, integer scales exact: %s\n", g_fail ? "FAIL" : "ok");
}

int main(void)
{
    check_mapping();

    /* Exact same aspect, no scale change: no borders. */
    check_case("exact-320x240", 320, 240, 320, 240, 320, 240, 0, 0);

    /* Exact 2x/3x integer multiples of 4:3: no borders (the --scale N path
     * this test also covers, since --scale just sets the window size to
     * fb*N with the same aspect ratio). */
    check_case("integer-2x", 640, 480, 320, 240, 640, 480, 0, 0);
    check_case("integer-3x", 960, 720, 320, 240, 960, 720, 0, 0);

    /* Wider-than-4:3 window (e.g. a 16:9 fullscreen desktop/handheld
     * display, or an ultrawide gamescope output): height-limited,
     * pillarboxed (black bars left/right). */
    check_case("wide-800x300", 800, 300, 320, 240, 400, 300, 200, 0);
    check_case("wide-1920x1080", 1920, 1080, 320, 240, 1440, 1080, 240, 0);
    check_case("wide-1920x1200", 1920, 1200, 320, 240, 1600, 1200, 160, 0);
    check_case("wide-2560x1600", 2560, 1600, 320, 240, 2133, 1600, 213, 0);

    /* Narrower-than-4:3 window (e.g. a portrait/rotated handheld or a
     * squeezed gamescope surface): width-limited, letterboxed (black bars
     * top/bottom). */
    check_case("tall-300x800", 300, 800, 320, 240, 300, 225, 0, 287);

    /* Downscaling still preserves aspect (a small gamescope preview
     * surface, or a window shrunk below the framebuffer size). */
    check_case("downscale-160x120", 160, 120, 320, 240, 160, 120, 0, 0);

    if (g_fail) { fprintf(stderr, "FAIL: letterbox/pillarbox scale math\n"); return 1; }
    printf("PASS: letterbox/pillarbox scale math (aspect preserved, black bars, nearest-neighbor sampling)\n");
    return 0;
}

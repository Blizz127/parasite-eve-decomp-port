/* Exercise production blit/dedup with a fake X sink, without a display. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
static atomic_int fail_point;
static int inject_failure(int point)
{
    int expected = point;
    return atomic_compare_exchange_strong(&fail_point, &expected, 0);
}
#define HOST_UPSCALE_FAIL(point) inject_failure(point)
#include "../platform/host_window.c"
static int requested_mode, put_count, reject_image;
int pe_enh_display_scaler(void) { return requested_mode; }
static int fake_put(void *d, Drawable w, GC gc, XImage *im,
                    int sx, int sy, int dx, int dy, unsigned width, unsigned height)
{
    (void)d; (void)w; (void)gc; (void)im; (void)sx; (void)sy;
    (void)dx; (void)dy; (void)width; (void)height;
    put_count++; return 0;
}
static int fake_flush(void *d) { (void)d; return 0; }
static XImage *fake_create(void *d, void *v, unsigned depth, int fmt, int off,
                           char *data, unsigned w, unsigned h, int pad, int stride)
{
    (void)d; (void)v; (void)depth; (void)fmt; (void)off; (void)data;
    (void)w; (void)h; (void)pad; (void)stride;
    return reject_image ? NULL : (XImage *)1;
}
static int fake_destroy(XImage *image) { (void)image; free(g_imgbuf); return 0; }
static void setup(void)
{
    HostUpscale_Shutdown();
    atomic_store(&fail_point, 0);
    free(g_last_rgb); g_last_rgb = NULL; g_last_size = 0;
    free(g_imgbuf); g_imgbuf = calloc(64u * 48u, 4u); assert(g_imgbuf);
    g_ximg = (XImage *)1; g_dpy = (void *)1; g_xr_on = 0;
    g_img_w = g_win_w = 64; g_img_h = g_win_h = 48;
    g_blit_dirty = 1; g_scaler_mode_last = HOST_UPSCALE_NEAREST;
    requested_mode = HOST_UPSCALE_FSR1; put_count = 0; reject_image = 0;
    XPutImage = fake_put; XFlush = fake_flush;
    XCreateImage = fake_create; XDestroyImage = fake_destroy;
}
int main(void)
{
    uint8_t rgb[16 * 12 * 3], expected[64 * 48 * 4];
    for (unsigned i = 0; i < sizeof rgb; i++) rgb[i] = (uint8_t)(i * 13u);
    setenv("PE_SCALER_THREADS", "1", 1);
    setup();
    atomic_store(&fail_point, HU_FAIL_ASYNC_START);
    assert(!HostUpscale_AsyncAvailable());
    atomic_store(&fail_point, HU_FAIL_SHARP_TABLE);
    HostWindow_Blit(rgb, 16, 12);
    assert(atomic_load(&fail_point) == 0 && g_blit_dirty && !g_last_rgb);
    HostWindow_ScaleBGRA(expected, 64, 48, rgb, 16, 12);
    assert(memcmp(expected, g_imgbuf, sizeof expected) == 0);
    HostWindow_Blit(rgb, 16, 12);
    assert(!g_blit_dirty && put_count == 2);
    assert(!HostUpscale_AsyncAvailable());
    assert(HostUpscale_Run(HOST_UPSCALE_SHARP_BILINEAR, expected, 64, 48, rgb, 16, 12)
           == HOST_UPSCALE_SHARP_BILINEAR);
    assert(memcmp(expected, g_imgbuf, sizeof expected) == 0);
    assert(!hu_async.started); /* no synchronous FSR or worker startup retry */
    HostWindow_Blit(rgb, 16, 12); assert(put_count == 2);

    /* FSR scratch failure must precede any successful FSR capacity growth. */
    const int failures[] = {HU_FAIL_ASYNC_FILTER, HU_FAIL_ASYNC_PENDING,
                            HU_FAIL_ASYNC_SOURCE, HU_FAIL_ASYNC_WORK};
    for (unsigned i = 0; i < sizeof failures / sizeof failures[0]; i++) {
        setup();
        atomic_store(&fail_point, failures[i]);
        HostWindow_Blit(rgb, 16, 12);
        HostUpscale_Drain();
        assert(atomic_load(&fail_point) == 0); /* actually reached the injected failure */
        /* The identical static frame must retry after the failed accepted job. */
        HostWindow_Blit(rgb, 16, 12);
        HostUpscale_Drain();
        HostWindow_Blit(rgb, 16, 12);
        HostUpscale_Drain();
        HostWindow_Blit(rgb, 16, 12);
        assert(put_count > 0);
        assert(HostUpscale_Run(HOST_UPSCALE_FSR1, expected, 64, 48, rgb, 16, 12)
               == HOST_UPSCALE_FSR1);
        assert(memcmp(expected, g_imgbuf, sizeof expected) == 0);
        assert(!HostUpscale_TakeFailure());
    }
    setup();
    reject_image = 1; g_img_w = 0;
    HostWindow_Blit(rgb, 16, 12);
    assert(g_blit_dirty && put_count == 0 && !g_last_rgb);
    reject_image = 0;
    HostWindow_Blit(rgb, 16, 12);
    HostUpscale_Drain();
    HostWindow_Blit(rgb, 16, 12);
    assert(put_count == 1 && !g_blit_dirty);
    assert(memcmp(expected, g_imgbuf, sizeof expected) == 0);
    HostUpscale_Shutdown();
    free(g_last_rgb); free(g_imgbuf);
    g_last_rgb = g_imgbuf = NULL;
    return 0;
}

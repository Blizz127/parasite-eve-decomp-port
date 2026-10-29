/* The real presenter (host_window.c's HostWindow_Blit) with the WINDOW FIT and
 * DISPLAY FILTER options, against a fake X sink, without a display.  Like
 * test_window_scaler_failures.c it includes host_window.c itself and replaces
 * the Xlib/XRender entry points, so every present goes through the production
 * wiring: the retail letterbox at the defaults, the planned rectangle and the
 * filter otherwise, on the XRender path, the synchronous CPU path and the
 * asynchronous filtered-scaler path.  The pure pieces have golden hashes in
 * the native suite (tests/test_enh_display.h). */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/host_window.c"

#define FW 16
#define FH 12

static int req_scaler, req_fit, req_crt;
int pe_enh_display_scaler(void) { return req_scaler; }
int pe_enh_display_fit(void) { return req_fit; }
int pe_enh_display_filter(void) { return req_crt; }

/* PAUSE ON FOCUS LOSS: the pad, menu, cheat and audio calls HostWindow_Poll and the pacers make are replaced by
 * recorders, and HostPad_Poll (the first thing each HostWindow_Poll does) is the clock of the scripted X events. */
static int req_pause, polls, focusin_after, close_after, ff_calls, ff_last, stop_calls;
static int ff_resume_pacer_started = -1;
static uint64_t ff_resume_pacer_deadline = 1, ff_resume_vblank_ns, pause_began_ns;
static XEvent queued[4];
static int queued_n, queued_i;
int pe_enh_pause_on_focus_loss(void) { return req_pause; }
static void queue_focus(int type, int mode, int detail)
{
    XEvent e;
    memset(&e, 0, sizeof e);
    e.xfocus.type = type; e.xfocus.mode = mode; e.xfocus.detail = detail;
    queued[queued_n++] = e;
}
void HostPad_Poll(void)
{
    polls++;
    if (polls == focusin_after) queue_focus(FocusIn, 0, 3);
    if (polls == close_after) {
        XEvent e;
        memset(&e, 0, sizeof e);
        e.xclient.type = ClientMessage; e.xclient.data[0] = (long)g_wm_delete;
        queued[queued_n++] = e;
    }
}
void HostDevMenu_Poll(void) {}
int HostDevMenu_CapturesInput(void) { return 0; }
uint8_t HostPad_Hotkeys(void) { return 0; }
int pe_plat_cheats_menu_open(void) { return 0; }
int pe_plat_cheats_key(PeCheatKey key, int pressed) { (void)key; (void)pressed; return 0; }
int pe_plat_cheats_ff_multiplier(void) { return 0; }
const HostPadRemap *HostPadRemap_Global(void) { return NULL; }
uint16_t HostPadRemap_Key(const HostPadRemap *r, unsigned long ks) { (void)r; (void)ks; return 0; }
void HostAudio_SetFastForward(int on)
{
    ff_calls++; ff_last = on;
    if (ff_calls == 2) {   /* the second call of a pause is the resume; it comes after the pacer restart */
        ff_resume_pacer_started = g_pacer.started; ff_resume_pacer_deadline = g_pacer.deadline_ns; ff_resume_vblank_ns = g_last_vblank_ns;
    }
}
int HostAudio_QueueFrames(void) { return 0; }
void PE_Port_RequestStop(PEPortStopReason reason) { (void)reason; stop_calls++; }
uint32_t PE_GPU_VSyncQuery(void) { return 0; }
uint32_t HostFB_ScanOutCount(void) { return 0; }
void HostFB_GetState(int *a, int *b, int *c, int *d) { if (a) *a = 0; if (b) *b = 0; if (c) *c = 0; if (d) *d = 0; }
void PE_StrFeed_GetStats(PeStrFeedStats *out) { memset(out, 0, sizeof *out); }
int PE_StrFeed_Active(void) { return 0; }
static int fake_pending(void *d) { (void)d; return queued_i < queued_n; }
static int fake_next(void *d, XEvent *e) { (void)d; *e = queued[queued_i++]; return 0; }

static int put_win, put_pix, comp_n, fill_n, xform_n;
static int comp_x, comp_y, comp_w, comp_h, xform_x, xform_y;
static uint8_t *captured;
static const Window WIN = 7;

static int fake_put(void *d, Drawable w, GC gc, XImage *im, int sx, int sy, int dx, int dy, unsigned width, unsigned height)
{
    (void)d; (void)gc; (void)im; (void)sx; (void)sy; (void)dx; (void)dy; (void)width; (void)height;
    if (w == WIN) {
        put_win++;
        memcpy(captured, g_imgbuf, (size_t)g_win_w * (size_t)g_win_h * 4u);
    } else put_pix++;
    return 0;
}
static int fake_flush(void *d) { (void)d; return 0; }
static XImage *fake_create(void *d, void *v, unsigned depth, int fmt, int off, char *data, unsigned w, unsigned h, int pad, int stride)
{
    (void)d; (void)v; (void)depth; (void)fmt; (void)off; (void)data; (void)w; (void)h; (void)pad; (void)stride;
    return (XImage *)1;
}
static int fake_destroy(XImage *image) { (void)image; free(g_imgbuf); return 0; }
static void fake_transform(void *d, Picture p, XTransform *t) { (void)d; (void)p; xform_n++; xform_x = t->matrix[0][0]; xform_y = t->matrix[1][1]; }
static void fake_fill(void *d, int op, Picture p, const XRenderColor *c, int x, int y, unsigned w, unsigned h)
{ (void)d; (void)op; (void)p; (void)c; (void)x; (void)y; (void)w; (void)h; fill_n++; }
static void fake_composite(void *d, int op, Picture s, Picture m, Picture dst, int sx, int sy, int mx, int my, int dx, int dy, unsigned w, unsigned h)
{ (void)d; (void)op; (void)s; (void)m; (void)dst; (void)sx; (void)sy; (void)mx; (void)my; comp_n++; comp_x = dx; comp_y = dy; comp_w = (int)w; comp_h = (int)h; }

static void setup(int ww, int wh, int scaler, int fit, int crt, int xr)
{
    HostUpscale_Shutdown();
    free(g_last_rgb); g_last_rgb = NULL; g_last_size = 0;
    free(g_imgbuf); g_imgbuf = calloc((size_t)ww * (size_t)wh, 4u); assert(g_imgbuf);
    free(captured); captured = calloc((size_t)ww * (size_t)wh, 4u); assert(captured);
    free(g_box); g_box = NULL; g_box_cap = 0;
    g_ximg = (XImage *)1; g_dpy = (void *)1; g_win = WIN;
    g_img_w = g_win_w = ww; g_img_h = g_win_h = wh;
    g_fb_w = FW; g_fb_h = FH;
    g_blit_dirty = 1; g_scaler_mode_last = req_scaler = scaler; req_fit = fit; req_crt = crt;
    g_fit_last = fit; g_filter_last = crt;
    g_xr_on = xr; g_xr_win_w = g_xr_win_h = 0;
    free(g_xr_buf); g_xr_buf = NULL;
    if (xr) {
        g_xr_buf = calloc((size_t)FW * FH, 4u); assert(g_xr_buf);
        g_xr_fb_w = FW; g_xr_fb_h = FH; g_xr_pix = 42; g_xr_gc = 43; g_xr_src = 44; g_xr_dst = 45; g_xr_img = (XImage *)1;
    }
    put_win = put_pix = comp_n = fill_n = xform_n = 0;
    XPutImage = fake_put; XFlush = fake_flush; XCreateImage = fake_create; XDestroyImage = fake_destroy;
    XRenderSetPictureTransform = fake_transform; XRenderFillRectangle = fake_fill; XRenderComposite = fake_composite;
}

/* The window buffer the documented pipeline gives: scaler at the planned geometry, then the filter. */
static void expected_window(uint8_t *exp, int ww, int wh, const uint8_t *rgb, int scaler, int fit, int crt)
{
    HostFilterPlan p;
    HostFilter_Plan(&p, fit, ww, wh, FW, FH, FW, FH);
    if (scaler == HOST_UPSCALE_NEAREST) {
        if (p.direct) HostWindow_ScaleBGRA(exp, ww, wh, rgb, FW, FH);
        else HostFilter_ScaleNearestRect(exp, ww, wh, rgb, FW, FH, &p.rect);
    } else if (p.direct) {
        assert(HostUpscale_Run(scaler, exp, ww, wh, rgb, FW, FH) == scaler);
    } else {
        uint8_t *box = malloc((size_t)p.bw * (size_t)p.bh * 4u);
        assert(box);
        assert(HostUpscale_Run(scaler, box, p.bw, p.bh, rgb, FW, FH) == scaler);
        HostFilter_Place(exp, ww, wh, box, p.bw, p.bh, &p.rect);
        free(box);
    }
    HostFilter_ApplyCrt(exp, ww, wh, &p.rect, FH, crt);
}

int main(void)
{
    uint8_t rgb[FW * FH * 3];
    static const int fits[] = {HOST_FIT_KEEP, HOST_FIT_INTEGER, HOST_FIT_STRETCH};
    static const int crts[] = {HOST_CRT_OFF, HOST_CRT_SCANLINES, HOST_CRT_MASK};
    const int WW = 90, WH = 70;                       /* not 4:3: every fit gives a different rectangle */
    size_t bytes = (size_t)WW * (size_t)WH * 4u;
    uint8_t *exp = malloc(bytes);
    for (unsigned i = 0; i < sizeof rgb; i++) rgb[i] = (uint8_t)(i * 13u + 5u);
    setenv("PE_SCALER_THREADS", "1", 1);
    assert(exp);

    /* The three fits really differ at this window. */
    {
        HostFilterPlan k, n, s;
        HostFilter_Plan(&k, HOST_FIT_KEEP, WW, WH, FW, FH, FW, FH);
        HostFilter_Plan(&n, HOST_FIT_INTEGER, WW, WH, FW, FH, FW, FH);
        HostFilter_Plan(&s, HOST_FIT_STRETCH, WW, WH, FW, FH, FW, FH);
        assert(k.direct && !n.direct && !s.direct);
        assert(k.rect.w == 90 && k.rect.h == 67 && n.rect.w == 80 && n.rect.h == 60 && s.rect.w == 90 && s.rect.h == 70);
    }

    /* 1. Synchronous CPU path, nearest: every fit x filter equals the documented pipeline; the defaults equal
     *    HostWindow_ScaleBGRA, i.e. the retail bytes. */
    for (unsigned f = 0; f < 3; f++) for (unsigned c = 0; c < 3; c++) {
        setup(WW, WH, HOST_UPSCALE_NEAREST, fits[f], crts[c], 0);
        HostWindow_Blit(rgb, FW, FH);
        assert(put_win == 1 && !g_blit_dirty);
        expected_window(exp, WW, WH, rgb, HOST_UPSCALE_NEAREST, fits[f], crts[c]);
        assert(memcmp(exp, captured, bytes) == 0);
        if (!f && !c) {
            uint8_t *retail = malloc(bytes);
            assert(retail);
            HostWindow_ScaleBGRA(retail, WW, WH, rgb, FW, FH);
            assert(memcmp(retail, captured, bytes) == 0);        /* defaults: the retail path, byte for byte */
            free(retail);
        } else if (f == 1 && !c) {                               /* INTEGER: a 5x block, centred (5,5) in 90x70 */
            const uint8_t *px = captured + ((size_t)(5 + 5 * 3 + 1) * WW + (size_t)(5 + 5 * 4 + 2)) * 4u;
            const uint8_t *src = rgb + ((size_t)3 * FW + 4) * 3u;
            assert(px[0] == src[2] && px[1] == src[1] && px[2] == src[0]);
            assert(*(const uint32_t *)captured == 0u);
        }
    }

    /* 2. A changed option redraws an unchanged frame; an unchanged one does not. */
    setup(WW, WH, HOST_UPSCALE_NEAREST, HOST_FIT_KEEP, HOST_CRT_OFF, 0);
    HostWindow_Blit(rgb, FW, FH); assert(put_win == 1);
    HostWindow_Blit(rgb, FW, FH); assert(put_win == 1);          /* same frame, same options: skipped */
    req_crt = HOST_CRT_SCANLINES;
    HostWindow_Blit(rgb, FW, FH); assert(put_win == 2);          /* the filter changed: redrawn */
    expected_window(exp, WW, WH, rgb, HOST_UPSCALE_NEAREST, HOST_FIT_KEEP, HOST_CRT_SCANLINES);
    assert(memcmp(exp, captured, bytes) == 0);
    HostWindow_Blit(rgb, FW, FH); assert(put_win == 2);
    req_fit = HOST_FIT_STRETCH;
    HostWindow_Blit(rgb, FW, FH); assert(put_win == 3);          /* the fit changed: redrawn */
    expected_window(exp, WW, WH, rgb, HOST_UPSCALE_NEAREST, HOST_FIT_STRETCH, HOST_CRT_SCANLINES);
    assert(memcmp(exp, captured, bytes) == 0);
    req_crt = HOST_CRT_OFF; req_fit = HOST_FIT_KEEP;
    HostWindow_Blit(rgb, FW, FH); assert(put_win == 4);          /* and back: the retail bytes again */
    {
        uint8_t *retail = malloc(bytes);
        assert(retail);
        HostWindow_ScaleBGRA(retail, WW, WH, rgb, FW, FH);
        assert(memcmp(retail, captured, bytes) == 0);
        free(retail);
    }

    /* 3. XRender path (nearest, filter off): the server composites into the planned rectangle with the
     *    nearest step of that rectangle; with the filter on the XRender path is not used. */
    for (unsigned f = 0; f < 3; f++) {
        HostFilterPlan p;
        setup(WW, WH, HOST_UPSCALE_NEAREST, fits[f], HOST_CRT_OFF, 1);
        HostFilter_Plan(&p, fits[f], WW, WH, FW, FH, FW, FH);
        HostWindow_Blit(rgb, FW, FH);
        assert(comp_n == 1 && put_win == 0 && put_pix == 1 && xform_n == 1);
        assert(comp_x == p.rect.x0 && comp_y == p.rect.y0 && comp_w == p.rect.w && comp_h == p.rect.h);
        assert(xform_x == HostWindow_NearestStep(FW, p.rect.w) && xform_y == HostWindow_NearestStep(FH, p.rect.h));
        if (!f) {   /* the retail letterbox */
            int x0, y0, w, h;
            HostWindow_FitRect(WW, WH, FW, FH, &x0, &y0, &w, &h);
            assert(comp_x == x0 && comp_y == y0 && comp_w == w && comp_h == h);
        }
        /* a fit change rebuilds the transform even at the same window size */
        req_fit = fits[(f + 1) % 3];
        HostFilter_Plan(&p, req_fit, WW, WH, FW, FH, FW, FH);
        HostWindow_Blit(rgb, FW, FH);
        assert(comp_n == 2 && comp_w == p.rect.w && comp_h == p.rect.h && xform_n == 2);
        assert(xform_x == HostWindow_NearestStep(FW, p.rect.w) && xform_y == HostWindow_NearestStep(FH, p.rect.h));
    }
    for (unsigned c = 1; c < 3; c++) {
        setup(WW, WH, HOST_UPSCALE_NEAREST, HOST_FIT_INTEGER, crts[c], 1);
        HostWindow_Blit(rgb, FW, FH);
        assert(comp_n == 0 && put_pix == 0 && put_win == 1);     /* the CPU path, not XRender */
        expected_window(exp, WW, WH, rgb, HOST_UPSCALE_NEAREST, HOST_FIT_INTEGER, crts[c]);
        assert(memcmp(exp, captured, bytes) == 0);
        req_crt = HOST_CRT_OFF;                                  /* turn it off: back to XRender */
        HostWindow_Blit(rgb, FW, FH);
        assert(comp_n == 1 && put_pix == 1);
    }

    /* 4. Asynchronous filtered scaler (sharp bilinear on the presenter thread): the box, the placement and the
     *    filter, at every fit.  The first present queues the frame, the third picks the image up. */
    for (unsigned f = 0; f < 3; f++) for (unsigned c = 0; c < 3; c += 2) {
        setup(WW, WH, HOST_UPSCALE_SHARP_BILINEAR, fits[f], crts[c], 0);
        HostWindow_Blit(rgb, FW, FH);
        HostUpscale_Drain();
        HostWindow_Blit(rgb, FW, FH);
        assert(put_win == 1);
        expected_window(exp, WW, WH, rgb, HOST_UPSCALE_SHARP_BILINEAR, fits[f], crts[c]);
        assert(memcmp(exp, captured, bytes) == 0);
        /* a fit/filter change drops the image of the old geometry and presents the new one */
        req_fit = fits[(f + 1) % 3]; req_crt = crts[(c + 1) % 3];
        HostWindow_Blit(rgb, FW, FH);
        HostUpscale_Drain();
        HostWindow_Blit(rgb, FW, FH);
        assert(put_win == 2);
        expected_window(exp, WW, WH, rgb, HOST_UPSCALE_SHARP_BILINEAR, req_fit, req_crt);
        assert(memcmp(exp, captured, bytes) == 0);
    }

    /* 5. PAUSE ON FOCUS LOSS through the real pacer entry points.  The pacer paces to 59.94 Hz, so each call costs
     *    up to one frame of wall time; nothing here depends on how long a sleep actually takes. */
    XPending = fake_pending; XNextEvent = fake_next;
    g_wm_delete = 99;
    {
        struct timespec t0;
        uint64_t before;
        g_fullscreen = 0;
        /* 5a. option OFF: a focus loss is tracked and nothing waits */
        setup(WW, WH, HOST_UPSCALE_NEAREST, HOST_FIT_KEEP, HOST_CRT_OFF, 0);
        HostFocus_Reset(&g_focus); req_pause = 0; polls = 0; focusin_after = close_after = -1; ff_calls = stop_calls = 0;
        queued_n = queued_i = 0; queue_focus(FocusOut, 0, 3);
        assert(HostWindow_Pace() == 0 && g_focus.focused == 0 && ff_calls == 0);
        before = (uint64_t)polls;
        assert(HostWindow_Pace() == 0 && ff_calls == 0 && (uint64_t)polls > before);        /* a paced frame polls; nothing is muted or held */
        /* a grab flicker and a child/pointer focus change are ignored */
        HostFocus_Reset(&g_focus); queued_n = queued_i = 0;
        queue_focus(FocusOut, 1, 3); queue_focus(FocusOut, 2, 0); queue_focus(FocusOut, 0, 2); queue_focus(FocusOut, 0, 5);
        queue_focus(FocusOut, 0, 6); queue_focus(FocusOut, 0, 7);
        assert(HostWindow_Pace() == 0 && g_focus.focused == 1);
        queue_focus(FocusOut, 0, 0); queue_focus(FocusIn, 1, 0);
        assert(HostWindow_Pace() == 0 && g_focus.focused == 0);                       /* a real loss; the grab FocusIn is ignored */
        queue_focus(FocusIn, 0, 0);
        assert(HostWindow_Pace() == 0 && g_focus.focused == 1);

        /* 5b. option ON, focus lost: the pacer holds until FocusIn, mutes meanwhile, restarts the pacer on the way out */
        setup(WW, WH, HOST_UPSCALE_NEAREST, HOST_FIT_KEEP, HOST_CRT_OFF, 0);
        HostFocus_Reset(&g_focus); req_pause = 1; polls = 0; ff_calls = stop_calls = 0; ff_last = -1;
        queued_n = queued_i = 0; queue_focus(FocusOut, 0, 3);
        g_fast_forward = g_ff_active = 1;                                              /* fast-forward was on: its mute is restored */
        assert(HostWindow_Pace() == 0 && g_focus.focused == 0);                       /* g_ff_active: a poll only; the loss is seen */
        assert(ff_calls == 0);
        polls = 0; focusin_after = 6; ff_resume_pacer_started = -1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        pause_began_ns = (uint64_t)t0.tv_sec * 1000000000ull + (uint64_t)t0.tv_nsec;
        assert(HostWindow_Pace() == 0);
        assert(polls >= 6 && g_focus.focused == 1);                                    /* it polled until the FocusIn arrived */
        assert(ff_calls == 2 && ff_last == 1);                                         /* muted, then back to the ff state it had (on) */
        assert(ff_resume_pacer_started == 0 && ff_resume_pacer_deadline == 0);        /* the pacer restarted before the audio was restored */
        assert(ff_resume_vblank_ns >= pause_began_ns);                                 /* and the present safety net got a fresh stamp */
        g_fast_forward = g_ff_active = 0;

        /* the same with pace_vblank (the emulated-VBlank pacer), fast-forward off */
        HostFocus_Reset(&g_focus); polls = 0; ff_calls = 0; ff_last = -1; focusin_after = -1;
        queued_n = queued_i = 0; queue_focus(FocusOut, 0, 3);
        pace_vblank();                                                                 /* sees the loss while pacing this VBlank */
        assert(g_focus.focused == 0 && ff_calls == 0 && stop_calls == 0);
        polls = 0; focusin_after = 4; queued_n = queued_i = 0;
        pace_vblank();                                                                 /* holds here until the FocusIn */
        assert(polls >= 4 && g_focus.focused == 1 && ff_calls == 2 && ff_last == 0 && stop_calls == 0);

        /* 5c. option ON but focused: never waits; option OFF with focus lost: never waits (5a) */
        HostFocus_Reset(&g_focus); polls = 0; focusin_after = -1; ff_calls = 0;
        pace_vblank();
        assert(ff_calls == 0 && polls >= 1);

        /* 5d. a window close while paused ends the wait and stops the game */
        HostFocus_Reset(&g_focus); g_close_requested = 0; polls = 0; ff_calls = stop_calls = 0; ff_last = -1;
        focusin_after = -1; close_after = -1;
        queued_n = queued_i = 0; queue_focus(FocusOut, 0, 3);
        pace_vblank();
        assert(g_focus.focused == 0);
        polls = 0; close_after = 5; queued_n = queued_i = 0;
        pace_vblank();
        assert(g_close_requested == 1 && stop_calls == 1 && polls >= 5);              /* ended by the close request, not by focus */
        assert(ff_calls == 1 && ff_last == 1);                                         /* muted; no resume on a close */
        g_close_requested = 0;
        /* HostWindow_Pace reports the close too */
        HostFocus_Reset(&g_focus); polls = 0; close_after = -1; queued_n = queued_i = 0; queue_focus(FocusOut, 0, 3);
        assert(HostWindow_Pace() == 0 && g_focus.focused == 0);
        polls = 0; close_after = 4; queued_n = queued_i = 0;
        assert(HostWindow_Pace() == 1 && g_close_requested == 1);
        g_close_requested = 0; req_pause = 0; focusin_after = close_after = -1;
        HostFocus_Reset(&g_focus);
    }

    HostUpscale_Shutdown();
    free(g_last_rgb); g_last_rgb = NULL;
    free(g_imgbuf); g_imgbuf = NULL;
    free(g_box); g_box = NULL;
    free(g_xr_buf); g_xr_buf = NULL;
    free(captured); free(exp);
    puts("enh-window: PASS");
    return 0;
}

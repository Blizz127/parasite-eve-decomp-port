/*
 * Phase 6B — X11 window via dlopen with event loop.
 *
 * Key symbols we need (XK_Escape = 9):
 * We don't include X11/keysymdef.h, so define the one we need.
 */
#include "host_window.h"
#include "host_framebuffer.h"
#include "host_frame_pacer.h"
#include "host_window_scale.h"
#include "host_pad.h"
#include "host_audio.h"
#include "pe_audio_driver.h"
#include "game_port.h"
#include "pe_plat/cheats.h"
#include "pe_gpu.h"
#include "pe_str_feed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <time.h>

typedef unsigned long KeySym;
#define XK_Escape 0xFF1B
#define XK_Return 0xFF0D
#define XK_space  0x0020
#define XK_Left   0xFF51
#define XK_Up     0xFF52
#define XK_Right  0xFF53
#define XK_Down   0xFF54
#define XK_z      0x007A
#define XK_x      0x0078
#define XK_Z      0x005A
#define XK_X      0x0058
#define XK_F6     0xFFC3
#define XK_F1     0xFFBE
#define XK_F8     0xFFC5
#define KeyPress    2
#define KeyRelease  3
#define ButtonPress 4
#define Expose     12
#define ConfigureNotify 22
#define FocusOut    10
#define ClientMessage 33
#define NoEventMask 0
#define KeyPressMask    (1L<<0)
#define ExposureMask    (1L<<15)
#define StructureNotifyMask (1L<<17)
#define FocusChangeMask (1L<<21)
#define SubstructureNotifyMask (1L<<19)
#define SubstructureRedirectMask (1L<<20)
#define PropModeReplace 0
#define XA_ATOM 4
/* EWMH _NET_WM_STATE source-indication values (wm-spec section 4.3). */
#define NET_WM_STATE_ADD 1

/* ── Xlib types ─────────────────────────────────────────────────────── */
typedef unsigned long XID;
typedef XID Window;
typedef XID Drawable;
typedef XID GC;
typedef unsigned long Atom;
typedef unsigned long Time;

typedef struct { int type; unsigned long serial; int send_event; void *display; Window window; } XAnyEvent;
typedef struct { int type; unsigned long serial; int send_event; void *display; Window window;
                 Window root, subwindow; Time time; int x, y, x_root, y_root;
                 unsigned state; unsigned keycode; int same_screen; } XKeyEvent;
typedef struct { int type; unsigned long serial; int send_event; void *display; Window window;
                 int x, y, width, height, count; } XExposeEvent;
typedef struct { int type; unsigned long serial; int send_event; void *display; Window window;
                 Atom message_type; int format; long data[5]; } XClientMessageEvent;
typedef struct { int type; unsigned long serial; int send_event; void *display; Window event, window;
                 int x, y, width, height, border_width; Window above; int override_redirect; } XConfigureEvent;
typedef union { int type; XConfigureEvent xconfigure; XAnyEvent xany; XKeyEvent xkey; XExposeEvent xexpose; XClientMessageEvent xclient; long pad[24]; } XEvent;

typedef struct { void *ext_data; int depth, bits_per_pixel, scanline_pad; } XImage;
typedef XID Pixmap;
typedef XID Picture;
typedef int XFixed;
typedef struct { XFixed matrix[3][3]; } XTransform;
typedef struct { unsigned short red, green, blue, alpha; } XRenderColor;
#define PictStandardRGB24 1
#define PictOpSrc 1

/* ── Function pointers from libX11 ──────────────────────────────────── */
static void *g_xlib = NULL;
static void *(*XOpenDisplay)(const char *);
static int   (*XCloseDisplay)(void *);
static int   (*XDefaultScreen)(void *);
static Window (*XDefaultRootWindow)(void *);
static Window (*XCreateSimpleWindow)(void *, Window, int, int, unsigned, unsigned, unsigned, unsigned, unsigned);
static int   (*XMapWindow)(void *, Window);
static int   (*XStoreName)(void *, Window, const char *);
static GC     (*XCreateGC)(void *, Drawable, unsigned long, void *);
static int   (*XSelectInput)(void *, Window, long);
static int   (*XNextEvent)(void *, XEvent *);
static int   (*XPeekEvent)(void *, XEvent *);
static int   (*XPending)(void *);
static KeySym (*XLookupKeysym)(XKeyEvent *, int);
static XImage *(*XCreateImage)(void *, void *, unsigned, int, int, char *, unsigned, unsigned, int, int);
static int   (*XDestroyImage)(XImage *);
static int   (*XPutImage)(void *, Drawable, GC, XImage *, int, int, int, int, unsigned, unsigned);
static int   (*XFlush)(void *);
static int   (*XDestroyWindow)(void *, Window);
static Atom  (*XInternAtom)(void *, const char *, int);
static int   (*XSetWMProtocols)(void *, Window, Atom *, int);
static int   (*XSendEvent)(void *, Window, int, long, XEvent *);
static int   (*XDisplayWidth)(void *, int);
static int   (*XDisplayHeight)(void *, int);
static int   (*XChangeProperty)(void *, Window, Atom, Atom, int, int,
                                 const unsigned char *, int);
static void *(*XDefaultVisual)(void *, int);
static int   (*XDefaultDepth)(void *, int);
static Pixmap (*XCreatePixmap)(void *, Drawable, unsigned, unsigned, unsigned);
static int   (*XFreePixmap)(void *, Pixmap);
static int   (*XFreeGC)(void *, GC);

/* ── Optional libXrender: server-side scaling ─────────────────────────
 * The 320x240 frame is uploaded once into a pixmap and the X server
 * (Xorg/Xwayland/gamescope, usually on the GPU) scales it into the
 * letterbox with a nearest filter, instead of the client scaling to the
 * full panel and pushing ~16 MB per blit through the socket. */
static void *g_xrender = NULL;
static int   (*XRenderQueryExtension)(void *, int *, int *);
static void *(*XRenderFindVisualFormat)(void *, const void *);
static void *(*XRenderFindStandardFormat)(void *, int);
static Picture (*XRenderCreatePicture)(void *, Drawable, const void *, unsigned long, const void *);
static void  (*XRenderFreePicture)(void *, Picture);
static void  (*XRenderSetPictureTransform)(void *, Picture, XTransform *);
static void  (*XRenderSetPictureFilter)(void *, Picture, const char *, XFixed *, int);
static void  (*XRenderComposite)(void *, int, Picture, Picture, Picture, int, int, int, int,
                                 int, int, unsigned, unsigned);
static void  (*XRenderFillRectangle)(void *, int, Picture, const XRenderColor *, int, int,
                                     unsigned, unsigned);

static void *g_dpy = NULL;
static Window g_win = 0;
static GC g_gc = 0;
static XImage *g_ximg = NULL;
static uint8_t *g_imgbuf = NULL;
static int g_win_w = 0, g_win_h = 0;
static int g_fb_w = 0, g_fb_h = 0;
static Atom g_wm_delete = 0;
static int g_close_requested = 0;
static int g_fullscreen = 0;
/* XRender path state (g_xr_on = 0: CPU scale of the full window). */
static int g_xr_on;
static Pixmap g_xr_pix;
static GC g_xr_gc;
static Picture g_xr_src, g_xr_dst;
static XImage *g_xr_img;
static uint8_t *g_xr_buf;
static int g_xr_fb_w, g_xr_fb_h, g_xr_win_w, g_xr_win_h;
/* Blit dedup: the last frame put on screen, and whether the window needs
 * a redraw regardless (open, resize, Expose). */
static uint8_t *g_last_rgb;
static size_t g_last_size;
static int g_blit_dirty;

int g_host_window_open = 0;
static uint16_t g_sony_held = 0;
static HostFramePacer g_pacer;
/* Fast-forward: F6 or controller L3 toggles g_fast_forward; controller R3
 * fast-forwards while held.  g_ff_active is the effective state. */
static int g_fast_forward, g_speed_key_held, g_ff_active;
static uint8_t g_pad_hotkeys;
/* Pacing is per emulated VBlank (see pe_audio_driver.h).  Presents only
 * poll input, unless a scene presents without advancing VBlanks. */
static int g_vblank_paced;
static void pace_vblank(void);
static uint64_t g_last_vblank_ns;
static char g_title[512];
static int monotonic_ns(uint64_t *out);

/* PE_FRAME_TRACE=1 (diagnostics, off by default): one [FTRACE] line per
 * wall second with presents, emulated VBlanks, host busy time per VBlank
 * (wall time outside the pacer's sleep, p50/p95/max), window blits and
 * their cost, idle-VBlank scan-outs, FMV frames published and the audio
 * device queue.  PE_FRAME_TRACE_MAX caps the line count (default 600). */
#define FTRACE_SAMPLES 512
static struct {
    int on, max, lines;
    uint64_t t0;
    uint32_t vbl0, scan0, fmv0; int pres0;
    uint32_t paced, blits, skipped;
    uint64_t blit_ns, blit_max;
    uint32_t n; uint64_t busy[FTRACE_SAMPLES];
} g_ft;

static void ftrace_init(void)
{
    const char *e = getenv("PE_FRAME_TRACE"), *m = getenv("PE_FRAME_TRACE_MAX");
    memset(&g_ft, 0, sizeof(g_ft));
    g_ft.on = e && *e && *e != '0';
    g_ft.max = m && atoi(m) > 0 ? atoi(m) : 600;
}

static int ftrace_cmp(const void *a, const void *b)
{
    uint64_t x = *(const uint64_t *)a, y = *(const uint64_t *)b;
    return x < y ? -1 : x > y;
}

static void ftrace_sample(uint64_t now)
{
    PeStrFeedStats fs;
    int pres = 0;
    uint32_t vbl = PE_GPU_VSyncQuery(), scan = HostFB_ScanOutCount();
    HostFB_GetState(NULL, NULL, &pres, NULL);
    PE_StrFeed_GetStats(&fs);
    if (g_ft.t0 && now - g_ft.t0 >= UINT64_C(1000000000) && g_ft.lines < g_ft.max) {
        double p50 = 0, p95 = 0, mx = 0;
        if (g_ft.n) {
            qsort(g_ft.busy, g_ft.n, sizeof(g_ft.busy[0]), ftrace_cmp);
            p50 = g_ft.busy[g_ft.n / 2] / 1e6;
            p95 = g_ft.busy[(g_ft.n * 95u) / 100u] / 1e6;
            mx = g_ft.busy[g_ft.n - 1] / 1e6;
        }
        fprintf(stderr, "[FTRACE] %.3fs pres=%d vbl=%u paced=%u busy_ms p50=%.2f p95=%.2f max=%.2f "
                "blits=%u skip=%u blit_ms=%.1f blit_max=%.2f scan=%u fmv=%u stream=%d aq=%d win=%dx%d\n",
                (now - g_ft.t0) / 1e9, pres - g_ft.pres0, vbl - g_ft.vbl0, g_ft.paced, p50, p95, mx,
                g_ft.blits, g_ft.skipped, g_ft.blit_ns / 1e6, g_ft.blit_max / 1e6,
                scan - g_ft.scan0,
                fs.frames_published >= g_ft.fmv0 ? fs.frames_published - g_ft.fmv0 : fs.frames_published, /* a new stream restarts the count */
                PE_StrFeed_Active(),
                HostAudio_QueueFrames(), g_win_w, g_win_h);
        g_ft.lines++;
    } else if (g_ft.t0 && now - g_ft.t0 < UINT64_C(1000000000)) return;
    g_ft.t0 = now; g_ft.vbl0 = vbl; g_ft.scan0 = scan; g_ft.pres0 = pres;
    g_ft.fmv0 = fs.frames_published;
    g_ft.paced = g_ft.blits = g_ft.skipped = 0; g_ft.blit_ns = g_ft.blit_max = 0; g_ft.n = 0;
}

static void *xlib_sym(const char *name) {
    void *p = dlsym(g_xlib, name);
    if (!p) { fprintf(stderr, "[WINDOW] missing symbol: %s\n", name); exit(1); }
    return p;
}

static void bind_all(void) {
    XOpenDisplay        = xlib_sym("XOpenDisplay");
    XCloseDisplay       = xlib_sym("XCloseDisplay");
    XDefaultScreen      = xlib_sym("XDefaultScreen");
    XDefaultRootWindow  = xlib_sym("XDefaultRootWindow");
    XCreateSimpleWindow = xlib_sym("XCreateSimpleWindow");
    XMapWindow          = xlib_sym("XMapWindow");
    XStoreName          = xlib_sym("XStoreName");
    XCreateGC           = xlib_sym("XCreateGC");
    XSelectInput        = xlib_sym("XSelectInput");
    XNextEvent          = xlib_sym("XNextEvent");
    XPeekEvent          = xlib_sym("XPeekEvent");
    XPending            = xlib_sym("XPending");
    XLookupKeysym       = xlib_sym("XLookupKeysym");
    XCreateImage        = xlib_sym("XCreateImage");
    XDestroyImage       = xlib_sym("XDestroyImage");
    XPutImage           = xlib_sym("XPutImage");
    XFlush              = xlib_sym("XFlush");
    XDestroyWindow      = xlib_sym("XDestroyWindow");
    XInternAtom         = xlib_sym("XInternAtom");
    XSetWMProtocols     = xlib_sym("XSetWMProtocols");
    XSendEvent          = xlib_sym("XSendEvent");
    XDisplayWidth       = xlib_sym("XDisplayWidth");
    XDisplayHeight      = xlib_sym("XDisplayHeight");
    XChangeProperty     = xlib_sym("XChangeProperty");
    XDefaultVisual      = xlib_sym("XDefaultVisual");
    XDefaultDepth       = xlib_sym("XDefaultDepth");
    XCreatePixmap       = xlib_sym("XCreatePixmap");
    XFreePixmap         = xlib_sym("XFreePixmap");
    XFreeGC             = xlib_sym("XFreeGC");
}

static void xr_release(void)
{
    if (g_xr_img) XDestroyImage(g_xr_img); /* owns g_xr_buf */
    g_xr_img = NULL; g_xr_buf = NULL;
    if (g_dpy) {
        if (g_xr_src) XRenderFreePicture(g_dpy, g_xr_src);
        if (g_xr_dst) XRenderFreePicture(g_dpy, g_xr_dst);
        if (g_xr_pix) XFreePixmap(g_dpy, g_xr_pix);
        if (g_xr_gc) XFreeGC(g_dpy, g_xr_gc);
    }
    g_xr_src = g_xr_dst = 0; g_xr_pix = 0; g_xr_gc = 0;
    g_xr_on = 0; g_xr_win_w = g_xr_win_h = 0;
}

/* Bind libXrender and build the source pixmap/picture pair.  Any missing
 * piece leaves g_xr_on = 0 and the CPU scaler in use.  PE_WINDOW_SCALER=cpu
 * forces the CPU path. */
static const char *g_xr_why = "";  /* why the CPU scaler is in use */

static void xr_setup(int screen)
{
    const char *want = getenv("PE_WINDOW_SCALER");
    int ev, er;
    void *vis_fmt, *rgb_fmt;
    uint8_t *buf;
    if (want && strcmp(want, "cpu") == 0) { g_xr_why = " (PE_WINDOW_SCALER=cpu)"; return; }
    if (XDefaultDepth(g_dpy, screen) != 24) { g_xr_why = " (screen depth is not 24)"; return; }
    g_xrender = dlopen("libXrender.so.1", RTLD_LAZY);
    if (!g_xrender) { g_xr_why = " (libXrender.so.1 not found)"; return; }
    g_xr_why = " (XRender setup failed)";
#define XR_BIND(f) if (!(*(void **)&f = dlsym(g_xrender, #f))) goto fail
    XR_BIND(XRenderQueryExtension); XR_BIND(XRenderFindVisualFormat);
    XR_BIND(XRenderFindStandardFormat); XR_BIND(XRenderCreatePicture);
    XR_BIND(XRenderFreePicture); XR_BIND(XRenderSetPictureTransform);
    XR_BIND(XRenderSetPictureFilter); XR_BIND(XRenderComposite);
    XR_BIND(XRenderFillRectangle);
#undef XR_BIND
    if (!XRenderQueryExtension(g_dpy, &ev, &er)) { g_xr_why = " (no RENDER extension)"; goto fail; }
    vis_fmt = XRenderFindVisualFormat(g_dpy, XDefaultVisual(g_dpy, screen));
    rgb_fmt = XRenderFindStandardFormat(g_dpy, PictStandardRGB24);
    if (!vis_fmt || !rgb_fmt) goto fail;
    buf = calloc((size_t)g_fb_w * (size_t)g_fb_h, 4u);
    if (!buf) goto fail;
    g_xr_img = XCreateImage(g_dpy, NULL, 24, 2, 0, (char *)buf, g_fb_w, g_fb_h, 32, 0);
    if (!g_xr_img) { free(buf); goto fail; }
    g_xr_buf = buf; g_xr_fb_w = g_fb_w; g_xr_fb_h = g_fb_h;
    g_xr_pix = XCreatePixmap(g_dpy, g_win, g_fb_w, g_fb_h, 24);
    g_xr_gc = XCreateGC(g_dpy, g_xr_pix, 0, NULL);
    g_xr_src = XRenderCreatePicture(g_dpy, g_xr_pix, rgb_fmt, 0, NULL);
    g_xr_dst = XRenderCreatePicture(g_dpy, g_win, vis_fmt, 0, NULL);
    if (!g_xr_pix || !g_xr_gc || !g_xr_src || !g_xr_dst) goto fail;
    XRenderSetPictureFilter(g_dpy, g_xr_src, "nearest", NULL, 0);
    g_xr_on = 1; g_xr_why = "";
    return;
fail:
    xr_release(); /* libXrender stays loaded: HostWindow_Close unloads it */
}

/* Upload the frame and let the server scale it into the letterbox. */
static void xr_blit(const uint8_t *rgb)
{
    int x0, y0, w, h, i;
    if (g_win_w != g_xr_win_w || g_win_h != g_xr_win_h) {
        XTransform t;
        HostWindow_FitRect(g_win_w, g_win_h, g_xr_fb_w, g_xr_fb_h, &x0, &y0, &w, &h);
        memset(&t, 0, sizeof(t));
        /* destination -> source, the same 16.16 step the CPU scaler samples
         * with (HostWindow_NearestIndex), so both paths are pixel-identical */
        t.matrix[0][0] = HostWindow_NearestStep(g_xr_fb_w, w);
        t.matrix[1][1] = HostWindow_NearestStep(g_xr_fb_h, h);
        t.matrix[2][2] = 1 << 16;
        XRenderSetPictureTransform(g_dpy, g_xr_src, &t);
        g_xr_win_w = g_win_w; g_xr_win_h = g_win_h;
    }
    HostWindow_FitRect(g_win_w, g_win_h, g_xr_fb_w, g_xr_fb_h, &x0, &y0, &w, &h);
    for (i = 0; i < g_xr_fb_w * g_xr_fb_h; i++) {
        g_xr_buf[i * 4 + 0] = rgb[i * 3 + 2];
        g_xr_buf[i * 4 + 1] = rgb[i * 3 + 1];
        g_xr_buf[i * 4 + 2] = rgb[i * 3 + 0];
        g_xr_buf[i * 4 + 3] = 0;
    }
    XPutImage(g_dpy, g_xr_pix, g_xr_gc, g_xr_img, 0, 0, 0, 0, g_xr_fb_w, g_xr_fb_h);
    if (g_blit_dirty) {   /* black bars around the letterbox */
        static const XRenderColor black = {0, 0, 0, 0xFFFF};
        if (y0 > 0) XRenderFillRectangle(g_dpy, PictOpSrc, g_xr_dst, &black, 0, 0, g_win_w, y0);
        if (y0 + h < g_win_h)
            XRenderFillRectangle(g_dpy, PictOpSrc, g_xr_dst, &black, 0, y0 + h, g_win_w, g_win_h - y0 - h);
        if (x0 > 0) XRenderFillRectangle(g_dpy, PictOpSrc, g_xr_dst, &black, 0, y0, x0, h);
        if (x0 + w < g_win_w)
            XRenderFillRectangle(g_dpy, PictOpSrc, g_xr_dst, &black, x0 + w, y0, g_win_w - x0 - w, h);
    }
    XRenderComposite(g_dpy, PictOpSrc, g_xr_src, 0, g_xr_dst, 0, 0, 0, 0, x0, y0, w, h);
}

/* Ask the window manager (or gamescope/Steam Game Mode) to make g_win
 * fullscreen per the EWMH wm-spec (_NET_WM_STATE_FULLSCREEN):
 *  - the property is set before mapping so WMs that read it at map time
 *    apply fullscreen without an intermediate windowed frame;
 *  - the ClientMessage is sent (to the root window, after mapping) as
 *    the spec-mandated fallback for WMs that only honour the message.
 * Under gamescope the compositor forces fullscreen regardless and this
 * is a no-op that does no harm; the resulting ConfigureNotify is handled
 * generically by resize_image() in HostWindow_Poll(). */
static void request_fullscreen(int mapped)
{
    Atom state = XInternAtom(g_dpy, "_NET_WM_STATE", 0);
    Atom fullscreen = XInternAtom(g_dpy, "_NET_WM_STATE_FULLSCREEN", 0);
    if (!mapped) {
        XChangeProperty(g_dpy, g_win, state, XA_ATOM, 32, PropModeReplace,
                         (const unsigned char *)&fullscreen, 1);
        return;
    }
    XEvent xev;
    memset(&xev, 0, sizeof(xev));
    xev.xclient.type = ClientMessage;
    xev.xclient.window = g_win;
    xev.xclient.message_type = state;
    xev.xclient.format = 32;
    xev.xclient.data[0] = NET_WM_STATE_ADD;
    xev.xclient.data[1] = (long)fullscreen;
    xev.xclient.data[2] = 0;
    xev.xclient.data[3] = 1; /* source indication: normal application */
    XSendEvent(g_dpy, XDefaultRootWindow(g_dpy), 0,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

static int resize_image(int width, int height)
{
    if (width <= 0 || height <= 0) return 0;
    if (g_xr_on) {   /* the server scales; no window-sized client buffer */
        if (width != g_win_w || height != g_win_h) g_blit_dirty = 1;
        g_win_w = width; g_win_h = height;
        return 0;
    }
    if (g_ximg && width == g_win_w && height == g_win_h) return 0;
    g_blit_dirty = 1;
    uint8_t *buffer = calloc((size_t)width * (size_t)height, 4u);
    if (!buffer) return -1;
    XImage *image = XCreateImage(g_dpy, NULL, 24, 2, 0, (char *)buffer,
                                 width, height, 32, 0);
    if (!image) { free(buffer); return -1; }
    if (g_ximg) XDestroyImage(g_ximg); /* owns its pixel buffer */
    g_ximg = image; g_imgbuf = buffer;
    g_win_w = width; g_win_h = height;
    return 0;
}

int HostWindow_Open(const char *display, int width, int height,
                    const char *title, int scale, int fullscreen)
{
    g_xlib = dlopen("libX11.so.6", RTLD_LAZY);
    if (!g_xlib) { fprintf(stderr, "[WINDOW] libX11.so.6 not found\n"); return -1; }
    bind_all();

    g_dpy = XOpenDisplay(display);
    if (!g_dpy) { fprintf(stderr, "[WINDOW] cannot open display %s\n", display); return -1; }

    int screen = XDefaultScreen(g_dpy);
    Window root = XDefaultRootWindow(g_dpy);
    /* Use scale to determine visible size: framebuffer is 320×240, scale × that */
    g_fb_w = PE_PORT_FB_WIDTH;
    g_fb_h = PE_PORT_FB_HEIGHT;
    g_fullscreen = fullscreen != 0;
    if (g_fullscreen) {
        /* Size to the screen; HostWindow_Blit/ScaleBGRA letterbox the
         * 320x240 source into whatever the WM/compositor gives us, and
         * ConfigureNotify (gamescope forcing fullscreen, or a live
         * resize) rescales via resize_image() below. */
        g_win_w = XDisplayWidth(g_dpy, screen);
        g_win_h = XDisplayHeight(g_dpy, screen);
    } else {
        g_win_w = width > 0 ? width : g_fb_w * scale;
        g_win_h = height > 0 ? height : g_fb_h * scale;
    }

    g_win = XCreateSimpleWindow(g_dpy, root, g_fullscreen ? 0 : 200,
                                 g_fullscreen ? 0 : 200, g_win_w, g_win_h,
                                 g_fullscreen ? 0 : 4, 0xFFFFFF, 0x000000);
    g_fast_forward=0;g_speed_key_held=0;g_ff_active=0;g_pad_hotkeys=0;
    g_vblank_paced=0;g_last_vblank_ns=0;
    ftrace_init();
    HostWindow_SetTitle(title);
    XSelectInput(g_dpy, g_win, ExposureMask | KeyPressMask | (1L<<1) | StructureNotifyMask | FocusChangeMask);
    g_wm_delete = XInternAtom(g_dpy, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(g_dpy, g_win, &g_wm_delete, 1);
    if (g_fullscreen) request_fullscreen(0); /* property, before map */
    XMapWindow(g_dpy, g_win);
    if (g_fullscreen) request_fullscreen(1); /* ClientMessage, after map */

    g_gc = XCreateGC(g_dpy, g_win, 0, NULL);
    xr_setup(screen);
    if (resize_image(g_win_w, g_win_h) != 0) { HostWindow_Close(); return -1; }
    g_blit_dirty = 1;

    g_close_requested = 0;
    g_sony_held = 0;
    memset(&g_pacer,0,sizeof(g_pacer));
    g_host_window_open = 1;
    PE_AudioDriver_SetVBlankPacer(pace_vblank);

    XFlush(g_dpy);
    fprintf(stderr, "[WINDOW] %dx%d (fb %dx%d) on %s title='%s'%s scaler=%s%s\n",
            g_win_w, g_win_h, g_fb_w, g_fb_h, display, title,
            g_fullscreen ? " fullscreen" : "", g_xr_on ? "xrender" : "cpu", g_xr_why);
    return 0;
}

void HostWindow_Blit(const uint8_t *rgb, int fb_w, int fb_h)
{
    uint64_t t0 = 0, t1;
    size_t size = (size_t)fb_w * (size_t)fb_h * 3u;
    if (!g_dpy || !rgb || fb_w <= 0 || fb_h <= 0) return;
    if (!g_xr_on && (!g_ximg || !g_imgbuf)) return;
    /* A present or scan-out that shows exactly the pixels already on
     * screen (a movie's catch-up presents, the static pause frame
     * rescanned every VBlank) changes nothing visible: skip it unless the
     * window itself needs a redraw. */
    if (!g_blit_dirty && g_last_rgb && g_last_size == size && memcmp(g_last_rgb, rgb, size) == 0) {
        if (g_ft.on) g_ft.skipped++;
        return;
    }
    if (g_ft.on) (void)monotonic_ns(&t0);
    if (g_xr_on) {
        if (fb_w != g_xr_fb_w || fb_h != g_xr_fb_h) return; /* fb is fixed 320x240 */
        xr_blit(rgb);
    } else {
        HostWindow_ScaleBGRA(g_imgbuf, g_win_w, g_win_h, rgb, fb_w, fb_h);
        XPutImage(g_dpy, g_win, g_gc, g_ximg, 0, 0, 0, 0, g_win_w, g_win_h);
    }
    XFlush(g_dpy);
    g_blit_dirty = 0;
    if (g_last_size != size) {
        uint8_t *p = realloc(g_last_rgb, size);
        if (p) { g_last_rgb = p; g_last_size = size; }
    }
    if (g_last_size == size) memcpy(g_last_rgb, rgb, size);
    if (g_ft.on && monotonic_ns(&t1)) {
        g_ft.blits++; g_ft.blit_ns += t1 - t0;
        if (t1 - t0 > g_ft.blit_max) g_ft.blit_max = t1 - t0;
    }
}

void HostWindow_SetTitle(const char *title)
{
    char label[560];
    if (title) snprintf(g_title,sizeof(g_title),"%s",title);
    snprintf(label,sizeof(label),"%s [%s]",g_title,g_ff_active?"Fast-forward":"Normal speed");
    if (g_dpy && g_win) { XStoreName(g_dpy, g_win, label); XFlush(g_dpy); }
}

/* Recompute the effective fast-forward state; on a change reset the pacer,
 * retitle, and mute/unmute the audio device (no chipmunk audio). */
static void ff_update(const char *why)
{
    int eff=g_fast_forward||(g_pad_hotkeys&HOST_PAD_HOTKEY_FF_HOLD)!=0;
    if (eff==g_ff_active) return;
    g_ff_active=eff;
    memset(&g_pacer,0,sizeof(g_pacer));
    HostWindow_SetTitle(NULL);
    HostAudio_SetFastForward(eff);
    fprintf(stderr,"[WINDOW] %s (%s; F6/L3 toggle, hold R3)\n",
            eff?"Fast-forward, audio muted":"Normal speed: 59.94 Hz",why);
}

int HostWindow_Poll(void)
{
    if (!g_dpy) return 0;
    HostPad_Poll();
    {
        uint8_t hk=HostPad_Hotkeys();
        if ((hk&HOST_PAD_HOTKEY_FF_TOGGLE)&&!(g_pad_hotkeys&HOST_PAD_HOTKEY_FF_TOGGLE))
            g_fast_forward=!g_fast_forward;
        g_pad_hotkeys=hk;
        ff_update("controller");
    }
    while (XPending(g_dpy)) {
        XEvent ev;
        XNextEvent(g_dpy, &ev);
        if (ev.type == Expose) {
            /* Redraw: the caller's next blit repaints (dedup bypassed). */
            g_blit_dirty = 1;
        } else if (ev.type == ConfigureNotify) {
            if (resize_image(ev.xconfigure.width, ev.xconfigure.height) != 0) {
                fprintf(stderr, "[WINDOW] resize allocation failed\n");
                g_close_requested = 1;
                return 1;
            }
        } else if (ev.type == FocusOut) {
            g_sony_held = 0;
            g_speed_key_held = 0;
        } else if (ev.type == KeyPress || ev.type == KeyRelease) {
            KeySym ks = XLookupKeysym(&ev.xkey, 0);
            uint16_t bit = 0;
            {
                /* Cheat console (pe_plat/cheats.h): F1-F5, F7, F8; arrows and
                 * Enter only while its warp menu is open. */
                PeCheatKey ck = PE_CHEAT_KEY_NONE;
                static uint16_t cheat_held;
                if (ks >= XK_F1 && ks <= XK_F8 && ks != XK_F6) {
                    static const PeCheatKey fk[8] = {PE_CHEAT_KEY_F1, PE_CHEAT_KEY_F2,
                        PE_CHEAT_KEY_F3, PE_CHEAT_KEY_F4, PE_CHEAT_KEY_F5, PE_CHEAT_KEY_NONE,
                        PE_CHEAT_KEY_F7, PE_CHEAT_KEY_F8};
                    ck = fk[ks - XK_F1];
                } else if (pe_plat_cheats_menu_open()) {
                    if (ks == XK_Up) ck = PE_CHEAT_KEY_UP;
                    else if (ks == XK_Down) ck = PE_CHEAT_KEY_DOWN;
                    else if (ks == XK_Return) ck = PE_CHEAT_KEY_ENTER;
                }
                if (ck != PE_CHEAT_KEY_NONE) {
                    uint16_t bitk = (uint16_t)(1u << ck);
                    if (ev.type==KeyRelease && XPending(g_dpy)) {
                        XEvent next;XPeekEvent(g_dpy,&next);
                        if (next.type==KeyPress && next.xkey.keycode==ev.xkey.keycode &&
                            next.xkey.time==ev.xkey.time) continue;   /* auto-repeat */
                    }
                    if (ev.type == KeyPress) {
                        int was = (cheat_held & bitk) != 0;
                        cheat_held |= bitk;
                        if (was && ck != PE_CHEAT_KEY_UP && ck != PE_CHEAT_KEY_DOWN) continue;
                    } else {
                        cheat_held &= (uint16_t)~bitk;
                    }
                    if (pe_plat_cheats_key(ck, ev.type == KeyPress)) continue;
                }
            }
            if (ks == XK_F6) {
                /* X11 auto-repeat may synthesize release/press pairs. */
                if (ev.type==KeyRelease && XPending(g_dpy)) {
                    XEvent next;XPeekEvent(g_dpy,&next);
                    if (next.type==KeyPress && next.xkey.keycode==ev.xkey.keycode &&
                        next.xkey.time==ev.xkey.time) continue;
                }
                if (ev.type==KeyPress && !g_speed_key_held) {
                    g_fast_forward=!g_fast_forward;
                    ff_update("F6");
                }
                g_speed_key_held=ev.type==KeyPress;
                continue;
            }
            if (ks == XK_Escape) { g_close_requested = 1; return 1; }
            if (ks == XK_Return || ks == XK_space || ks == XK_z ||
                ks == XK_x || ks == XK_Z || ks == XK_X)
                bit = 0x4000u;
            else if (ks == XK_Up)
                bit = 0x0010u;
            else if (ks == XK_Right)
                bit = 0x0020u;
            else if (ks == XK_Down)
                bit = 0x0040u;
            else if (ks == XK_Left)
                bit = 0x0080u;
            else if (ks == 'c' || ks == 'C')
                bit = 0x2000u; /* Circle */
            else if (ks == 'v' || ks == 'V')
                bit = 0x1000u; /* Triangle */
            else if (ks == 's' || ks == 'S')
                bit = 0x8000u; /* Square */
            else if (ks == 'q' || ks == 'Q')
                bit = 0x0400u; /* L1 */
            else if (ks == 'e' || ks == 'E')
                bit = 0x0800u; /* R1 */
            else if (ks == '1')
                bit = 0x0100u; /* L2 */
            else if (ks == '3')
                bit = 0x0200u; /* R2 */
            else if (ks == 0xFF09u)
                bit = 0x0001u; /* Tab: Select */
            else if (ks == 'p' || ks == 'P')
                bit = 0x0008u; /* Start */
            if (bit != 0u) {
                if (ev.type == KeyPress)
                    g_sony_held |= bit;
                else
                    g_sony_held &= (uint16_t)~bit;
            }
        } else if (ev.type == ClientMessage) {
            if ((Atom)ev.xclient.data[0] == g_wm_delete) { g_close_requested = 1; return 1; }
        }
    }
    return g_close_requested ? 1 : 0;
}

uint16_t HostWindow_PadRaw(void)
{
    /* Keyboard and controller both drive the pad at once. */
    return (uint16_t)(~(g_sony_held | HostPad_Held()));
}

static int monotonic_ns(uint64_t *out)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC,&now)!=0) {perror("[WINDOW] monotonic clock");return 0;}
    *out=(uint64_t)now.tv_sec*UINT64_C(1000000000)+(uint64_t)now.tv_nsec;
    return 1;
}

/* Sleep to the next 59.94 Hz deadline, polling input.  Returns 1 on
 * close or a host clock failure. */
static int pace_one_frame(void)
{
    uint64_t now,deadline;
    if (!monotonic_ns(&now)) return 1;
    deadline=HostFramePacer_Deadline(&g_pacer,now);
    for (;;) {
        struct timespec delay;
        uint64_t remaining;
        if (HostWindow_Poll() || !monotonic_ns(&now)) return 1;
        if (g_ff_active && pe_plat_cheats_ff_multiplier() == 0) return 0;
        if (now>=deadline) return 0;
        remaining=deadline-now;
        delay.tv_sec=0;
        delay.tv_nsec=(long)(remaining>4000000u?4000000u:remaining);
        /* Recheck the absolute deadline after interruptions and oversleep. */
        (void)nanosleep(&delay,NULL);
    }
}

/* PeVBlankPacer: one emulated VBlank = one 59.94 Hz frame of wall time. */
static void pace_vblank(void)
{
    if (!g_dpy) return;
    g_vblank_paced=1;
    if (g_close_requested) return;
    if (g_ff_active) {
        /* Cheat console F7: fixed N-x speed pays one wall frame per N
         * emulated VBlanks; 0 = uncapped (the F6 default). */
        int mult = pe_plat_cheats_ff_multiplier();
        static unsigned n;
        if (mult > 0) {
            if ((++n % (unsigned)mult) == 0u && pace_one_frame())
                PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
            return;
        }
        /* uncapped, but keep input (and the fast-forward keys) live */
        if ((++n & 7u)==0u) (void)HostWindow_Poll();
        return;
    }
    if (g_ft.on) {
        uint64_t now;
        if (monotonic_ns(&now)) {
            if (g_last_vblank_ns && g_ft.n < FTRACE_SAMPLES) g_ft.busy[g_ft.n++] = now - g_last_vblank_ns;
            g_ft.paced++;
        }
    }
    if (pace_one_frame()) PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
    (void)monotonic_ns(&g_last_vblank_ns);
    if (g_ft.on) ftrace_sample(g_last_vblank_ns);
}

int HostWindow_Pace(void)
{
    if (g_ff_active) return HostWindow_Poll();
    /* Emulated VBlanks pace the game; a present only polls (catch-up
     * presents such as pe_str_feed's cost no time).  Safety net: if no
     * VBlank has paced for 100 ms of wall time, a scene is presenting
     * without VBlanks and presents are paced one frame each. */
    if (g_vblank_paced) {
        uint64_t now;
        if (monotonic_ns(&now) && now-g_last_vblank_ns<UINT64_C(100000000))
            return HostWindow_Poll();
    }
    return pace_one_frame();
}

int HostWindow_Run(int milliseconds)
{
    if (!g_dpy) return 0;
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        int rc = HostWindow_Poll();
        if (rc) return rc;

        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / 1000000;
        if (milliseconds > 0 && elapsed >= milliseconds) return 0;

        usleep(16000); /* ~60 Hz poll */
    }
}

void HostWindow_Close(void)
{
    PE_AudioDriver_SetVBlankPacer(NULL);
    xr_release();
    free(g_last_rgb); g_last_rgb = NULL; g_last_size = 0;
    if (g_ximg) XDestroyImage(g_ximg);
    g_imgbuf = NULL; g_ximg = NULL;
    g_gc = 0; g_win = 0;
    if (g_dpy) { XCloseDisplay(g_dpy); g_dpy = NULL; }
    /* libXrender hooks XCloseDisplay for its extension data: unload it
     * only after the display is gone. */
    if (g_xrender) { dlclose(g_xrender); g_xrender = NULL; }
    if (g_xlib) { dlclose(g_xlib); g_xlib = NULL; }
    g_host_window_open = 0;
    g_sony_held = 0;
    HostPad_Close();
}

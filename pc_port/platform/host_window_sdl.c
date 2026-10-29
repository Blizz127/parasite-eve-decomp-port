/*
 * SDL2 host window backend (PE_HOST_BACKEND=sdl2; always on macOS).
 *
 * Implements platform/host_window.h like host_window.c (X11) and
 * host_window_win32.c.  SDL2 is dlopened at run time, so the executable has
 * no link-time SDL dependency and needs no SDL headers: the few SDL2 ABI
 * types used here are spelled out below (SDL 2.0.x keeps them stable).
 *
 *   window      SDL_CreateWindow, resizable, HiDPI drawable; --fullscreen (or
 *               HostWindow_WantFullscreen: gamescope / Steam Game Mode) is
 *               SDL_WINDOW_FULLSCREEN_DESKTOP, a borderless window covering
 *               the desktop, with the 320x240 frame letterboxed into it.
 *   presenter   the X11 presenter's choices: DISPLAY SCALER NEAREST (the
 *               retail default) uploads the 320x240 frame into a texture and
 *               lets the renderer scale it into the letterbox (the analogue
 *               of the X11 XRender path); PE_WINDOW_SCALER=cpu, or no
 *               usable texture, runs HostWindow_ScaleBGRA into a
 *               window-sized texture; SHARP BILINEAR / FSR 1 run
 *               host_upscale.h exactly as on X11 (presenter thread, one
 *               present of latency).  Unchanged frames are not re-presented.
 *   input       keyboard through the shared keysym table (host_pad_remap.c;
 *               SDL keycodes are translated to X11 keysyms, so controls.conf
 *               key names are the same), cheat console F1-F5/F7/F8, F6
 *               fast-forward, Esc quits; controllers through host_pad.c /
 *               host_pad_sdl.c and the dev-menu combo (host_dev_menu.c,
 *               host_pad_combo.c), unchanged.
 *   pacing      per emulated VBlank at 60000/1001 Hz, identical to X11.
 *
 * The SDL library loader (HostSDL_Library) is shared with host_audio_sdl.c.
 * Library search: PE_SDL2_LIB, then libSDL2-2.0.so.0 (Linux) or, on macOS,
 * libSDL2-2.0.0.dylib next to the executable / in ../Frameworks, the dyld
 * search path, Homebrew and MacPorts prefixes, then SDL2.framework.
 * docs/port/SDL_BACKEND.md describes the backend.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1   /* dladdr */
#endif
#include "host_window.h"
#include "host_framebuffer.h"
#include "host_frame_pacer.h"
#include "host_window_scale.h"
#include "host_upscale.h"
#include "host_png.h"
#include "host_qol.h"
#include "host_pad.h"
#include "host_pad_remap.h"
#include "host_pad_sdl.h"
#include "host_dev_menu.h"
#include "host_audio.h"
#include "pe_audio_driver.h"
#include "game_port.h"
#include "pe_plat/cheats.h"
#include "pe_plat/enhance.h"
#include "pe_gpu.h"
#include "pe_str_feed.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* ── SDL2 ABI subset (SDL.h is not needed to build) ─────────────────── */
#define HS_INIT_VIDEO            0x00000020u
#define HS_WINDOW_SHOWN          0x00000004u
#define HS_WINDOW_RESIZABLE      0x00000020u
#define HS_WINDOW_FULLSCREEN_DESKTOP 0x00001001u
#define HS_WINDOW_ALLOW_HIGHDPI  0x00002000u
#define HS_WINDOWPOS_CENTERED    0x2FFF0000
#define HS_RENDERER_SOFTWARE     0x00000001u
#define HS_RENDERER_ACCELERATED  0x00000002u
#define HS_PIXELFORMAT_XRGB8888  0x16161804u   /* SDL_PIXELFORMAT_RGB888: bytes B,G,R,X */
#define HS_TEXTUREACCESS_STREAMING 1
#define HS_BLENDMODE_NONE        0
#define HS_SCALEMODE_NEAREST     0
/* event types */
#define HS_QUIT                  0x100u
#define HS_WINDOWEVENT           0x200u
#define HS_KEYDOWN               0x300u
#define HS_KEYUP                 0x301u
#define HS_RENDER_TARGETS_RESET  0x2000u
#define HS_RENDER_DEVICE_RESET   0x2001u
/* SDL_WindowEventID */
#define HS_WE_EXPOSED            3
#define HS_WE_RESIZED            5
#define HS_WE_SIZE_CHANGED       6
#define HS_WE_FOCUS_LOST         13
#define HS_WE_CLOSE              14
/* SDL_Keycode: printable keys are their (layout) character; the rest are
 * SDLK_SCANCODE_MASK | SDL_Scancode. */
#define HS_K_SCANCODE_MASK       0x40000000

typedef struct { uint8_t major, minor, patch; } HsVersion;
typedef struct { int x, y, w, h; } HsRect;
typedef struct {
    const char *name; uint32_t flags; uint32_t num_texture_formats;
    uint32_t texture_formats[16]; int max_texture_width, max_texture_height;
} HsRendererInfo;
typedef struct { uint32_t type, timestamp, windowID; uint8_t event, pad1, pad2, pad3; int32_t data1, data2; } HsWindowEvent;
typedef struct { int32_t scancode; int32_t sym; uint16_t mod; uint32_t unused; } HsKeysym;
typedef struct { uint32_t type, timestamp, windowID; uint8_t state, repeat, pad2, pad3; HsKeysym keysym; } HsKeyboardEvent;
/* SDL_Event is a 56-byte union with pointer members; 64 bytes, pointer-aligned, holds it. */
typedef union {
    uint32_t type; HsWindowEvent window; HsKeyboardEvent key;
    void *align_ptr; uint64_t align_u64; uint8_t padding[64];
} HsEvent;
_Static_assert(sizeof(HsKeyboardEvent) == 32, "SDL_KeyboardEvent layout");
_Static_assert(sizeof(HsWindowEvent) == 24, "SDL_WindowEvent layout");
_Static_assert(sizeof(HsEvent) >= 56, "SDL_Event size");

/* ── shared SDL2 loader (also used by host_audio_sdl.c) ─────────────── */
void *HostSDL_Library(const char **loaded_from);

static void *g_sdl_lib;
static const char *g_sdl_from;
static char g_sdl_path[1024];

void *HostSDL_Library(const char **loaded_from)
{
    static int tried;
    if (!tried) {
        const char *env = getenv("PE_SDL2_LIB");
#ifdef __APPLE__
        static const char *const names[] = {
            "@executable_path/libSDL2-2.0.0.dylib",
            "@executable_path/../Frameworks/libSDL2-2.0.0.dylib",
            "@executable_path/../Frameworks/SDL2.framework/SDL2",
            "libSDL2-2.0.0.dylib",                       /* DYLD_* search path, /usr/local/lib */
            "/opt/homebrew/lib/libSDL2-2.0.0.dylib",     /* Homebrew, Apple silicon */
            "/usr/local/lib/libSDL2-2.0.0.dylib",        /* Homebrew, Intel */
            "/opt/local/lib/libSDL2-2.0.0.dylib",        /* MacPorts */
            "SDL2.framework/SDL2",                       /* DYLD_FRAMEWORK_PATH, ~/Library/Frameworks */
            "/Library/Frameworks/SDL2.framework/SDL2",
        };
#else
        static const char *const names[] = { "libSDL2-2.0.so.0" };
#endif
        tried = 1;
        if (env && *env) {
            g_sdl_lib = dlopen(env, RTLD_NOW | RTLD_LOCAL);
            if (g_sdl_lib) g_sdl_from = env;
        }
        for (size_t i = 0; !g_sdl_lib && i < sizeof(names) / sizeof(names[0]); i++) {
            g_sdl_lib = dlopen(names[i], RTLD_NOW | RTLD_LOCAL);
            if (g_sdl_lib) g_sdl_from = names[i];
        }
        if (g_sdl_lib) {
            Dl_info info;
            void *sym = dlsym(g_sdl_lib, "SDL_Init");
            snprintf(g_sdl_path, sizeof(g_sdl_path), "%s", g_sdl_from);
            if (sym && dladdr(sym, &info) && info.dli_fname && *info.dli_fname)
                snprintf(g_sdl_path, sizeof(g_sdl_path), "%s", info.dli_fname);
            /* host_pad_sdl.c opens "libSDL2-2.0.so.0" unless PE_SDL2_LIB names
             * another library: point it at this one (macOS), so the
             * controllers and the window share one SDL. */
            if (strcmp(g_sdl_from, "libSDL2-2.0.so.0") != 0 && !(env && *env))
                setenv("PE_SDL2_LIB", g_sdl_path, 0);
        } else {
            fprintf(stderr, "[SDL] SDL2 library not found (%s)%s\n", dlerror(),
                    env && *env ? " [PE_SDL2_LIB]" : "");
        }
    }
    if (loaded_from) *loaded_from = g_sdl_lib ? g_sdl_path : NULL;
    return g_sdl_lib;
}

/* The library stays loaded for the process: host_pad_sdl.c and
 * host_audio_sdl.c may still use it after HostWindow_Close. */
static struct {
    int bound;
    int (*InitSubSystem)(uint32_t);
    void (*QuitSubSystem)(uint32_t);
    int (*SetHint)(const char *, const char *);
    const char *(*GetError)(void);
    void (*GetVersion)(HsVersion *);
    const char *(*GetCurrentVideoDriver)(void);
    int (*GetDisplayBounds)(int, HsRect *);
    void *(*CreateWindow)(const char *, int, int, int, int, uint32_t);
    void (*DestroyWindow)(void *);
    void (*SetWindowTitle)(void *, const char *);
    void *(*CreateRenderer)(void *, int, uint32_t);
    void (*DestroyRenderer)(void *);
    int (*GetRendererInfo)(void *, HsRendererInfo *);
    int (*GetRendererOutputSize)(void *, int *, int *);
    void *(*CreateTexture)(void *, uint32_t, int, int, int);
    void (*DestroyTexture)(void *);
    int (*UpdateTexture)(void *, const HsRect *, const void *, int);
    int (*LockTexture)(void *, const HsRect *, void **, int *);
    void (*UnlockTexture)(void *);
    int (*SetTextureBlendMode)(void *, int);
    int (*SetWindowFullscreen)(void *, uint32_t);   /* F11: 0 = windowed, FULLSCREEN_DESKTOP = borderless */
    void (*GetWindowSize)(void *, int *, int *);    /* F11: the size each toggle lands on (log) */
    int (*SetRenderDrawColor)(void *, uint8_t, uint8_t, uint8_t, uint8_t);
    int (*RenderClear)(void *);
    int (*RenderCopy)(void *, void *, const HsRect *, const HsRect *);
    void (*RenderPresent)(void *);
    int (*PollEvent)(HsEvent *);
    /* optional (newer SDL 2.0.x) */
    int (*SetTextureScaleMode)(void *, int);
    void (*StopTextInput)(void);
} SDL;

static int hs_bind(void)
{
    void *lib;
    const char *miss = NULL;
    if (SDL.bound) return SDL.bound > 0;
    SDL.bound = -1;
    lib = HostSDL_Library(NULL);
    if (!lib) return 0;
#define HS_SYM(field, name) do { *(void **)&SDL.field = dlsym(lib, name); if (!SDL.field) { miss = name; goto fail; } } while (0)
    HS_SYM(InitSubSystem, "SDL_InitSubSystem");   HS_SYM(QuitSubSystem, "SDL_QuitSubSystem");
    HS_SYM(SetHint, "SDL_SetHint");               HS_SYM(GetError, "SDL_GetError");
    HS_SYM(GetVersion, "SDL_GetVersion");         HS_SYM(GetCurrentVideoDriver, "SDL_GetCurrentVideoDriver");
    HS_SYM(GetDisplayBounds, "SDL_GetDisplayBounds");
    HS_SYM(CreateWindow, "SDL_CreateWindow");     HS_SYM(DestroyWindow, "SDL_DestroyWindow");
    HS_SYM(SetWindowTitle, "SDL_SetWindowTitle");
    HS_SYM(CreateRenderer, "SDL_CreateRenderer"); HS_SYM(DestroyRenderer, "SDL_DestroyRenderer");
    HS_SYM(GetRendererInfo, "SDL_GetRendererInfo");
    HS_SYM(GetRendererOutputSize, "SDL_GetRendererOutputSize");
    HS_SYM(CreateTexture, "SDL_CreateTexture");   HS_SYM(DestroyTexture, "SDL_DestroyTexture");
    HS_SYM(UpdateTexture, "SDL_UpdateTexture");   HS_SYM(LockTexture, "SDL_LockTexture");
    HS_SYM(UnlockTexture, "SDL_UnlockTexture");   HS_SYM(SetTextureBlendMode, "SDL_SetTextureBlendMode");
    HS_SYM(SetWindowFullscreen, "SDL_SetWindowFullscreen"); HS_SYM(GetWindowSize, "SDL_GetWindowSize");
    HS_SYM(SetRenderDrawColor, "SDL_SetRenderDrawColor");
    HS_SYM(RenderClear, "SDL_RenderClear");       HS_SYM(RenderCopy, "SDL_RenderCopy");
    HS_SYM(RenderPresent, "SDL_RenderPresent");   HS_SYM(PollEvent, "SDL_PollEvent");
#undef HS_SYM
    *(void **)&SDL.SetTextureScaleMode = dlsym(lib, "SDL_SetTextureScaleMode");
    *(void **)&SDL.StopTextInput = dlsym(lib, "SDL_StopTextInput");
    SDL.bound = 1;
    return 1;
fail:
    fprintf(stderr, "[WINDOW] SDL2 missing symbol %s\n", miss);
    return 0;
}

/* ── window state ───────────────────────────────────────────────────── */
static void *g_win, *g_ren;
static void *g_tex_fb;                 /* fb-sized texture: renderer-scaled NEAREST */
static int g_tex_fb_w, g_tex_fb_h;
static void *g_tex_win;                /* window-sized texture: CPU / filtered scalers */
static int g_tex_win_w, g_tex_win_h;
static uint8_t *g_imgbuf;              /* window-sized BGRX client image */
static size_t g_imgbuf_cap;
static int g_img_w, g_img_h;
static int g_video_on;
static int g_win_w, g_win_h;           /* drawable size in pixels */
static int g_fb_w, g_fb_h;
static int g_close_requested;
static int g_fullscreen;
static int g_fs_now;                   /* F11: the fullscreen state asked for (starts at g_fullscreen) */
static uint8_t g_hot_held;             /* F11/F12 autorepeat guard (bit 0 F11, bit 1 F12) */
static int g_bright_pct = 100;         /* BRIGHTNESS in effect (100 = retail) */
static int g_gpu_on;                   /* NEAREST through the renderer's texture scaling */
static const char *g_gpu_why = "";
static char g_ren_name[64];
static int g_ren_accel;
static char g_display_label[128];
/* Blit dedup: the last frame put on screen, and whether the window needs
 * a redraw regardless (open, resize, expose). */
static uint8_t *g_last_rgb;
static size_t g_last_size;
static int g_last_w, g_last_h;         /* F12: the presented frame's size */
static int g_blit_dirty;

int g_host_window_open = 0;
static uint16_t g_sony_held;
static uint16_t g_cheat_held;
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

/* PE_FRAME_TRACE=1 (diagnostics, off by default): the same [FTRACE] line as
 * the X11 backend, once per wall second.  PE_FRAME_TRACE_MAX caps the count. */
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
                fs.frames_published >= g_ft.fmv0 ? fs.frames_published - g_ft.fmv0 : fs.frames_published,
                PE_StrFeed_Active(),
                HostAudio_QueueFrames(), g_win_w, g_win_h);
        g_ft.lines++;
    } else if (g_ft.t0 && now - g_ft.t0 < UINT64_C(1000000000)) return;
    g_ft.t0 = now; g_ft.vbl0 = vbl; g_ft.scan0 = scan; g_ft.pres0 = pres;
    g_ft.fmv0 = fs.frames_published;
    g_ft.paced = g_ft.blits = g_ft.skipped = 0; g_ft.blit_ns = g_ft.blit_max = 0; g_ft.n = 0;
}

static void ftrace_blit(uint64_t t0)
{
    uint64_t t1;
    if (g_ft.on && monotonic_ns(&t1)) {
        g_ft.blits++; g_ft.blit_ns += t1 - t0;
        if (t1 - t0 > g_ft.blit_max) g_ft.blit_max = t1 - t0;
    }
}

/* Drawable size in pixels (HiDPI: larger than the window's point size). */
static void refresh_output_size(void)
{
    int w = 0, h = 0;
    if (!g_ren || SDL.GetRendererOutputSize(g_ren, &w, &h) != 0 || w <= 0 || h <= 0) return;
    if (w != g_win_w || h != g_win_h) g_blit_dirty = 1;
    g_win_w = w; g_win_h = h;
}

static void release_textures(void)
{
    if (g_tex_fb) SDL.DestroyTexture(g_tex_fb);
    if (g_tex_win) SDL.DestroyTexture(g_tex_win);
    g_tex_fb = g_tex_win = NULL;
    g_tex_fb_w = g_tex_fb_h = g_tex_win_w = g_tex_win_h = 0;
}

static void *make_texture(int w, int h)
{
    void *t = SDL.CreateTexture(g_ren, HS_PIXELFORMAT_XRGB8888, HS_TEXTUREACCESS_STREAMING, w, h);
    if (!t) return NULL;
    (void)SDL.SetTextureBlendMode(t, HS_BLENDMODE_NONE);
    if (SDL.SetTextureScaleMode) (void)SDL.SetTextureScaleMode(t, HS_SCALEMODE_NEAREST);
    return t;
}

static int create_renderer(void)
{
    HsRendererInfo info;
    /* Accelerated first (SDL's order; SDL_RENDER_DRIVER selects one), the
     * software renderer as the fallback.  No vsync: the VBlank pacer
     * owns the 59.94 Hz schedule, as on X11. */
    g_ren = SDL.CreateRenderer(g_win, -1, 0);
    if (!g_ren) g_ren = SDL.CreateRenderer(g_win, -1, HS_RENDERER_SOFTWARE);
    if (!g_ren) return 0;
    memset(&info, 0, sizeof(info));
    if (SDL.GetRendererInfo(g_ren, &info) == 0 && info.name)
        snprintf(g_ren_name, sizeof(g_ren_name), "%s", info.name);
    else snprintf(g_ren_name, sizeof(g_ren_name), "unknown");
    g_ren_accel = (info.flags & HS_RENDERER_ACCELERATED) != 0;
    return 1;
}

static const char *host_env(const char *name) { return getenv(name); }

static void present_black(void)
{
    if (!g_ren) return;
    SDL.SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL.RenderClear(g_ren);
    SDL.RenderPresent(g_ren);
}

int HostWindow_Open(const char *display, int width, int height,
                    const char *title, int scale, int fullscreen)
{
    const char *lib_path = NULL, *driver, *want;
    uint32_t flags;
    int w, h, x = HS_WINDOWPOS_CENTERED, y = HS_WINDOWPOS_CENTERED;
    HsVersion ver;
    char label[560];

    if (!hs_bind()) { fprintf(stderr, "[WINDOW] SDL2 unavailable: no window\n"); return -1; }
    (void)HostSDL_Library(&lib_path);
    /* The port installs its own SIGINT/SIGTERM handlers (port_main.c). */
    SDL.SetHint("SDL_NO_SIGNAL_HANDLERS", "1");
    SDL.SetHint("SDL_APP_NAME", "Parasite Eve");
    SDL.SetHint("SDL_RENDER_SCALE_QUALITY", "nearest");
#ifndef __APPLE__
    /* WM_CLASS (SDL reads it from the environment only): a task bar shows
     * the game's name instead of the executable's. */
    setenv("SDL_VIDEO_X11_WMCLASS", "parasite-eve", 0);
#endif
    if (SDL.InitSubSystem(HS_INIT_VIDEO) != 0) {
        fprintf(stderr, "[WINDOW] SDL video init failed: %s\n", SDL.GetError());
        return -1;
    }
    g_video_on = 1;
    g_fb_w = PE_PORT_FB_WIDTH;
    g_fb_h = PE_PORT_FB_HEIGHT;
    if (scale < 1) scale = 1;
    g_fullscreen = HostWindow_WantFullscreen(fullscreen, host_env);
    g_fs_now = g_fullscreen;
    g_hot_held = 0;
    if (g_fullscreen && !fullscreen)
        fprintf(stderr, "[WINDOW] gamescope/Steam Game Mode detected: fullscreen (PE_FULLSCREEN=0 to disable)\n");
    w = width > 0 ? width : g_fb_w * scale;
    h = height > 0 ? height : g_fb_h * scale;

    g_fast_forward = 0; g_speed_key_held = 0; g_ff_active = 0; g_pad_hotkeys = 0;
    g_vblank_paced = 0; g_last_vblank_ns = 0; g_cheat_held = 0;
    ftrace_init();
    if (title) snprintf(g_title, sizeof(g_title), "%s", title);
    snprintf(label, sizeof(label), "%s", g_title);

    flags = HS_WINDOW_SHOWN | HS_WINDOW_RESIZABLE | HS_WINDOW_ALLOW_HIGHDPI;
    if (g_fullscreen) {
        HsRect b;
        /* Size to the screen, as the X11 backend does: SDL then never
         * believes in a fullscreen size the window does not have (an X
         * server without a window manager ignores the fullscreen request),
         * and a later resize is handled like any other. */
        flags |= HS_WINDOW_FULLSCREEN_DESKTOP;
        if (SDL.GetDisplayBounds(0, &b) == 0 && b.w > 0 && b.h > 0) {
            x = b.x; y = b.y; w = b.w; h = b.h;
        }
    }
    g_win = SDL.CreateWindow(label, x, y, w, h, flags);
    if (!g_win) {
        fprintf(stderr, "[WINDOW] SDL_CreateWindow failed: %s\n", SDL.GetError());
        HostWindow_Close();
        return -1;
    }
    /* No text entry here: keep IME/compose UI out of the game's keys. */
    if (SDL.StopTextInput) SDL.StopTextInput();
    if (!create_renderer()) {
        fprintf(stderr, "[WINDOW] SDL_CreateRenderer failed: %s\n", SDL.GetError());
        HostWindow_Close();
        return -1;
    }
    want = getenv("PE_WINDOW_SCALER");
    g_gpu_on = !(want && strcmp(want, "cpu") == 0);
    g_gpu_why = g_gpu_on ? "" : " (PE_WINDOW_SCALER=cpu)";
    refresh_output_size();
    if (g_win_w <= 0 || g_win_h <= 0) { g_win_w = w; g_win_h = h; }
    g_blit_dirty = 1;
    g_close_requested = 0;
    g_sony_held = 0;
    memset(&g_pacer, 0, sizeof(g_pacer));
    g_host_window_open = 1;
    PE_AudioDriver_SetVBlankPacer(pace_vblank);
    present_black();

    driver = SDL.GetCurrentVideoDriver();
    if (driver && strcmp(driver, "x11") == 0 && display && *display)
        snprintf(g_display_label, sizeof(g_display_label), "%s", display);
    else
        snprintf(g_display_label, sizeof(g_display_label), "%s", driver ? driver : "sdl");
    memset(&ver, 0, sizeof(ver));
    SDL.GetVersion(&ver);
    /* Same shape as the X11 line; scaler=sdl-<renderer> is the renderer's
     * texture scaling (the X11 xrender analogue), cpu the CPU scaler. */
    fprintf(stderr, "[WINDOW] %dx%d (fb %dx%d) on %s title='%s'%s scaler=%s%s%s\n",
            g_win_w, g_win_h, g_fb_w, g_fb_h, g_display_label, title ? title : "",
            g_fullscreen ? " fullscreen" : "", g_gpu_on ? "sdl-" : "cpu",
            g_gpu_on ? g_ren_name : "", g_gpu_why);
    fprintf(stderr, "[WINDOW] SDL %u.%u.%u video=%s renderer=%s%s lib=%s\n",
            ver.major, ver.minor, ver.patch, driver ? driver : "?", g_ren_name,
            g_ren_accel ? " (accelerated)" : " (software)", lib_path ? lib_path : "?");
    /* Opt-in controls.conf (pad.* / key.* remaps), read once per window. */
    (void)HostPadRemap_Init();
    return 0;
}

/* The DISPLAY SCALER graphics option (plat_enhance.c).  Weak so a
 * consumer without the enhancement registry keeps the nearest path. */
extern int pe_enh_display_scaler(void) __attribute__((weak));
static int g_scaler_mode_last;
static int display_scaler_mode(void)
{
    int mode = pe_enh_display_scaler ? pe_enh_display_scaler() : 0;
    if (mode != g_scaler_mode_last) {
        g_scaler_mode_last = mode; g_blit_dirty = 1; /* redraw even if the frame is unchanged */
        HostUpscale_DropReady();
    }
    return mode;
}

/* BRIGHTNESS (plat_enhance.c): weak like the scaler's.  A change redraws an
 * unchanged frame.  The gain is applied to freshly written pixels only. */
extern int pe_enh_brightness_pct(void) __attribute__((weak));
static int brightness_sync(void)
{
    int pct = pe_enh_brightness_pct ? pe_enh_brightness_pct() : 100;
    if (pct != g_bright_pct) { g_bright_pct = pct; g_blit_dirty = 1; }
    return pct;
}
/* BRIGHTNESS over the picture rectangle of the window image (BGRX): the
 * letterbox the scaler just wrote.  Black stays black at every step. */
static void brightness_picture(int fb_w, int fb_h)
{
    uint8_t lut[256];
    int x0, y0, w, h;
    if (!HostQol_BrightnessLut(lut, g_bright_pct)) return;
    HostWindow_FitRect(g_win_w, g_win_h, fb_w, fb_h, &x0, &y0, &w, &h);
    HostQol_LutRect(g_imgbuf, g_win_w, 4, x0, y0, w, h, lut);
}

/* A window-sized client image (BGRX) and its streaming texture, for the CPU
 * and filtered scalers. */
static int ensure_client_image(void)
{
    size_t need = (size_t)g_win_w * (size_t)g_win_h * 4u;
    if (g_win_w <= 0 || g_win_h <= 0) return 0;
    if (need > g_imgbuf_cap) {
        uint8_t *p = (uint8_t *)realloc(g_imgbuf, need);
        if (!p) return 0;
        g_imgbuf = p; g_imgbuf_cap = need;
    }
    if (g_img_w != g_win_w || g_img_h != g_win_h) {
        memset(g_imgbuf, 0, need);
        g_img_w = g_win_w; g_img_h = g_win_h;
        g_blit_dirty = 1;
    }
    if (!g_tex_win || g_tex_win_w != g_win_w || g_tex_win_h != g_win_h) {
        if (g_tex_win) SDL.DestroyTexture(g_tex_win);
        g_tex_win = make_texture(g_win_w, g_win_h);
        if (!g_tex_win) { g_tex_win_w = g_tex_win_h = 0; return 0; }
        g_tex_win_w = g_win_w; g_tex_win_h = g_win_h;
    }
    return 1;
}

/* Put the window-sized client image on screen 1:1. */
static void present_client_image(void)
{
    if (SDL.UpdateTexture(g_tex_win, NULL, g_imgbuf, g_win_w * 4) != 0) return;
    SDL.RenderCopy(g_ren, g_tex_win, NULL, NULL);
    SDL.RenderPresent(g_ren);
}

/* NEAREST through the renderer: upload the frame once and let it scale into
 * the letterbox (black bars from the clear).  0 = no texture: use the CPU path. */
static int gpu_blit(const uint8_t *rgb, int fb_w, int fb_h)
{
    void *pixels = NULL;
    int pitch = 0, x0, y0, w, h, x, y;
    HsRect dst;
    if (!g_tex_fb || g_tex_fb_w != fb_w || g_tex_fb_h != fb_h) {
        if (g_tex_fb) SDL.DestroyTexture(g_tex_fb);
        g_tex_fb = make_texture(fb_w, fb_h);
        if (!g_tex_fb) {
            g_tex_fb_w = g_tex_fb_h = 0;
            g_gpu_on = 0; g_gpu_why = " (texture creation failed)";
            fprintf(stderr, "[WINDOW] SDL texture failed (%s): CPU scaler\n", SDL.GetError());
            return 0;
        }
        g_tex_fb_w = fb_w; g_tex_fb_h = fb_h;
    }
    if (SDL.LockTexture(g_tex_fb, NULL, &pixels, &pitch) != 0 || !pixels) return 0;
    {
        /* BRIGHTNESS on the source texel: NEAREST (SDL_RENDER_SCALE_QUALITY
         * "nearest") copies each source value unchanged, so LUT(source) is the
         * post-scaler result.  At 100 the copy is the retail one. */
        uint8_t lut[256];
        int bright = HostQol_BrightnessLut(lut, g_bright_pct);
        for (y = 0; y < fb_h; y++) {
            uint8_t *d = (uint8_t *)pixels + (size_t)y * (size_t)pitch;
            const uint8_t *s = rgb + (size_t)y * (size_t)fb_w * 3u;
            for (x = 0; x < fb_w; x++, d += 4, s += 3) {
                d[0] = bright ? lut[s[2]] : s[2];
                d[1] = bright ? lut[s[1]] : s[1];
                d[2] = bright ? lut[s[0]] : s[0];
                d[3] = 0;
            }
        }
    }
    SDL.UnlockTexture(g_tex_fb);
    HostWindow_FitRect(g_win_w, g_win_h, fb_w, fb_h, &x0, &y0, &w, &h);
    dst.x = x0; dst.y = y0; dst.w = w; dst.h = h;
    SDL.SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL.RenderClear(g_ren);
    SDL.RenderCopy(g_ren, g_tex_fb, NULL, &dst);
    SDL.RenderPresent(g_ren);
    return 1;
}

static void remember_frame(const uint8_t *rgb, int fb_w, int fb_h)
{
    size_t size = (size_t)fb_w * (size_t)fb_h * 3u;
    if (g_last_size != size) {
        uint8_t *p = realloc(g_last_rgb, size);
        if (p) { g_last_rgb = p; g_last_size = size; }
    }
    if (g_last_size == size) {
        memcpy(g_last_rgb, rgb, size);
        g_last_w = fb_w; g_last_h = fb_h;   /* the frame F12 saves */
    }
}

void HostWindow_Blit(const uint8_t *rgb, int fb_w, int fb_h)
{
    uint64_t t0 = 0;
    size_t size = (size_t)fb_w * (size_t)fb_h * 3u;
    int scaler, unchanged, retry_filtered = 0;
#ifdef PE_UPSCALE_THREADS
    int async_available;
#endif
    if (!g_ren || !rgb || fb_w <= 0 || fb_h <= 0) return;
    scaler = display_scaler_mode();
    (void)brightness_sync();
#ifdef PE_UPSCALE_THREADS
    if (HostUpscale_TakeFailure()) g_blit_dirty = 1;
    async_available = scaler != HOST_UPSCALE_NEAREST && HostUpscale_AsyncAvailable();
    /* Never run CPU FSR on the game thread (as on X11). */
    if (!async_available && scaler == HOST_UPSCALE_FSR1) {
        static int fallback_said;
        if (!fallback_said) {
            fprintf(stderr, "[WINDOW] FSR 1 presenter unavailable; using SHARP BILINEAR\n");
            fallback_said = 1;
        }
        scaler = HOST_UPSCALE_SHARP_BILINEAR;
    }
#endif
    unchanged = !g_blit_dirty && g_last_rgb && g_last_size == size && memcmp(g_last_rgb, rgb, size) == 0;
#ifdef PE_UPSCALE_THREADS
    if (scaler != HOST_UPSCALE_NEAREST && async_available) {
        /* Filtered scalers run on the presenter thread: show whatever
         * finished since the last call, then queue this frame. */
        if (ensure_client_image()) {
            if (HostUpscale_TakeReadyFor(g_imgbuf, scaler, g_win_w, g_win_h)) {
                if (g_ft.on) (void)monotonic_ns(&t0);
                brightness_picture(fb_w, fb_h);
                present_client_image();
                ftrace_blit(t0);
            }
            if (unchanged) { if (g_ft.on) g_ft.skipped++; return; }
            if (HostUpscale_SubmitAsync(scaler, rgb, fb_w, fb_h, g_win_w, g_win_h)) {
                g_blit_dirty = 0;
                remember_frame(rgb, fb_w, fb_h);
                return;
            }
        }
        /* A failed allocation stays retryable; nearest meanwhile. */
        retry_filtered = 1;
        g_blit_dirty = 1;
        unchanged = 0;
        scaler = HOST_UPSCALE_NEAREST;
    }
#endif
    /* A present that shows exactly the pixels already on screen changes
     * nothing visible: skip it unless the window needs a redraw. */
    if (unchanged) {
        if (g_ft.on) g_ft.skipped++;
        return;
    }
    if (g_ft.on) (void)monotonic_ns(&t0);
    if (!(g_gpu_on && scaler == HOST_UPSCALE_NEAREST && gpu_blit(rgb, fb_w, fb_h))) {
        int actual_mode;
        if (!ensure_client_image()) return;
        /* NEAREST here is HostWindow_ScaleBGRA unchanged. */
        actual_mode = HostUpscale_Run(scaler, g_imgbuf, g_win_w, g_win_h, rgb, fb_w, fb_h);
        if (scaler != HOST_UPSCALE_NEAREST && actual_mode != scaler) {
            retry_filtered = 1;
            g_blit_dirty = 1;
        }
        brightness_picture(fb_w, fb_h);   /* BRIGHTNESS on the rectangle just written */
        present_client_image();
    }
    if (!retry_filtered) {
        g_blit_dirty = 0;
        remember_frame(rgb, fb_w, fb_h);
    }
    ftrace_blit(t0);
}

/* The title is the game's name alone; " [Fast-forward]" is appended while
 * fast-forward runs. */
void HostWindow_SetTitle(const char *title)
{
    char label[560];
    if (title) snprintf(g_title, sizeof(g_title), "%s", title);
    if (g_ff_active) snprintf(label, sizeof(label), "%s [Fast-forward]", g_title);
    else snprintf(label, sizeof(label), "%s", g_title);
    if (g_win) SDL.SetWindowTitle(g_win, label);
}

/* Recompute the effective fast-forward state; on a change reset the pacer,
 * retitle, and mute/unmute the audio device (no chipmunk audio). */
static void ff_update(const char *why)
{
    int eff = g_fast_forward || (g_pad_hotkeys & HOST_PAD_HOTKEY_FF_HOLD) != 0;
    if (eff == g_ff_active) return;
    g_ff_active = eff;
    memset(&g_pacer, 0, sizeof(g_pacer));
    HostWindow_SetTitle(NULL);
    HostAudio_SetFastForward(eff);
    fprintf(stderr, "[WINDOW] %s (%s; F6/L3 toggle, hold R3)\n",
            eff ? "Fast-forward, audio muted" : "Normal speed: 59.94 Hz", why);
}

/* SDL keycode -> the X11 keysym XLookupKeysym(index 0) gives for the same
 * key, so host_pad_remap.c's keysym table (defaults and controls.conf names)
 * serves this backend unchanged.  0 = no keysym. */
static unsigned long sdl_key_to_keysym(int32_t k)
{
    if (k & HS_K_SCANCODE_MASK) {
        int sc = (int)(k & ~HS_K_SCANCODE_MASK);
        if (sc >= 58 && sc <= 69) return 0xFFBEul + (unsigned long)(sc - 58);   /* F1..F12 */
        if (sc >= 89 && sc <= 97) return 0xFFB1ul + (unsigned long)(sc - 89);   /* KP_1..KP_9 */
        switch (sc) {
        case 57:  return 0xFFE5;   /* Caps_Lock */
        case 70:  return 0xFF61;   /* Print */
        case 71:  return 0xFF14;   /* Scroll_Lock */
        case 72:  return 0xFF13;   /* Pause */
        case 73:  return 0xFF63;   /* Insert */
        case 74:  return 0xFF50;   /* Home */
        case 75:  return 0xFF55;   /* Prior */
        case 77:  return 0xFF57;   /* End */
        case 78:  return 0xFF56;   /* Next */
        case 79:  return 0xFF53;   /* Right */
        case 80:  return 0xFF51;   /* Left */
        case 81:  return 0xFF54;   /* Down */
        case 82:  return 0xFF52;   /* Up */
        case 83:  return 0xFF7F;   /* Num_Lock */
        case 84:  return 0xFFAF;   /* KP_Divide */
        case 85:  return 0xFFAA;   /* KP_Multiply */
        case 86:  return 0xFFAD;   /* KP_Subtract */
        case 87:  return 0xFFAB;   /* KP_Add */
        case 88:  return 0xFF8D;   /* KP_Enter */
        case 98:  return 0xFFB0;   /* KP_0 */
        case 99:  return 0xFFAE;   /* KP_Decimal */
        case 224: return 0xFFE3;   /* Control_L */
        case 225: return 0xFFE1;   /* Shift_L */
        case 226: return 0xFFE9;   /* Alt_L */
        case 227: return 0xFFEB;   /* Super_L */
        case 228: return 0xFFE4;   /* Control_R */
        case 229: return 0xFFE2;   /* Shift_R */
        case 230: return 0xFFEA;   /* Alt_R */
        case 231: return 0xFFEC;   /* Super_R */
        default:  return 0;
        }
    }
    switch (k) {
    case 0x08: return 0xFF08;      /* BackSpace */
    case 0x09: return 0xFF09;      /* Tab */
    case 0x0D: return 0xFF0D;      /* Return */
    case 0x1B: return 0xFF1B;      /* Escape */
    case 0x7F: return 0xFFFF;      /* Delete */
    default: break;
    }
    if ((k >= 0x20 && k <= 0x7E) || (k >= 0xA0 && k <= 0xFF)) return (unsigned long)k;  /* Latin-1 */
    if (k > 0xFF && k <= 0x10FFFF) return 0x01000000ul | (unsigned long)k;            /* Unicode keysym */
    return 0;
}

/* F11: borderless fullscreen (SDL_WINDOW_FULLSCREEN_DESKTOP) or windowed, at
 * runtime.  SDL may resize the window itself; the size it lands on is logged. */
static void toggle_fullscreen(void)
{
    int w = 0, h = 0;
    if (!g_win) return;
    g_fs_now = !g_fs_now;
    if (SDL.SetWindowFullscreen(g_win, g_fs_now ? HS_WINDOW_FULLSCREEN_DESKTOP : 0u) != 0)
        fprintf(stderr, "[WINDOW] fullscreen change failed: %s\n", SDL.GetError());
    SDL.GetWindowSize(g_win, &w, &h);
    refresh_output_size();
    g_blit_dirty = 1;
    fprintf(stderr, "[WINDOW] fullscreen %s (F11): window %dx%d\n", g_fs_now ? "on" : "off", w, h);
}

/* One key event.  Returns 1 when it requests close. */
static int handle_key(const HsKeyboardEvent *ke)
{
    int down = ke->type == HS_KEYDOWN;
    unsigned long ks = sdl_key_to_keysym(ke->keysym.sym);
    PeCheatKey ck = PE_CHEAT_KEY_NONE;
    uint16_t bit;
    if (!ks) return 0;
    /* Cheat console (pe_plat/cheats.h): F1-F5, F7, F8; Esc, arrows and Enter
     * only while its menu is open.  SDL marks auto-repeat presses (no release
     * in between): repeats reach the menu only as Up/Down. */
    if (ks >= 0xFFBEul && ks <= 0xFFC5ul && ks != 0xFFC3ul) {
        static const PeCheatKey fk[8] = {PE_CHEAT_KEY_F1, PE_CHEAT_KEY_F2,
            PE_CHEAT_KEY_F3, PE_CHEAT_KEY_F4, PE_CHEAT_KEY_F5, PE_CHEAT_KEY_NONE,
            PE_CHEAT_KEY_F7, PE_CHEAT_KEY_F8};
        ck = fk[ks - 0xFFBEul];
    } else if (pe_plat_cheats_menu_open()) {
        if (ks == 0xFF1Bul) ck = PE_CHEAT_KEY_BACK;
        else if (ks == 0xFF52ul) ck = PE_CHEAT_KEY_UP;
        else if (ks == 0xFF54ul) ck = PE_CHEAT_KEY_DOWN;
        else if (ks == 0xFF0Dul) ck = PE_CHEAT_KEY_ENTER;
    }
    if (ck != PE_CHEAT_KEY_NONE) {
        uint16_t bitk = (uint16_t)(1u << ck);
        if (down) {
            int was = (g_cheat_held & bitk) != 0 || ke->repeat;
            g_cheat_held |= bitk;
            if (was && ck != PE_CHEAT_KEY_UP && ck != PE_CHEAT_KEY_DOWN) return 0;
        } else {
            g_cheat_held &= (uint16_t)~bitk;
        }
        if (pe_plat_cheats_key(ck, down)) return 0;
    }
    if (ks == 0xFFC3ul) {   /* F6 */
        if (pe_plat_cheats_menu_open() || HostDevMenu_CapturesInput()) return 0;
        if (down && !g_speed_key_held) {
            g_fast_forward = !g_fast_forward;
            ff_update("F6");
        }
        g_speed_key_held = down;
        return 0;
    }
    if (ks == 0xFF1Bul) {
        /* Quit on the press only: the release of an Esc that just closed the
         * cheat menu (consumed above as BACK) must not also quit. */
        if (!down) return 0;
        g_close_requested = 1;
        return 1;
    }
    /* Host hotkeys: F11 fullscreen toggle, F12 screenshot of the presented
     * frame.  Consumed before the pad remap (a controls.conf that binds F11 or
     * F12 to a button loses that binding).  One action per press: SDL marks
     * auto-repeat presses (ke->repeat), and the held bit covers the rest. */
    if (ks == 0xFFC8ul || ks == 0xFFC9ul) {
        uint8_t hot = ks == 0xFFC8ul ? 1u : 2u;
        if (down) {
            if (!ke->repeat && !(g_hot_held & hot)) {
                g_hot_held |= hot;
                if (hot == 1u) toggle_fullscreen();
                else (void)HostShot_Take(g_last_rgb, g_last_w, g_last_h);
            }
        } else {
            g_hot_held &= (uint8_t)~hot;
        }
        return 0;
    }
    /* Keyboard table (host_pad_remap.c): Return/space/z/x = Cross, c/v/s =
     * Circle/Triangle/Square, q/e = L1/R1, 1/3 = L2/R2, Tab = Select, p =
     * Start, arrows; controls.conf may replace a button's keys.  Esc and
     * F1-F8 never bind. */
    bit = HostPadRemap_Key(HostPadRemap_Global(), ks);
    if (bit != 0u) {
        if (down) g_sony_held |= bit;
        else g_sony_held &= (uint16_t)~bit;
    }
    return 0;
}

static void reset_renderer(void)
{
    /* SDL_RENDER_DEVICE_RESET: textures and the renderer are gone. */
    release_textures();
    if (g_ren) SDL.DestroyRenderer(g_ren);
    g_ren = NULL;
    if (!create_renderer()) {
        fprintf(stderr, "[WINDOW] SDL renderer lost and not recreated: %s\n", SDL.GetError());
        g_close_requested = 1;
        return;
    }
    refresh_output_size();
    g_blit_dirty = 1;
}

int HostWindow_Poll(void)
{
    HsEvent ev;
    if (!g_win) return 0;
    HostPad_Poll();
    HostDevMenu_Poll();
    {
        uint8_t hk = (pe_plat_cheats_menu_open() || HostDevMenu_CapturesInput()) ? 0 : HostPad_Hotkeys();
        if ((hk & HOST_PAD_HOTKEY_FF_TOGGLE) && !(g_pad_hotkeys & HOST_PAD_HOTKEY_FF_TOGGLE))
            g_fast_forward = !g_fast_forward;
        g_pad_hotkeys = hk;
        ff_update("controller");
    }
    while (SDL.PollEvent(&ev)) {
        switch (ev.type) {
        case HS_QUIT:
            g_close_requested = 1;
            return 1;
        case HS_WINDOWEVENT:
            switch (ev.window.event) {
            case HS_WE_EXPOSED: g_blit_dirty = 1; break;   /* the next blit repaints */
            case HS_WE_RESIZED:
            case HS_WE_SIZE_CHANGED: refresh_output_size(); g_blit_dirty = 1; break;
            case HS_WE_FOCUS_LOST: g_sony_held = 0; g_speed_key_held = 0; g_cheat_held = 0; break;
            case HS_WE_CLOSE: g_close_requested = 1; return 1;
            default: break;
            }
            break;
        case HS_KEYDOWN:
        case HS_KEYUP:
            if (handle_key(&ev.key)) return 1;
            break;
        case HS_RENDER_TARGETS_RESET:
            g_blit_dirty = 1;
            break;
        case HS_RENDER_DEVICE_RESET:
            reset_renderer();
            break;
        default:
            break;   /* controller/joystick events: host_pad_sdl.c polls state */
        }
    }
    return g_close_requested ? 1 : 0;
}

/* Opt-in ANALOG STICK on and a GameController open: the pad replies analog. */
static int window_pad_analog(uint8_t sticks[4])
{
    return pe_enh_analog_stick() && HostPadSDL_AnalogSticks(sticks);
}

uint16_t HostWindow_PadRaw(void)
{
    uint8_t sticks[4];
    /* Keyboard and controller both drive the pad (see host_window.c): the
     * controller word goes through the dev-menu combo filter, then the
     * optional controls.conf pad.* remap; the keyboard is mapped by its table. */
    if (pe_plat_cheats_menu_open() || HostDevMenu_CapturesInput()) return 0xFFFFu;
    return HostPadRemap_PadWord(HostPadRemap_Global(), g_sony_held,
                                HostDevMenu_FilterPad(window_pad_analog(sticks) ? HostPad_HeldAnalog()
                                                                                 : HostPad_Held()));
}

int HostWindow_PadAnalog(uint8_t sticks[4])
{
    if (!window_pad_analog(sticks)) return 0;
    if (pe_plat_cheats_menu_open() || HostDevMenu_CapturesInput())
        memset(sticks, 0x80, 4);   /* the menu holds the pad: centred */
    return 1;
}

static int monotonic_ns(uint64_t *out)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) { perror("[WINDOW] monotonic clock"); return 0; }
    *out = (uint64_t)now.tv_sec * UINT64_C(1000000000) + (uint64_t)now.tv_nsec;
    return 1;
}

/* Sleep to the next 59.94 Hz deadline, polling input.  Returns 1 on
 * close or a host clock failure. */
static int pace_one_frame(void)
{
    uint64_t now, deadline;
    if (!monotonic_ns(&now)) return 1;
    deadline = HostFramePacer_Deadline(&g_pacer, now);
    for (;;) {
        struct timespec delay;
        uint64_t remaining;
        if (HostWindow_Poll() || !monotonic_ns(&now)) return 1;
        if (g_ff_active && pe_plat_cheats_ff_multiplier() == 0) return 0;
        if (now >= deadline) return 0;
        remaining = deadline - now;
        delay.tv_sec = 0;
        delay.tv_nsec = (long)(remaining > 4000000u ? 4000000u : remaining);
        /* Recheck the absolute deadline after interruptions and oversleep. */
        (void)nanosleep(&delay, NULL);
    }
}

/* PeVBlankPacer: one emulated VBlank = one 59.94 Hz frame of wall time. */
static void pace_vblank(void)
{
    if (!g_win) return;
    g_vblank_paced = 1;
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
        if ((++n & 7u) == 0u) (void)HostWindow_Poll();
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
    /* Emulated VBlanks pace the game; a present only polls.  Safety net:
     * if no VBlank has paced for 100 ms, presents are paced one frame each. */
    if (g_vblank_paced) {
        uint64_t now;
        if (monotonic_ns(&now) && now - g_last_vblank_ns < UINT64_C(100000000))
            return HostWindow_Poll();
    }
    return pace_one_frame();
}

int HostWindow_Run(int milliseconds)
{
    struct timespec start, now;
    if (!g_win) return 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (;;) {
        long elapsed;
        int rc = HostWindow_Poll();
        if (rc) return rc;
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / 1000000;
        if (milliseconds > 0 && elapsed >= milliseconds) return 0;
        usleep(16000); /* ~60 Hz poll */
    }
}

void HostWindow_Close(void)
{
    HostUpscale_Shutdown();
    PE_AudioDriver_SetVBlankPacer(NULL);
    free(g_last_rgb); g_last_rgb = NULL; g_last_size = 0;
    free(g_imgbuf); g_imgbuf = NULL; g_imgbuf_cap = 0; g_img_w = g_img_h = 0;
    if (SDL.bound > 0) {
        release_textures();
        if (g_ren) SDL.DestroyRenderer(g_ren);
        if (g_win) SDL.DestroyWindow(g_win);
        /* Only VIDEO: the controller and audio subsystems belong to
         * host_pad_sdl.c / host_audio_sdl.c, and the library stays loaded. */
        if (g_video_on) SDL.QuitSubSystem(HS_INIT_VIDEO);
    }
    g_ren = NULL; g_win = NULL; g_video_on = 0;
    g_host_window_open = 0;
    g_sony_held = 0; g_cheat_held = 0;
    HostPadRemap_Reset();
    HostDevMenu_Close();
    HostPad_Close();
}

#ifdef PE_HOST_PAD_SDL_ONLY
/* No Linux joystick API here (macOS): host_pad.c is not built and the
 * controllers are the SDL2 GameController backend alone, through the same
 * HostPad_* entry points host_pad.c provides (its js* half simply absent). */
void HostPad_Poll(void) { HostPadSDL_Poll(); }
uint16_t HostPad_Held(void) { return HostPadSDL_Held(); }
uint16_t HostPad_HeldAnalog(void) { return HostPadSDL_HeldAnalog(); }
uint8_t HostPad_Hotkeys(void) { return HostPadSDL_Hotkeys(); }
int HostPad_Origins(HostPadOrigin *out, int max) { return HostPadSDL_Origins(out, max); }
void HostPad_Close(void) { HostPadSDL_Close(); }
#endif

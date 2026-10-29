/*
 * Phase WIN1 — Native Win32 window backend (GDI, no extra dependencies).
 *
 * Implements the same platform/host_window.h interface as host_window.c
 * (X11). Selected by CMake when WIN32 is true. Uses only Win32 API calls
 * present since Windows XP: RegisterClassA, CreateWindowExA, GetDC,
 * StretchDIBits, PeekMessageA, SetWindowTextA. No SDL, no DirectX, no
 * additional libraries — links against stock gdi32/user32.
 *
 * It follows the X11 backend's host behaviour:
 *  - pacing per emulated VBlank (PE_AudioDriver_SetVBlankPacer): one
 *    emulated VBlank = one 59.94 Hz frame of wall time, so the 30 fps game
 *    and its music run at retail speed; a present only polls input;
 *  - controllers through host_pad.h (XInput on Windows, host_pad_xinput.c)
 *    and the dev-menu combo (host_dev_menu.c: Back+Start on one controller);
 *  - fast-forward: F6 or controller L3 toggles, R3 holds; the audio device
 *    is muted while it runs;
 *  - PE_FRAME_TRACE=1 [FTRACE] lines; identical frames are not re-blitted.
 *
 * Pad mapping mirrors the X11 backend exactly:
 *   Cross  = Return / Space / Z / X  (raw 0x4000)
 *   Up/Right/Down/Left               (raw 0x10/0x20/0x40/0x80)
 *   Circle/Triangle/Square = C/V/S; L1/R1 = Q/E; L2/R2 = 1/3
 *   Select/Start = Tab/P
 *   Escape or window-close           (close requested)
 * Cheat console and dev menu keys (pe_plat/cheats.h): F1-F5, F7, F8 (F8
 * opens the dev menu); while a menu is open Up/Down, Enter and Escape (back)
 * navigate it instead of reaching the game.
 * An optional controls.conf (host_pad_remap.h) rebinds the keys through the
 * same keysym table as the X11 backend; key names are X11 keysym names, and
 * modifier keys are not bindable here.
 */

#include "host_window.h"
#include "host_framebuffer.h"
#include "host_frame_pacer.h"
#include "host_window_scale.h"
#include "host_upscale.h"
#include "host_png.h"
#include "host_qol.h"
#include "host_pad.h"
#include "host_pad_win32.h"
#include "host_pad_remap.h"
#include "host_dev_menu.h"
#include "host_audio.h"
#include "pe_audio_driver.h"
#include "game_port.h"
#include "pe_plat/cheats.h"
#include "pe_plat/enhance.h"
#include "pe_gpu.h"
#include "pe_str_feed.h"

/* GetTickCount64 needs Vista+ declarations on older MinGW defaults. */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Framebuffer dimensions shared with the X11 backend. */
#ifndef PE_PORT_FB_WIDTH
#define PE_PORT_FB_WIDTH 320
#endif
#ifndef PE_PORT_FB_HEIGHT
#define PE_PORT_FB_HEIGHT 240
#endif

static HWND     g_hwnd = NULL;
static uint8_t *g_imgbuf = NULL;   /* BGRA back buffer, win_w * win_h * 4 */
static int      g_win_w = 0, g_win_h = 0;
static int      g_fb_w = 0, g_fb_h = 0;
static BITMAPINFO g_bmi;
static int      g_close_requested = 0;

int g_host_window_open = 0;
static uint16_t g_sony_held = 0;
static HostFramePacer g_pacer;
static HANDLE g_frame_timer;
static LARGE_INTEGER g_clock_frequency;
/* Fast-forward: F6 or controller L3 toggles g_fast_forward; controller R3
 * fast-forwards while held.  g_ff_active is the effective state. */
static int g_fast_forward, g_speed_key_held, g_ff_active;
static uint8_t g_pad_hotkeys;
/* Pacing is per emulated VBlank (see pe_audio_driver.h).  Presents only
 * poll input, unless a scene presents without advancing VBlanks. */
static int g_vblank_paced;
static uint64_t g_last_vblank_ns;
static char g_title[512];
/* Blit dedup: the last frame put on screen, and whether the window needs
 * a redraw regardless (open, resize). */
static uint8_t *g_last_rgb;
static size_t g_last_size;
static int g_last_w, g_last_h;     /* F12: the presented frame's size */
static int g_blit_dirty;
static uint16_t g_cheat_held;      /* cheat keys down (auto-repeat filter) */
/* F11 / Alt+Enter: borderless fullscreen toggle.  g_fs_now is the state asked
 * for (starts at the open mode); g_wp_windowed keeps the windowed placement. */
static int g_fs_now;
static WINDOWPLACEMENT g_wp_windowed = {sizeof(WINDOWPLACEMENT)};
static int g_wp_valid;             /* g_wp_windowed holds a captured windowed placement */
static int g_req_w, g_req_h;       /* the windowed client size asked for at open */
static uint8_t g_hot_held;         /* F11/F12 autorepeat guard (bit 0 F11, bit 1 F12) */
static int g_bright_pct = 100;     /* BRIGHTNESS in effect (100 = retail) */

static uint64_t monotonic_ns(void)
{
    LARGE_INTEGER counter;
    uint64_t ticks,frequency=(uint64_t)g_clock_frequency.QuadPart;
    if (!frequency) return 0;
    QueryPerformanceCounter(&counter);ticks=(uint64_t)counter.QuadPart;
    return (ticks/frequency)*UINT64_C(1000000000)+
           ((ticks%frequency)*UINT64_C(1000000000))/frequency;
}

/* PE_FRAME_TRACE=1 (diagnostics, off by default): one [FTRACE] line per
 * wall second with presents, emulated VBlanks, host busy time per VBlank
 * (wall time outside the pacer's sleep, p50/p95/max), window blits and
 * their cost, idle-VBlank scan-outs, FMV frames published and the audio
 * device queue -- the X11 backend's format.  PE_FRAME_TRACE_MAX caps the
 * line count (default 600). */
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

/* Recompute the effective fast-forward state; on a change reset the pacer,
 * retitle, and mute/unmute the audio device (no chipmunk audio). */
static void ff_update(const char *why)
{
    int eff=g_fast_forward||(g_pad_hotkeys&HOST_PAD_HOTKEY_FF_HOLD)!=0;
    if (eff==g_ff_active) return;
    g_ff_active=eff;
    ZeroMemory(&g_pacer,sizeof(g_pacer));
    HostWindow_SetTitle(NULL);
    HostAudio_SetFastForward(eff);
    fprintf(stderr,"[WINDOW] %s (%s; F6/L3 toggle, hold R3)\n",
            eff?"Fast-forward, audio muted":"Normal speed: 59.94 Hz",why);
}

/* Translate a virtual-key code to a Sony pad bit (0 = unmapped).  The shared
 * keysym-keyed table (host_pad_remap.c) holds the defaults listed at the top of
 * this file; an optional controls.conf (key.<button>=<keysym>,...) replaces a
 * button's keys.  Esc and F1-F8 are never bindable. */
static uint16_t vk_to_pad_bit(UINT vk)
{
    return HostPadRemap_Key(HostPadRemap_Global(),
                            HostPadRemap_KeysymFromWin32Vk((unsigned)vk));
}

static void paint_window(HDC hdc)
{
    if (!g_imgbuf || g_win_w <= 0 || g_win_h <= 0) return;
    StretchDIBits(hdc, 0, 0, g_win_w, g_win_h,
                  0, 0, g_win_w, g_win_h,
                  g_imgbuf, &g_bmi, DIB_RGB_COLORS, SRCCOPY);
}

/* Cheat console and dev menu (pe_plat/cheats.h): F1-F5, F7, F8; Escape
 * (back), arrows and Enter only while a menu is open, as in the X11 backend.
 * repeat = key auto-repeat: ignored except for Up/Down.  Returns 1 when the
 * console consumed the key. */
static int win32_cheat_key(WPARAM vk, int pressed, int repeat)
{
    PeCheatKey ck = PE_CHEAT_KEY_NONE;
    uint16_t bit;
    switch (vk) {
    case VK_F1: ck = PE_CHEAT_KEY_F1; break;
    case VK_F2: ck = PE_CHEAT_KEY_F2; break;
    case VK_F3: ck = PE_CHEAT_KEY_F3; break;
    case VK_F4: ck = PE_CHEAT_KEY_F4; break;
    case VK_F5: ck = PE_CHEAT_KEY_F5; break;
    case VK_F7: ck = PE_CHEAT_KEY_F7; break;
    case VK_F8: ck = PE_CHEAT_KEY_F8; break;
    case VK_ESCAPE: if (pe_plat_cheats_menu_open()) ck = PE_CHEAT_KEY_BACK; break;
    case VK_UP: if (pe_plat_cheats_menu_open()) ck = PE_CHEAT_KEY_UP; break;
    case VK_DOWN: if (pe_plat_cheats_menu_open()) ck = PE_CHEAT_KEY_DOWN; break;
    case VK_RETURN: if (pe_plat_cheats_menu_open()) ck = PE_CHEAT_KEY_ENTER; break;
    default: break;
    }
    if (ck == PE_CHEAT_KEY_NONE) return 0;
    bit = (uint16_t)(1u << ck);
    if (pressed) {
        int was = repeat || (g_cheat_held & bit) != 0;
        g_cheat_held |= bit;
        if (was && ck != PE_CHEAT_KEY_UP && ck != PE_CHEAT_KEY_DOWN) return 1;
    } else {
        g_cheat_held &= (uint16_t)~bit;
    }
    return pe_plat_cheats_key(ck, pressed);
}

/* F11 and Alt+Enter: borderless fullscreen on the window's monitor, and back
 * to the windowed placement the window had (or, when the game started
 * fullscreen, the open size).  The client area follows the new size through
 * WM_SIZE, which redraws the next blit. */
static void toggle_fullscreen(void)
{
    LONG style;
    if (!g_hwnd) return;
    style = GetWindowLongA(g_hwnd, GWL_STYLE);
    g_fs_now = !g_fs_now;
    if (g_fs_now) {
        MONITORINFO mi;
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        g_wp_windowed.length = sizeof(WINDOWPLACEMENT);
        if (GetWindowPlacement(g_hwnd, &g_wp_windowed)) g_wp_valid = 1;
        if (GetMonitorInfoA(MonitorFromWindow(g_hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLongA(g_hwnd, GWL_STYLE, (style & ~(LONG)WS_OVERLAPPEDWINDOW) | (LONG)WS_POPUP);
            SetWindowPos(g_hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        }
    } else {
        RECT rc;
        SetWindowLongA(g_hwnd, GWL_STYLE, (style & ~(LONG)WS_POPUP) | (LONG)WS_OVERLAPPEDWINDOW);
        if (g_wp_valid) {
            SetWindowPlacement(g_hwnd, &g_wp_windowed);
        } else {
            rc.left = 0; rc.top = 0; rc.right = g_req_w; rc.bottom = g_req_h;
            AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
            SetWindowPos(g_hwnd, NULL, 100, 100, rc.right - rc.left, rc.bottom - rc.top,
                         SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        }
        SetWindowPos(g_hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
    g_blit_dirty = 1;
    fprintf(stderr, "[WINDOW] fullscreen %s (F11 / Alt+Enter)\n", g_fs_now ? "on" : "off");
}

/* Host hotkeys, checked before the cheat console and the pad: F11 and
 * Alt+Enter toggle fullscreen, F12 saves a screenshot of the presented frame.
 * They never reach the pad (a controls.conf binding of F11 or F12 loses to
 * them).  Auto-repeat (lparam bit 30) does nothing; the held bit covers the
 * rest.  Returns 1 when the key was consumed. */
static int win32_hotkey_down(WPARAM vk, LPARAM lparam)
{
    int alt = (lparam & ((LPARAM)1 << 29)) != 0;      /* context code: Alt is down */
    int repeat = (lparam & ((LPARAM)1 << 30)) != 0;   /* previous state: already down */
    uint8_t hot;
    if (vk == VK_RETURN && alt) hot = 4u;
    else if (vk == VK_F11) hot = 1u;
    else if (vk == VK_F12) hot = 2u;
    else return 0;
    if (!repeat && !(g_hot_held & hot)) {
        g_hot_held |= hot;
        if (hot == 2u) (void)HostShot_Take(g_last_rgb, g_last_w, g_last_h);
        else toggle_fullscreen();
    }
    return 1;
}

/* The release of a host hotkey clears its held bit.  Enter's release is also
 * a plain Enter, so it still reaches the cheat console and the pad (returns 0). */
static int win32_hotkey_up(WPARAM vk)
{
    if (vk == VK_F11) { g_hot_held &= (uint8_t)~1u; return 1; }
    if (vk == VK_F12) { g_hot_held &= (uint8_t)~2u; return 1; }
    if (vk == VK_RETURN) g_hot_held &= (uint8_t)~4u;
    return 0;
}

static LRESULT CALLBACK PE_WndProc(HWND hwnd, UINT msg,
                                   WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        paint_window(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_SIZE:
        g_blit_dirty = 1;   /* the next blit rescales into the new client area */
        return 0;
    case WM_SYSCHAR:
        /* Alt+Enter (the fullscreen toggle) is also translated to WM_SYSCHAR '\r';
         * DefWindowProc would beep on it.  Everything else goes to the default. */
        if (wparam == '\r') return 0;
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (win32_hotkey_down(wparam, lparam)) return 0;
        if (win32_cheat_key(wparam, 1, (lparam & ((LPARAM)1 << 30)) != 0)) return 0;
        if (wparam==VK_F6) {
            if (pe_plat_cheats_menu_open() || HostDevMenu_CapturesInput()) return 0;
            if (!g_speed_key_held) {
                g_fast_forward=!g_fast_forward;
                ff_update("F6");
            }
            g_speed_key_held=1;
            return 0;
        }
        if (wparam == VK_ESCAPE) { g_close_requested = 1; return 0; }
        g_sony_held |= vk_to_pad_bit((UINT)wparam);
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (win32_hotkey_up(wparam)) return 0;   /* F11 / F12 release (Enter falls through) */
        if (win32_cheat_key(wparam, 0, 0)) return 0;
        if (wparam==VK_F6) {
            if (!(pe_plat_cheats_menu_open() || HostDevMenu_CapturesInput())) g_speed_key_held=0;
            return 0;
        }
        g_sony_held &= (uint16_t)~vk_to_pad_bit((UINT)wparam);
        return 0;
    case WM_KILLFOCUS:
        g_sony_held = 0;
        g_speed_key_held = 0;
        g_cheat_held = 0;
        return 0;
    case WM_CLOSE:
        g_close_requested = 1;
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        g_close_requested = 1;
        g_hwnd = NULL;
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void pace_vblank(void);

int HostWindow_Open(const char *display, int width, int height,
                    const char *title, int scale, int fullscreen)
{
    WNDCLASSA wc;
    RECT rc;
    DWORD style = fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
    (void)display; /* no display string on Windows */
    g_fs_now = fullscreen ? 1 : 0;
    g_hot_held = 0;
    g_wp_valid = 0;

    if (scale < 1) scale = 1;
    g_fb_w = PE_PORT_FB_WIDTH;
    g_fb_h = PE_PORT_FB_HEIGHT;
    g_req_w = width > 0 ? width : g_fb_w * scale;
    g_req_h = height > 0 ? height : g_fb_h * scale;
    if (fullscreen) {
        /* Borderless window sized to the primary monitor; HostWindow_Blit's
         * letterbox/pillarbox math (host_window_scale.h) fits the 320x240
         * source into it, same as the X11 backend's EWMH fullscreen path. */
        g_win_w = GetSystemMetrics(SM_CXSCREEN);
        g_win_h = GetSystemMetrics(SM_CYSCREEN);
    } else {
        g_win_w = width > 0 ? width : g_fb_w * scale;
        g_win_h = height > 0 ? height : g_fb_h * scale;
    }

    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc   = PE_WndProc;
    wc.hInstance     = GetModuleHandleA(NULL);
    wc.lpszClassName = "PEPortWindow";
    wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (!RegisterClassA(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        fprintf(stderr, "[WINDOW] RegisterClassA failed (%lu)\n",
                (unsigned long)GetLastError());
        return -1;
    }

    /* Size the *client* area to the requested dimensions. */
    rc.left = 0; rc.top = 0; rc.right = g_win_w; rc.bottom = g_win_h;
    AdjustWindowRect(&rc, style, FALSE);

    g_hwnd = CreateWindowExA(0, "PEPortWindow", title ? title : "PE",
                             style,
                             fullscreen ? 0 : CW_USEDEFAULT,
                             fullscreen ? 0 : CW_USEDEFAULT,
                             fullscreen ? g_win_w : rc.right - rc.left,
                             fullscreen ? g_win_h : rc.bottom - rc.top,
                             NULL, NULL, wc.hInstance, NULL);
    if (!g_hwnd) {
        fprintf(stderr, "[WINDOW] CreateWindowExA failed (%lu)\n",
                (unsigned long)GetLastError());
        return -1;
    }

    g_imgbuf = (uint8_t *)calloc((size_t)g_win_w * (size_t)g_win_h, 4);
    if (!g_imgbuf) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
        return -1;
    }

    ZeroMemory(&g_bmi, sizeof(g_bmi));
    g_bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    g_bmi.bmiHeader.biWidth       = g_win_w;
    g_bmi.bmiHeader.biHeight      = -g_win_h; /* top-down */
    g_bmi.bmiHeader.biPlanes      = 1;
    g_bmi.bmiHeader.biBitCount    = 32;
    g_bmi.bmiHeader.biCompression = BI_RGB;

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    g_close_requested = 0;
    g_sony_held = 0;
    g_cheat_held = 0;
    g_fast_forward = 0; g_speed_key_held = 0; g_ff_active = 0; g_pad_hotkeys = 0;
    g_vblank_paced = 0; g_last_vblank_ns = 0;
    g_blit_dirty = 1;
    ZeroMemory(&g_pacer,sizeof(g_pacer));
    QueryPerformanceFrequency(&g_clock_frequency);
    /* Windows 10 1803+ supports high-resolution timers; older hosts use
     * the same deadline schedule with the ordinary waitable timer. */
    g_frame_timer=CreateWaitableTimerExA(NULL,NULL,0x2u,TIMER_ALL_ACCESS);
    if (!g_frame_timer) g_frame_timer=CreateWaitableTimerA(NULL,FALSE,NULL);
    ftrace_init();
    g_host_window_open = 1;
    HostWindow_SetTitle(title);
    PE_AudioDriver_SetVBlankPacer(pace_vblank);
    fprintf(stderr, "[WINDOW] %dx%d (fb %dx%d) title='%s'%s scaler=gdi\n",
            g_win_w, g_win_h, g_fb_w, g_fb_h, title ? title : "PE",
            fullscreen ? " fullscreen" : "");
    /* Opt-in controls.conf (pad.* / key.* remaps), read once per window; no
     * file leaves the pad word and the key table exactly as before. */
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

/* BRIGHTNESS (plat_enhance.c): weak like the scaler's; a change redraws an
 * unchanged frame.  The gain is applied to freshly written pixels only. */
extern int pe_enh_brightness_pct(void) __attribute__((weak));
static int brightness_sync(void)
{
    int pct = pe_enh_brightness_pct ? pe_enh_brightness_pct() : 100;
    if (pct != g_bright_pct) { g_bright_pct = pct; g_blit_dirty = 1; }
    return pct;
}
/* BRIGHTNESS over the picture rectangle of the window image (BGRA): the
 * letterbox the scaler just wrote.  Black stays black at every step. */
static void brightness_picture(int fb_w, int fb_h)
{
    uint8_t lut[256];
    int x0, y0, w, h;
    if (!HostQol_BrightnessLut(lut, g_bright_pct)) return;
    HostWindow_FitRect(g_win_w, g_win_h, fb_w, fb_h, &x0, &y0, &w, &h);
    HostQol_LutRect(g_imgbuf, g_win_w, 4, x0, y0, w, h, lut);
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
    HDC hdc;
    RECT client;
    int width, height, scaler, actual, unchanged;
    uint64_t t0 = 0, t1;
    size_t size;
    if (!g_hwnd || !g_imgbuf || !rgb || fb_w <= 0 || fb_h <= 0) return;
    if (!GetClientRect(g_hwnd, &client)) return;
    width = client.right; height = client.bottom;
    if (width <= 0 || height <= 0) return;
    if (width != g_win_w || height != g_win_h) {
        uint8_t *buffer = realloc(g_imgbuf, (size_t)width * (size_t)height * 4u);
        if (!buffer) { g_close_requested = 1; return; }
        g_imgbuf = buffer; g_win_w = width; g_win_h = height;
        g_bmi.bmiHeader.biWidth = width;
        g_bmi.bmiHeader.biHeight = -height;
        g_blit_dirty = 1;
    }
    /* DISPLAY SCALER graphics option; NEAREST (the default) is
     * HostWindow_ScaleBGRA unchanged.  This backend has no presenter
     * thread, so a filtered mode scales on the game thread. */
    scaler = display_scaler_mode();
    (void)brightness_sync();
    size = (size_t)fb_w * (size_t)fb_h * 3u;
    /* A present or scan-out that shows exactly the pixels already on
     * screen (a movie's catch-up presents, the static pause frame
     * rescanned every VBlank) changes nothing visible: skip it unless the
     * window itself needs a redraw. */
    unchanged = !g_blit_dirty && g_last_rgb && g_last_size == size && memcmp(g_last_rgb, rgb, size) == 0;
    if (unchanged) {
        if (g_ft.on) g_ft.skipped++;
        return;
    }
    if (g_ft.on) t0 = monotonic_ns();
    actual = HostUpscale_Run(scaler, g_imgbuf, g_win_w, g_win_h, rgb, fb_w, fb_h);
    brightness_picture(fb_w, fb_h);   /* BRIGHTNESS on the rectangle just written */
    hdc = GetDC(g_hwnd);
    if (hdc) {
        paint_window(hdc);
        ReleaseDC(g_hwnd, hdc);
    }
    if (scaler == HOST_UPSCALE_NEAREST || actual == scaler) {
        g_blit_dirty = 0;
        remember_frame(rgb, fb_w, fb_h);
    } else {
        g_blit_dirty = 1;   /* a failed filter stays retryable */
    }
    if (g_ft.on) {
        t1 = monotonic_ns();
        g_ft.blits++; g_ft.blit_ns += t1 - t0;
        if (t1 - t0 > g_ft.blit_max) g_ft.blit_max = t1 - t0;
    }
}

/* The title is the game's name alone ("Parasite Eve" from the packaged
 * launchers); " [Fast-forward]" is appended only while fast-forward runs. */
void HostWindow_SetTitle(const char *title)
{
    char label[560];
    if (title) snprintf(g_title,sizeof(g_title),"%s",title);
    if (g_ff_active) snprintf(label,sizeof(label),"%s [Fast-forward]",g_title);
    else snprintf(label,sizeof(label),"%s",g_title);
    if (g_hwnd) SetWindowTextA(g_hwnd,label);
}

int HostWindow_Poll(void)
{
    MSG msg;

    if (!g_hwnd && !g_close_requested) return 0;
    HostPad_Poll();
    HostDevMenu_Poll();
    {
        uint8_t hk=(pe_plat_cheats_menu_open() || HostDevMenu_CapturesInput()) ? 0 : HostPad_Hotkeys();
        if ((hk&HOST_PAD_HOTKEY_FF_TOGGLE)&&!(g_pad_hotkeys&HOST_PAD_HOTKEY_FF_TOGGLE))
            g_fast_forward=!g_fast_forward;
        g_pad_hotkeys=hk;
        ff_update("controller");
    }
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) { g_close_requested = 1; break; }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        if (g_close_requested) return 1;
    }
    return g_close_requested ? 1 : 0;
}

/* Opt-in ANALOG STICK on and a controller connected: the pad replies analog. */
static int window_pad_analog(uint8_t sticks[4])
{
    return pe_enh_analog_stick() && HostPadXInput_AnalogSticks(sticks);
}

uint16_t HostWindow_PadRaw(void)
{
    uint8_t sticks[4];
    /* Keyboard and controller both drive the pad at once.  The controller
     * word goes through the dev-menu combo filter (a lone Select/Start is
     * withheld only while it could still become Select+Start), which reads the
     * physical bits; the optional controls.conf pad.* remap applies to the
     * filter's output only, so the combo stays reachable whatever the remap and
     * the keyboard (already mapped by its own table) is never remapped twice.
     * In an analog reply the left stick is the analog bytes, not the d-pad. */
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

/* Sleep to the next 59.94 Hz deadline, polling input.  Returns 1 on
 * close or a host clock failure. */
static int pace_one_frame(void)
{
    uint64_t deadline;
    if (!g_frame_timer || !g_clock_frequency.QuadPart) return 1;
    deadline=HostFramePacer_Deadline(&g_pacer,monotonic_ns());
    for (;;) {
        uint64_t now,remaining;
        LARGE_INTEGER due;
        if (HostWindow_Poll()) return 1;
        if (g_ff_active && pe_plat_cheats_ff_multiplier() == 0) return 0;
        now=monotonic_ns();
        if (now>=deadline) return 0;
        remaining=deadline-now;
        if (remaining>4000000u) remaining=4000000u;
        /* Recheck the absolute deadline after early wake-ups and oversleep. */
        due.QuadPart=-(LONGLONG)((remaining+99u)/100u);
        if (!SetWaitableTimer(g_frame_timer,&due,0,NULL,NULL,FALSE)) return 1;
        if (WaitForSingleObject(g_frame_timer,INFINITE)!=WAIT_OBJECT_0) return 1;
    }
}

/* PeVBlankPacer: one emulated VBlank = one 59.94 Hz frame of wall time. */
static void pace_vblank(void)
{
    if (!g_hwnd) return;
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
        uint64_t now = monotonic_ns();
        if (g_last_vblank_ns && g_ft.n < FTRACE_SAMPLES) g_ft.busy[g_ft.n++] = now - g_last_vblank_ns;
        g_ft.paced++;
    }
    if (pace_one_frame()) PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
    g_last_vblank_ns = monotonic_ns();
    if (g_ft.on) ftrace_sample(g_last_vblank_ns);
}

int HostWindow_Pace(void)
{
    if (g_ff_active) return HostWindow_Poll();
    /* Emulated VBlanks pace the game; a present only polls (catch-up
     * presents such as pe_str_feed's cost no time).  Safety net: if no
     * VBlank has paced for 100 ms of wall time, a scene is presenting
     * without VBlanks and presents are paced one frame each. */
    if (g_vblank_paced && monotonic_ns()-g_last_vblank_ns<UINT64_C(100000000))
        return HostWindow_Poll();
    return pace_one_frame();
}

int HostWindow_Run(int milliseconds)
{
    ULONGLONG start = GetTickCount64();

    if (!g_hwnd) return 0;
    for (;;) {
        int rc = HostWindow_Poll();
        ULONGLONG elapsed;
        if (rc) return rc;
        elapsed = GetTickCount64() - start;
        if (milliseconds > 0 && elapsed >= (ULONGLONG)milliseconds) return 0;
        Sleep(16); /* ~60 Hz poll */
    }
}

void HostWindow_Close(void)
{
    HostUpscale_Shutdown();
    PE_AudioDriver_SetVBlankPacer(NULL);
    if (g_frame_timer) {CloseHandle(g_frame_timer);g_frame_timer=NULL;}
    free(g_last_rgb); g_last_rgb = NULL; g_last_size = 0;
    g_sony_held = 0;
    g_cheat_held = 0;
    HostPadRemap_Reset();
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
    free(g_imgbuf);
    g_imgbuf = NULL;
    g_host_window_open = 0;
    HostDevMenu_Close();
    HostPad_Close();
}

/*
 * Phase WIN1 — Native Win32 window backend (GDI, no extra dependencies).
 *
 * Implements the same platform/host_window.h interface as host_window.c
 * (X11). Selected by CMake when WIN32 is true. Uses only Win32 API calls
 * present since Windows XP: RegisterClassA, CreateWindowExA, GetDC,
 * StretchDIBits, PeekMessageA, SetWindowTextA. No SDL, no DirectX, no
 * additional libraries — links against stock gdi32/user32.
 *
 * Pad mapping mirrors the X11 backend exactly:
 *   Cross  = Return / Space / Z / X  (raw 0x4000)
 *   Up/Right/Down/Left               (raw 0x10/0x20/0x40/0x80)
 *   Circle/Triangle/Square = C/V/S; L1/R1 = Q/E; L2/R2 = 1/3
 *   Select/Start = Tab/P
 *   Escape or window-close           (close requested)
 */

#include "host_window.h"
#include "host_frame_pacer.h"
#include "host_window_scale.h"
#include "pe_plat/cheats.h"

/* GetTickCount64 needs Vista+ declarations on older MinGW defaults. */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

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
static int g_fast_forward;
static char g_title[512];

/* Translate a virtual-key code to a Sony pad bit (0 = unmapped). */
static uint16_t vk_to_pad_bit(UINT vk)
{
    switch (vk) {
    case VK_RETURN:
    case VK_SPACE:
    case 'Z':
    case 'X':
        return 0x4000u;
    case VK_UP:    return 0x0010u;
    case VK_RIGHT: return 0x0020u;
    case VK_DOWN:  return 0x0040u;
    case VK_LEFT:  return 0x0080u;
    case 'C':      return 0x2000u;
    case 'V':      return 0x1000u;
    case 'S':      return 0x8000u;
    case 'Q':      return 0x0400u;
    case 'E':      return 0x0800u;
    case '1':      return 0x0100u;
    case '3':      return 0x0200u;
    case VK_TAB:   return 0x0001u;
    case 'P':      return 0x0008u;
    default:       return 0u;
    }
}

static void paint_window(HDC hdc)
{
    if (!g_imgbuf || g_win_w <= 0 || g_win_h <= 0) return;
    StretchDIBits(hdc, 0, 0, g_win_w, g_win_h,
                  0, 0, g_win_w, g_win_h,
                  g_imgbuf, &g_bmi, DIB_RGB_COLORS, SRCCOPY);
}

/* Cheat console (pe_plat/cheats.h): F1-F5, F7, F8; arrows and Enter only
 * while its warp menu is open.  repeat = key auto-repeat (ignored for F-keys). */
static int win32_cheat_key(WPARAM vk, int pressed, int repeat)
{
    PeCheatKey ck = PE_CHEAT_KEY_NONE;
    switch (vk) {
    case VK_F1: ck = PE_CHEAT_KEY_F1; break;
    case VK_F2: ck = PE_CHEAT_KEY_F2; break;
    case VK_F3: ck = PE_CHEAT_KEY_F3; break;
    case VK_F4: ck = PE_CHEAT_KEY_F4; break;
    case VK_F5: ck = PE_CHEAT_KEY_F5; break;
    case VK_F7: ck = PE_CHEAT_KEY_F7; break;
    case VK_F8: ck = PE_CHEAT_KEY_F8; break;
    case VK_UP: if (pe_plat_cheats_menu_open()) ck = PE_CHEAT_KEY_UP; break;
    case VK_DOWN: if (pe_plat_cheats_menu_open()) ck = PE_CHEAT_KEY_DOWN; break;
    case VK_RETURN: if (pe_plat_cheats_menu_open()) ck = PE_CHEAT_KEY_ENTER; break;
    default: break;
    }
    if (ck == PE_CHEAT_KEY_NONE) return 0;
    if (pressed && repeat && ck != PE_CHEAT_KEY_UP && ck != PE_CHEAT_KEY_DOWN) return 1;
    return pe_plat_cheats_key(ck, pressed);
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
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (win32_cheat_key(wparam, 1, (lparam & ((LPARAM)1 << 30)) != 0)) return 0;
        if (wparam==VK_F6) {
            if (!(lparam & ((LPARAM)1 << 30))) {
                g_fast_forward=!g_fast_forward;
                ZeroMemory(&g_pacer,sizeof(g_pacer));
                HostWindow_SetTitle(NULL);
                fprintf(stderr,"[WINDOW] %s (F6 toggles speed)\n",g_fast_forward?"Fast-forward":"Normal speed: 59.94 Hz");
            }
            return 0;
        }
        if (wparam == VK_ESCAPE) { g_close_requested = 1; return 0; }
        g_sony_held |= vk_to_pad_bit((UINT)wparam);
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (win32_cheat_key(wparam, 0, 0)) return 0;
        g_sony_held &= (uint16_t)~vk_to_pad_bit((UINT)wparam);
        return 0;
    case WM_KILLFOCUS:
        g_sony_held = 0;
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

int HostWindow_Open(const char *display, int width, int height,
                    const char *title, int scale, int fullscreen)
{
    WNDCLASSA wc;
    RECT rc;
    DWORD style = fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
    (void)display; /* no display string on Windows */

    if (scale < 1) scale = 1;
    g_fb_w = PE_PORT_FB_WIDTH;
    g_fb_h = PE_PORT_FB_HEIGHT;
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
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
    g_fast_forward = 0;
    ZeroMemory(&g_pacer,sizeof(g_pacer));
    QueryPerformanceFrequency(&g_clock_frequency);
    /* Windows 10 1803+ supports high-resolution timers; older hosts use
     * the same deadline schedule with the ordinary waitable timer. */
    g_frame_timer=CreateWaitableTimerExA(NULL,NULL,0x2u,TIMER_ALL_ACCESS);
    if (!g_frame_timer) g_frame_timer=CreateWaitableTimerA(NULL,FALSE,NULL);
    g_host_window_open = 1;
    HostWindow_SetTitle(title);
    fprintf(stderr, "[WINDOW] %dx%d (fb %dx%d) title='%s'\n",
            g_win_w, g_win_h, g_fb_w, g_fb_h, title ? title : "PE");
    return 0;
}

void HostWindow_Blit(const uint8_t *rgb, int fb_w, int fb_h)
{
    HDC hdc;
    RECT client;
    int width, height;
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
    }
    HostWindow_ScaleBGRA(g_imgbuf, g_win_w, g_win_h, rgb, fb_w, fb_h);
    hdc = GetDC(g_hwnd);
    if (hdc) {
        paint_window(hdc);
        ReleaseDC(g_hwnd, hdc);
    }
}

void HostWindow_SetTitle(const char *title)
{
    char label[560];
    if (title) snprintf(g_title,sizeof(g_title),"%s",title);
    snprintf(label,sizeof(label),"%s [%s]",g_title,g_fast_forward?"Fast-forward":"Normal speed");
    if (g_hwnd) SetWindowTextA(g_hwnd,label);
}

int HostWindow_Poll(void)
{
    MSG msg;

    if (!g_hwnd && !g_close_requested) return 0;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) { g_close_requested = 1; break; }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        if (g_close_requested) return 1;
    }
    return g_close_requested ? 1 : 0;
}

uint16_t HostWindow_PadRaw(void)
{
    return (uint16_t)(~g_sony_held);
}

static uint64_t monotonic_ns(void)
{
    LARGE_INTEGER counter;
    uint64_t ticks,frequency=(uint64_t)g_clock_frequency.QuadPart;
    QueryPerformanceCounter(&counter);ticks=(uint64_t)counter.QuadPart;
    return (ticks/frequency)*UINT64_C(1000000000)+
           ((ticks%frequency)*UINT64_C(1000000000))/frequency;
}

int HostWindow_Pace(void)
{
    uint64_t deadline;
    if (g_fast_forward) {
        /* Cheat console F7: fixed N-x speed pays one wall frame per N
         * presents; 0 = uncapped (the F6 default). */
        static unsigned ff_n;
        int mult = pe_plat_cheats_ff_multiplier();
        if (mult <= 0 || (++ff_n % (unsigned)mult) != 0u) return HostWindow_Poll();
    }
    if (!g_frame_timer || !g_clock_frequency.QuadPart) return 1;
    deadline=HostFramePacer_Deadline(&g_pacer,monotonic_ns());
    for (;;) {
        uint64_t now,remaining;
        LARGE_INTEGER due;
        if (HostWindow_Poll()) return 1;
        if (g_fast_forward && pe_plat_cheats_ff_multiplier() == 0) return 0;
        now=monotonic_ns();
        if (now>=deadline) return 0;
        remaining=deadline-now;
        if (remaining>4000000u) remaining=4000000u;
        due.QuadPart=-(LONGLONG)((remaining+99u)/100u);
        if (!SetWaitableTimer(g_frame_timer,&due,0,NULL,NULL,FALSE)) return 1;
        if (WaitForSingleObject(g_frame_timer,INFINITE)!=WAIT_OBJECT_0) return 1;
    }
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
    if (g_frame_timer) {CloseHandle(g_frame_timer);g_frame_timer=NULL;}
    g_sony_held = 0;
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
    free(g_imgbuf);
    g_imgbuf = NULL;
    g_host_window_open = 0;
}

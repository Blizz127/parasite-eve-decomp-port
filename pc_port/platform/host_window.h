/*
 * Phase 6B — Native X11 window backend (dlopen, no dev headers).
 *
 * Loads libX11.so.6 at runtime.  No manual struct layouts — we use
 * only function pointers and primitive types (Window = unsigned long).
 * The XImage buffer is caller-allocated.
 *
 * Backend: libX11 via dlopen (SDL2 preferred but unavailable on this VPS).
 */

#ifndef HOST_WINDOW_H
#define HOST_WINDOW_H

#include <stdint.h>

/* Open a window.  width/height are the output (client) size (ignored, in
 * favor of the screen size, when fullscreen is nonzero).
 * title sets the window title bar. scale is the initial integer zoom factor.
 * fullscreen requests EWMH _NET_WM_STATE_FULLSCREEN (X11 backend) / an
 * OS-native borderless-fullscreen equivalent; the 320x240 (or current
 * display-mode) framebuffer is then letterboxed/pillarboxed into whatever
 * size the window manager or compositor (e.g. gamescope/Steam Game Mode)
 * actually grants, including on a later resize.
 * Returns 0 on success, -1 on failure. */
int  HostWindow_Open(const char *display, int width, int height,
                     const char *title, int scale, int fullscreen);

/* Upload RGB 8:8:8 framebuffer pixels to the window.
 * fb_w/fb_h are the SOURCE framebuffer dimensions (always 320×240).
 * Nearest-neighbor scaling fits the current client area with black borders
 * while preserving the source aspect ratio. */
void HostWindow_Blit(const uint8_t *rgb, int fb_w, int fb_h);

/* Process pending X11 events.  Returns:
 *  0 = normal
 *  1 = close requested (Escape or window-close) */
int  HostWindow_Poll(void);

/* Wait for the next 60000/1001 Hz presentation deadline while polling input.
 * Returns 1 on close or a host clock failure, 0 when the frame is due. */
int  HostWindow_Pace(void);

/* Update the window title (used for debug overlay). */
void HostWindow_SetTitle(const char *title);

/* Run the event loop for a fixed number of milliseconds, or until
 * close is requested.  Returns 0 on timeout, 1 on close. */
int  HostWindow_Run(int milliseconds);

/* Close the window and release resources. */
void HostWindow_Close(void);

/* Nonzero when a window was successfully opened. */
extern int g_host_window_open;

/* Active-low Sony digital word from X11 keys OR-ed with any Linux
 * joystick (/dev/input/js*, see host_pad.h). Idle is 0xFFFF.
 * Cross is XK_Return / XK_space / XK_z / XK_x / pad A (raw 0x4000). */
uint16_t HostWindow_PadRaw(void);

#endif

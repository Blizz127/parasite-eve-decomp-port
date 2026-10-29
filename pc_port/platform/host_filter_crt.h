#ifndef HOST_FILTER_CRT_H
#define HOST_FILTER_CRT_H
/* CRT display filter for the presenter (the "DISPLAY FILTER" graphics option).
 *
 * Darkens the gaps between the frame's scanlines and, optionally, tints the
 * window columns with a repeating red/green/blue aperture-grille mask, on
 * the finished window image.  It runs after the Display Scaler and the
 * Window Fit, in place, over the frame's rectangle only (black bars stay
 * black); emulated VRAM, screenshots and every counter never see it.
 *
 * Backend independent: a pure function over a caller-owned BGRA window
 * buffer, no X11/Win32/SDL, no global state, integer arithmetic only (the
 * result is the same bytes on every compiler and thread).  Header-only
 * (static inline) like host_window_scale.h.  A GPU backend can use
 * HostFilter_CrtRowGain / HostFilter_CrtMaskGain as the constants of its
 * own shader instead of calling HostFilter_ApplyCrt.
 *
 *   HOST_CRT_OFF        retail: nothing is touched (the function returns)
 *   HOST_CRT_SCANLINES  scanlines only
 *   HOST_CRT_MASK       scanlines plus the aperture mask
 *
 * Scanlines.  The frame has `lines` source scanlines (240; the native height
 * even when the frame is a larger presentation frame).  Window row yy of h
 * sits at source position p = (yy + 0.5) * lines / h; t is its distance
 * from the centre of the source line it falls on (0 on the centre, 1 on the
 * edge between two lines).  The row's brightness is
 *     gain * (1 - depth * t * t)
 * a smooth beam profile: full brightness on a line's centre, `depth` darker
 * on the gap.  The gain (> 1) keeps the picture's average brightness close
 * to the original; a channel that would pass 255 is clamped.  With fewer
 * than HOST_CRT_MIN_ROWS rows per source line there is no room for a gap,
 * so the scanlines are skipped (the frame's rectangle must be at least 2x the
 * native height, 480 rows, for them to show).
 *
 * Mask.  Window columns repeat in threes (counted from the rectangle's left
 * edge): in column c the channel c is at HOST_CRT_MASK_HI and the other two
 * at HOST_CRT_MASK_LO (BGRA memory: red is byte 2).  The mask is the same
 * on every row.  At 3x the native width one source pixel spans one triad.
 *
 * Gains are 8.8 fixed point (256 = 1.0). */
#include "host_filter_fit.h"

enum { HOST_CRT_OFF = 0, HOST_CRT_SCANLINES = 1, HOST_CRT_MASK = 2 };

#define HOST_CRT_SCAN_DEPTH16   52429   /* 0.80 in 16.16: the gap keeps 20 % of the line's brightness */
#define HOST_CRT_SCAN_GAIN      320     /* 1.25 */
#define HOST_CRT_MASK_HI        320     /* 1.25: the triad's lit channel */
#define HOST_CRT_MASK_LO        198     /* 0.77: its other two */
#define HOST_CRT_MIN_ROWS       2       /* window rows per source line needed for a gap */

/* Gain of window row yy (0..h-1) of a frame rectangle h rows tall, for a
 * frame of `lines` source scanlines. */
static inline int HostFilter_CrtRowGain(int yy, int h, int lines)
{
    int64_t a = (int64_t)(2 * yy + 1) * lines;
    int64_t rem = a % (2 * (int64_t)h);                 /* position in the source line, 0..2h-1 */
    int64_t dist = rem > h ? rem - h : h - rem;         /* 0 on the centre, h on the edge */
    int64_t tt = (dist * dist * 65536) / ((int64_t)h * h);       /* t*t, 16.16 */
    int64_t w = 65536 - ((HOST_CRT_SCAN_DEPTH16 * tt) >> 16);    /* 1 - depth*t*t, 16.16 */
    return (int)((w * HOST_CRT_SCAN_GAIN + 32768) >> 16);
}

/* Gain of colour channel ch (0 red, 1 green, 2 blue) in mask column c (0..2). */
static inline int HostFilter_CrtMaskGain(int c, int ch)
{
    return c == ch ? HOST_CRT_MASK_HI : HOST_CRT_MASK_LO;
}

/* Apply `mode` to rectangle r of the width x height BGRA window buffer. */
static inline void HostFilter_ApplyCrt(uint8_t *out, int width, int height,
                                       const HostFilterRect *r, int lines, int mode)
{
    int scan, mask, yy, x, c;
    size_t row = (size_t)width * 4u;
    if (mode == HOST_CRT_OFF || !out) return;
    if (r->w <= 0 || r->h <= 0 || r->x0 < 0 || r->y0 < 0 || r->x0 + r->w > width || r->y0 + r->h > height) return;
    scan = lines > 0 && r->h >= HOST_CRT_MIN_ROWS * lines;
    mask = mode >= HOST_CRT_MASK;
    if (!scan && !mask) return;
    for (yy = 0; yy < r->h; yy++) {
        int rg = scan ? HostFilter_CrtRowGain(yy, r->h, lines) : 256;
        int g[3][3];                       /* [column phase][byte: B, G, R] */
        uint8_t *p = out + row * (size_t)(r->y0 + yy) + (size_t)r->x0 * 4u;
        if (rg == 256 && !mask) continue;
        for (c = 0; c < 3; c++) {
            for (x = 0; x < 3; x++) {      /* byte x holds channel 2 - x */
                int mg = mask ? HostFilter_CrtMaskGain(c, 2 - x) : 256;
                g[c][x] = (rg * mg + 128) >> 8;
            }
        }
        for (x = 0; x < r->w; x++, p += 4) {
            const int *gc = g[x % 3];
            int v;
            v = (p[0] * gc[0] + 128) >> 8; p[0] = (uint8_t)(v > 255 ? 255 : v);
            v = (p[1] * gc[1] + 128) >> 8; p[1] = (uint8_t)(v > 255 ? 255 : v);
            v = (p[2] * gc[2] + 128) >> 8; p[2] = (uint8_t)(v > 255 ? 255 : v);
        }
    }
}

#endif

#ifndef HOST_FILTER_FIT_H
#define HOST_FILTER_FIT_H
/* Window-fit helpers for the presenter (the "WINDOW FIT" graphics option).
 *
 * Where the finished frame lands in the window, and how a scaler's output is
 * put there.  Backend independent: pure functions over caller-owned buffers,
 * no X11/Win32/SDL, no global state, integer arithmetic only (so the result
 * is the same bytes on every compiler and every thread).  Header-only
 * (static inline) like host_window_scale.h, so the X11, Win32 and SDL
 * presenters and the unit tests all include it.
 *
 *   HOST_FIT_KEEP     the retail letterbox: HostWindow_FitRect, the largest
 *                     aspect-preserving fit (default; byte-identical)
 *   HOST_FIT_INTEGER  the largest whole multiple of the native frame that
 *                     fits, centred, black around it (crisp square pixels).
 *                     A window smaller than 1x falls back to HOST_FIT_KEEP.
 *   HOST_FIT_STRETCH  the whole window, ignoring the aspect ratio
 *
 * A backend asks HostFilter_Plan once per present.  plan.direct means the
 * rectangle is the retail letterbox: run the scaler exactly as before into
 * the window.  Otherwise either scale with HostFilter_ScaleNearestRect
 * straight into plan.rect (nearest), or run any scaler on a plan.bw x
 * plan.bh box (a window of that size, which the frame fits exactly: the
 * scaler's own letterbox leaves no bar) and put the box into the window with
 * HostFilter_Place.  A GPU backend can use plan.rect as its viewport and
 * ignore the rest.
 *
 * Pixel formats: the source is RGB888 (3 bytes per pixel), every window
 * buffer is BGRA (4 bytes, alpha 0) like HostWindow_ScaleBGRA's. */
#include "host_window_scale.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum { HOST_FIT_KEEP = 0, HOST_FIT_INTEGER = 1, HOST_FIT_STRETCH = 2 };

typedef struct { int x0, y0, w, h; } HostFilterRect;

typedef struct {
    HostFilterRect rect;   /* where the frame lands in the window */
    int direct;            /* 1: rect is the retail letterbox (HostWindow_FitRect) */
    int bw, bh;            /* box a scaler runs at when !direct (see above) */
} HostFilterPlan;

static inline int HostFilter_RectEq(const HostFilterRect *a, const HostFilterRect *b)
{
    return a->x0 == b->x0 && a->y0 == b->y0 && a->w == b->w && a->h == b->h;
}

/* The rectangle for `fit`.  fb_w x fb_h is the frame (it only fixes the
 * aspect); unit_w x unit_h is the native pixel grid whole multiples are
 * taken of (320x240 even when the frame is a larger presentation frame). */
static inline void HostFilter_FitRect(int fit, int win_w, int win_h, int fb_w, int fb_h,
                                      int unit_w, int unit_h, HostFilterRect *r)
{
    if (fit == HOST_FIT_INTEGER && unit_w > 0 && unit_h > 0) {
        int k = win_w / unit_w;
        if (win_h / unit_h < k) k = win_h / unit_h;
        if (k >= 1) {
            r->w = unit_w * k; r->h = unit_h * k;
            r->x0 = (win_w - r->w) / 2; r->y0 = (win_h - r->h) / 2;
            return;
        }
    } else if (fit == HOST_FIT_STRETCH && win_w > 0 && win_h > 0) {
        r->x0 = 0; r->y0 = 0; r->w = win_w; r->h = win_h;
        return;
    }
    HostWindow_FitRect(win_w, win_h, fb_w, fb_h, &r->x0, &r->y0, &r->w, &r->h);
}

/* The largest box of the frame's exact aspect inside w x h: the frame
 * fb_w x fb_h reduced by its gcd (4:3 for 320x240) times the largest whole
 * factor that fits.  A scaler run at that size fills its box with no bar
 * (HostWindow_FitRect of the box is the box itself); a box that is merely the
 * aspect fit, rounded to the pixel, can be a row or column off 4:3 and leave a
 * one-pixel black edge that a stretch would then enlarge. */
static inline void HostFilter_ExactBox(int w, int h, int fb_w, int fb_h, int *bw, int *bh)
{
    int a = fb_w, b = fb_h, t;
    while (b) { int r = a % b; a = b; b = r; }          /* a = gcd(fb_w, fb_h) */
    t = w / (fb_w / a);
    if (h / (fb_h / a) < t) t = h / (fb_h / a);
    if (t < 1) { *bw = w; *bh = h; return; }
    *bw = (fb_w / a) * t; *bh = (fb_h / a) * t;
}

static inline void HostFilter_Plan(HostFilterPlan *p, int fit, int win_w, int win_h,
                                   int fb_w, int fb_h, int unit_w, int unit_h)
{
    HostFilterRect keep;
    HostWindow_FitRect(win_w, win_h, fb_w, fb_h, &keep.x0, &keep.y0, &keep.w, &keep.h);
    HostFilter_FitRect(fit, win_w, win_h, fb_w, fb_h, unit_w, unit_h, &p->rect);
    p->direct = HostFilter_RectEq(&p->rect, &keep);
    if (p->direct) { p->bw = win_w; p->bh = win_h; }
    else if (fit == HOST_FIT_STRETCH)                                       /* stretched into rect by Place */
        HostFilter_ExactBox(keep.w, keep.h, fb_w, fb_h, &p->bw, &p->bh);
    else { p->bw = p->rect.w; p->bh = p->rect.h; }                         /* a whole multiple: copied */
}

/* Black everywhere in the window outside r. */
static inline void HostFilter_Bars(uint8_t *out, int width, int height, const HostFilterRect *r)
{
    size_t row = (size_t)width * 4u;
    int y;
    if (r->y0 > 0) memset(out, 0, row * (size_t)r->y0);
    if (r->y0 + r->h < height) memset(out + row * (size_t)(r->y0 + r->h), 0, row * (size_t)(height - r->y0 - r->h));
    for (y = r->y0; y < r->y0 + r->h; y++) {
        uint8_t *line = out + row * (size_t)y;
        if (r->x0 > 0) memset(line, 0, (size_t)r->x0 * 4u);
        if (r->x0 + r->w < width) memset(line + (size_t)(r->x0 + r->w) * 4u, 0, (size_t)(width - r->x0 - r->w) * 4u);
    }
}

/* Nearest-neighbour scale of rgb (fb_w x fb_h) into rectangle r of the
 * width x height BGRA window buffer, black outside.  For r =
 * HostWindow_FitRect this is HostWindow_ScaleBGRA byte for byte (same
 * sampling), so the nearest scaler and the XRender path keep agreeing. */
static inline void HostFilter_ScaleNearestRect(uint8_t *out, int width, int height,
                                               const uint8_t *rgb, int fb_w, int fb_h,
                                               const HostFilterRect *r)
{
    size_t row = (size_t)width * 4u;
    int *xmap;
    int x, y, prev = -1;
    int32_t xstep, ystep;
    if (r->w <= 0 || r->h <= 0 || r->x0 < 0 || r->y0 < 0 || r->x0 + r->w > width ||
        r->y0 + r->h > height || fb_w <= 0 || fb_h <= 0) {
        memset(out, 0, row * (size_t)height);
        return;
    }
    xmap = (int *)malloc((size_t)r->w * sizeof *xmap);
    if (!xmap) { memset(out, 0, row * (size_t)height); return; }
    xstep = HostWindow_NearestStep(fb_w, r->w); ystep = HostWindow_NearestStep(fb_h, r->h);
    for (x = 0; x < r->w; x++) xmap[x] = HostWindow_NearestIndex(x, xstep) * 3;
    HostFilter_Bars(out, width, height, r);
    for (y = 0; y < r->h; y++) {
        int sy = HostWindow_NearestIndex(y, ystep);
        uint8_t *dst = out + row * (size_t)(y + r->y0) + (size_t)r->x0 * 4u;
        if (sy == prev) { memcpy(dst, dst - row, (size_t)r->w * 4u); continue; }
        prev = sy;
        {
            const uint8_t *src = rgb + (size_t)sy * (size_t)fb_w * 3u;
            for (x = 0; x < r->w; x++) {
                const uint8_t *p = src + xmap[x];
                dst[0] = p[2]; dst[1] = p[1]; dst[2] = p[0]; dst[3] = 0; dst += 4;
            }
        }
    }
    free(xmap);
}

/* Sample positions along one axis of HostFilter_Place: dest index x of
 * `dst` samples reads source samples i0 and i1 of a `src`-sample line with
 * weight wt/256 on i1.  Pixel centres are aligned.  An axis within 1 % of
 * the destination length (including equal) is mapped by nearest index with
 * weight 0 instead (at worst one repeated line in 100): a window a few
 * pixels off 4:3 then costs a one-axis stretch, not a two-axis one. */
static inline void HostFilter_PlaceAxis(int dst, int src, int *idx)
{
    int snap = (int64_t)(dst > src ? dst - src : src - dst) * 100 <= src, x;
    for (x = 0; x < dst; x++) {
        int i0, i1 = 0, wt = 0;
        if (snap) {
            i0 = (int)(((int64_t)(2 * x + 1) * src) / (2 * (int64_t)dst));
            if (i0 >= src) i0 = src - 1;
            i1 = i0;
        } else {
            int64_t num = (int64_t)(2 * x + 1) * src - dst;     /* source position * 2 * dst */
            i0 = 0;
            if (num > 0) {
                int64_t den = 2 * (int64_t)dst;
                i0 = (int)(num / den);
                if (i0 >= src - 1) { i0 = i1 = src - 1; }
                else { i1 = i0 + 1; wt = (int)(((num % den) * 256) / den); }
            } else i1 = 0;
        }
        idx[x * 3] = i0; idx[x * 3 + 1] = i1; idx[x * 3 + 2] = wt;
    }
}

/* Put a bw x bh BGRA box (a scaler's output) into rectangle r of the window
 * buffer, black outside.  A box the size of r is copied; any other size is
 * stretched bilinearly with 8-bit weights (the box is already smooth, so
 * the stretch only has to spread it along the axis that differs; an axis
 * within 1 % is mapped by nearest index, see HostFilter_PlaceAxis). */
static inline void HostFilter_Place(uint8_t *out, int width, int height,
                                    const uint8_t *box, int bw, int bh, const HostFilterRect *r)
{
    size_t row = (size_t)width * 4u;
    int y;
    if (r->w <= 0 || r->h <= 0 || bw <= 0 || bh <= 0 || r->x0 < 0 || r->y0 < 0 ||
        r->x0 + r->w > width || r->y0 + r->h > height) {
        memset(out, 0, row * (size_t)height);
        return;
    }
    HostFilter_Bars(out, width, height, r);
    if (bw == r->w && bh == r->h) {
        for (y = 0; y < r->h; y++)
            memcpy(out + row * (size_t)(y + r->y0) + (size_t)r->x0 * 4u,
                   box + (size_t)y * (size_t)bw * 4u, (size_t)r->w * 4u);
        return;
    }
    {
        int *xa = (int *)malloc(((size_t)r->w + (size_t)r->h) * 3u * sizeof(int));   /* columns, then rows */
        int *ya, x;
        if (!xa) { memset(out, 0, row * (size_t)height); return; }
        ya = xa + (size_t)r->w * 3u;
        HostFilter_PlaceAxis(r->w, bw, xa);
        HostFilter_PlaceAxis(r->h, bh, ya);
        for (y = 0; y < r->h; y++) {
            const uint8_t *ra = box + (size_t)ya[y * 3] * (size_t)bw * 4u, *rb = box + (size_t)ya[y * 3 + 1] * (size_t)bw * 4u;
            int wy = ya[y * 3 + 2];
            uint8_t *dst = out + row * (size_t)(y + r->y0) + (size_t)r->x0 * 4u;
            if (!wy) {                                            /* one source row: copy or stretch along the row */
                for (x = 0; x < r->w; x++, dst += 4) {
                    const uint8_t *p0 = ra + (size_t)xa[x * 3] * 4u, *p1 = ra + (size_t)xa[x * 3 + 1] * 4u;
                    int wx = xa[x * 3 + 2];
                    if (!wx) { memcpy(dst, p0, 4u); continue; }
                    dst[0] = (uint8_t)((p0[0] * (256 - wx) + p1[0] * wx + 128) >> 8);
                    dst[1] = (uint8_t)((p0[1] * (256 - wx) + p1[1] * wx + 128) >> 8);
                    dst[2] = (uint8_t)((p0[2] * (256 - wx) + p1[2] * wx + 128) >> 8);
                    dst[3] = 0;
                }
                continue;
            }
            for (x = 0; x < r->w; x++, dst += 4) {
                const uint8_t *p00 = ra + (size_t)xa[x * 3] * 4u, *p01 = ra + (size_t)xa[x * 3 + 1] * 4u;
                const uint8_t *p10 = rb + (size_t)xa[x * 3] * 4u, *p11 = rb + (size_t)xa[x * 3 + 1] * 4u;
                int wx = xa[x * 3 + 2], c;
                if (!wx) {                                        /* vertical only */
                    dst[0] = (uint8_t)((p00[0] * (256 - wy) + p10[0] * wy + 128) >> 8);
                    dst[1] = (uint8_t)((p00[1] * (256 - wy) + p10[1] * wy + 128) >> 8);
                    dst[2] = (uint8_t)((p00[2] * (256 - wy) + p10[2] * wy + 128) >> 8);
                    dst[3] = 0;
                    continue;
                }
                for (c = 0; c < 3; c++)
                    dst[c] = (uint8_t)((p00[c] * (256 - wx) * (256 - wy) + p01[c] * wx * (256 - wy) +
                                        p10[c] * (256 - wx) * wy + p11[c] * wx * wy + 32768) >> 16);
                dst[3] = 0;
            }
        }
        free(xa);
    }
}

#endif

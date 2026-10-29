#ifndef HOST_UPSCALE_H
#define HOST_UPSCALE_H
/* Display-side scalers for the presenter (the "DISPLAY SCALER" graphics
 * option).  They fit the finished 320x240 RGB888 frame into the window's
 * letterbox rectangle and write BGRA like HostWindow_ScaleBGRA does.  They
 * run after HostFB_GetPixels, so emulated VRAM, the present buffer,
 * screenshots, frame counters and guest RAM never see them.
 *
 *   mode 0  NEAREST         HostWindow_ScaleBGRA itself (retail default;
 *                           byte-identical, no extra code path)
 *   mode 1  SHARP BILINEAR  integer pre-scale then bilinear (pixel-art
 *                           friendly: crisp texels, no shimmer at odd ratios)
 *   mode 2  FSR 1           AMD FidelityFX Super Resolution 1.0, a CPU port
 *                           of the public EASU + RCAS reference (MIT
 *                           licence, github.com/GPUOpen-Effects/FidelityFX-FSR),
 *                           perceptual space, RCAS sharpness 0.2 with the
 *                           optional denoise term on (PS1 output is dithered).
 *
 * The two filtered modes cost real CPU at large windows, so rows are split
 * over a small persistent worker pool (PE_SCALER_THREADS overrides the
 * count; 1 = single thread).  Everything here is deterministic: the same
 * input, size and mode give the same bytes on any thread count.
 * Header-only (static) like host_window_scale.h so the X11 and Win32
 * backends and the unit test include it. C math library linkage is required.
 * AMD's notice for the adapted reference kernels is preserved in
 * ../third_party/FidelityFX-FSR-LICENSE.txt. */
#include "host_window_scale.h"
#include <math.h>
#include <stdio.h>
#if !defined(_WIN32) && !defined(PE_UPSCALE_NO_THREADS)
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#define PE_UPSCALE_THREADS 1
#endif

/* Compile-time failure injection for the isolated scaler tests. */
enum { HU_FAIL_ASYNC_MUTEX = 1, HU_FAIL_ASYNC_COND, HU_FAIL_ASYNC_START,
       HU_FAIL_ASYNC_PENDING, HU_FAIL_ASYNC_SOURCE, HU_FAIL_ASYNC_WORK,
       HU_FAIL_ASYNC_FILTER, HU_FAIL_SHARP_TABLE, HU_FAIL_FSR_SOURCE,
       HU_FAIL_FSR_DIRECTION, HU_FAIL_FSR_COLUMN, HU_FAIL_FSR_COLUMN_FRAC,
       HU_FAIL_FSR_ROW, HU_FAIL_FSR_ROW_FRAC };
#ifndef HOST_UPSCALE_FAIL
#define HOST_UPSCALE_FAIL(point) 0
#endif

enum { HOST_UPSCALE_NEAREST = 0, HOST_UPSCALE_SHARP_BILINEAR = 1, HOST_UPSCALE_FSR1 = 2 };

/* ── shared helpers ─────────────────────────────────────────────────── */
static float hu_clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
/* The shaders use an approximate reciprocal that never traps; keep 0*inf
 * from turning into NaN by treating a zero denominator as a huge value. */
static float hu_rcp(float x) { return fabsf(x) < 1e-30f ? copysignf(1e30f, x) : 1.0f / x; }
static uint8_t hu_to_u8(float v)
{
    float s = hu_clampf(v, 0.0f, 1.0f) * 255.0f + 0.5f;
    return (uint8_t)s;
}

typedef struct {
    const uint8_t *rgb; int fb_w, fb_h;   /* source frame */
    uint8_t *out; int width, height;      /* BGRA destination, full window */
    int x0, y0, w, h;                     /* letterbox rectangle */
    float *tmp;                           /* FSR: EASU result, w*h*4 floats (rgb + luma) */
    int mode;
} HuJob;

/* ── sharp bilinear ─────────────────────────────────────────────────── */
/* Pre-scale by the largest integer factor that fits, then bilinear between
 * those pre-scaled texels (the "sharp-bilinear" shader).  Per axis: texel
 * space t, region_range = 0.5 - 0.5/k, the fractional part is pushed to the
 * texel centre except within 1/k of an edge, where it ramps. */
static void hu_sharp_axis(int dst, int src, int k, int *i0, int *i1, float *wt)
{
    int x;
    for (x = 0; x < dst; x++) {
        float t = ((float)x + 0.5f) * (float)src / (float)dst;
        float tf = floorf(t), s = t - tf, rr = 0.5f - 0.5f / (float)k, cd = s - 0.5f;
        float f = (cd - hu_clampf(cd, -rr, rr)) * (float)k + 0.5f;
        float p = tf + f - 0.5f, pf = floorf(p);
        int a = (int)pf, b = a + 1;
        wt[x] = p - pf;
        if (a < 0) a = 0; else if (a >= src) a = src - 1;
        if (b < 0) b = 0; else if (b >= src) b = src - 1;
        i0[x] = a; i1[x] = b;
    }
}

typedef struct { int *xi0, *xi1, *yi0, *yi1; float *xw, *yw; } HuSharpTables;

static void hu_sharp_rows(const HuJob *j, const HuSharpTables *t, int r0, int r1)
{
    int dy, dx;
    for (dy = r0; dy < r1; dy++) {
        const uint8_t *ra = j->rgb + (size_t)t->yi0[dy] * (size_t)j->fb_w * 3u;
        const uint8_t *rb = j->rgb + (size_t)t->yi1[dy] * (size_t)j->fb_w * 3u;
        float wy = t->yw[dy];
        uint8_t *d = j->out + ((size_t)(j->y0 + dy) * (size_t)j->width + (size_t)j->x0) * 4u;
        for (dx = 0; dx < j->w; dx++, d += 4) {
            const uint8_t *aa = ra + t->xi0[dx] * 3, *ab = ra + t->xi1[dx] * 3;
            const uint8_t *ba = rb + t->xi0[dx] * 3, *bb = rb + t->xi1[dx] * 3;
            float wx = t->xw[dx], c;
            int ch;
            for (ch = 0; ch < 3; ch++) {
                float top = aa[ch] + (ab[ch] - aa[ch]) * wx;
                float bot = ba[ch] + (bb[ch] - ba[ch]) * wx;
                c = top + (bot - top) * wy;
                d[2 - ch] = (uint8_t)(hu_clampf(c, 0.0f, 255.0f) + 0.5f);
            }
            d[3] = 0;
        }
    }
}

/* ── FSR 1.0: EASU ──────────────────────────────────────────────────── */
/* PS1 frames reach a 1080p+ window at 4-6x; applying the spatial filter
 * directly at that ratio softens their hard 2D edges (text boxes, HUD).
 * Above 2x the source is first pre-scaled by the smallest integer factor k
 * that brings EASU back to <=2x (nearest: texel-exact, no new colours).
 * The pre-scaled frame is decoded once into an RGB+luma float plane, and the
 * per-texel direction/length terms that the reference recomputes 4x per
 * output pixel are computed once per texel.  Output pixels then gather from
 * those planes through per-column/per-row index tables.  At k=1 the result
 * retains the per-pixel reference kernel, including its clamped borders. */
#define HU_MIN(a, b) ((a) < (b) ? (a) : (b))
#define HU_MAX(a, b) ((a) > (b) ? (a) : (b))

static int hu_fsr_prescale(int fb_w, int fb_h, int w, int h)
{
    int k = 1;
    while (w > 2 * fb_w * k || h > 2 * fb_h * k) k++;
    return k;
}

static float hu_luma(const float c[3]) { return c[2] * 0.5f + (c[0] * 0.5f + c[1]); }

typedef struct {
    int k, sw, sh;              /* pre-scale factor, pre-scaled source size */
    float *src;                 /* sw*sh*4: r, g, b, luma */
    float *dir;                 /* sw*sh*4: dirX, dirY, lenX, lenY */
    int *col;                   /* w*4 clamped source columns ix-1..ix+2 */
    float *colf;                /* w   fractional x */
    int *row;                   /* h*4 clamped source rows iy-1..iy+2 */
    float *rowf;                /* h   fractional y */
} HuFsr;

static void hu_fsr_axis(int dst, int src, int *idx, float *frac)
{
    const float s = (float)src / (float)dst;
    int d, t;
    for (d = 0; d < dst; d++) {
        float p = ((float)d + 0.5f) * s - 0.5f, fp = floorf(p);
        int i = (int)fp;
        frac[d] = p - fp;
        for (t = 0; t < 4; t++) {
            int v = i - 1 + t;
            idx[d * 4 + t] = v < 0 ? 0 : v >= src ? src - 1 : v;
        }
    }
}

static void hu_fsr_src_rows(const HuJob *j, const HuFsr *f, int r0, int r1)
{
    int y, x;
    for (y = r0; y < r1; y++) {
        const uint8_t *s = j->rgb + (size_t)(y / f->k) * (size_t)j->fb_w * 3u;
        float *d = f->src + (size_t)y * (size_t)f->sw * 4u;
        for (x = 0; x < f->sw; x++, d += 4) {
            const uint8_t *p = s + (size_t)(x / f->k) * 3u;
            d[0] = p[0] * (1.0f / 255.0f); d[1] = p[1] * (1.0f / 255.0f); d[2] = p[2] * (1.0f / 255.0f);
            d[3] = hu_luma(d);
        }
    }
}

/* One reference "set": centre C with up A, left B, right D, down E. */
static void hu_fsr_direction(float *d, float lA, float lB, float lC, float lD, float lE)
{
    float dc = lD - lC, cb = lC - lB, ec = lE - lC, ca = lC - lA, len;
    d[0] = lD - lB;
    len = hu_rcp(HU_MAX(fabsf(dc), fabsf(cb)));
    len = hu_clampf(fabsf(d[0]) * len, 0.0f, 1.0f); d[2] = len * len;
    d[1] = lE - lA;
    len = hu_rcp(HU_MAX(fabsf(ec), fabsf(ca)));
    len = hu_clampf(fabsf(d[1]) * len, 0.0f, 1.0f); d[3] = len * len;
}


static void hu_fsr_dir_rows(const HuJob *j, const HuFsr *f, int r0, int r1)
{
    int y, x;
    (void)j;
    for (y = r0; y < r1; y++) {
        const float *up = f->src + (size_t)(y > 0 ? y - 1 : 0) * (size_t)f->sw * 4u;
        const float *mid = f->src + (size_t)y * (size_t)f->sw * 4u;
        const float *dn = f->src + (size_t)(y + 1 < f->sh ? y + 1 : y) * (size_t)f->sw * 4u;
        float *d = f->dir + (size_t)y * (size_t)f->sw * 4u;
        for (x = 0; x < f->sw; x++, d += 4) {
            int xl = x > 0 ? x - 1 : 0, xr = x + 1 < f->sw ? x + 1 : x;
            float lA = up[x * 4 + 3], lB = mid[xl * 4 + 3], lC = mid[x * 4 + 3];
            float lD = mid[xr * 4 + 3], lE = dn[x * 4 + 3];
            hu_fsr_direction(d, lA, lB, lC, lD, lE);
        }
    }
}

static void hu_easu_tap(float aC[3], float *aW, float offx, float offy, float dirx, float diry,
                        float len2x, float len2y, float lob, float clp, const float *c)
{
    float vx = offx * dirx + offy * diry, vy = offx * -diry + offy * dirx, d2, wB, wA, w;
    vx *= len2x; vy *= len2y;
    d2 = vx * vx + vy * vy; d2 = HU_MIN(d2, clp);
    wB = 0.4f * d2 - 1.0f; wA = lob * d2 - 1.0f;
    wB *= wB; wA *= wA;
    wB = 1.5625f * wB - 0.5625f;
    w = wB * wA;
    aC[0] += c[0] * w; aC[1] += c[1] * w; aC[2] += c[2] * w; *aW += w;
}

static void hu_easu_rows(const HuJob *j, const HuFsr *f, int r0, int r1)
{
    int dy, dx, ch;
    for (dy = r0; dy < r1; dy++) {
        const int *ry = f->row + dy * 4;
        const float *R0 = f->src + (size_t)ry[0] * (size_t)f->sw * 4u, *R1 = f->src + (size_t)ry[1] * (size_t)f->sw * 4u;
        const float *R2 = f->src + (size_t)ry[2] * (size_t)f->sw * 4u, *R3 = f->src + (size_t)ry[3] * (size_t)f->sw * 4u;
        const float *D1 = f->dir + (size_t)ry[1] * (size_t)f->sw * 4u, *D2 = f->dir + (size_t)ry[2] * (size_t)f->sw * 4u;
        const float ppy = f->rowf[dy];
        float *out = j->tmp + (size_t)dy * (size_t)j->w * 4u;
        for (dx = 0; dx < j->w; dx++, out += 4) {
            const int *cx = f->col + dx * 4;
            const int c0 = cx[0] * 4, c1 = cx[1] * 4, c2 = cx[2] * 4, c3 = cx[3] * 4;
            const float ppx = f->colf[dx];
            /*  b c
             *e f g h
             *i j k l
             *  n o   */
            const float *b = R0 + c1, *c = R0 + c2, *e = R1 + c0, *ff = R1 + c1, *g = R1 + c2, *h = R1 + c3;
            const float *i = R2 + c0, *jj = R2 + c1, *k = R2 + c2, *l = R2 + c3, *n = R3 + c1, *o = R3 + c2;
            const float *dS = D1 + c1, *dT = D1 + c2, *dU = D2 + c1, *dV = D2 + c2;
            float edge[4][4];
            /* A clamped centre's cached neighbours differ from independently
             * clamped reference taps outside the source. Recompute only the
             * border sets so edge pixels retain the reference behaviour. */
            if (cx[0] == cx[1] || cx[2] == cx[3] || ry[0] == ry[1] || ry[2] == ry[3]) {
                hu_fsr_direction(edge[0], b[3], e[3], ff[3], g[3], jj[3]);
                hu_fsr_direction(edge[1], c[3], ff[3], g[3], h[3], k[3]);
                hu_fsr_direction(edge[2], ff[3], i[3], jj[3], k[3], n[3]);
                hu_fsr_direction(edge[3], g[3], jj[3], k[3], l[3], o[3]);
                dS = edge[0]; dT = edge[1]; dU = edge[2]; dV = edge[3];
            }
            const float wS = (1.0f - ppx) * (1.0f - ppy), wT = ppx * (1.0f - ppy);
            const float wU = (1.0f - ppx) * ppy, wV = ppx * ppy;
            float dirx = 0.0f, diry = 0.0f, len = 0.0f, dirR, stretch, len2x, len2y, lob, clp;
            float mn[3], mx[3], aC[3] = {0, 0, 0}, aW = 0.0f, r;
            int zro;
            dirx += dS[0] * wS; len += dS[2] * wS; diry += dS[1] * wS; len += dS[3] * wS;
            dirx += dT[0] * wT; len += dT[2] * wT; diry += dT[1] * wT; len += dT[3] * wT;
            dirx += dU[0] * wU; len += dU[2] * wU; diry += dU[1] * wU; len += dU[3] * wU;
            dirx += dV[0] * wV; len += dV[2] * wV; diry += dV[1] * wV; len += dV[3] * wV;
            dirR = dirx * dirx + diry * diry;
            zro = dirR < (1.0f / 32768.0f);
            dirR = zro ? 1.0f : 1.0f / sqrtf(dirR);
            if (zro) dirx = 1.0f;
            dirx *= dirR; diry *= dirR;
            len = len * 0.5f; len *= len;
            stretch = (dirx * dirx + diry * diry) * hu_rcp(HU_MAX(fabsf(dirx), fabsf(diry)));
            len2x = 1.0f + (stretch - 1.0f) * len; len2y = 1.0f - 0.5f * len;
            lob = 0.5f + ((1.0f / 4.0f - 0.04f) - 0.5f) * len;
            clp = hu_rcp(lob);
            for (ch = 0; ch < 3; ch++) {
                mn[ch] = HU_MIN(HU_MIN(ff[ch], g[ch]), HU_MIN(jj[ch], k[ch]));
                mx[ch] = HU_MAX(HU_MAX(ff[ch], g[ch]), HU_MAX(jj[ch], k[ch]));
            }
            hu_easu_tap(aC, &aW,  0.0f - ppx, -1.0f - ppy, dirx, diry, len2x, len2y, lob, clp, b);
            hu_easu_tap(aC, &aW,  1.0f - ppx, -1.0f - ppy, dirx, diry, len2x, len2y, lob, clp, c);
            hu_easu_tap(aC, &aW, -1.0f - ppx,  1.0f - ppy, dirx, diry, len2x, len2y, lob, clp, i);
            hu_easu_tap(aC, &aW,  0.0f - ppx,  1.0f - ppy, dirx, diry, len2x, len2y, lob, clp, jj);
            hu_easu_tap(aC, &aW,  0.0f - ppx,  0.0f - ppy, dirx, diry, len2x, len2y, lob, clp, ff);
            hu_easu_tap(aC, &aW, -1.0f - ppx,  0.0f - ppy, dirx, diry, len2x, len2y, lob, clp, e);
            hu_easu_tap(aC, &aW,  1.0f - ppx,  1.0f - ppy, dirx, diry, len2x, len2y, lob, clp, k);
            hu_easu_tap(aC, &aW,  2.0f - ppx,  1.0f - ppy, dirx, diry, len2x, len2y, lob, clp, l);
            hu_easu_tap(aC, &aW,  2.0f - ppx,  0.0f - ppy, dirx, diry, len2x, len2y, lob, clp, h);
            hu_easu_tap(aC, &aW,  1.0f - ppx,  0.0f - ppy, dirx, diry, len2x, len2y, lob, clp, g);
            hu_easu_tap(aC, &aW,  1.0f - ppx,  2.0f - ppy, dirx, diry, len2x, len2y, lob, clp, o);
            hu_easu_tap(aC, &aW,  0.0f - ppx,  2.0f - ppy, dirx, diry, len2x, len2y, lob, clp, n);
            r = hu_rcp(aW);
            for (ch = 0; ch < 3; ch++) out[ch] = HU_MIN(mx[ch], HU_MAX(mn[ch], aC[ch] * r));
            out[3] = hu_luma(out);
        }
    }
}

/* ── FSR 1.0: RCAS ──────────────────────────────────────────────────── */
#define HU_RCAS_SHARPNESS 0.2f          /* stops; 0 = sharpest */
#define HU_RCAS_LIMIT (0.25f - 1.0f / 16.0f)

static void hu_rcas_rows(const HuJob *j, int r0, int r1)
{
    const float con = exp2f(-HU_RCAS_SHARPNESS);
    const size_t stride = (size_t)j->w * 4u;
    int dy, dx;
    for (dy = r0; dy < r1; dy++) {
        const float *mid = j->tmp + (size_t)dy * stride;
        const float *up = dy > 0 ? mid - stride : mid, *dn = dy + 1 < j->h ? mid + stride : mid;
        uint8_t *dst = j->out + ((size_t)(j->y0 + dy) * (size_t)j->width + (size_t)j->x0) * 4u;
        for (dx = 0; dx < j->w; dx++, dst += 4) {
            const size_t xo = (size_t)dx * 4u;
            const float *b = up + xo, *e = mid + xo, *h = dn + xo;
            const float *d = dx > 0 ? e - 4 : e, *f = dx + 1 < j->w ? e + 4 : e;
            float bL = b[3], dL = d[3], eL = e[3], fL = f[3], hL = h[3];
            float nz = 0.25f * bL + 0.25f * dL + 0.25f * fL + 0.25f * hL - eL;
            float lobe = 1e30f, pix[3], rcpL;
            int ch;
            nz = hu_clampf(fabsf(nz) * hu_rcp(HU_MAX(HU_MAX(HU_MAX(bL, dL), eL), HU_MAX(fL, hL))
                                             - HU_MIN(HU_MIN(HU_MIN(bL, dL), eL), HU_MIN(fL, hL))), 0.0f, 1.0f);
            nz = -0.5f * nz + 1.0f;
            for (ch = 0; ch < 3; ch++) {
                float mn4 = HU_MIN(HU_MIN(b[ch], d[ch]), HU_MIN(f[ch], h[ch]));
                float mx4 = HU_MAX(HU_MAX(b[ch], d[ch]), HU_MAX(f[ch], h[ch]));
                float hitMin = mn4 * hu_rcp(4.0f * mx4);
                float hitMax = (1.0f - mx4) * hu_rcp(4.0f * mn4 - 4.0f);
                float lc = HU_MAX(-hitMin, hitMax);
                lobe = ch == 0 ? lc : HU_MAX(lobe, lc);
            }
            lobe = HU_MAX(-HU_RCAS_LIMIT, HU_MIN(lobe, 0.0f)) * con * nz;
            rcpL = hu_rcp(4.0f * lobe + 1.0f);
            for (ch = 0; ch < 3; ch++) pix[ch] = ((b[ch] + d[ch] + f[ch] + h[ch]) * lobe + e[ch]) * rcpL;
            dst[0] = hu_to_u8(pix[2]); dst[1] = hu_to_u8(pix[1]); dst[2] = hu_to_u8(pix[0]); dst[3] = 0;
        }
    }
}
/* ── row-parallel driver ────────────────────────────────────────────── */
typedef void (*HuRowFn)(const HuJob *j, const void *ctx, int r0, int r1);

#ifdef PE_UPSCALE_THREADS
typedef struct {
    pthread_t *threads; int nthreads;        /* workers, excluding the caller */
    pthread_mutex_t mu; pthread_cond_t go, done;
    unsigned gen; int finished, quit;
    HuRowFn fn; const HuJob *job; const void *ctx; int rows; int next; int chunk;
} HuPool;
static HuPool hu_pool;

static void hu_pool_work(HuPool *p)
{
    for (;;) {
        int r0, r1;
        pthread_mutex_lock(&p->mu);
        r0 = p->next; p->next += p->chunk;
        pthread_mutex_unlock(&p->mu);
        if (r0 >= p->rows) return;
        r1 = r0 + p->chunk; if (r1 > p->rows) r1 = p->rows;
        p->fn(p->job, p->ctx, r0, r1);
    }
}

static void *hu_pool_thread(void *arg)
{
    HuPool *p = (HuPool *)arg;
    unsigned seen = 0;
    for (;;) {
        pthread_mutex_lock(&p->mu);
        while (p->gen == seen && !p->quit) pthread_cond_wait(&p->go, &p->mu);
        if (p->quit) { pthread_mutex_unlock(&p->mu); return NULL; }
        seen = p->gen;
        pthread_mutex_unlock(&p->mu);
        hu_pool_work(p);
        pthread_mutex_lock(&p->mu);
        if (++p->finished == p->nthreads) pthread_cond_signal(&p->done);
        pthread_mutex_unlock(&p->mu);
    }
}

static int hu_pool_size(void)
{
    static int n = -1;
    if (n < 0) {
        const char *e = getenv("PE_SCALER_THREADS");
        long cpus = sysconf(_SC_NPROCESSORS_ONLN);
        n = e && *e ? atoi(e) : (int)(cpus > 0 ? cpus : 1);
        if (n < 1) n = 1;
        if (n > 8) n = 8;
    }
    return n;
}

static void hu_pool_start(void)
{
    HuPool *p = &hu_pool;
    int want = hu_pool_size() - 1, i;
    if (p->threads || want <= 0) return;
    if (pthread_mutex_init(&p->mu, NULL) != 0) return;
    if (pthread_cond_init(&p->go, NULL) != 0) {
        pthread_mutex_destroy(&p->mu); return;
    }
    if (pthread_cond_init(&p->done, NULL) != 0) {
        pthread_cond_destroy(&p->go); pthread_mutex_destroy(&p->mu); return;
    }
    p->threads = (pthread_t *)calloc((size_t)want, sizeof(pthread_t));
    if (!p->threads) {
        pthread_cond_destroy(&p->done); pthread_cond_destroy(&p->go);
        pthread_mutex_destroy(&p->mu); return;
    }
    for (i = 0; i < want; i++)
        if (pthread_create(&p->threads[i], NULL, hu_pool_thread, p) != 0) break;
    p->nthreads = i;
    if (!i) {
        free(p->threads); p->threads = NULL;
        pthread_cond_destroy(&p->done); pthread_cond_destroy(&p->go);
        pthread_mutex_destroy(&p->mu);
    }
}
#endif

static void hu_run_rows(HuRowFn fn, const HuJob *job, const void *ctx, int rows)
{
#ifdef PE_UPSCALE_THREADS
    HuPool *p = &hu_pool;
    hu_pool_start();
    if (p->nthreads > 0) {
        pthread_mutex_lock(&p->mu);
        p->fn = fn; p->job = job; p->ctx = ctx; p->rows = rows; p->next = 0;
        p->chunk = 8; p->finished = 0; p->gen++;
        pthread_cond_broadcast(&p->go);
        pthread_mutex_unlock(&p->mu);
        hu_pool_work(p);                       /* the caller helps */
        pthread_mutex_lock(&p->mu);
        while (p->finished < p->nthreads) pthread_cond_wait(&p->done, &p->mu);
        pthread_mutex_unlock(&p->mu);
        return;
    }
#endif
    fn(job, ctx, 0, rows);
}

static void hu_row_sharp(const HuJob *j, const void *ctx, int r0, int r1) { hu_sharp_rows(j, (const HuSharpTables *)ctx, r0, r1); }
static void hu_row_fsr_src(const HuJob *j, const void *ctx, int r0, int r1) { hu_fsr_src_rows(j, (const HuFsr *)ctx, r0, r1); }
static void hu_row_fsr_dir(const HuJob *j, const void *ctx, int r0, int r1) { hu_fsr_dir_rows(j, (const HuFsr *)ctx, r0, r1); }
static void hu_row_easu(const HuJob *j, const void *ctx, int r0, int r1) { hu_easu_rows(j, (const HuFsr *)ctx, r0, r1); }
static void hu_row_rcas(const HuJob *j, const void *ctx, int r0, int r1) { (void)ctx; hu_rcas_rows(j, r0, r1); }

/* Black bars around the letterbox rectangle. */
static void hu_bars(uint8_t *out, int width, int height, int x0, int y0, int w, int h)
{
    size_t row_bytes = (size_t)width * 4u;
    int y;
    if (y0 > 0) memset(out, 0, row_bytes * (size_t)y0);
    if (y0 + h < height) memset(out + row_bytes * (size_t)(y0 + h), 0, row_bytes * (size_t)(height - y0 - h));
    for (y = y0; y < y0 + h; y++) {
        uint8_t *line = out + row_bytes * (size_t)y;
        if (x0 > 0) memset(line, 0, (size_t)x0 * 4u);
        if (x0 + w < width) memset(line + (size_t)(x0 + w) * 4u, 0, (size_t)(width - x0 - w) * 4u);
    }
}

/* Fit rgb (fb_w x fb_h) into out (width x height BGRA) with the given mode.
 * Mode 0 is exactly HostWindow_ScaleBGRA.  Returns the mode actually used
 * (a failed allocation falls back to nearest). */
static int hu_run(int mode, uint8_t *out, int width, int height,
                  const uint8_t *rgb, int fb_w, int fb_h, int strict_async)
{
    HuJob j;
    if (mode == HOST_UPSCALE_NEAREST || width <= 0 || height <= 0 || fb_w <= 0 || fb_h <= 0) {
        if (!strict_async) HostWindow_ScaleBGRA(out, width, height, rgb, fb_w, fb_h);
        return HOST_UPSCALE_NEAREST;
    }
#ifndef PE_UPSCALE_THREADS
    if (mode == HOST_UPSCALE_FSR1) {
        /* Without a presenter thread (the Win32 build today) FSR 1 would run
         * on the game thread and slow the game at large windows; use the
         * cheaper filter and say so once. */
        static int said;
        if (!said) { said = 1; fprintf(stderr, "[WINDOW] FSR 1 needs the presenter thread; using SHARP BILINEAR on this build\n"); }
        mode = HOST_UPSCALE_SHARP_BILINEAR;
    }
#endif
    memset(&j, 0, sizeof j);
    j.rgb = rgb; j.fb_w = fb_w; j.fb_h = fb_h; j.out = out; j.width = width; j.height = height; j.mode = mode;
    HostWindow_FitRect(width, height, fb_w, fb_h, &j.x0, &j.y0, &j.w, &j.h);
    if (j.w <= 0 || j.h <= 0) { if (!strict_async) HostWindow_ScaleBGRA(out, width, height, rgb, fb_w, fb_h); return HOST_UPSCALE_NEAREST; }
    hu_bars(out, width, height, j.x0, j.y0, j.w, j.h);
    if (mode == HOST_UPSCALE_SHARP_BILINEAR) {
        static HuSharpTables t; static int cap_w, cap_h;
        int k = j.w / fb_w < j.h / fb_h ? j.w / fb_w : j.h / fb_h;
        if (k < 1) k = 1;
        if (j.w > cap_w) {
            int *a = HOST_UPSCALE_FAIL(HU_FAIL_SHARP_TABLE) ? NULL : (int *)realloc(t.xi0, (size_t)j.w * sizeof *a);
            int *b = (int *)realloc(t.xi1, (size_t)j.w * sizeof *b);
            float *c = (float *)realloc(t.xw, (size_t)j.w * sizeof *c);
            if (a) t.xi0 = a;
            if (b) t.xi1 = b;
            if (c) t.xw = c;
            if (!a || !b || !c) { if (!strict_async) HostWindow_ScaleBGRA(out, width, height, rgb, fb_w, fb_h); return HOST_UPSCALE_NEAREST; }
            cap_w = j.w;
        }
        if (j.h > cap_h) {
            int *a = (int *)realloc(t.yi0, (size_t)j.h * sizeof *a), *b = (int *)realloc(t.yi1, (size_t)j.h * sizeof *b);
            float *c = (float *)realloc(t.yw, (size_t)j.h * sizeof *c);
            if (a) t.yi0 = a;
            if (b) t.yi1 = b;
            if (c) t.yw = c;
            if (!a || !b || !c) { if (!strict_async) HostWindow_ScaleBGRA(out, width, height, rgb, fb_w, fb_h); return HOST_UPSCALE_NEAREST; }
            cap_h = j.h;
        }
        hu_sharp_axis(j.w, fb_w, k, t.xi0, t.xi1, t.xw);
        hu_sharp_axis(j.h, fb_h, k, t.yi0, t.yi1, t.yw);
        hu_run_rows(hu_row_sharp, &j, &t, j.h);
        return mode;
    }
    {
        static float *tmp, *src, *dir, *colf, *rowf; static int *col, *row;
        static size_t tmp_cap, src_cap, col_cap, row_cap;
        HuFsr f;
        size_t need = (size_t)j.w * (size_t)j.h * 4u, plane;
        f.k = hu_fsr_prescale(fb_w, fb_h, j.w, j.h);
        f.sw = fb_w * f.k; f.sh = fb_h * f.k;
        plane = (size_t)f.sw * (size_t)f.sh * 4u;
        if (need > tmp_cap) {
            float *p = HOST_UPSCALE_FAIL(HU_FAIL_ASYNC_FILTER) ? NULL : (float *)realloc(tmp, need * sizeof *p);
            if (!p) { if (!strict_async) HostWindow_ScaleBGRA(out, width, height, rgb, fb_w, fb_h); return HOST_UPSCALE_NEAREST; }
            tmp = p; tmp_cap = need;
        }
        if (plane > src_cap) {
            float *a = HOST_UPSCALE_FAIL(HU_FAIL_FSR_SOURCE) ? NULL : (float *)realloc(src, plane * sizeof *a);
            float *b = a && !HOST_UPSCALE_FAIL(HU_FAIL_FSR_DIRECTION) ? (float *)realloc(dir, plane * sizeof *b) : NULL;
            if (a) src = a;
            if (b) dir = b;
            if (!a || !b) { if (!strict_async) HostWindow_ScaleBGRA(out, width, height, rgb, fb_w, fb_h); return HOST_UPSCALE_NEAREST; }
            src_cap = plane;
        }
        if ((size_t)j.w > col_cap) {
            int *a = HOST_UPSCALE_FAIL(HU_FAIL_FSR_COLUMN) ? NULL : (int *)realloc(col, (size_t)j.w * 4u * sizeof *a);
            float *b = a && !HOST_UPSCALE_FAIL(HU_FAIL_FSR_COLUMN_FRAC) ? (float *)realloc(colf, (size_t)j.w * sizeof *b) : NULL;
            if (a) col = a;
            if (b) colf = b;
            if (!a || !b) { if (!strict_async) HostWindow_ScaleBGRA(out, width, height, rgb, fb_w, fb_h); return HOST_UPSCALE_NEAREST; }
            col_cap = (size_t)j.w;
        }
        if ((size_t)j.h > row_cap) {
            int *a = HOST_UPSCALE_FAIL(HU_FAIL_FSR_ROW) ? NULL : (int *)realloc(row, (size_t)j.h * 4u * sizeof *a);
            float *b = a && !HOST_UPSCALE_FAIL(HU_FAIL_FSR_ROW_FRAC) ? (float *)realloc(rowf, (size_t)j.h * sizeof *b) : NULL;
            if (a) row = a;
            if (b) rowf = b;
            if (!a || !b) { if (!strict_async) HostWindow_ScaleBGRA(out, width, height, rgb, fb_w, fb_h); return HOST_UPSCALE_NEAREST; }
            row_cap = (size_t)j.h;
        }
        f.src = src; f.dir = dir; f.col = col; f.colf = colf; f.row = row; f.rowf = rowf;
        hu_fsr_axis(j.w, f.sw, f.col, f.colf);
        hu_fsr_axis(j.h, f.sh, f.row, f.rowf);
        j.tmp = tmp;
        hu_run_rows(hu_row_fsr_src, &j, &f, f.sh);
        hu_run_rows(hu_row_fsr_dir, &j, &f, f.sh);
        hu_run_rows(hu_row_easu, &j, &f, j.h);
        hu_run_rows(hu_row_rcas, &j, NULL, j.h);
        return HOST_UPSCALE_FSR1;
    }
}

/* The async coordinator rejects degraded output without running nearest:
 * HostWindow_ScaleBGRA's column-map cache belongs to the game thread. */
static int HostUpscale_Run(int mode, uint8_t *out, int width, int height,
                           const uint8_t *rgb, int fb_w, int fb_h)
{
    return hu_run(mode, out, width, height, rgb, fb_w, fb_h, 0);
}

/* ── asynchronous presenter path ────────────────────────────────────── */
/* The filtered modes can cost more than a frame at large windows, and the
 * blit runs on the game thread, so they are scaled on a coordinator thread
 * (which drives the worker pool) with one present of latency: the game
 * thread copies the 320x240 frame in (HostUpscale_SubmitAsync), carries on,
 * and picks up the most recent finished window image on a later call
 * (HostUpscale_TakeReady).  A new frame submitted while one is in flight
 * replaces the pending one, so a slow filter drops frames instead of
 * slowing the game.  NEAREST never comes through here. */
#ifdef PE_UPSCALE_THREADS
typedef struct {
    pthread_t thread; int started, unavailable, quit, inflight, failed;
    pthread_mutex_t mu; pthread_cond_t cv;
    uint8_t *pend; size_t pend_cap; int pend_new, pend_mode, pend_fw, pend_fh, pend_ww, pend_wh;
    uint8_t *src; size_t src_cap;          /* coordinator's private frame copy */
    uint8_t *work; size_t work_cap;        /* coordinator's output in progress */
    uint8_t *ready; size_t ready_cap; int ready_new, ready_mode, ready_ww, ready_wh;
} HuAsync;
static HuAsync hu_async;

static int hu_grow(uint8_t **buf, size_t *cap, size_t need)
{
    uint8_t *p;
    if (need <= *cap) return 1;
    p = (uint8_t *)realloc(*buf, need);
    if (!p) return 0;
    *buf = p; *cap = need;
    return 1;
}

static void *hu_async_thread(void *arg)
{
    HuAsync *a = (HuAsync *)arg;
    for (;;) {
        int mode, fw, fh, ww, wh;
        size_t need;
        pthread_mutex_lock(&a->mu);
        while (!a->pend_new && !a->quit) pthread_cond_wait(&a->cv, &a->mu);
        if (a->quit) { pthread_mutex_unlock(&a->mu); return NULL; }
        mode = a->pend_mode; fw = a->pend_fw; fh = a->pend_fh; ww = a->pend_ww; wh = a->pend_wh;
        need = (size_t)fw * (size_t)fh * 3u;
        if (HOST_UPSCALE_FAIL(HU_FAIL_ASYNC_SOURCE) || !hu_grow(&a->src, &a->src_cap, need)) {
            a->pend_new = 0; a->inflight = 0; a->failed = 1;
            pthread_mutex_unlock(&a->mu); continue;
        }
        memcpy(a->src, a->pend, need);
        a->pend_new = 0; a->inflight = 1;
        pthread_mutex_unlock(&a->mu);
        need = (size_t)ww * (size_t)wh * 4u;
        if (HOST_UPSCALE_FAIL(HU_FAIL_ASYNC_WORK) || !hu_grow(&a->work, &a->work_cap, need) ||
            hu_run(mode, a->work, ww, wh, a->src, fw, fh, 1) != mode) {
            pthread_mutex_lock(&a->mu); a->inflight = 0; a->failed = 1;
            pthread_mutex_unlock(&a->mu); continue;
        }
        pthread_mutex_lock(&a->mu);
        { uint8_t *t = a->ready; size_t tc = a->ready_cap; a->ready = a->work; a->ready_cap = a->work_cap; a->work = t; a->work_cap = tc; }
        a->ready_new = 1; a->ready_mode = mode; a->ready_ww = ww; a->ready_wh = wh; a->inflight = 0;
        pthread_mutex_unlock(&a->mu);
    }
}

/* Called only on the presenter/game thread. Startup failure is permanent
 * until Shutdown; clean up each successfully initialized primitive. */
static int HostUpscale_AsyncAvailable(void)
{
    HuAsync *a = &hu_async;
    if (a->started) return 1;
    if (a->unavailable) return 0;
    a->unavailable = 1;
    if (HOST_UPSCALE_FAIL(HU_FAIL_ASYNC_MUTEX) || pthread_mutex_init(&a->mu, NULL) != 0) return 0;
    if (HOST_UPSCALE_FAIL(HU_FAIL_ASYNC_COND) || pthread_cond_init(&a->cv, NULL) != 0) {
        pthread_mutex_destroy(&a->mu); return 0;
    }
    if (HOST_UPSCALE_FAIL(HU_FAIL_ASYNC_START) || pthread_create(&a->thread, NULL, hu_async_thread, a) != 0) {
        pthread_cond_destroy(&a->cv); pthread_mutex_destroy(&a->mu); return 0;
    }
    a->started = 1; a->unavailable = 0;
    return 1;
}

/* One-shot failure notification. Poll before static-frame deduplication and
 * mark the frame dirty so an accepted but failed job can be resubmitted.
 * Never run synchronous filtered scaling while this coordinator is alive:
 * HostUpscale_Run's scratch tables are shared with it. */
static int HostUpscale_TakeFailure(void)
{
    HuAsync *a = &hu_async;
    int failed;
    if (!a->started) return 0;
    pthread_mutex_lock(&a->mu);
    failed = a->failed; a->failed = 0;
    pthread_mutex_unlock(&a->mu);
    return failed;
}

/* Queue rgb for filtered scaling into a ww x wh window. Zero means the
 * request was not accepted: keep the source dirty and show cheap nearest.
 * If AsyncAvailable is false, map FSR to SHARP before synchronous Run. */
static int HostUpscale_SubmitAsync(int mode, const uint8_t *rgb, int fb_w, int fb_h, int ww, int wh)
{
    HuAsync *a = &hu_async;
    size_t need;
    if (!rgb || fb_w <= 0 || fb_h <= 0 || ww <= 0 || wh <= 0 ||
        (mode != HOST_UPSCALE_SHARP_BILINEAR && mode != HOST_UPSCALE_FSR1)) return 0;
    if (!HostUpscale_AsyncAvailable()) return 0;
    need = (size_t)fb_w * (size_t)fb_h * 3u;
    pthread_mutex_lock(&a->mu);
    if (HOST_UPSCALE_FAIL(HU_FAIL_ASYNC_PENDING) || !hu_grow(&a->pend, &a->pend_cap, need)) {
        a->failed = 1; pthread_mutex_unlock(&a->mu); return 0;
    }
    memcpy(a->pend, rgb, need);
    a->pend_new = 1; a->pend_mode = mode; a->pend_fw = fb_w; a->pend_fh = fb_h; a->pend_ww = ww; a->pend_wh = wh;
    pthread_cond_signal(&a->cv);
    pthread_mutex_unlock(&a->mu);
    return 1;
}

/* Copy the newest finished window image (ww x wh BGRA) into dst.  Returns 1
 * when dst was written; a finished image of another size/mode is discarded.
 * Negative mode is reserved for the compatibility wrapper. */
static int HostUpscale_TakeReadyFor(uint8_t *dst, int mode, int ww, int wh)
{
    HuAsync *a = &hu_async;
    int got = 0;
    if (!a->started) return 0;
    pthread_mutex_lock(&a->mu);
    if (a->ready_new) {
        if ((mode < 0 || a->ready_mode == mode) && a->ready_ww == ww && a->ready_wh == wh) { memcpy(dst, a->ready, (size_t)ww * (size_t)wh * 4u); got = 1; }
        a->ready_new = 0;
    }
    pthread_mutex_unlock(&a->mu);
    return got;
}

/* The scaler changed: a finished image belongs to an older frame. */
static void HostUpscale_DropReady(void)
{
    HuAsync *a = &hu_async;
    if (!a->started) return;
    pthread_mutex_lock(&a->mu);
    a->ready_new = 0;
    pthread_mutex_unlock(&a->mu);
}

/* Compatibility for callers that do not select a filter. */
static int HostUpscale_TakeReady(uint8_t *dst, int ww, int wh)
{
    return HostUpscale_TakeReadyFor(dst, -1, ww, wh);
}

/* Block until nothing is pending or in flight (tests, shutdown). */
static void HostUpscale_Drain(void)
{
    HuAsync *a = &hu_async;
    if (!a->started) return;
    for (;;) {
        int busy;
        pthread_mutex_lock(&a->mu);
        busy = a->pend_new || a->inflight;
        pthread_mutex_unlock(&a->mu);
        if (!busy) break;
        sched_yield();
    }
}
/* Stop before any synchronous filtered scaling or session reset. */
static void HostUpscale_Shutdown(void)
{
    HuAsync *a = &hu_async;
    HuPool *p = &hu_pool;
    int i;
    if (a->started) {
        pthread_mutex_lock(&a->mu); a->quit = 1;
        pthread_cond_signal(&a->cv); pthread_mutex_unlock(&a->mu);
        pthread_join(a->thread, NULL);
        pthread_cond_destroy(&a->cv); pthread_mutex_destroy(&a->mu);
    }
    free(a->pend); free(a->src); free(a->work); free(a->ready);
    memset(a, 0, sizeof *a);
    if (p->threads) {
        pthread_mutex_lock(&p->mu); p->quit = 1;
        pthread_cond_broadcast(&p->go); pthread_mutex_unlock(&p->mu);
        for (i = 0; i < p->nthreads; i++) pthread_join(p->threads[i], NULL);
        free(p->threads);
        pthread_cond_destroy(&p->done); pthread_cond_destroy(&p->go);
        pthread_mutex_destroy(&p->mu); memset(p, 0, sizeof *p);
    }
}
#else
static int HostUpscale_AsyncAvailable(void) { return 0; }
static int HostUpscale_TakeFailure(void) { return 0; }
static void HostUpscale_DropReady(void) { }
static void HostUpscale_Shutdown(void) { }
#endif
#endif

/* Native GTE command arithmetic adapted from this project's
 * pe_gte_cop2.c at 14e32898717e61b00e636a4601595382f965dc9a.
 * Source SHA256 e74c0e296bdfba07d69ebe3c598929943b1059f4145043592196b04640cc3a6e.
 * Hardware authority: psx-spx Geometry Transformation Engine and the retained
 * MIT ps1-tests real-console log. No CPU instruction execution is supplied.
 * Tested by pe-gte-native-command-tests: fixed cases, plus every case of
 * ps1-tests gte-fuzz/gte_valid_0xc0ffee_50.log when PE_GTE_HW_LOG names it.
 */
#include "pe_sdk.h"
#include "pe_gte_commands.h"
#include <string.h>

/* ── FLAG accumulation (psx-spx "cop2r63 - FLAG") ─────────────────────── */
static uint32_t g_flag;

static int64_t wrap44(int64_t v)
{
    uint64_t bits = (uint64_t)v & UINT64_C(0xFFFFFFFFFFF);
    return (bits & UINT64_C(0x80000000000)) ? (int64_t)bits - INT64_C(0x100000000000) : (int64_t)bits;
}
static int64_t mac44(int64_t v, unsigned i);
/* one partial product added to a running MAC sum */
static int64_t mac_add(int64_t acc, int64_t term, unsigned i)
{
    acc += term;
    return wrap44(mac44(acc, i));
}

#define FLAG_ERROR_MASK 0x7F87E000u   /* bits 30..23 and 18..13 */

/* FLAG 30-i (positive) / 27-i (negative) when a MAC1-3 sum leaves the
 * signed 44-bit range; the value is returned unchanged (mac_add wraps). */
static int64_t mac44(int64_t v, unsigned i)
{
    if (v > INT64_C(0x7FFFFFFFFFF)) g_flag |= 1u << (30u - i);
    else if (v < -INT64_C(0x80000000000)) g_flag |= 1u << (27u - i);
    return v;
}

/* IR1-3: lm=0 -> -8000h..7FFFh, lm=1 -> 0..7FFFh; FLAG 24-i. */
static int32_t sat_ir(int64_t v, unsigned i, int lm)
{
    int64_t lo = lm ? 0 : -0x8000;
    if (v < lo) { g_flag |= 1u << (24u - i); return (int32_t)lo; }
    if (v > 0x7FFF) { g_flag |= 1u << (24u - i); return 0x7FFF; }
    return (int32_t)v;
}

static int32_t clamp_ir_noflag(int64_t v, int lm)
{
    int64_t lo = lm ? 0 : -0x8000;
    return v < lo ? (int32_t)lo : v > 0x7FFF ? 0x7FFF : (int32_t)v;
}

/* MAC0: FLAG 16 (> +31 bits) / 15 (< -31 bits); the register keeps 32 bits. */
static int32_t mac0_chk(int64_t v)
{
    if (v > INT32_MAX) g_flag |= 1u << 16;
    else if (v < INT32_MIN) g_flag |= 1u << 15;
    return (int32_t)v;
}

/* Colour FIFO channel: MACi/16 clamped to 0..FFh, FLAG 21-i. */
static uint32_t sat_color(int32_t mac, unsigned i)
{
    int32_t c = mac >> 4;
    if (c < 0) { g_flag |= 1u << (21u - i); return 0; }
    if (c > 0xFF) { g_flag |= 1u << (21u - i); return 0xFF; }
    return (uint32_t)c;
}

/* "Colour output": push [MAC1/16, MAC2/16, MAC3/16, CODE] and IR = MAC. */
static void color_out(int lm)
{
    uint32_t c = g_pe_gte.rgbc & 0xFF000000u;
    for (unsigned i = 0; i < 3u; i++) {
        c |= sat_color(g_pe_gte.mac[i], i) << (8u * i);
        g_pe_gte.ir[i] = sat_ir(g_pe_gte.mac[i], i, lm);
    }
    g_pe_gte.rgb_fifo[0] = g_pe_gte.rgb_fifo[1];
    g_pe_gte.rgb_fifo[1] = g_pe_gte.rgb_fifo[2];
    g_pe_gte.rgb_fifo[2] = c;
}

static uint32_t pack16(int32_t lo, int32_t hi)
{
    return (uint32_t)(uint16_t)lo | ((uint32_t)(uint16_t)hi << 16);
}

/* ── sub-pixel sidecar (host presentation only) ───────────────────────── */
/* Graphics > Sub-pixel Geometry.  While capture is on, each RTPS/RTPT
 * result also keeps the fraction of its 16.16 screen position (MAC0, the
 * value SX2/SY2 truncate) under the packed SXY word it writes.  The shadow
 * rasterizer looks polygon vertex words up here.  No GTE register, FLAG or
 * guest-visible value changes.  A saturated coordinate keeps fraction 0,
 * its integer result.  The same key is replaced in place (the last
 * projection wins); a new key takes the least recently written slot of its
 * probe run. */
#define SUBPIXEL_SLOTS 8192u
#define SUBPIXEL_PROBES 8u
static struct { uint32_t key, frac, stamp; } g_subpixel[SUBPIXEL_SLOTS];
static int g_subpixel_on;
static uint32_t g_subpixel_stamp;

void PE_GTE_SetSubpixelCapture(int on)
{
    on = on != 0;
    if (on != g_subpixel_on) { memset(g_subpixel, 0, sizeof g_subpixel); g_subpixel_stamp = 0; }
    g_subpixel_on = on;
}
int PE_GTE_GetSubpixelCapture(void) { return g_subpixel_on; }
static unsigned subpixel_slot(uint32_t key) { return (key * 2654435761u) >> 19; }
static void subpixel_record(uint32_t key, uint32_t frac)
{
    unsigned base = subpixel_slot(key), victim = base;
    if (!++g_subpixel_stamp) { memset(g_subpixel, 0, sizeof g_subpixel); g_subpixel_stamp = 1; }
    for (unsigned i = 0; i < SUBPIXEL_PROBES; i++) {
        unsigned k = (base + i) & (SUBPIXEL_SLOTS - 1u);
        if (g_subpixel[k].stamp && g_subpixel[k].key == key) { victim = k; break; }
        if (g_subpixel[k].stamp < g_subpixel[victim].stamp) victim = k;
    }
    g_subpixel[victim].key = key; g_subpixel[victim].frac = frac; g_subpixel[victim].stamp = g_subpixel_stamp;
}
int PE_GTE_SubpixelLookup(uint32_t sxy, uint32_t *frac)
{
    unsigned base = subpixel_slot(sxy);
    for (unsigned i = 0; i < SUBPIXEL_PROBES; i++) {
        unsigned k = (base + i) & (SUBPIXEL_SLOTS - 1u);
        if (g_subpixel[k].stamp && g_subpixel[k].key == sxy) { *frac = g_subpixel[k].frac; return 1; }
    }
    return 0;
}

/* ── division (psx-spx "GTE Division Inaccuracy") ─────────────────────── */
static uint32_t gte_divide(uint32_t h, uint32_t sz3)
{
    if (h >= sz3 * 2u) { g_flag |= 1u << 17; return 0x1FFFFu; }
    unsigned z = 0;
    while (z < 16u && !((sz3 << z) & 0x8000u)) z++;
    uint32_t n = h << z, d = sz3 << z;
    uint32_t i = (d - 0x7FC0u) >> 7;
    int32_t t = (int32_t)((0x40000u / (i + 0x100u) + 1u) / 2u) - 0x101;
    uint32_t u = (uint32_t)(t < 0 ? 0 : t) + 0x101u;
    d = (0x2000080u - d * u) >> 8;
    d = (0x80u + d * u) >> 8;
    uint64_t r = ((uint64_t)n * d + 0x8000u) >> 16;
    return r > 0x1FFFFu ? 0x1FFFFu : (uint32_t)r;
}

/* ── commands (psx-spx "GTE Coordinate / Colour Calculation Commands") ── */
static void rtp(const int16_t *v, int sf, int lm, int last)
{
    const unsigned sh = sf ? 12u : 0u;
    int64_t acc[3], s[3];
    for (unsigned i = 0; i < 3u; i++) {
        int64_t a = (int64_t)g_pe_gte.tr[i] * 0x1000;
        for (unsigned k = 0; k < 3u; k++) a = mac_add(a, (int64_t)g_pe_gte.rt[i][k] * v[k], i);
        a = mac44(a, i);
        acc[i] = a; s[i] = a >> sh;
        g_pe_gte.mac[i] = (int32_t)s[i];
    }
    g_pe_gte.ir[0] = sat_ir((int32_t)s[0], 0, lm);
    g_pe_gte.ir[1] = sat_ir((int32_t)s[1], 1, lm);
    /* IR3: stored per lm, FLAG 22 checked as lm=0 on MAC3 (sf=1) or MAC3 SAR 12 (sf=0). */
    g_pe_gte.ir[2] = clamp_ir_noflag((int32_t)s[2], lm);
    {
        int64_t chk = sf ? s[2] : (acc[2] >> 12);
        if (chk < -0x8000 || chk > 0x7FFF) g_flag |= 1u << 22;
    }
    int64_t z = acc[2] >> 12;                               /* MAC3 SAR ((1-sf)*12) */
    if (z < 0) { z = 0; g_flag |= 1u << 18; } else if (z > 0xFFFF) { z = 0xFFFF; g_flag |= 1u << 18; }
    g_pe_gte.sz[0] = g_pe_gte.sz[1]; g_pe_gte.sz[1] = g_pe_gte.sz[2]; g_pe_gte.sz[2] = g_pe_gte.sz[3];
    g_pe_gte.sz[3] = (uint32_t)z;
    uint32_t n = gte_divide((uint32_t)g_pe_gte.h & 0xFFFFu, (uint32_t)z);
    int64_t m = (int64_t)n * (int16_t)g_pe_gte.ir[0] + g_pe_gte.ofx;
    g_pe_gte.mac0 = mac0_chk(m);
    int64_t sx = m >> 16;
    uint32_t fx = (uint32_t)(m & 0xFFFF);
    m = (int64_t)n * (int16_t)g_pe_gte.ir[1] + g_pe_gte.ofy;
    g_pe_gte.mac0 = mac0_chk(m);
    int64_t sy = m >> 16;
    uint32_t fy = (uint32_t)(m & 0xFFFF);
    if (sx < -0x400) { sx = -0x400; fx = 0; g_flag |= 1u << 14; } else if (sx > 0x3FF) { sx = 0x3FF; fx = 0; g_flag |= 1u << 14; }
    if (sy < -0x400) { sy = -0x400; fy = 0; g_flag |= 1u << 13; } else if (sy > 0x3FF) { sy = 0x3FF; fy = 0; g_flag |= 1u << 13; }
    g_pe_gte.sxy[0] = g_pe_gte.sxy[1]; g_pe_gte.sxy[1] = g_pe_gte.sxy[2];
    g_pe_gte.sxy[2] = pack16((int32_t)sx, (int32_t)sy);
    if (g_subpixel_on) subpixel_record(g_pe_gte.sxy[2], (fx >> 8) | ((fy >> 8) << 8));
    if (last) {                                              /* depth cueing */
        m = (int64_t)n * (int16_t)g_pe_gte.dqa + g_pe_gte.dqb;
        g_pe_gte.mac0 = mac0_chk(m);
        int64_t i0 = m >> 12;
        if (i0 < 0) { i0 = 0; g_flag |= 1u << 12; } else if (i0 > 0x1000) { i0 = 0x1000; g_flag |= 1u << 12; }
        g_pe_gte.ir0 = (int32_t)i0;
    }
}

/* MAC = (T*1000h + M*V) SAR sh (T may be NULL), IR = sat(MAC, lm). */
static void mat_vec(const int16_t m[3][3], const int16_t v[3], const int32_t *t, int sf, int lm)
{
    const unsigned sh = sf ? 12u : 0u;
    for (unsigned i = 0; i < 3u; i++) {
        int64_t a = t ? (int64_t)t[i] * 0x1000 : 0;
        for (unsigned k = 0; k < 3u; k++) a = mac_add(a, (int64_t)m[i][k] * v[k], i);
        a = mac44(a, i);
        g_pe_gte.mac[i] = (int32_t)(a >> sh);
        g_pe_gte.ir[i] = sat_ir(g_pe_gte.mac[i], i, lm);
    }
}

static void ir_vec(int16_t out[3])
{
    for (unsigned i = 0; i < 3u; i++) out[i] = (int16_t)g_pe_gte.ir[i];
}

static void mvmva(uint32_t w, int sf, int lm)
{
    const unsigned mx = (w >> 17) & 3u, vs = (w >> 15) & 3u, cv = (w >> 13) & 3u;
    const unsigned sh = sf ? 12u : 0u;
    int16_t m[3][3], v[3];
    const int32_t *t = cv == 0 ? g_pe_gte.tr : cv == 1 ? g_pe_gte.bk : cv == 2 ? g_pe_gte.fc : NULL;
    if (mx == 3) {                                           /* "garbage" matrix */
        int16_t r = (int16_t)(g_pe_gte.rgbc & 0xFFu);
        int16_t g3[9] = { (int16_t)(-r * 0x10), (int16_t)(r * 0x10), (int16_t)g_pe_gte.ir0,
                          g_pe_gte.rt[0][2], g_pe_gte.rt[0][2], g_pe_gte.rt[0][2],
                          g_pe_gte.rt[1][1], g_pe_gte.rt[1][1], g_pe_gte.rt[1][1] };
        for (unsigned k = 0; k < 9u; k++) m[k / 3u][k % 3u] = g3[k];
    } else {
        int16_t (*src)[3] = mx == 0 ? g_pe_gte.rt : mx == 1 ? g_pe_gte.llm : g_pe_gte.lcm;
        for (unsigned k = 0; k < 9u; k++) m[k / 3u][k % 3u] = src[k / 3u][k % 3u];
    }
    if (vs == 3) ir_vec(v);
    else { const int16_t *s = vs == 0 ? g_pe_gte.v0 : vs == 1 ? g_pe_gte.v1 : g_pe_gte.v2;
           v[0] = s[0]; v[1] = s[1]; v[2] = s[2]; }
    if (cv != 2) { mat_vec(m, v, t, sf, lm); return; }
    /* cv=2 (FC) bug: the result is (M_i2*V2 + M_i3*V3) SAR sh, while the
     * discarded first part (FC*1000h + M_i1*V1) still raises its MAC and IR
     * (lm=0) flags ("some bits in the FLAG register seem to be adjusted as if
     * the full operation would have been executed").  This split was fitted
     * to the real-hardware log (docs/evidence/gte-hardware/REPORT.md). */
    for (unsigned i = 0; i < 3u; i++) {
        int64_t first = mac_add((int64_t)t[i] * 0x1000, (int64_t)m[i][0] * v[0], i);
        first = mac44(first, i);
        (void)sat_ir((int32_t)(first >> sh), i, 0);                     /* flags only */
        int64_t part = mac_add(0, (int64_t)m[i][1] * v[1], i);
        part = mac44(mac_add(part, (int64_t)m[i][2] * v[2], i), i);
        g_pe_gte.mac[i] = (int32_t)(part >> sh);
        g_pe_gte.ir[i] = sat_ir(g_pe_gte.mac[i], i, lm);
    }
}

/* Signed scaling uses bounded int64 multiplication: shifting a negative operand
 * is undefined in host C. FC is int32 and IR is int16, so these products fit.
 * MAC + (FC-MAC)*IR0 on the pre-shift value x[i]:
 * IR = ((FC SHL 12) - x) SAR sh saturated as lm=0; x = IR*IR0 + x. */
static void depth_cue(int64_t x[3], int sf)
{
    const unsigned sh = sf ? 12u : 0u;
    for (unsigned i = 0; i < 3u; i++) {
        int64_t d = mac44(((int64_t)g_pe_gte.fc[i] * INT64_C(4096)) - x[i], i);
        int32_t ir = sat_ir((int32_t)(d >> sh), i, 0);
        x[i] = mac44((int64_t)ir * (int16_t)g_pe_gte.ir0 + x[i], i);
    }
}

static void finish_color(int64_t x[3], int sf, int lm)
{
    const unsigned sh = sf ? 12u : 0u;
    for (unsigned i = 0; i < 3u; i++) g_pe_gte.mac[i] = (int32_t)(x[i] >> sh);
    color_out(lm);
}

/* NCS/NCCS/NCDS core for one vertex; kind 0 = NC, 1 = NCC, 2 = NCD. */
static void nc_vertex(const int16_t *v, int kind, int sf, int lm)
{
    int16_t irv[3];
    mat_vec(g_pe_gte.llm, v, NULL, sf, lm);
    ir_vec(irv);
    mat_vec(g_pe_gte.lcm, irv, g_pe_gte.bk, sf, lm);
    if (kind == 0) { color_out(lm); return; }
    int64_t x[3];
    for (unsigned i = 0; i < 3u; i++)
        x[i] = mac44(((int64_t)((g_pe_gte.rgbc >> (8u * i)) & 0xFFu) * (int16_t)g_pe_gte.ir[i]) * INT64_C(16), i);
    if (kind == 2) depth_cue(x, sf);
    finish_color(x, sf, lm);
}

static void cc_cdp(int cdp, int sf, int lm)
{
    int16_t irv[3];
    ir_vec(irv);
    mat_vec(g_pe_gte.lcm, irv, g_pe_gte.bk, sf, lm);
    int64_t x[3];
    for (unsigned i = 0; i < 3u; i++)
        x[i] = mac44(((int64_t)((g_pe_gte.rgbc >> (8u * i)) & 0xFFu) * (int16_t)g_pe_gte.ir[i]) * INT64_C(16), i);
    if (cdp) depth_cue(x, sf);
    finish_color(x, sf, lm);
}

static uint32_t native_avsz_flags(int64_t m)
{
    uint32_t f = 0;
    int64_t o = m >> 12;
    if (m > INT32_MAX) f |= 1u << 16; else if (m < INT32_MIN) f |= 1u << 15;   /* MAC0 overflow */
    if (o < 0 || o > 0xFFFF) f |= 1u << 18;                                    /* OTZ saturated */
    return f | ((f & FLAG_ERROR_MASK) ? 0x80000000u : 0u);
}

int PE_GTE_ExecuteNative(uint32_t word)
{
    const int sf = (int)((word >> 19) & 1u), lm = (int)((word >> 10) & 1u);
    const unsigned sh = sf ? 12u : 0u;
    int64_t x[3];
    g_flag = 0;
    switch (word & 0x3Fu) {
    case 0x01: rtp(g_pe_gte.v0, sf, lm, 1); break;                          /* RTPS */
    case 0x30: rtp(g_pe_gte.v0, sf, lm, 0); rtp(g_pe_gte.v1, sf, lm, 0);   /* RTPT */
               rtp(g_pe_gte.v2, sf, lm, 1); break;
    case 0x06: {                                                            /* NCLIP */
        int64_t sx[3], sy[3];
        for (unsigned i = 0; i < 3u; i++) {
            sx[i] = (int16_t)g_pe_gte.sxy[i]; sy[i] = (int16_t)(g_pe_gte.sxy[i] >> 16);
        }
        g_pe_gte.mac0 = mac0_chk(sx[0] * sy[1] + sx[1] * sy[2] + sx[2] * sy[0]
                               - sx[0] * sy[2] - sx[1] * sy[0] - sx[2] * sy[1]);
        break;
    }
    case 0x0C: {                                                            /* OP */
        int64_t i1 = (int16_t)g_pe_gte.ir[0], i2 = (int16_t)g_pe_gte.ir[1], i3 = (int16_t)g_pe_gte.ir[2];
        int64_t d1 = g_pe_gte.rt[0][0], d2 = g_pe_gte.rt[1][1], d3 = g_pe_gte.rt[2][2];
        x[0] = mac44(i3 * d2 - i2 * d3, 0); x[1] = mac44(i1 * d3 - i3 * d1, 1); x[2] = mac44(i2 * d1 - i1 * d2, 2);
        for (unsigned i = 0; i < 3u; i++) {
            g_pe_gte.mac[i] = (int32_t)(x[i] >> sh); g_pe_gte.ir[i] = sat_ir(x[i] >> sh, i, lm);
        }
        break;
    }
    case 0x10:                                                              /* DPCS */
        for (unsigned i = 0; i < 3u; i++) x[i] = (int64_t)((g_pe_gte.rgbc >> (8u * i)) & 0xFFu) << 16;
        depth_cue(x, sf); finish_color(x, sf, lm); break;
    case 0x2A:                                                              /* DPCT */
        for (unsigned r = 0; r < 3u; r++) {
            uint32_t c = g_pe_gte.rgb_fifo[0];
            for (unsigned i = 0; i < 3u; i++) x[i] = (int64_t)((c >> (8u * i)) & 0xFFu) << 16;
            depth_cue(x, sf); finish_color(x, sf, lm);
        }
        break;
    case 0x11:                                                              /* INTPL */
        for (unsigned i = 0; i < 3u; i++) x[i] = (int64_t)(int16_t)g_pe_gte.ir[i] * INT64_C(4096);
        depth_cue(x, sf); finish_color(x, sf, lm); break;
    case 0x29:                                                              /* DCPL */
        for (unsigned i = 0; i < 3u; i++)
            x[i] = mac44(((int64_t)((g_pe_gte.rgbc >> (8u * i)) & 0xFFu) * (int16_t)g_pe_gte.ir[i]) * INT64_C(16), i);
        depth_cue(x, sf); finish_color(x, sf, lm); break;
    case 0x12: mvmva(word, sf, lm); break;                                  /* MVMVA */
    case 0x1E: nc_vertex(g_pe_gte.v0, 0, sf, lm); break;                    /* NCS */
    case 0x20: nc_vertex(g_pe_gte.v0, 0, sf, lm); nc_vertex(g_pe_gte.v1, 0, sf, lm);
               nc_vertex(g_pe_gte.v2, 0, sf, lm); break;                    /* NCT */
    case 0x1B: nc_vertex(g_pe_gte.v0, 1, sf, lm); break;                    /* NCCS */
    case 0x3F: nc_vertex(g_pe_gte.v0, 1, sf, lm); nc_vertex(g_pe_gte.v1, 1, sf, lm);
               nc_vertex(g_pe_gte.v2, 1, sf, lm); break;                    /* NCCT */
    case 0x13: nc_vertex(g_pe_gte.v0, 2, sf, lm); break;                    /* NCDS */
    case 0x16: nc_vertex(g_pe_gte.v0, 2, sf, lm); nc_vertex(g_pe_gte.v1, 2, sf, lm);
               nc_vertex(g_pe_gte.v2, 2, sf, lm); break;                    /* NCDT */
    case 0x1C: cc_cdp(0, sf, lm); break;                                    /* CC */
    case 0x14: cc_cdp(1, sf, lm); break;                                    /* CDP */
    case 0x28:                                                              /* SQR */
        for (unsigned i = 0; i < 3u; i++) {
            int64_t q = mac44((int64_t)(int16_t)g_pe_gte.ir[i] * (int16_t)g_pe_gte.ir[i], i) >> sh;
            g_pe_gte.mac[i] = (int32_t)q; g_pe_gte.ir[i] = sat_ir(q, i, 0);
        }
        break;
    case 0x2D: case 0x2E: {                                                 /* AVSZ3 / AVSZ4 */
        int64_t m = (word & 1u)
            ? (int64_t)(int16_t)g_pe_gte.zsf3 * ((int64_t)g_pe_gte.sz[1] + g_pe_gte.sz[2] + g_pe_gte.sz[3])
            : (int64_t)(int16_t)g_pe_gte.zsf4 * ((int64_t)g_pe_gte.sz[0] + g_pe_gte.sz[1] + g_pe_gte.sz[2] + g_pe_gte.sz[3]);
        g_flag |= native_avsz_flags(m) & ~0x80000000u;   /* the one MAC0/OTZ FLAG rule */
        g_pe_gte.mac0 = (int32_t)m;
        int64_t o = m >> 12;
        if (o < 0) o = 0; else if (o > 0xFFFF) o = 0xFFFF;
        g_pe_gte.otz = (uint32_t)o;
        break;
    }
    case 0x3D: case 0x3E:                                                   /* GPF / GPL */
        for (unsigned i = 0; i < 3u; i++) {
            int64_t base = (word & 0x3Fu) == 0x3E ? (int64_t)g_pe_gte.mac[i] * (1 << sh) : 0;
            x[i] = mac44(base + (int64_t)(int16_t)g_pe_gte.ir[i] * (int16_t)g_pe_gte.ir0, i);
        }
        finish_color(x, sf, lm); break;
    default:
        return -1;                                           /* not a psx-spx command */
    }
    g_pe_gte.projection_flags =
        g_flag | ((g_flag & FLAG_ERROR_MASK) ? 0x80000000u : 0u);
    return 0;
}

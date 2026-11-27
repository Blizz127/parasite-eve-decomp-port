/*
 * Host model of the PlayStation GTE (COP2): register file, loaders/setters and
 * the cop2 command set, over the g_pe_gte state object declared in pe_sdk.h.
 *
 * Written clean-room from the psx-spx GTE documentation and the
 * hardware-captured / black-box vector sets gte60a-c
 * (docs/evidence/native-gte60-20261126); no SDK source, decompiled source,
 * tables or disassembly were read.
 *
 * Register conventions (psx-spx "GTE Registers"):
 *  - data 0..31 are accessed through PE_GTE_ReadData / WriteData and control
 *    0..31 through ReadCtrl / WriteCtrl; the register index acts modulo 32;
 *  - 16-bit registers (VZn, IRn, IR0, SZn, OTZ, ctrl 4/12/20 RT33-style
 *    halves, H, DQA, ZSF3, ZSF4) read back sign-extended except SZn/OTZ,
 *    which read back zero-extended; H is written as an unsigned halfword and
 *    stored zero-extended, but reads back sign-extended (hardware quirk);
 *  - FLAG (ctrl 31) lives in g_pe_gte.projection_flags; bit 31 is the
 *    summary of bits 30..23 and 18..13 and is recomputed on every read.
 *
 * The unsigned-Newton-Raphson reciprocal table of the perspective divide is
 * generated from the psx-spx formula at first use; nothing is tabulated.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_sdk.h"
#include "pe_gte_commands.h"

PeGteState g_pe_gte;

/* data register 23 (RES1) has no field in PeGteState; it just holds a word */
static uint32_t g_pe_gte_res1;

#define G g_pe_gte

/* FLAG bits (psx-spx "GTE Control Registers") */
#define FL_SUMMARY_MASK 0x7F87E000u
#define FL_ERROR        0x80000000u
#define FL_MAC1_POS     0x40000000u /* bit 30; MAC2 29, MAC3 28 follow */
#define FL_MAC1_NEG     0x08000000u /* bit 27; MAC2 26, MAC3 25 follow */
#define FL_IR1_SAT      0x01000000u /* bit 24; IR2 23, IR3 22 follow */
#define FL_COLOR_R      0x00200000u /* bit 21; G 20, B 19 follow */
#define FL_SZ_OTZ       0x00040000u /* bit 18 */
#define FL_DIV          0x00020000u /* bit 17 */
#define FL_MAC0_POS     0x00010000u /* bit 16 */
#define FL_MAC0_NEG     0x00008000u /* bit 15 */
#define FL_SX           0x00004000u /* bit 14 */
#define FL_SY           0x00002000u /* bit 13 */
#define FL_IR0          0x00001000u /* bit 12 */

/* ------------------------------------------------------------------ */
/* arithmetic helpers                                                  */
/* ------------------------------------------------------------------ */

static void flag_commit(void)
{
    if (G.projection_flags & FL_SUMMARY_MASK) G.projection_flags |= FL_ERROR;
    else G.projection_flags &= ~FL_ERROR;
}

/* MAC1..3 accumulator step: 44-bit signed range check (flag per overflow
 * direction), then the value wraps to 44 bits signed. */
static int64_t acc44(unsigned n, int64_t v)
{
    if (v >= ((int64_t)1 << 43)) G.projection_flags |= FL_MAC1_POS >> n;
    else if (v < -((int64_t)1 << 43)) G.projection_flags |= FL_MAC1_NEG >> n;
    return (int64_t)((uint64_t)v << 20) >> 20;
}

/* MAC0 is a 32-bit register; flags for leaving the 32-bit range. The
 * unwrapped 64-bit value is returned for the callers that shift it. */
static int64_t acc32(int64_t v)
{
    if (v > INT32_MAX) G.projection_flags |= FL_MAC0_POS;
    else if (v < INT32_MIN) G.projection_flags |= FL_MAC0_NEG;
    return v;
}

/* IRn saturation: [0,7FFF] with lm, [-8000,7FFF] without; flag n = 0..2. */
static int32_t ir_sat(unsigned n, int32_t v, unsigned lm)
{
    const int32_t lo = lm ? 0 : -0x8000;
    if (v < lo) { G.projection_flags |= FL_IR1_SAT >> n; return lo; }
    if (v > 0x7FFF) { G.projection_flags |= FL_IR1_SAT >> n; return 0x7FFF; }
    return v;
}

/* store a full-precision accumulator into MACn / IRn (shift = sf*12) */
static void set_mac_ir(unsigned n, int64_t full, unsigned shift, unsigned lm)
{
    G.mac[n] = (int32_t)(full >> shift);
    G.ir[n] = ir_sat(n, G.mac[n], lm);
}

static int32_t sx16(uint32_t w) { return (int16_t)(uint16_t)w; }
static int32_t sy16(uint32_t w) { return (int16_t)(uint16_t)(w >> 16); }

static void push_sz(int32_t v)
{
    if (v < 0) { G.projection_flags |= FL_SZ_OTZ; v = 0; }
    else if (v > 0xFFFF) { G.projection_flags |= FL_SZ_OTZ; v = 0xFFFF; }
    G.sz[0] = G.sz[1]; G.sz[1] = G.sz[2]; G.sz[2] = G.sz[3];
    G.sz[3] = (uint32_t)v;
}

static void set_otz(int64_t v)
{
    if (v < 0) { G.projection_flags |= FL_SZ_OTZ; v = 0; }
    else if (v > 0xFFFF) { G.projection_flags |= FL_SZ_OTZ; v = 0xFFFF; }
    G.otz = (uint32_t)v;
}

static void push_sxy(int32_t x, int32_t y)
{
    if (x < -0x400) { G.projection_flags |= FL_SX; x = -0x400; }
    else if (x > 0x3FF) { G.projection_flags |= FL_SX; x = 0x3FF; }
    if (y < -0x400) { G.projection_flags |= FL_SY; y = -0x400; }
    else if (y > 0x3FF) { G.projection_flags |= FL_SY; y = 0x3FF; }
    G.sxy[0] = G.sxy[1]; G.sxy[1] = G.sxy[2];
    G.sxy[2] = (uint32_t)(uint16_t)x | ((uint32_t)(uint16_t)y << 16);
}

/* colour FIFO entry from MAC1..3 / 16, saturated to 0..255 (flags 21..19);
 * the CODE byte comes from RGBC. */
static void push_color(void)
{
    uint32_t c = G.rgbc & 0xFF000000u, sh = 0;
    unsigned n;
    for (n = 0; n < 3; n++, sh += 8) {
        int32_t v = G.mac[n] >> 4;
        if (v < 0) { G.projection_flags |= FL_COLOR_R >> n; v = 0; }
        else if (v > 255) { G.projection_flags |= FL_COLOR_R >> n; v = 255; }
        c |= (uint32_t)v << sh;
    }
    G.rgb_fifo[0] = G.rgb_fifo[1]; G.rgb_fifo[1] = G.rgb_fifo[2]; G.rgb_fifo[2] = c;
}

/* The reciprocal table of psx-spx: unr[i] = max(0, (0x40000/(i+0x100)+1)/2 - 0x101), i = 0..0x100 */
static uint32_t unr_entry(unsigned i)
{
    static uint16_t table[0x101];
    static int built;
    if (!built) {
        unsigned k;
        for (k = 0; k <= 0x100; k++) {
            int v = (int)((0x40000u / (k + 0x100u) + 1u) / 2u) - 0x101;
            table[k] = (uint16_t)(v < 0 ? 0 : v);
        }
        built = 1;
    }
    return table[i];
}

/* H/SZ3 perspective divide (psx-spx "GTE Division Inaccuracy"): result is
 * limited to 1FFFF; divide overflow (H >= SZ3*2) sets FLAG bit 17. */
static uint32_t gte_divide(uint32_t h, uint32_t sz3)
{
    uint32_t shift = 0, n, d, u, r;
    if (h >= sz3 * 2u) { G.projection_flags |= FL_DIV; return 0x1FFFFu; }
    d = sz3;
    while (!(d & 0x8000u)) { d <<= 1; shift++; }
    n = h << shift;
    u = unr_entry((d - 0x7FC0u) >> 7) + 0x101u;
    d = (0x2000080u - d * u) >> 8;
    d = (0x80u + d * u) >> 8;
    r = (uint32_t)(((uint64_t)n * d + 0x8000u) >> 16);
    return r > 0x1FFFFu ? 0x1FFFFu : r;
}

/* ------------------------------------------------------------------ */
/* matrix / vector products                                            */
/* ------------------------------------------------------------------ */

/* Mx element (mx 0 RT, 1 LLM, 2 LCM, 3 the reserved "garbage" matrix) */
static int32_t mat_elem(unsigned mx, unsigned r, unsigned c)
{
    switch (mx) {
    case 0: return G.rt[r][c];
    case 1: return G.llm[r][c];
    case 2: return G.lcm[r][c];
    default: {
        const int32_t red = (int32_t)(G.rgbc & 0xFF);
        if (r == 0) return c == 0 ? -(red << 4) : c == 1 ? (red << 4) : G.ir0;
        if (r == 1) return G.rt[0][2];
        return G.rt[1][1];
    }
    }
}

/* MAC = (Tx*1000h + Mx*v) SAR (sf*12), IR = MAC saturated.  cv: 0 TR, 1 BK,
 * 2 FC (hardware quirk), 3 none. v is the already-fetched input vector. */
static void mat_vec(unsigned mx, const int32_t v[3], unsigned cv, unsigned shift, unsigned lm)
{
    int64_t full[3];
    unsigned r;
    for (r = 0; r < 3; r++) {
        int64_t t = 0, a;
        if (cv == 0) t = (int64_t)G.tr[r] << 12;
        else if (cv == 1) t = (int64_t)G.bk[r] << 12;
        else if (cv == 2) t = (int64_t)G.fc[r] << 12;
        if (cv == 2) {
            /* Far colour: the FC + Mx1*Vx1 partial sum is only checked for MAC and
             * IR overflow flags (IR as lm=0); the stored result omits it. */
            a = acc44(r, t + (int64_t)mat_elem(mx, r, 0) * v[0]);
            (void)ir_sat(r, (int32_t)(a >> shift), 0);
            a = acc44(r, (int64_t)mat_elem(mx, r, 1) * v[1]);
        } else {
            a = acc44(r, t + (int64_t)mat_elem(mx, r, 0) * v[0]);
            a = acc44(r, a + (int64_t)mat_elem(mx, r, 1) * v[1]);
        }
        a = acc44(r, a + (int64_t)mat_elem(mx, r, 2) * v[2]);
        full[r] = a;
    }
    for (r = 0; r < 3; r++) set_mac_ir(r, full[r], shift, lm);
}

static void vec_fetch(unsigned which, int32_t out[3])
{
    unsigned i;
    for (i = 0; i < 3; i++) {
        switch (which) {
        case 0: out[i] = G.v0[i]; break;
        case 1: out[i] = G.v1[i]; break;
        case 2: out[i] = G.v2[i]; break;
        default: out[i] = (int16_t)G.ir[i]; break;
        }
    }
}

/* ------------------------------------------------------------------ */
/* perspective transform                                               */
/* ------------------------------------------------------------------ */

static void rtp_vertex(const int32_t v[3], unsigned shift, unsigned lm, int last)
{
    int64_t full[3];
    uint32_t q;
    int64_t sx, sy, d;
    unsigned r;
    for (r = 0; r < 3; r++) {
        int64_t a = acc44(r, ((int64_t)G.tr[r] << 12) + (int64_t)G.rt[r][0] * v[0]);
        a = acc44(r, a + (int64_t)G.rt[r][1] * v[1]);
        full[r] = acc44(r, a + (int64_t)G.rt[r][2] * v[2]);
        G.mac[r] = (int32_t)(full[r] >> shift);
    }
    G.ir[0] = ir_sat(0, G.mac[0], lm);
    G.ir[1] = ir_sat(1, G.mac[1], lm);
    {   /* IR3: the saturation flag judges MAC3 as if it were shifted by 12 */
        const int64_t z = full[2] >> 12;
        int32_t i3 = G.mac[2];
        const int32_t lo = lm ? 0 : -0x8000;
        if (z < -0x8000 || z > 0x7FFF) G.projection_flags |= FL_IR1_SAT >> 2;
        if (i3 < lo) i3 = lo; else if (i3 > 0x7FFF) i3 = 0x7FFF;
        G.ir[2] = i3;
        push_sz((int32_t)z);
    }
    q = gte_divide((uint32_t)(uint16_t)G.h, G.sz[3] & 0xFFFFu);
    sx = acc32((int64_t)q * G.ir[0] + G.ofx);
    sy = acc32((int64_t)q * G.ir[1] + G.ofy);
    push_sxy((int32_t)(sx >> 16), (int32_t)(sy >> 16));
    if (!last) return;
    d = acc32((int64_t)q * (int16_t)G.dqa + G.dqb);
    G.mac0 = (int32_t)d;
    {
        int32_t i0 = (int32_t)(d >> 12);
        if (i0 < 0) { G.projection_flags |= FL_IR0; i0 = 0; }
        else if (i0 > 0x1000) { G.projection_flags |= FL_IR0; i0 = 0x1000; }
        G.ir0 = i0;
    }
}

/* ------------------------------------------------------------------ */
/* colour commands                                                     */
/* ------------------------------------------------------------------ */

static void channel(int32_t c[3])
{
    c[0] = (int32_t)(G.rgbc & 0xFF);
    c[1] = (int32_t)((G.rgbc >> 8) & 0xFF);
    c[2] = (int32_t)((G.rgbc >> 16) & 0xFF);
}

/* [IR] = [MAC] = (BK*1000h + LCM*IR) SAR (sf*12) */
static void light_to_color(unsigned shift, unsigned lm)
{
    int32_t v[3];
    vec_fetch(3, v);
    mat_vec(2, v, 1, shift, lm);
}

/* [MAC] = [R*IR1, G*IR2, B*IR3] SHL 4 */
static void color_times_ir(int64_t m[3])
{
    int32_t c[3];
    unsigned i;
    channel(c);
    for (i = 0; i < 3; i++) m[i] = ((int64_t)c[i] * G.ir[i]) << 4;
}

/* Store m (unshifted) as MAC/IR. */
static void store_m(const int64_t m[3], unsigned shift, unsigned lm)
{
    unsigned i;
    for (i = 0; i < 3; i++) set_mac_ir(i, acc44(i, m[i]), shift, lm);
}

/* [MAC] = m + (FC - m)*IR0 : IR = ((FC<<12) - m) SAR (sf*12) (lm=0), then
 * MAC = IR*IR0 + m. */
static void interpolate(const int64_t m[3], unsigned shift, unsigned lm)
{
    int64_t full[3];
    unsigned i;
    for (i = 0; i < 3; i++) {
        int64_t d = acc44(i, ((int64_t)G.fc[i] << 12) - m[i]);
        G.ir[i] = ir_sat(i, (int32_t)(d >> shift), 0);
    }
    for (i = 0; i < 3; i++) full[i] = acc44(i, (int64_t)G.ir[i] * G.ir0 + m[i]);
    for (i = 0; i < 3; i++) set_mac_ir(i, full[i], shift, lm);
}

/* NCS/NCCS/NCDS for one vertex; kind 0 NCS, 1 NCCS, 2 NCDS */
static void nc_vertex(const int32_t v[3], unsigned kind, unsigned shift, unsigned lm)
{
    int64_t m[3];
    mat_vec(1, v, 3, shift, lm);
    light_to_color(shift, lm);
    if (kind == 1) { color_times_ir(m); store_m(m, shift, lm); }
    else if (kind == 2) { color_times_ir(m); interpolate(m, shift, lm); }
    push_color();
}

/* ------------------------------------------------------------------ */
/* command dispatch                                                    */
/* ------------------------------------------------------------------ */

static void do_command(uint32_t word)
{
    const unsigned sf = (word >> 19) & 1u, lm = (word >> 10) & 1u;
    const unsigned shift = sf ? 12u : 0u;
    int32_t v[3];
    int64_t m[3];
    unsigned i;

    G.projection_flags = 0;
    switch (word & 0x3Fu) {
    case 0x01: vec_fetch(0, v); rtp_vertex(v, shift, lm, 1); break;
    case 0x30:
        for (i = 0; i < 3; i++) { vec_fetch(i, v); rtp_vertex(v, shift, lm, i == 2); }
        break;
    case 0x06: {
        int64_t a = (int64_t)sx16(G.sxy[0]) * sy16(G.sxy[1]) + (int64_t)sx16(G.sxy[1]) * sy16(G.sxy[2])
                  + (int64_t)sx16(G.sxy[2]) * sy16(G.sxy[0]) - (int64_t)sx16(G.sxy[0]) * sy16(G.sxy[2])
                  - (int64_t)sx16(G.sxy[1]) * sy16(G.sxy[0]) - (int64_t)sx16(G.sxy[2]) * sy16(G.sxy[1]);
        G.mac0 = (int32_t)acc32(a);
        break;
    }
    case 0x0C: {   /* OP: cross product of IR with the rotation diagonal */
        const int32_t d1 = G.rt[0][0], d2 = G.rt[1][1], d3 = G.rt[2][2];
        const int32_t i1 = (int16_t)G.ir[0], i2 = (int16_t)G.ir[1], i3 = (int16_t)G.ir[2];
        set_mac_ir(0, acc44(0, (int64_t)i3 * d2 - (int64_t)i2 * d3), shift, lm);
        set_mac_ir(1, acc44(1, (int64_t)i1 * d3 - (int64_t)i3 * d1), shift, lm);
        set_mac_ir(2, acc44(2, (int64_t)i2 * d1 - (int64_t)i1 * d2), shift, lm);
        break;
    }
    case 0x10:   /* DPCS */
        channel(v);
        for (i = 0; i < 3; i++) m[i] = (int64_t)v[i] << 16;
        interpolate(m, shift, lm);
        push_color();
        break;
    case 0x2A:   /* DPCT: three passes over the oldest colour FIFO entry */
        for (i = 0; i < 3; i++) {
            const uint32_t c = G.rgb_fifo[0];
            m[0] = (int64_t)(c & 0xFF) << 16;
            m[1] = (int64_t)((c >> 8) & 0xFF) << 16;
            m[2] = (int64_t)((c >> 16) & 0xFF) << 16;
            interpolate(m, shift, lm);
            push_color();
        }
        break;
    case 0x11:   /* INTPL */
        for (i = 0; i < 3; i++) m[i] = (int64_t)(int16_t)G.ir[i] << 12;
        interpolate(m, shift, lm);
        push_color();
        break;
    case 0x29:   /* DCPL */
        color_times_ir(m);
        interpolate(m, shift, lm);
        push_color();
        break;
    case 0x12: {  /* MVMVA */
        vec_fetch((word >> 15) & 3u, v);
        mat_vec((word >> 17) & 3u, v, (word >> 13) & 3u, shift, lm);
        break;
    }
    case 0x13: vec_fetch(0, v); nc_vertex(v, 2, shift, lm); break;
    case 0x16: for (i = 0; i < 3; i++) { vec_fetch(i, v); nc_vertex(v, 2, shift, lm); } break;
    case 0x1B: vec_fetch(0, v); nc_vertex(v, 1, shift, lm); break;
    case 0x3F: for (i = 0; i < 3; i++) { vec_fetch(i, v); nc_vertex(v, 1, shift, lm); } break;
    case 0x1E: vec_fetch(0, v); nc_vertex(v, 0, shift, lm); break;
    case 0x20: for (i = 0; i < 3; i++) { vec_fetch(i, v); nc_vertex(v, 0, shift, lm); } break;
    case 0x1C:   /* CC */
        light_to_color(shift, lm);
        color_times_ir(m); store_m(m, shift, lm);
        push_color();
        break;
    case 0x14:   /* CDP */
        light_to_color(shift, lm);
        color_times_ir(m); interpolate(m, shift, lm);
        push_color();
        break;
    case 0x28:   /* SQR */
        for (i = 0; i < 3; i++) {
            const int64_t q = (int64_t)(int16_t)G.ir[i] * (int16_t)G.ir[i];
            set_mac_ir(i, acc44(i, q), shift, lm);
        }
        break;
    case 0x2D:   /* AVSZ3 on the SZ FIFO */
    case 0x2E: { /* AVSZ4 */
        const int four = (word & 0x3Fu) == 0x2E;
        int64_t s = (int64_t)(G.sz[1] & 0xFFFF) + (G.sz[2] & 0xFFFF) + (G.sz[3] & 0xFFFF);
        int64_t r;
        if (four) s += G.sz[0] & 0xFFFF;
        r = acc32(s * (int16_t)(four ? G.zsf4 : G.zsf3));
        G.mac0 = (int32_t)r;
        set_otz(r >> 12);
        break;
    }
    case 0x3D:   /* GPF */
        for (i = 0; i < 3; i++)
            set_mac_ir(i, acc44(i, (int64_t)(int16_t)G.ir[i] * G.ir0), shift, lm);
        push_color();
        break;
    case 0x3E:   /* GPL */
        for (i = 0; i < 3; i++)
            set_mac_ir(i, acc44(i, ((int64_t)G.mac[i] << shift) + (int64_t)(int16_t)G.ir[i] * G.ir0), shift, lm);
        push_color();
        break;
    default:
        fprintf(stderr, "PE_GTE_Command: undocumented cop2 operation 0x%02X (word %07X)\n",
                (unsigned)(word & 0x3Fu), (unsigned)(word & 0x1FFFFFFu));
        abort();
    }
    flag_commit();
}

/* Perspective results go through PE_GTE_ExecuteNative while the sub-pixel
 * sidecar is recording, which is the only place that can store the fraction. */
static void run_command(uint32_t word)
{
    word &= 0x1FFFFFFu;
    if (PE_GTE_GetSubpixelCapture() && ((word & 0x3Fu) == 0x01u || (word & 0x3Fu) == 0x30u)) {
        if (PE_GTE_ExecuteNative(word) == 0) return;
    }
    do_command(word);
}

void PE_GTE_Command(uint32_t cmd) { run_command(cmd); }

/* ------------------------------------------------------------------ */
/* register file                                                       */
/* ------------------------------------------------------------------ */

static uint32_t pack16(uint32_t lo, uint32_t hi) { return (lo & 0xFFFFu) | (hi << 16); }
static uint32_t ext16(int32_t v) { return (uint32_t)(int32_t)(int16_t)v; }

/* RT/LLM/LCM pair `k` (0..4): halfwords 2k and 2k+1 of the row-major matrix */
static uint32_t mat_ctrl_read(const int16_t m[3][3], unsigned k)
{
    const int16_t *f = &m[0][0];
    if (k == 4) return (uint32_t)(int32_t)f[8];
    return pack16((uint16_t)f[2 * k], (uint16_t)f[2 * k + 1]);
}
static void mat_ctrl_write(int16_t m[3][3], unsigned k, uint32_t v)
{
    int16_t *f = &m[0][0];
    f[2 * k] = (int16_t)v;
    if (k != 4) f[2 * k + 1] = (int16_t)(v >> 16);
}

static uint32_t orgb(void)
{
    uint32_t out = 0, i;
    for (i = 0; i < 3; i++) {
        int32_t c = (int16_t)G.ir[i] >> 7;
        if (c < 0) c = 0; else if (c > 0x1F) c = 0x1F;
        out |= (uint32_t)c << (5 * i);
    }
    return out;
}

uint32_t PE_GTE_ReadData(uint32_t reg)
{
    switch (reg & 31u) {
    case 0: return pack16((uint16_t)G.v0[0], (uint16_t)G.v0[1]);
    case 1: return ext16(G.v0[2]);
    case 2: return pack16((uint16_t)G.v1[0], (uint16_t)G.v1[1]);
    case 3: return ext16(G.v1[2]);
    case 4: return pack16((uint16_t)G.v2[0], (uint16_t)G.v2[1]);
    case 5: return ext16(G.v2[2]);
    case 6: return G.rgbc;
    case 7: return G.otz & 0xFFFFu;
    case 8: return ext16(G.ir0);
    case 9: case 10: case 11: return ext16(G.ir[(reg & 31u) - 9u]);
    case 12: case 13: case 14: return G.sxy[(reg & 31u) - 12u];
    case 15: return G.sxy[2];
    case 16: case 17: case 18: case 19: return G.sz[(reg & 31u) - 16u] & 0xFFFFu;
    case 20: case 21: case 22: return G.rgb_fifo[(reg & 31u) - 20u];
    case 23: return g_pe_gte_res1;
    case 24: return (uint32_t)G.mac0;
    case 25: case 26: case 27: return (uint32_t)G.mac[(reg & 31u) - 25u];
    case 28: case 29: return orgb();
    case 30: return G.lzcs;
    default: return G.lzcr;
    }
}

static uint32_t lzcr_of(uint32_t v)
{
    uint32_t t = (v & 0x80000000u) ? ~v : v, n = 32;
    while (t) { t >>= 1; n--; }
    return n;
}

uint32_t PE_GTE_LZCR(uint32_t v) { return lzcr_of(v); }

void PE_GTE_SetLZCS(uint32_t v) { G.lzcs = v; G.lzcr = lzcr_of(v); }

void PE_GTE_WriteData(uint32_t reg, uint32_t v)
{
    switch (reg & 31u) {
    case 0: G.v0[0] = (int16_t)v; G.v0[1] = (int16_t)(v >> 16); break;
    case 1: G.v0[2] = (int16_t)v; break;
    case 2: G.v1[0] = (int16_t)v; G.v1[1] = (int16_t)(v >> 16); break;
    case 3: G.v1[2] = (int16_t)v; break;
    case 4: G.v2[0] = (int16_t)v; G.v2[1] = (int16_t)(v >> 16); break;
    case 5: G.v2[2] = (int16_t)v; break;
    case 6: G.rgbc = v; break;
    case 7: G.otz = v & 0xFFFFu; break;
    case 8: G.ir0 = (int16_t)v; break;
    case 9: case 10: case 11: G.ir[(reg & 31u) - 9u] = (int16_t)v; break;
    case 12: case 13: case 14: G.sxy[(reg & 31u) - 12u] = v; break;
    case 15: G.sxy[0] = G.sxy[1]; G.sxy[1] = G.sxy[2]; G.sxy[2] = v; break;
    case 16: case 17: case 18: case 19: G.sz[(reg & 31u) - 16u] = v & 0xFFFFu; break;
    case 20: case 21: case 22: G.rgb_fifo[(reg & 31u) - 20u] = v; break;
    case 23: g_pe_gte_res1 = v; break;
    case 24: G.mac0 = (int32_t)v; break;
    case 25: case 26: case 27: G.mac[(reg & 31u) - 25u] = (int32_t)v; break;
    case 28:   /* IRGB: five bits per channel, scaled by 80h */
        G.ir[0] = (int32_t)(v & 0x1Fu) * 0x80;
        G.ir[1] = (int32_t)((v >> 5) & 0x1Fu) * 0x80;
        G.ir[2] = (int32_t)((v >> 10) & 0x1Fu) * 0x80;
        break;
    case 29: break;   /* ORGB is read-only */
    case 30: PE_GTE_SetLZCS(v); break;
    default: break;   /* LZCR is read-only */
    }
}

uint32_t PE_GTE_ReadCtrl(uint32_t reg)
{
    const uint32_t r = reg & 31u;
    if (r <= 4) return mat_ctrl_read(G.rt, r);
    if (r <= 7) return (uint32_t)G.tr[r - 5u];
    if (r <= 12) return mat_ctrl_read(G.llm, r - 8u);
    if (r <= 15) return (uint32_t)G.bk[r - 13u];
    if (r <= 20) return mat_ctrl_read(G.lcm, r - 16u);
    if (r <= 23) return (uint32_t)G.fc[r - 21u];
    switch (r) {
    case 24: return (uint32_t)G.ofx;
    case 25: return (uint32_t)G.ofy;
    case 26: return ext16(G.h);
    case 27: return ext16(G.dqa);
    case 28: return (uint32_t)G.dqb;
    case 29: return ext16(G.zsf3);
    case 30: return ext16(G.zsf4);
    default: {
        uint32_t f = G.projection_flags & ~FL_ERROR;
        return (f & FL_SUMMARY_MASK) ? (f | FL_ERROR) : f;
    }
    }
}

void PE_GTE_WriteCtrl(uint32_t reg, uint32_t v)
{
    const uint32_t r = reg & 31u;
    if (r <= 4) { mat_ctrl_write(G.rt, r, v); return; }
    if (r <= 7) { G.tr[r - 5u] = (int32_t)v; return; }
    if (r <= 12) { mat_ctrl_write(G.llm, r - 8u, v); return; }
    if (r <= 15) { G.bk[r - 13u] = (int32_t)v; return; }
    if (r <= 20) { mat_ctrl_write(G.lcm, r - 16u, v); return; }
    if (r <= 23) { G.fc[r - 21u] = (int32_t)v; return; }
    switch (r) {
    case 24: G.ofx = (int32_t)v; break;
    case 25: G.ofy = (int32_t)v; break;
    case 26: G.h = (int32_t)(v & 0xFFFFu); break;
    case 27: G.dqa = (int16_t)v; break;
    case 28: G.dqb = (int32_t)v; break;
    case 29: G.zsf3 = (int16_t)v; break;
    case 30: G.zsf4 = (int16_t)v; break;
    default:
        G.projection_flags = v & 0x7FFFF000u;
        flag_commit();
        break;
    }
}

/* ------------------------------------------------------------------ */
/* loaders and setters                                                 */
/* ------------------------------------------------------------------ */

static void load_matrix3(int16_t m[3][3], pe_addr_t a)
{
    unsigned i;
    for (i = 0; i < 9; i++) m[i / 3][i % 3] = (int16_t)PE_LoadU16(a + 2u * i);
}

void PE_GTE_LoadRT33(pe_addr_t matrix) { load_matrix3(G.rt, matrix); }

void PE_GTE_LoadRT(pe_addr_t matrix)
{
    unsigned i;
    load_matrix3(G.rt, matrix);
    for (i = 0; i < 3; i++) G.tr[i] = (int32_t)PE_LoadU32(matrix + 0x14u + 4u * i);
}

void PE_GTE_LoadLCM(pe_addr_t matrix) { load_matrix3(G.lcm, matrix); }

void PE_GTE_LoadLLM_halfs(const int16_t *halfs)
{
    unsigned i;
    for (i = 0; i < 9; i++) G.llm[i / 3][i % 3] = halfs[i];
}

void PE_GTE_SetIR(int16_t ir1, int16_t ir2, int16_t ir3)
{
    G.ir[0] = ir1; G.ir[1] = ir2; G.ir[2] = ir3;
}

void PE_GTE_SetV0(int16_t vx, int16_t vy, int16_t vz) { G.v0[0] = vx; G.v0[1] = vy; G.v0[2] = vz; }
void PE_GTE_SetV1(int16_t vx, int16_t vy, int16_t vz) { G.v1[0] = vx; G.v1[1] = vy; G.v1[2] = vz; }
void PE_GTE_SetV2(int16_t vx, int16_t vy, int16_t vz) { G.v2[0] = vx; G.v2[1] = vy; G.v2[2] = vz; }
void PE_GTE_SetRGBC(uint32_t rgbc) { G.rgbc = rgbc; }
void PE_GTE_SetBK(int32_t rbk, int32_t gbk, int32_t bbk) { G.bk[0] = rbk; G.bk[1] = gbk; G.bk[2] = bbk; }

/* ------------------------------------------------------------------ */
/* command wrappers                                                    */
/* ------------------------------------------------------------------ */

static uint32_t sf_lm(int sf, int lm) { return ((uint32_t)(sf != 0) << 19) | ((uint32_t)(lm != 0) << 10); }

/* only the sf/mx/v/cv/lm fields of cmd are used; the opcode is implied */
void PE_GTE_MVMVA(uint32_t cmd) { run_command(0x12u | (cmd & 0x1FFFFC0u)); }
void PE_GTE_GPF(int sf, int lm) { run_command(0x3Du | sf_lm(sf, lm)); }
void PE_GTE_GPL(int sf, int lm) { run_command(0x3Eu | sf_lm(sf, lm)); }
void PE_GTE_OP(int sf, int lm) { run_command(0x0Cu | sf_lm(sf, lm)); }
void PE_GTE_NCCT(void) { run_command(0x3Fu | sf_lm(1, 1)); }

void PE_GTE_RTPS_coordinates(uint32_t *xy, uint32_t *z)
{
    run_command(0x01u | sf_lm(1, 0));
    *xy = G.sxy[2];
    *z = G.sz[3] & 0xFFFFu;
}

void PE_GTE_RTPT_coordinates(uint32_t xy[3], uint32_t z[3])
{
    unsigned i;
    run_command(0x30u | sf_lm(1, 0));
    for (i = 0; i < 3; i++) { xy[i] = G.sxy[i]; z[i] = G.sz[i + 1u] & 0xFFFFu; }
}

/* AVSZ on caller-supplied depths: the SZ FIFO itself is not read or written */
static void avsz_array(const uint32_t *z, unsigned n, int32_t zsf)
{
    int64_t s = 0, r;
    unsigned i;
    G.projection_flags = 0;
    for (i = 0; i < n; i++) s += z[i] & 0xFFFFu;
    r = acc32(s * (int16_t)zsf);
    G.mac0 = (int32_t)r;
    set_otz(r >> 12);
    flag_commit();
}

void PE_GTE_AVSZ3(const uint32_t z[3]) { avsz_array(z, 3, G.zsf3); }
void PE_GTE_AVSZ4(const uint32_t z[4]) { avsz_array(z, 4, G.zsf4); }

int32_t PE_GTE_NCLIP(void)
{
    run_command(0x06u);
    return G.mac0;
}

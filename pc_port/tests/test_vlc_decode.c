/*
 * test_vlc_decode.c - tests for pe_vlc_decode (BS bitstream -> MDEC words).
 *
 * Provenance: written clean-room from the psx-spx MDEC and STR documentation
 * only, no SDK or third-party decoder source.
 *
 * Build/run:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include \
 *      pc_port/tests/test_vlc_decode.c pc_port/platform/pe_vlc_decode.c \
 *      -o build-vlc-test && ./build-vlc-test
 * Mutant control: add -DPE_VLC_MUTANT=n (1..8,10) -- every mutant must FAIL.
 *
 * The encoder below (tables as psx-spx pattern strings such as "00100xxxs")
 * is a separate transcription from the decoder's tables.  AC pattern rows were
 * generated mechanically from the fetched psx-spx text (see REPORT.md).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_vlc_decode.h"
#ifdef PE_VLC_MUTANT
#define MUTVAL PE_VLC_MUTANT
#else
#define MUTVAL 0
#endif

static unsigned long n_checks, n_fail;
static unsigned long cat_fail[16];
static int cat;
enum { C_HAND, C_ROUND, C_ALLCODES, C_ERR, C_BOUNDS, C_TABLE, C_MDEC, C_V3, N_CATS };
static const char *cat_name[] = {"hand-computed", "random round-trip", "all-codes",
                                 "error/desync", "bounds (trunc/cap)", "table sanity",
                                 "mdec-layout", "v3 DC"};
#define CHECK(c, ...)                                                          \
    do {                                                                       \
        n_checks++;                                                            \
        if (!(c)) {                                                            \
            n_fail++;                                                          \
            cat_fail[cat]++;                                                   \
            if (n_fail <= 12) {                                                \
                printf("FAIL %s:%d: ", __FILE__, __LINE__);                    \
                printf(__VA_ARGS__);                                           \
                printf("\n");                                                  \
            }                                                                  \
        }                                                                      \
    } while (0)

/* ---------------- rng ---------------- */
static uint32_t rs = 12345;
static uint32_t rnd(void)
{
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}
static uint32_t rndn(uint32_t n) { return rnd() % n; }

/* ---------------- bit writer ---------------- */
typedef struct {
    uint8_t *b;
    size_t n, cap;
} bw;
static void bw_init(bw *w) { w->b = NULL; w->n = w->cap = 0; }
static void bw_free(bw *w) { free(w->b); bw_init(w); }
static void bw_bit(bw *w, int v)
{
    if (w->n == w->cap) {
        w->cap = w->cap ? w->cap * 2 : 4096;
        w->b = (uint8_t *)realloc(w->b, w->cap);
    }
    w->b[w->n++] = (uint8_t)(v & 1);
}
static void bw_str(bw *w, const char *s)
{
    for (; *s; s++)
        if (*s == '0' || *s == '1')
            bw_bit(w, *s - '0');
}
static void bw_val(bw *w, uint32_t v, int n)
{
    int i;
    for (i = n - 1; i >= 0; i--)
        bw_bit(w, (int)((v >> i) & 1));
}
/* Build a frame: 8-byte header + bitstream packed MSB-first into LE halfwords.
 * Returns malloc'ed exact-size buffer. */
static uint8_t *mk_frame(const bw *w, unsigned ver, unsigned quant, unsigned size_field,
                         unsigned id, size_t *len)
{
    size_t nh = (w->n + 15) / 16, i, j;
    uint8_t *f = (uint8_t *)malloc(8 + nh * 2);
    f[0] = (uint8_t)size_field; f[1] = (uint8_t)(size_field >> 8);
    f[2] = (uint8_t)id; f[3] = (uint8_t)(id >> 8);
    f[4] = (uint8_t)quant; f[5] = (uint8_t)(quant >> 8);
    f[6] = (uint8_t)ver; f[7] = (uint8_t)(ver >> 8);
    for (i = 0; i < nh; i++) {
        unsigned h = 0;
        for (j = 0; j < 16; j++) {
            size_t k = i * 16 + j;
            h = (h << 1) | (k < w->n ? w->b[k] : 0u);
        }
        f[8 + 2 * i] = (uint8_t)h;
        f[8 + 2 * i + 1] = (uint8_t)(h >> 8);
    }
    *len = 8 + nh * 2;
    return f;
}
static uint8_t *frame_from_str(const char *bits, unsigned ver, unsigned quant, size_t *len)
{
    bw w; uint8_t *f;
    bw_init(&w); bw_str(&w, bits);
    f = mk_frame(&w, ver, quant, 0, 0x3800, len);
    bw_free(&w);
    return f;
}

/* ---------------- decode wrapper with sentinel/bounds ---------------- */
/* Decodes with cap=nexp+extra into a heap buffer with sentinels after it. */
static int decode_checked(const uint8_t *frame, size_t n, size_t cap, uint16_t **out,
                          size_t *rl_len)
{
    uint16_t *buf = (uint16_t *)malloc((cap + 4) * 2);
    int rc, i;
    uint8_t *exact = (uint8_t *)malloc(n ? n : 1); /* exact-size input copy */
    if (n) memcpy(exact, frame, n);
    for (i = 0; i < (int)cap + 4; i++) buf[i] = 0xDEAD;
    *rl_len = 12345;
    rc = pe_vlc_decode_frame(exact, n, buf, cap, rl_len);
    free(exact);
    for (i = 0; i < 4; i++)
        CHECK(buf[cap + i] == 0xDEAD, "write past rl_cap (cap=%zu)", cap);
    CHECK(*rl_len <= cap, "rl_len %zu > cap %zu", *rl_len, cap);
    *out = buf;
    return rc;
}

static void expect_ok(const char *name, const uint8_t *f, size_t n, const uint16_t *exp,
                      size_t nexp)
{
    uint16_t *o; size_t len; size_t i;
    int rc = decode_checked(f, n, nexp + 16, &o, &len);
    CHECK(rc == 0, "%s: rc=%d", name, rc);
    CHECK(len == nexp, "%s: len=%zu expected %zu", name, len, nexp);
    for (i = 0; i < nexp && i < len; i++)
        CHECK(o[i] == exp[i], "%s: word %zu = %04X expected %04X", name, i, o[i], exp[i]);
    free(o);
}
static void expect_rc(const char *name, const uint8_t *f, size_t n, int exp_rc)
{
    uint16_t *o; size_t len;
    int rc = decode_checked(f, n, 4096, &o, &len);
    CHECK(rc == exp_rc, "%s: rc=%d expected %d", name, rc, exp_rc);
    free(o);
}

/* ---------------- encoder tables (test-side transcription) ---------------- */
typedef struct { const char *pat, *vals; } prow;
static const prow ac_prows[] = {
    {"11s", "0001"},
    {"011s", "0401"},
    {"010xs", "0002,0801"},
    {"0011xs", "1001,0C01"},
    {"00101s", "0003"},
    {"00100xxxs", "3401,0006,3001,2C01,0C02,0403,0005,2801"},
    {"0001xxs", "1C01,1801,0402,1401"},
    {"00001xxs", "0802,2401,0004,2001"},
    {"0000001xxxs", "4001,1402,0007,0803,0404,3C01,3801,1002"},
    {"00000001xxxxs", "000B,2002,1003,000A,0804,1C02,5401,5001,0009,4C01,4801,0405,0C03,0008,1802,4401"},
    {"000000001xxxxs", "2802,2402,1403,0C04,0805,0407,0406,000F,000E,000D,000C,6801,6401,6001,5C01,5801"},
    {"0000000001xxxxs", "001F,001E,001D,001C,001B,001A,0019,0018,0017,0016,0015,0014,0013,0012,0011,0010"},
    {"00000000001xxxxs", "0028,0027,0026,0025,0024,0023,0022,0021,0020,040E,040D,040C,040B,040A,0409,0408"},
    {"000000000001xxxxs", "0412,0411,0410,040F,1803,4002,3C02,3802,3402,3002,2C02,7C01,7801,7401,7001,6C01"},
};
#define N_ACP (sizeof ac_prows / sizeof ac_prows[0])

typedef struct { uint32_t code; int len; uint16_t word; } acode;
static acode ac_codes[2048];
static int n_ac;
static int ac_of_word[65536]; /* index into ac_codes or -1 (positive+negative forms) */

static int plen_of(const char *p)
{
    int n = 0;
    while (p[n] == '0' || p[n] == '1') n++;
    return n;
}
static void build_ac(void)
{
    size_t r; int i;
    for (i = 0; i < 65536; i++) ac_of_word[i] = -1;
    for (r = 0; r < N_ACP; r++) {
        const char *pat = ac_prows[r].pat;
        int pl = plen_of(pat), nx = 0, hasS = 0, nv = 0, idx, s;
        uint16_t vals[32];
        const char *c;
        for (c = pat + pl; *c; c++) { if (*c == 'x') nx++; if (*c == 's') hasS = 1; }
        for (c = ac_prows[r].vals; *c;) {
            vals[nv++] = (uint16_t)strtoul(c, NULL, 16);
            while (*c && *c != ',') c++;
            if (*c == ',') c++;
        }
        if (nv != (1 << nx)) { printf("table build: row %s has %d vals\n", pat, nv); exit(2); }
        for (idx = 0; idx < nv; idx++)
            for (s = 0; s <= hasS; s++) {
                acode a; uint32_t code = 0; int len = 0, k;
                uint16_t w = vals[idx];
                for (k = 0; k < pl; k++) { code = code << 1 | (uint32_t)(pat[k] - '0'); len++; }
                for (k = nx - 1; k >= 0; k--) { code = code << 1 | (uint32_t)((idx >> k) & 1); len++; }
                if (hasS) { code = code << 1 | (uint32_t)s; len++; }
                if (s) w = (uint16_t)((w & 0xFC00) | ((0x400 - (w & 0x3FF)) & 0x3FF));
                a.code = code; a.len = len; a.word = w;
                ac_codes[n_ac] = a;
                if (ac_of_word[w] != -1) { printf("table build: duplicate word %04X\n", w); exit(2); }
                ac_of_word[w] = n_ac++;
            }
    }
}
static void enc_ac(bw *w, uint16_t word, int force_escape)
{
    int i = ac_of_word[word];
    if (i >= 0 && !force_escape) bw_val(w, ac_codes[i].code, ac_codes[i].len);
    else { bw_str(w, "000001"); bw_val(w, word, 16); }
}

/* DC v3 tables */
static const char *dc3_chroma_rows[] = {"01s", "10sn", "110snn", "1110snnn", "11110snnnn",
                                        "111110snnnnn", "1111110snnnnnn", "11111110snnnnnnn"};
static const char *dc3_luma_rows[] = {"00s", "01sn", "101snn", "110snnn", "1110snnnn",
                                      "11110snnnnn", "111110snnnnnn", "1111110snnnnnnn"};
static void enc_dc3(bw *w, int luma, int d) /* d multiple of 4, |d|<=1020 */
{
    int m, s, r, base, nb, i, idx = -1;
    const char **rows = luma ? dc3_luma_rows : dc3_chroma_rows;
    const char *c;
    if (d == 0) { bw_str(w, luma ? "100" : "00"); return; }
    m = (d < 0 ? -d : d) / 4; s = d > 0;
    for (r = 0; r < 8; r++) {
        const char *pat = rows[r];
        nb = 0;
        for (c = pat; *c; c++) if (*c == 'n') nb++;
        base = 1 << nb;
        if (m < base || m > 2 * base - 1) continue;
        /* explicit lists as printed in psx-spx: negative descending, positive ascending */
        for (i = 0; i < base; i++) {
            int mag = s ? base + i : 2 * base - 1 - i;
            if (mag == m) idx = i;
        }
        for (c = pat; *c; c++) {
            if (*c == '0' || *c == '1') bw_bit(w, *c - '0');
            else if (*c == 's') bw_bit(w, s);
        }
        bw_val(w, (uint32_t)idx, nb);
        return;
    }
    printf("enc_dc3: no row for %d\n", d); exit(2);
}

/* Kraft sums (fixed-point 2^-16) */
static unsigned kraft(const char *const *pats, int n)
{
    unsigned sum = 0; int i;
    for (i = 0; i < n; i++) sum += 1u << (16 - plen_of(pats[i]));
    return sum;
}

/* ---------------- model-based frame generation ---------------- */
typedef struct {
    uint16_t *words; size_t n, cap;
} wlist;
static void wl_put(wlist *l, uint16_t v)
{
    if (l->n == l->cap) { l->cap = l->cap ? l->cap * 2 : 1024; l->words = (uint16_t *)realloc(l->words, l->cap * 2); }
    l->words[l->n++] = v;
}

static void gen_block_ac(bw *w, wlist *exp, unsigned ver)
{
    unsigned k = 0; int m = 0, maxm;
    switch (rndn(5)) { case 0: maxm = 0; break; case 1: maxm = 3; break; case 2: maxm = 12; break;
                       default: maxm = 63; break; }
    for (;;) {
        unsigned run, level, left = 63 - k;
        int neg; uint16_t word;
        if (m >= maxm || left == 0) break;
        if (rndn(4) == 0) run = rndn(left); else run = rndn(left < 4 ? left : 4);
        if (run > left - 1) run = left - 1;
        switch (rndn(4)) { case 0: level = 1 + rndn(512); break; case 1: level = 1 + rndn(31); break;
                           default: level = 1 + rndn(3); break; }
        neg = (int)rndn(2);
        if (neg && level == 512) level = 511;
        word = (uint16_t)((run << 10) | ((neg ? (0x400 - level) : level) & 0x3FF));
        if (word == 0xFE00) continue;
        if (ver == 1 && rndn(12) == 0 && k + run + 1 <= 63) { /* v1 nonsense run/level0 escape */
            unsigned run2 = rndn(63 - k);
            uint16_t nw = (uint16_t)(run2 << 10);
            bw_str(w, "000001"); bw_val(w, nw, 16); wl_put(exp, nw);
            k += run2 + 1;
            if (k > 62) break;
            continue;
        }
        enc_ac(w, word, rndn(10) == 0);
        wl_put(exp, word);
        k += run + 1; m++;
    }
    bw_str(w, "10"); wl_put(exp, 0xFE00);
}

static size_t g_stream_bits; /* bits up to and including the EOF code */
static uint8_t *gen_frame(unsigned ver, unsigned quant, int nmb, wlist *exp, size_t *len)
{
    bw w; int mb, blk;
    int oc[3] = {0, 0, 0};
    uint8_t *f;
    bw_init(&w);
    for (mb = 0; mb < nmb; mb++)
        for (blk = 0; blk < 6; blk++) {
            uint16_t dcw;
            if (ver == 3) {
                int comp = blk < 2 ? blk : 2, d, nd;
                d = (int)rndn(511) * 4 - 1020; /* -1020..1020 */
                if (rndn(4) == 0) d = 0;
                enc_dc3(&w, blk >= 2, d);
                nd = (oc[comp] + d) & 0x3FF;
                nd = (nd ^ 0x200) - 0x200;
                oc[comp] = nd;
                dcw = (uint16_t)((quant << 10) | (unsigned)(nd & 0x3FF));
            } else {
                uint32_t raw;
                do raw = rndn(1024); while (raw == 0x1FF);
                bw_val(&w, raw, 10);
                dcw = (uint16_t)((quant << 10) | raw);
            }
            wl_put(exp, dcw);
            gen_block_ac(&w, exp, ver);
        }
    bw_str(&w, ver == 3 ? "1111111111" : "0111111111");
    g_stream_bits = w.n;
    {
        int pad = (int)rndn(16); while (pad--) bw_bit(&w, 0);
    }
    f = mk_frame(&w, ver, quant, 0, 0x3800, len);
    bw_free(&w);
    return f;
}

/* ====================================================================== */
static void test_tables(void)
{
    static const char *ac_all[] = {"10", "11s", "011s", "010xs", "0011xs", "00101s", "00100xxxs",
        "0001xxs", "00001xxs", "000001", "0000001xxxs", "00000001xxxxs", "000000001xxxxs",
        "0000000001xxxxs", "00000000001xxxxs", "000000000001xxxxs", "000000000000"};
    static const char *cc_all[] = {"00", "01s", "10sn", "110snn", "1110snnn", "11110snnnn",
        "111110snnnnn", "1111110snnnnnn", "11111110snnnnnnn", "111111110", "1111111110", "1111111111"};
    static const char *yy_all[] = {"100", "00s", "01sn", "101snn", "110snnn", "1110snnnn",
        "11110snnnnn", "111110snnnnnn", "1111110snnnnnnn", "11111110", "111111110", "1111111110",
        "1111111111"};
    int i, j;
    cat = C_TABLE;
    CHECK(kraft(ac_all, 17) == 65536u, "AC kraft sum %u", kraft(ac_all, 17));
    CHECK(kraft(cc_all, 12) == 65536u, "chroma DC kraft %u", kraft(cc_all, 12));
    CHECK(kraft(yy_all, 13) == 65536u, "luma DC kraft %u", kraft(yy_all, 13));
    /* every table pattern row in ac_prows matches the doc's row layout:
       prefix + x* + optional s, and 2^x values */
    for (i = 0; i < (int)N_ACP; i++) {
        int pl = plen_of(ac_prows[i].pat), nx = 0;
        const char *c;
        for (c = ac_prows[i].pat + pl; *c == 'x'; c++) nx++;
        CHECK(c[0] == 's' || c[0] == 0, "row %s malformed", ac_prows[i].pat);
    }
    CHECK(n_ac == 2 * (1 + 1 + 2 + 2 + 1 + 8 + 4 + 4 + 8 + 16 * 5), "AC code count %d", n_ac);
    /* no zero level except EOB; no duplicate words (checked at build); prefix-freeness */
    for (i = 0; i < n_ac; i++) {
        CHECK((ac_codes[i].word & 0x3FF) != 0, "zero level in table word %04X", ac_codes[i].word);
        for (j = 0; j < n_ac; j++) {
            if (i == j) continue;
            if (ac_codes[i].len <= ac_codes[j].len)
                CHECK((ac_codes[j].code >> (ac_codes[j].len - ac_codes[i].len)) != ac_codes[i].code,
                      "code %d is a prefix of code %d", i, j);
        }
    }
}

static void hand_ac_cases(unsigned ver)
{
    static const struct { const char *bits; uint16_t w; } cs[] = {
        {"110", 0x0001}, {"111", 0x03FF}, {"0110", 0x0401}, {"0111", 0x07FF},
        {"01000", 0x0002}, {"01010", 0x0801}, {"01011", 0x0BFF},
        {"001100", 0x1001}, {"001111", 0x0FFF}, {"001010", 0x0003}, {"001011", 0x03FD},
        {"001000000", 0x3401}, {"001000111", 0x2FFF}, {"001001111", 0x2BFF}, {"001000110", 0x2C01},
        {"0001000", 0x1C01}, {"0001111", 0x17FF},
        {"00001000", 0x0802}, {"00001111", 0x23FF},
        {"00000010000", 0x4001}, {"00000011111", 0x13FE}, {"00000010100", 0x0007},
        {"000000010000" "0", 0x000B}, {"000000011111" "1", 0x47FF},
        {"0000000010000" "0", 0x2802}, {"0000000011111" "1", 0x5BFF},
        {"00000000010000" "0", 0x001F}, {"00000000011111" "1", 0x03F0},
        {"000000000010000" "0", 0x0028}, {"000000000011111" "1", 0x07F8},
        {"0000000000010000" "0", 0x0412}, {"0000000000011111" "1", 0x6FFF},
        {"0000000000011110" "0", 0x7001}, {"0000000000010100" "0", 0x1803},
        {"000001" "1111100000000001", 0xF801}, {"000001" "0000000000000000", 0x0000},
        {"000001" "1000000000000000", 0x8000}, {"000001" "0000010000000001", 0x0401},
    };
    size_t i; int j;
    unsigned q = 5;
    for (i = 0; i < sizeof cs / sizeof cs[0]; i++) {
        char buf[1024]; size_t len; uint8_t *f; uint16_t exp[16]; int ne = 0;
        buf[0] = 0;
        strcat(buf, "0000000000"); strcat(buf, cs[i].bits); strcat(buf, "10");
        for (j = 0; j < 5; j++) strcat(buf, "000000000010");
        strcat(buf, "0111111111");
        exp[ne++] = (uint16_t)(q << 10); exp[ne++] = cs[i].w; exp[ne++] = 0xFE00;
        for (j = 0; j < 5; j++) { exp[ne++] = (uint16_t)(q << 10); exp[ne++] = 0xFE00; }
        if (ver == 3) {
            /* v3 variant: Cr DC code "00" (=+0) then same AC; others luma "100"/chroma "00" */
            buf[0] = 0;
            strcat(buf, "00"); strcat(buf, cs[i].bits); strcat(buf, "10");
            strcat(buf, "0010"); /* Cb zero */
            for (j = 0; j < 4; j++) strcat(buf, "10010");
            strcat(buf, "1111111111");
        }
        f = frame_from_str(buf, ver, q, &len);
        expect_ok("hand AC", f, len, exp, (size_t)ne);
        free(f);
    }
}

static void test_hand(void)
{
    size_t len; uint8_t *f; unsigned v;
    cat = C_HAND;
    /* endianness pin: empty v2 frame, bitstream 0111111111 -> halfword 7FC0 -> bytes C0 7F */
    f = frame_from_str("0111111111", 2, 0, &len);
    CHECK(len == 10 && f[8] == 0xC0 && f[9] == 0x7F, "endianness pin %02X %02X", f[8], f[9]);
    expect_ok("empty v2", f, len, NULL, 0); free(f);
    f = frame_from_str("0111111111", 1, 0, &len); expect_ok("empty v1", f, len, NULL, 0); free(f);
    f = frame_from_str("1111111111", 3, 0, &len); expect_ok("empty v3", f, len, NULL, 0); free(f);
    /* bits consumed through the end-of-frame code (guest cursor = 8 + 2*ceil((bits+7)/16)) */
    {
        uint16_t o[8]; size_t l, eb; int rc;
        f = frame_from_str("0111111111", 2, 0, &len);
        rc = pe_vlc_decode_frame_mdec_ex(f, len, o, 8, &l, &eb);
        CHECK(rc == 0 && eb == 10 && l == 2, "end bits v2 empty: rc=%d eb=%zu", rc, eb); free(f);
        f = frame_from_str("1111111111", 3, 0, &len);
        rc = pe_vlc_decode_frame_mdec_ex(f, len, o, 8, &l, &eb);
        CHECK(rc == 0 && eb == 10, "end bits v3 empty: eb=%zu", eb); free(f);
        f = frame_from_str("0000000000" "10" "0111111111", 2, 0, &len); /* EOF mid-macroblock after 12 bits */
        rc = pe_vlc_decode_frame_mdec_ex(f, len, o, 8, &l, &eb);
        CHECK(rc == 0 && eb == 22 && l == 4, "end bits mid-mb: eb=%zu l=%zu", eb, l); free(f);
    }
    /* trailing zero padding accepted */
    {
        uint8_t g[64]; f = frame_from_str("0111111111", 2, 0, &len);
        memset(g, 0, sizeof g); memcpy(g, f, len);
        expect_ok("padded", g, 40, NULL, 0); free(f);
    }
    /* DC variants, v2: Cr=-1, Cb=-512, Y1=+510, Y2=+511?? (0x1FF forbidden) use 0x1FE, Y3=0x200, Y4=1 */
    {
        const char *bits = "1111111111" "10" "1000000000" "10" "0111111110" "10" "0111111101" "10"
                           "0000000001" "10" "1000000001" "10" "0111111111";
        static const uint16_t exp[] = {0x17FF, 0xFE00, 0x1600, 0xFE00, 0x15FE, 0xFE00, 0x15FD, 0xFE00,
                                       0x1401, 0xFE00, 0x1601, 0xFE00};
        f = frame_from_str(bits, 2, 5, &len); expect_ok("v2 DC", f, len, exp, 12); free(f);
        f = frame_from_str(bits, 1, 5, &len); expect_ok("v1 DC", f, len, exp, 12); free(f);
    }
    /* v1 nonsense escape: run 3 level 0 passes through */
    {
        const char *bits = "0000000000" "000001" "000011" "0000000000" "110" "10" /* Cr */
                           "0000000000" "10" "0000000000" "10" "0000000000" "10" "0000000000" "10"
                           "0000000000" "10" "0111111111";
        static const uint16_t exp[] = {0x0000, 0x0C00, 0x0001, 0xFE00, 0, 0xFE00, 0, 0xFE00, 0, 0xFE00,
                                       0, 0xFE00, 0, 0xFE00};
        f = frame_from_str(bits, 1, 0, &len); expect_ok("v1 nonsense escape", f, len, exp, 14); free(f);
    }
    /* raw FE00 escape ends the block (no extra EOB emitted) */
    {
        /* vectors: a raw FE00 escape is plain data; only the 10 code ends a block */
        const char *bits = "0000000000" "000001" "1111111000000000" "10" /* block 0 */
                           "0000000000" "10" "0000000000" "10" "0000000000" "10" "0000000000" "10"
                           "0000000000" "10" "0111111111";
        static const uint16_t exp[] = {0, 0xFE00, 0xFE00, 0, 0xFE00, 0, 0xFE00, 0, 0xFE00, 0, 0xFE00, 0, 0xFE00};
        f = frame_from_str(bits, 2, 0, &len); expect_ok("raw FE00 escape", f, len, exp, 13); free(f);
    }
    /* maximum run: escape run 62 level 1 reaches k=63 */
    for (v = 1; v <= 3; v++) hand_ac_cases(v);
    /* all 63 AC words in a block (no EOB needed? bitstream still carries EOB): 63 x "110" */
    {
        char buf[2048]; uint16_t exp[80]; int i, ne = 0, j;
        strcpy(buf, "0000000000");
        exp[ne++] = 0;
        for (i = 0; i < 63; i++) { strcat(buf, "110"); exp[ne++] = 1; }
        strcat(buf, "10"); exp[ne++] = 0xFE00;
        for (j = 0; j < 5; j++) { strcat(buf, "000000000010"); exp[ne++] = 0; exp[ne++] = 0xFE00; }
        strcat(buf, "0111111111");
        f = frame_from_str(buf, 2, 0, &len); expect_ok("63 AC words", f, len, exp, (size_t)ne); free(f);
    }
    /* v3 hand frames A and B */
    {
        const char *A = "011" "10" "010" "10" "100" "10" "001" "10" "000" "10" "0111" "10"
                        "1000" "10" "1011" "10" "101011" "10" "100" "10" "100" "10" "100" "10"
                        "1111111111";
        static const uint16_t eA[] = {0xA804, 0xFE00, 0xABFC, 0xFE00, 0xA800, 0xFE00, 0xA804, 0xFE00,
                                      0xA800, 0xFE00, 0xA80C, 0xFE00, 0xABF8, 0xFE00, 0xA808, 0xFE00,
                                      0xABFC, 0xFE00, 0xABFC, 0xFE00, 0xABFC, 0xFE00, 0xABFC, 0xFE00};
        const char *B = "11111110" "1" "1111111" "10" "11111110" "0" "0000000" "10"
                        "1111110" "1" "0000000" "10" "1111110" "0" "1111111" "10" "100" "10" "100" "10"
                        "1111111111";
        static const uint16_t eB[] = {0x07FC, 0xFE00, 0x0404, 0xFE00, 0x0600, 0xFE00, 0x0400, 0xFE00,
                                      0x0400, 0xFE00, 0x0400, 0xFE00};
        cat = C_V3;
        f = frame_from_str(A, 3, 42, &len); expect_ok("v3 A", f, len, eA, 24); free(f);
        f = frame_from_str(B, 3, 1, &len); expect_ok("v3 B (wrap)", f, len, eB, 12); free(f);
    }
    cat = C_HAND;
}

static void test_errors(void)
{
    size_t len; uint8_t *f; int i, j;
    char buf[4096];
    uint16_t o[16]; size_t rl;
    cat = C_ERR;
    f = frame_from_str("0111111111", 2, 0, &len);
    for (i = 0; i < 8; i++) expect_rc("short header", f, (size_t)i, PE_VLC_ERR_HEADER);
    CHECK(pe_vlc_decode_frame(NULL, 10, o, 16, &rl) == PE_VLC_ERR_ARG, "null frame");
    CHECK(pe_vlc_decode_frame(f, len, NULL, 4, &rl) == PE_VLC_ERR_ARG, "null out");
    CHECK(pe_vlc_decode_frame(f, len, NULL, 0, &rl) == 0 && rl == 0, "null out cap0");
    CHECK(pe_vlc_decode_frame(f, len, o, 16, NULL) == 0, "null rl_len");
    { /* vectors: the id word is ignored; quant is masked to 6 bits; ver 0..2 = v2, >=3 = v3 */
        uint16_t oa[16], ob[16]; size_t la, lb;
        f[3] = 0x39; expect_rc("id ignored", f, len, 0); f[3] = 0x38;
        f[4] = 0x40; f[5] = 0;
        CHECK(pe_vlc_decode_frame(f, len, oa, 16, &la) == 0 && la == 0, "quant 0x40 decodes");
        f[4] = 0x3F; expect_rc("quant 0x3F ok", f, len, 0);
        f[4] = 0; f[5] = 1; expect_rc("quant 0x100 masked", f, len, 0);
        f[5] = 0;
        free(f);
        { /* one macroblock, DC=3 then EOB: quant masking appears in the DC word */
          const char *bits = "0000000011" "10" "0000000000" "10" "0000000000" "10" "0000000000" "10"
                             "0000000000" "10" "0000000000" "10" "0111111111";
          unsigned qs[] = {0x40u, 0x41u, 0x100u, 0xFFFFu};
          unsigned ex[] = {0u, 1u, 0u, 0x3Fu};
          for (i = 0; i < 4; i++) {
              f = frame_from_str(bits, 2, 0, &len);
              f[4] = (uint8_t)qs[i]; f[5] = (uint8_t)(qs[i] >> 8);
              CHECK(pe_vlc_decode_frame(f, len, oa, 16, &la) == 0 && la == 12 &&
                    oa[0] == (uint16_t)((ex[i] << 10) | 3u), "quant mask %x -> %x got %04X", qs[i], ex[i], oa[0]);
              free(f);
          }
          for (i = 0; i < 3; i++) { /* versions 0,1,2 identical */
              f = frame_from_str(bits, 2, 0, &len);
              la = lb = 0;
              pe_vlc_decode_frame(f, len, oa, 16, &la);
              f[6] = (uint8_t)i;
              CHECK(pe_vlc_decode_frame(f, len, ob, 16, &lb) == 0 && la == lb && !memcmp(oa, ob, la * 2),
                    "version %d != v2", i);
              free(f);
          }
        }
        { /* every version >= 3 decodes as v3 */
          unsigned vs[] = {3, 4, 5, 7, 0x100, 0x8000, 0xFFFF};
          const char *bits = "011" "10" "010" "10" "100" "10" "001" "10" "000" "10" "0111" "10" "1111111111";
          for (i = 0; i < 7; i++) {
              f = frame_from_str(bits, 3, 0, &len);
              f[6] = (uint8_t)vs[i]; f[7] = (uint8_t)(vs[i] >> 8);
              CHECK(pe_vlc_decode_frame(f, len, oa, 16, &la) == 0 && la == 12 && oa[0] == 4 && oa[2] == 0x3FC &&
                    oa[10] == 12, "version %x as v3", vs[i]);
              free(f);
          }
        }
    }
    /* no EOF code at all: header only, and zero bits */
    f = frame_from_str("", 2, 0, &len); expect_rc("no bits", f, len, PE_VLC_ERR_TRUNC); free(f);
    /* unused AC code 000000000000 */
    strcpy(buf, "0000000000" "000000000000");
    f = frame_from_str(buf, 2, 0, &len); expect_rc("unused AC", f, len, PE_VLC_ERR_DESYNC); free(f);
    f = frame_from_str("0000000000" "000000000000", 1, 0, &len); expect_rc("unused AC v1", f, len, PE_VLC_ERR_DESYNC); free(f);
    f = frame_from_str("00" "000000000000", 3, 0, &len); expect_rc("unused AC v3", f, len, PE_VLC_ERR_DESYNC); free(f);
    /* EOF mid-macroblock */
    /* vectors: end-of-frame is recognised at ANY block start */
    { static const uint16_t e1[] = {0, 0xFE00}, e3[] = {0, 0xFE00, 0, 0xFE00, 0, 0xFE00};
      f = frame_from_str("0000000000" "10" "0111111111", 2, 0, &len); expect_ok("EOF at Cb v2", f, len, e1, 2); free(f);
      f = frame_from_str("00" "10" "1111111111", 3, 0, &len); expect_ok("EOF at Cb v3", f, len, e1, 2); free(f);
      strcpy(buf, "0000000000" "10" "0000000000" "10" "0000000000" "10" "0111111111");
      f = frame_from_str(buf, 2, 0, &len); expect_ok("EOF at Y2 v2", f, len, e3, 6); free(f); }
    /* run overflow */
    /* vectors: no run-overflow check; the words are plain data */
    { static const uint16_t e1[] = {0, 0xFC01, 0xFE00}, e2[] = {0, 0xF801, 0x0001, 0xFE00};
      f = frame_from_str("0000000000" "000001" "1111110000000001" "10" "0111111111", 2, 0, &len); /* run 63 */
      expect_ok("run 63", f, len, e1, 3); free(f);
      f = frame_from_str("0000000000" "000001" "1111100000000001" "110" "10" "0111111111", 2, 0, &len);
      expect_ok("k overflow is data", f, len, e2, 4); free(f); }
    /* v3 unused DC codes */
    f = frame_from_str("111111110", 3, 0, &len); expect_rc("v3 chroma unused 8", f, len, PE_VLC_ERR_DESYNC); free(f);
    f = frame_from_str("1111111110", 3, 0, &len); expect_rc("v3 chroma unused 9", f, len, PE_VLC_ERR_DESYNC); free(f);
    f = frame_from_str("00" "10" "00" "10" "11111110", 3, 0, &len); expect_rc("v3 luma unused 7", f, len, PE_VLC_ERR_DESYNC); free(f);
    f = frame_from_str("00" "10" "00" "10" "111111110", 3, 0, &len); expect_rc("v3 luma unused 8", f, len, PE_VLC_ERR_DESYNC); free(f);
    f = frame_from_str("00" "10" "00" "10" "1111111110", 3, 0, &len); expect_rc("v3 luma unused 9", f, len, PE_VLC_ERR_DESYNC); free(f);
    /* AC is truncated in the middle of the longest code */
    f = frame_from_str("0000000000" "10" "0000000000" "0000000001", 2, 0, &len); /* 32 bits */
    expect_rc("truncated long code", f, len, PE_VLC_ERR_TRUNC); free(f);
    f = frame_from_str("0000000000" "000000", 2, 0, &len); /* 16 bits: 6 bits of an unused/long code */
    expect_rc("truncated at 6 zero bits", f, len, PE_VLC_ERR_TRUNC); free(f);
    /* escape cut short */
    f = frame_from_str("0000000000" "000001", 2, 0, &len); /* 16 bits: escape prefix, no payload */
    expect_rc("truncated escape", f, len, PE_VLC_ERR_TRUNC); free(f);
    /* odd frame_bytes: trailing odd byte ignored */
    f = frame_from_str("0111111111", 2, 0, &len);
    { uint8_t g[16]; memcpy(g, f, len); g[len] = 0xAA; expect_ok("odd tail", g, len + 1, NULL, 0); }
    free(f);
    (void)j;
}

static void test_roundtrip(void)
{
    unsigned ver; int seed;
    cat = C_ROUND;
    for (ver = 1; ver <= 3; ver++)
        for (seed = 0; seed < 1500; seed++) {
            wlist exp = {0, 0, 0}; size_t len; uint8_t *f;
            unsigned q = rndn(64);
            int nmb = (int)rndn(seed % 50 == 0 ? 40 : 5);
            rs = 0x9E3779B9u * (uint32_t)(seed + 1) + ver;
            if (!rs) rs = 1;
            q = rndn(64); nmb = (int)rndn(seed % 50 == 0 ? 40 : 5);
            f = gen_frame(ver, q, nmb, &exp, &len);
            expect_ok("roundtrip", f, len, exp.words, exp.n);
            free(f); free(exp.words);
        }
}

static void test_allcodes(void)
{
    int i, d, luma, j; unsigned ver;
    cat = C_ALLCODES;
    for (ver = 1; ver <= 3; ver++)
        for (i = 0; i < n_ac; i++)
            for (j = 0; j < 2; j++) {
                bw w; wlist e = {0, 0, 0}; size_t len; uint8_t *f; int b;
                bw_init(&w);
                for (b = 0; b < 6; b++) {
                    if (ver == 3) enc_dc3(&w, b >= 2, 0); else bw_str(&w, "0000000000");
                    wl_put(&e, 0x2000);
                    if (b == 3) { bw_val(&w, ac_codes[i].code, ac_codes[i].len); wl_put(&e, ac_codes[i].word);
                                  if (j) { /* second pass: same word as a forced escape */ } }
                    if (b == 3 && j) { /* replace table code by escape */
                        w.n -= ac_codes[i].len; enc_ac(&w, ac_codes[i].word, 1);
                    }
                    bw_str(&w, "10"); wl_put(&e, 0xFE00);
                }
                bw_str(&w, ver == 3 ? "1111111111" : "0111111111");
                f = mk_frame(&w, ver, 8, 0, 0x3800, &len);
                expect_ok("all codes", f, len, e.words, e.n);
                free(f); free(e.words); bw_free(&w);
            }
    /* every v3 offset, both tables */
    cat = C_V3;
    for (luma = 0; luma < 2; luma++)
        for (d = -1020; d <= 1020; d += 4) {
            bw w; wlist e = {0, 0, 0}; size_t len; uint8_t *f; int b;
            int old[3] = {0, 0, 0};
            bw_init(&w);
            for (b = 0; b < 6; b++) {
                int comp = b < 2 ? b : 2, dd = 0, nd;
                if ((luma == 0 && b < 2) || (luma == 1 && b >= 2)) dd = d;
                enc_dc3(&w, b >= 2, dd);
                nd = (old[comp] + dd) & 0x3FF; nd = (nd ^ 0x200) - 0x200; old[comp] = nd;
                wl_put(&e, (uint16_t)((8u << 10) | (unsigned)(nd & 0x3FF)));
                bw_str(&w, "10"); wl_put(&e, 0xFE00);
            }
            bw_str(&w, "1111111111");
            f = mk_frame(&w, 3, 8, 0, 0x3800, &len);
            expect_ok("v3 offsets", f, len, e.words, e.n);
            free(f); free(e.words); bw_free(&w);
        }
}

static void test_bounds(void)
{
    unsigned ver; int seed;
    cat = C_BOUNDS;
    for (ver = 1; ver <= 3; ver++)
        for (seed = 0; seed < 60; seed++) {
            wlist exp = {0, 0, 0}; size_t len, cut, cap, minlen; uint8_t *f, *g;
            rs = 777u + 31u * (uint32_t)seed + ver;
            f = gen_frame(ver, rndn(64), 1 + (int)rndn(3), &exp, &len);
            minlen = 8 + 2 * ((g_stream_bits + 15) / 16);
            /* truncation at every byte length: must fail, never over-read (exact-size copies) */
            for (cut = 0; cut < len; cut++) {
                uint16_t *o; size_t rl; int rc;
                g = (uint8_t *)malloc(cut ? cut : 1);
                if (cut) memcpy(g, f, cut);
                rc = decode_checked(g, cut, exp.n + 8, &o, &rl);
                if (cut < minlen)
                    CHECK(rc < 0, "truncation to %zu of %zu bytes (min %zu) returned %d", cut, len, minlen, rc);
                else
                    CHECK(rc == 0, "cut %zu >= min %zu (only padding removed) returned %d", cut, minlen, rc);
                free(o); free(g);
            }
            /* rl_cap sweep */
            for (cap = 0; cap <= exp.n; cap++) {
                uint16_t *o; size_t rl; int rc = decode_checked(f, len, cap, &o, &rl);
                if (cap < exp.n) {
                    CHECK(rc == PE_VLC_ERR_OVERFLOW, "cap %zu < %zu rc=%d", cap, exp.n, rc);
                    CHECK(rl == cap, "overflow rl_len=%zu cap=%zu", rl, cap);
                } else CHECK(rc == 0 && rl == exp.n, "cap==need rc=%d", rc);
                free(o);
            }
            free(f); free(exp.words);
        }
}

static void test_mdec(void)
{
    unsigned ver;
    cat = C_MDEC;
    for (ver = 1; ver <= 3; ver++) {
        int seed;
        for (seed = 0; seed < 100; seed++) {
            wlist exp = {0, 0, 0}; size_t len, i, n32; uint8_t *f; uint16_t *o; size_t rl; int rc;
            rs = 4242u + (uint32_t)seed * 17u + ver;
            f = gen_frame(ver, rndn(64), (int)rndn(4), &exp, &len);
            n32 = (exp.n + 1) / 2;                 /* words */
            n32 = (n32 + 0x1F) / 0x20 * 0x20;       /* rounded to 0x80 bytes */
            f[0] = (uint8_t)n32; f[1] = (uint8_t)(n32 >> 8);
            o = (uint16_t *)malloc((n32 * 2 + 8) * 2);
            rc = pe_vlc_decode_frame_mdec(f, len, o, n32 * 2 + 2, &rl);
            CHECK(rc == 0 && rl == n32 * 2 + 2, "mdec rc=%d rl=%zu want %zu", rc, rl, n32 * 2 + 2);
            if (rc == 0) {
                CHECK(o[0] == n32 && o[1] == 0x3800, "mdec header %04X %04X", o[0], o[1]);
                for (i = 0; i < exp.n; i++) CHECK(o[2 + i] == exp.words[i], "mdec data %zu", i);
                for (; i < n32 * 2; i++) CHECK(o[2 + i] == 0xFE00, "mdec pad %zu", i);
            }
            { /* vectors: the count is not a bound; longer streams decode in full */
                f[0] = 0; f[1] = 0;
                rc = pe_vlc_decode_frame_mdec(f, len, o, n32 * 2 + 2, &rl);
                CHECK(rc == 0 && rl == exp.n + 2, "mdec count 0: rc=%d rl=%zu want %zu", rc, rl, exp.n + 2);
                CHECK(o[0] == 0, "mdec count 0 header");
            }
            free(o); free(f); free(exp.words);
        }
    }
}

/* resumable API: chains of bounded steps concatenate to the complete-frame output; state errors; cap safety */
static void test_resume(void)
{
    static const uint32_t bounds[] = {0, 1, 2, 3, 4, 9, 50, 0x00FFFFFFu};
    unsigned ver; int seed;
    cat = C_BOUNDS;
    for (ver = 1; ver <= 3; ver++)
        for (seed = 0; seed < 60; seed++) {
            wlist exp = {0, 0, 0}; size_t len, bi; uint8_t *f; uint16_t *full; size_t fl; int rc;
            rs = 777u + (uint32_t)seed * 31u + ver;
            f = gen_frame(ver, rndn(64), 1 + (int)rndn(6), &exp, &len);
            full = (uint16_t *)malloc((exp.n + 8) * 2);
            rc = pe_vlc_decode_frame_mdec(f, len, full, exp.n + 8, &fl);
            CHECK(rc == 0, "resume ref rc=%d", rc);
            for (bi = 0; bi < sizeof bounds / sizeof bounds[0]; bi++) {
                pe_vlc_decoder_t st; uint16_t *acc = (uint16_t *)malloc((exp.n + 16) * 2);
                uint16_t chunk[8]; size_t total = 0, w = 0, bits = 0, steps = 0, endbits_full = 0;
                int r;
                CHECK(pe_vlc_decode_begin(&st, f, len) == 0, "begin");
                do {
                    uint16_t *tmp = (uint16_t *)malloc((exp.n + 16) * 2);
                    r = pe_vlc_decode_step(&st, tmp, exp.n + 16, bounds[bi], &w, &bits);
                    CHECK(r == 0 || r == 1, "step rc %d", r);
                    memcpy(acc + total, tmp, w * 2); total += w; steps++;
                    free(tmp);
                    if (r < 0) break;
                } while (r == 1 && steps < 100000);
                CHECK(r == 0 && 2 + total == fl, "chain len %zu+2 vs %zu (bound %u)", total, fl, (unsigned)bounds[bi]);
                CHECK(memcmp(acc, full + 2, total * 2 < (fl - 2) * 2 ? total * 2 : (fl - 2) * 2) == 0, "chain words differ (bound %u)", (unsigned)bounds[bi]);
                if (bounds[bi] == 0x00FFFFFFu) CHECK(steps == 1, "default bound bounded the frame");
                if (bounds[bi] == 0) CHECK(steps > 1 || exp.n <= 3, "bound 0 did not split");
                (void)pe_vlc_decode_frame_mdec_ex(f, len, full, exp.n + 8, &fl, &endbits_full);
                CHECK(bits == endbits_full, "final end_bits %zu vs single-call %zu", bits, endbits_full);
                /* nothing to resume after completion, after an error and before begin */
                CHECK(pe_vlc_decode_step(&st, chunk, 8, 5, &w, &bits) == PE_VLC_ERR_STATE, "step after done");
                memset(&st, 0, sizeof st);
                CHECK(pe_vlc_decode_step(&st, chunk, 8, 5, &w, &bits) == PE_VLC_ERR_STATE, "step before begin");
                free(acc);
            }
            { /* cap too small: error, no write past the cap, decoder dead */
                pe_vlc_decoder_t st; uint16_t buf[8]; size_t w, bits; int r;
                memset(buf + 3, 0xAB, 5 * 2);
                pe_vlc_decode_begin(&st, f, len);
                r = pe_vlc_decode_step(&st, buf, 3, 0x00FFFFFFu, &w, &bits);
                if (exp.n > 3) {
                    CHECK(r == PE_VLC_ERR_OVERFLOW && w == 3 && buf[3] == 0xABAB, "step cap overflow r=%d w=%zu", r, w);
                    CHECK(pe_vlc_decode_step(&st, buf, 8, 0x00FFFFFFu, &w, &bits) == PE_VLC_ERR_STATE, "step after overflow");
                }
            }
            free(full); free(f); free(exp.words);
        }
}

int main(void)
{
    int i;
    build_ac();
    test_tables();
    test_hand();
    test_errors();
    test_roundtrip();
    test_allcodes();
    test_bounds();
    test_mdec();
    test_resume();
    printf("pe_vlc_decode tests (PE_VLC_MUTANT=%d): %lu checks, %lu failed\n", MUTVAL,
           n_checks, n_fail);
    for (i = 0; i < N_CATS; i++)
        printf("  %-20s failed %lu\n", cat_name[i], cat_fail[i]);
    printf(n_fail ? "RESULT: FAIL\n" : "RESULT: PASS\n");
    return n_fail ? 1 : 0;
}

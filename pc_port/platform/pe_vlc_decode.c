/*
 * pe_vlc_decode.c - BS (STR video frame) bitstream -> MDEC run-length words.
 *
 * Provenance: written clean-room from the psx-spx documentation and black-box
 * vectors of the original code, with no SDK or third-party decoder source.
 *   psx-spx "CDROM File Formats" -> CDROM File Video BS Compression
 *     (Headers, DC Values, AC Values) ; psx-spx "Macroblock Decoder (MDEC)"
 *     -> MDEC Decompression / MDEC Data Format.
 * See docs/evidence/native-vlc-20261125/REPORT.md for every judgment call.
 *
 * The resumable pe_vlc_decode_begin()/pe_vlc_decode_step() API follows the
 * observed bounded/resume behaviour of the original (vectors only).
 *
 * PE_VLC_MUTANT (compile-time, tests only): deliberately corrupts one table
 * entry / rule so the test program can prove it detects table errors.  It is
 * 0 (off) in every real build.
 */
#include "pe_vlc_decode.h"

#include <string.h>

#ifndef PE_VLC_MUTANT
#define PE_VLC_MUTANT 0
#endif
/* bounded-exit test on n = halfwords this call has advanced the output pointer by
 * (the first call counts its two MDEC header halfwords): n > bound */
#define PE_VLC_BOUND_HIT(n, bound) \
    ((PE_VLC_MUTANT) == 14 ? ((uint64_t)(n) >= (uint64_t)(bound)) : ((uint64_t)(n) > (uint64_t)(bound)))
#define MUT(n, good, bad) ((PE_VLC_MUTANT) == (n) ? (bad) : (good))

/* ------------------------------------------------------------------ */
/* Bit reader: MSB-first inside little-endian 16-bit halfwords.        */
/* ------------------------------------------------------------------ */
typedef struct {
    const uint8_t *p; /* first halfword of the bitstream */
    size_t total;     /* number of valid bits            */
    size_t pos;       /* current bit position            */
} bitrd;

static uint64_t hw_at(const bitrd *b, size_t idx)
{
    if (idx * 16 >= b->total)
        return 0;
    return (uint64_t)b->p[idx * 2] | ((uint64_t)b->p[idx * 2 + 1] << 8);
}

/* Peek n (1..32) bits; bits beyond the end read as 0 (callers verify the
 * real consumed length with take()). */
static uint32_t peek(const bitrd *b, unsigned n)
{
    size_t idx = b->pos >> 4;
    unsigned off = (unsigned)(b->pos & 15);
    uint64_t w = (hw_at(b, idx) << 32) | (hw_at(b, idx + 1) << 16) |
                 hw_at(b, idx + 2);
    return (uint32_t)((w >> (48 - off - n)) & ((1ull << n) - 1));
}

static int take(bitrd *b, unsigned n)
{
    if (b->pos + n > b->total)
        return PE_VLC_ERR_TRUNC;
    b->pos += n;
    return 0;
}

/* ------------------------------------------------------------------ */
/* AC table (psx-spx "Huffman codes for AC values BS v1/v2/v3/iki").   */
/* Each row: prefix, nidx index bits, then a sign bit s (1 = negative). */
/* ------------------------------------------------------------------ */
typedef struct {
    uint8_t plen;
    uint8_t nidx;
    uint16_t pval;
    const uint16_t *vals;
} ac_row;

static const uint16_t r11[] = {0x0001};                       /* 11s        */
static const uint16_t r011[] = {0x0401};                      /* 011s       */
static const uint16_t r010[] = {0x0002, 0x0801};              /* 010xs      */
static const uint16_t r0011[] = {0x1001, 0x0C01};             /* 0011xs     */
static const uint16_t r00101[] = {0x0003};                    /* 00101s     */
static const uint16_t r00100[] = {0x3401, 0x0006, 0x3001, MUT(1, 0x2C01, 0x2C03),
                                  0x0C02, 0x0403, 0x0005, 0x2801}; /* 00100xxxs */
static const uint16_t r0001[] = {0x1C01, 0x1801, 0x0402, 0x1401};  /* 0001xxs   */
static const uint16_t r00001[] = {0x0802, 0x2401, 0x0004, 0x2001}; /* 00001xxs  */
static const uint16_t r0000001[] = {0x4001, 0x1402, 0x0007, 0x0803,
                                    0x0404, 0x3C01, 0x3801, 0x1002}; /* 0000001xxxs */
static const uint16_t r00000001[] = {
    0x000B, 0x2002, 0x1003, 0x000A, 0x0804, 0x1C02, 0x5401, 0x5001,
    0x0009, 0x4C01, 0x4801, 0x0405, 0x0C03, 0x0008, 0x1802, 0x4401};
static const uint16_t r000000001[] = {
    0x2802, 0x2402, 0x1403, 0x0C04, 0x0805, 0x0407, 0x0406, 0x000F,
    0x000E, 0x000D, 0x000C, 0x6801, 0x6401, 0x6001, 0x5C01, 0x5801};
static const uint16_t r0000000001[] = {
    0x001F, 0x001E, 0x001D, 0x001C, 0x001B, 0x001A, 0x0019, 0x0018,
    0x0017, 0x0016, 0x0015, 0x0014, 0x0013, 0x0012, 0x0011, 0x0010};
static const uint16_t r00000000001[] = {
    0x0028, 0x0027, 0x0026, 0x0025, 0x0024, 0x0023, 0x0022, 0x0021,
    0x0020, 0x040E, 0x040D, 0x040C, 0x040B, 0x040A, 0x0409, 0x0408};
static const uint16_t r000000000001[] = {
    0x0412, 0x0411, 0x0410, 0x040F, 0x1803, 0x4002, 0x3C02, 0x3802,
    0x3402, 0x3002, 0x2C02, 0x7C01, 0x7801, 0x7401, 0x7001, MUT(2, 0x6C01, 0x6C02)};

static const ac_row ac_rows[] = {
    {2, 0, 0x3, r11},           {3, 0, 0x3, r011},
    {3, 1, 0x2, r010},          {4, 1, 0x3, r0011},
    {5, 0, 0x5, r00101},        {5, 3, 0x4, r00100},
    {4, 2, 0x1, r0001},         {5, 2, 0x1, r00001},
    {7, 3, 0x1, r0000001},      {8, 4, 0x1, r00000001},
    {9, 4, 0x1, r000000001},    {10, 4, 0x1, r0000000001},
    {11, 4, 0x1, r00000000001}, {12, 4, 0x1, r000000000001},
};
#define N_AC_ROWS (sizeof ac_rows / sizeof ac_rows[0])

#define MDEC_EOB 0xFE00u

/* Returns 0 ok (*word set, *eob=0 or 1), negative error. */
static int ac_decode(bitrd *b, uint16_t *word, int *eob)
{
    uint32_t w = peek(b, 17);
    size_t i;
    int rc;

    *eob = 0;
    if ((w >> 15) == 2u) { /* 10 : end of block */
        *eob = 1;
        *word = MDEC_EOB;
        return take(b, 2);
    }
    if ((w >> 11) == 1u) { /* 000001 + 16 raw bits (escape) */
        unsigned width = MUT(5, 16, 15);
        rc = take(b, 6);
        if (rc)
            return rc;
        if (b->pos + width > b->total)
            return PE_VLC_ERR_TRUNC;
        *word = (uint16_t)peek(b, width);
        b->pos += width;
        return 0;
    }
    if ((w >> 5) == 0u) { /* 000000000000 : unused (12 real bits needed) */
        if (b->pos + 12 > b->total)
            return PE_VLC_ERR_TRUNC;
        return PE_VLC_ERR_DESYNC;
    }
    for (i = 0; i < N_AC_ROWS; i++) {
        const ac_row *r = &ac_rows[i];
        if ((w >> (17 - r->plen)) == r->pval) {
            unsigned len = r->plen + r->nidx + 1u;
            uint32_t is = (w >> (17 - len)) & ((2u << r->nidx) - 1u);
            unsigned s, idx;
            uint16_t v;
            if (PE_VLC_MUTANT == 3 && r->nidx > 0) { /* sign first, index last */
                s = is >> r->nidx;
                idx = is & ((1u << r->nidx) - 1u);
            } else {
                s = is & 1u;
                idx = is >> 1;
            }
            rc = take(b, len);
            if (rc)
                return rc;
            v = r->vals[idx];
            if (s)
                v = (uint16_t)((v & 0xFC00u) | ((0u - (v & 0x3FFu)) & 0x3FFu));
            *word = v;
            return 0;
        }
    }
    return PE_VLC_ERR_DESYNC; /* unreachable: table is complete */
}

/* ------------------------------------------------------------------ */
/* DC decoding.                                                        */
/* ------------------------------------------------------------------ */
#define DC_EOF 1 /* positive return: end-of-frame code */

/* v3 offset magnitude from nbits + n + sign: pos = base+n,
 * neg = 2*base-1-n  with base = 1<<nbits (psx-spx "-(03h..02h)" lists the
 * negative magnitudes in descending order, positive in ascending). */
static int v3_offset(unsigned nbits, unsigned s, unsigned n)
{
    int base = 1 << nbits;
    int mag;
    if (!s)
        mag = MUT(4, 2 * base - 1 - (int)n, base + (int)n);
    else
        mag = base + (int)n;
    /* psx-spx: s=0 negative, s=1 positive */
    return s ? mag * MUT(10, 4, 2) : -mag * MUT(10, 4, 2);
}

/* chroma table: 00 / 01s / 10sn / 110snn / ... / 11111110snnnnnnn */
static int dc3_chroma(bitrd *b, int *off)
{
    uint32_t w = peek(b, 17);
    unsigned j = 0, nb, s, n, len;
    int rc;
    if ((w >> 7) == 0x3FFu) { /* 1111111111 */
        return DC_EOF;
    }
    if ((w >> 15) == 0u) { /* 00 */
        *off = 0;
        return take(b, 2);
    }
    if ((w >> 15) == 1u) { /* 01s */
        s = (w >> 14) & 1u;
        *off = v3_offset(0, s, 0);
        return take(b, 3);
    }
    while (j < 10 && ((w >> (16 - j)) & 1u))
        j++;
    if (j > 7)
        return PE_VLC_ERR_DESYNC; /* 111111110, 1111111110 unused */
    nb = j;
    len = 2 * j + 2;
    s = (w >> (16 - j - 1)) & 1u;
    n = (w >> (17 - len)) & ((1u << nb) - 1u);
    *off = v3_offset(nb, s, n);
    rc = take(b, len);
    return rc;
}

/* luma table: 100 / 00s / 01sn / 101snn / 110snnn / 1110snnnn / ... */
static int dc3_luma(bitrd *b, int *off)
{
    uint32_t w = peek(b, 17);
    unsigned nb, s, n, len, j, plen;
    if ((w >> 7) == 0x3FFu)
        return DC_EOF;
    if ((w >> 14) == 4u) { /* 100 */
        *off = 0;
        return take(b, 3);
    }
    if ((w >> 15) == 0u) { /* 00s */
        s = (w >> 14) & 1u;
        *off = v3_offset(0, s, 0);
        return take(b, 3);
    }
    if ((w >> 15) == 1u) { /* 01sn */
        s = (w >> 14) & 1u;
        n = (w >> 13) & 1u;
        *off = v3_offset(1, s, n);
        return take(b, 4);
    }
    j = 0;
    while (j < 10 && ((w >> (16 - j)) & 1u))
        j++;
    if (j == 1) { /* 101snn: 3-bit prefix */
        nb = 2;
        plen = 3;
    } else if (j == 2) { /* 110snnn */
        nb = 3;
        plen = 3;
    } else if (j <= 6) { /* 1110snnnn .. 1111110snnnnnnn: j ones + 0 */
        nb = j + 1;
        plen = j + 1;
    } else {
        return PE_VLC_ERR_DESYNC; /* 11111110, 111111110, 1111111110 */
    }
    len = plen + 1 + nb;
    s = (w >> (17 - plen - 1)) & 1u;
    n = (w >> (17 - len)) & ((1u << nb) - 1u);
    *off = v3_offset(nb, s, n);
    return take(b, len);
}

/* ------------------------------------------------------------------ */
/* Frame decode.                                                       */
/* ------------------------------------------------------------------ */
typedef struct {
    uint16_t *out;
    size_t cap;
    size_t n;
} sink;

static int put(sink *o, uint16_t v)
{
    if (o->n >= o->cap)
        return PE_VLC_ERR_OVERFLOW;
    o->out[o->n++] = v;
    return 0;
}

/* One decode run over the resumable state.  Returns 0 (frame complete), 1
 * (bounded exit, state saved) or a negative error.  A bounded exit is taken
 * right after a block's DC word is written (vector-derived, see REPORT.md,
 * "resumable API"): when the halfwords THIS call has advanced the output
 * pointer by (DC included; the first call's two MDEC header halfwords count)
 * exceed the bound, i.e. before any of the block's AC words and its EOB word. */
static int vlc_run(pe_vlc_decoder_t *st, bitrd *b, sink *o, uint32_t bound, size_t hdr, size_t *end_bits)
{
    int rc;

    for (;;) { /* one macroblock per iteration: Cr, Cb, Y1..Y4 */
        while (st->blk < 6) {
            unsigned blk = st->blk;

            if (!st->in_ac) {
                uint16_t dcw;

                if (st->version == 3) {
                    int off, *old;
                    if (PE_VLC_MUTANT == 8)
                        old = &st->old_cr;
                    else
                        old = (blk == 0) ? &st->old_cr : (blk == 1) ? &st->old_cb : &st->old_y;
                    if (PE_VLC_MUTANT == 6)
                        rc = (blk < 2) ? dc3_luma(b, &off) : dc3_chroma(b, &off);
                    else
                        rc = (blk < 2) ? dc3_chroma(b, &off) : dc3_luma(b, &off);
                    if (rc == DC_EOF) { /* end of frame at ANY block start */
                        if (PE_VLC_MUTANT == 11 && blk != 0)
                            return PE_VLC_ERR_DESYNC;
                        if (end_bits)
                            *end_bits = b->pos + 10;
                        return 0;
                    }
                    if (rc)
                        return rc;
                    {
                        int dc = *old + off;
                        /* wrap to signed 10 bit (the "newer v3 decoder" rule) */
                        dc = (dc & 0x3FF);
                        dc = (dc ^ 0x200) - 0x200;
                        *old = dc;
                        dcw = (uint16_t)((st->quant << 10) | ((unsigned)dc & 0x3FFu));
                    }
                } else {
                    uint32_t raw;
                    if (b->pos + 10 > b->total)
                        return PE_VLC_ERR_TRUNC;
                    raw = peek(b, 10);
                    b->pos += 10;
                    if (raw == 0x1FFu) { /* 0111111111: end of frame at ANY block start */
                        if (PE_VLC_MUTANT == 11 && blk != 0)
                            return PE_VLC_ERR_DESYNC;
                        if (end_bits)
                            *end_bits = b->pos;
                        return 0;
                    }
                    if (PE_VLC_MUTANT == 7) /* arithmetic add: negative DC borrows */
                        dcw = (uint16_t)((st->quant << 10) + (int)((raw ^ 0x200u) - 0x200u));
                    else
                        dcw = (uint16_t)((st->quant << 10) | raw);
                }
                if ((rc = put(o, dcw)) != 0)
                    return rc;
                st->in_ac = 1;
                if (PE_VLC_MUTANT != 13 && PE_VLC_BOUND_HIT(o->n + hdr, bound)) {
                    if (PE_VLC_MUTANT == 15) /* predictors lost across the exit */
                        st->old_cr = st->old_cb = st->old_y = 0;
                    if (end_bits) /* same convention as the end-of-frame case: next code position + 10 */
                        *end_bits = b->pos + (PE_VLC_MUTANT == 16 ? 0 : 10);
                    return 1;
                }
            }

            for (;;) { /* AC words until EOB */
                uint16_t w;
                int eob;
                if ((rc = ac_decode(b, &w, &eob)) != 0)
                    return rc;
                if ((rc = put(o, w)) != 0)
                    return rc;
                if (eob || (PE_VLC_MUTANT == 12 && w == MDEC_EOB)) /* only the 10 code ends a block; a raw FE00 escape is data */
                    break;
            }
            st->in_ac = 0;
            st->blk++;
            if (PE_VLC_MUTANT == 13 && PE_VLC_BOUND_HIT(o->n + hdr, bound)) { /* mutant: checkpoint after the EOB */
                if (end_bits)
                    *end_bits = b->pos + 10;
                return 1;
            }
        }
        st->blk = 0;
    }
}

int pe_vlc_decode_begin(pe_vlc_decoder_t *st, const uint8_t *frame, size_t frame_bytes)
{
    unsigned version;
    if (!st)
        return PE_VLC_ERR_ARG;
    memset(st, 0, sizeof *st);
    st->state = PE_VLC_ST_FAILED;
    if (!frame)
        return PE_VLC_ERR_ARG;
    if (frame_bytes < 8)
        return PE_VLC_ERR_HEADER;
    /* Vector-derived (round 2): the id word is ignored, quant is masked to 6
     * bits, versions 0..2 decode as v2 and every version >= 3 as v3. */
    st->frame = frame;
    st->frame_bytes = frame_bytes;
    st->quant = (unsigned)(frame[4] | (frame[5] << 8)) & 0x3Fu;
    version = (unsigned)(frame[6] | (frame[7] << 8));
    st->version = version >= 3 ? 3u : 2u;
    st->total_bits = ((frame_bytes - 8) / 2) * 16; /* a trailing odd byte is ignored */
    st->state = PE_VLC_ST_RUN;
    return 0;
}

static int decode_core(const uint8_t *frame, size_t frame_bytes, sink *o, size_t *end_bits)
{
    pe_vlc_decoder_t st;
    bitrd b;
    int rc;

    if (end_bits)
        *end_bits = 0;
    if ((rc = pe_vlc_decode_begin(&st, frame, frame_bytes)) != 0)
        return rc;
    b.p = frame + 8;
    b.total = st.total_bits;
    b.pos = 0;
    return vlc_run(&st, &b, o, 0xFFFFFFFFu, 0, end_bits); /* never bounded: returns 0 or < 0 */
}

int pe_vlc_decode_step(pe_vlc_decoder_t *st, uint16_t *rl_out, size_t rl_cap,
                       uint32_t bound_halfwords, size_t *halfwords_written, size_t *end_bits)
{
    sink o;
    bitrd b;
    int rc;

    if (halfwords_written)
        *halfwords_written = 0;
    if (end_bits)
        *end_bits = 0;
    if (!st)
        return PE_VLC_ERR_ARG;
    if (rl_cap > 0 && !rl_out)
        return PE_VLC_ERR_ARG;
    /* A call with no frame to start (never begun, already complete, failed
     * earlier, or the previous call did not exit at a bound) has no saved
     * position to continue from; the original never returns from it. */
    if (st->state != PE_VLC_ST_RUN && st->state != PE_VLC_ST_PAUSED)
        return PE_VLC_ERR_STATE;
    o.out = rl_out;
    o.cap = rl_cap;
    o.n = 0;
    b.p = st->frame + 8;
    b.total = st->total_bits;
    b.pos = st->bit_pos;
    rc = vlc_run(st, &b, &o, bound_halfwords, st->calls == 0 ? 2u : 0u, end_bits);
    st->calls++;
    st->bit_pos = b.pos;
    if (rc == 0) {
        /* complete frame: pad with FE00 up to 2 + 2*count halfwords in total
         * (the two header halfwords belong to the first call; the end word
         * and the guest registers are the adapter's job) */
        size_t want = 2u + 2u * (size_t)(st->frame[0] | (st->frame[1] << 8));
        while (rc == 0 && 2u + st->out_halfwords + o.n < want)
            rc = put(&o, (uint16_t)MDEC_EOB);
    }
    st->out_halfwords += o.n;
    if (halfwords_written)
        *halfwords_written = o.n;
    st->state = rc == 1 ? PE_VLC_ST_PAUSED : rc == 0 ? PE_VLC_ST_DONE : PE_VLC_ST_FAILED;
    if (rc < 0 && end_bits)
        *end_bits = 0;
    return rc;
}

int pe_vlc_decode_frame(const uint8_t *frame, size_t frame_bytes,
                        uint16_t *rl_out, size_t rl_cap, size_t *rl_len)
{
    sink o;
    int rc;
    if (rl_len)
        *rl_len = 0;
    if (rl_cap > 0 && !rl_out)
        return PE_VLC_ERR_ARG;
    o.out = rl_out;
    o.cap = rl_cap;
    o.n = 0;
    rc = decode_core(frame, frame_bytes, &o, NULL);
    if (rl_len)
        *rl_len = o.n;
    return rc;
}

int pe_vlc_decode_frame_mdec_ex(const uint8_t *frame, size_t frame_bytes,
                                uint16_t *rl_out, size_t rl_cap, size_t *rl_len,
                                size_t *stream_end_bits)
{
    sink o;
    int rc;
    size_t want;
    if (rl_len)
        *rl_len = 0;
    if (stream_end_bits)
        *stream_end_bits = 0;
    if (rl_cap > 0 && !rl_out)
        return PE_VLC_ERR_ARG;
    if (!frame)
        return PE_VLC_ERR_ARG;
    if (frame_bytes < 8)
        return PE_VLC_ERR_HEADER;
    o.out = rl_out;
    o.cap = rl_cap;
    o.n = 0;
    if ((rc = put(&o, (uint16_t)(frame[0] | (frame[1] << 8)))) != 0 ||
        (rc = put(&o, (uint16_t)(frame[2] | (frame[3] << 8)))) != 0)
        goto done;
    rc = decode_core(frame, frame_bytes, &o, stream_end_bits);
    if (rc)
        goto done;
    /* header count is not a bound: longer streams decode in full, shorter
     * output is padded with FE00 up to 2 + 2*count halfwords */
    want = 2u + 2u * (size_t)(frame[0] | (frame[1] << 8));
    while (o.n < want) {
        if ((rc = put(&o, (uint16_t)MDEC_EOB)) != 0)
            goto done;
    }
done:
    if (rl_len)
        *rl_len = o.n;
    return rc;
}

int pe_vlc_decode_frame_mdec(const uint8_t *frame, size_t frame_bytes,
                             uint16_t *rl_out, size_t rl_cap, size_t *rl_len)
{
    return pe_vlc_decode_frame_mdec_ex(frame, frame_bytes, rl_out, rl_cap, rl_len, NULL);
}

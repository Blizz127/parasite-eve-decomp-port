/*
 * Native SPU synthesizer — see pe_spu.h for the contract.
 *
 * Hardware reference: psx-spx "Sound Processing Unit (SPU)"
 * (https://psx-spx.consoledev.net/soundprocessingunitspu/): ADPCM filter
 * table, flag semantics, ADSR/sweep rate formula, noise generator and the
 * reverb algorithm are transcribed from that document's pseudocode.
 */
#include "pe_spu.h"
#include "pe_spu_dma.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NVOICE 24
#define CD_PREBUF 5880u          /* 133 ms CD/XA jitter buffer: the movie feed
                                  * delivers 5 XA sectors per 16 VBlanks as
                                  * 4 singles + 1 pair (needs >= 2 sectors =
                                  * 4704 frames of cushion, plus margin) */
#define CD_PLAYOUT_IDLE 4410u    /* no sector for 100 ms: play the rest out */
#define CD_FADE 64

typedef struct {
    /* envelope */
    PeSpuEnvelope env;
    /* ADPCM stream */
    uint32_t cur_addr;       /* byte address of the next block */
    uint32_t loop_addr;      /* byte address */
    int ignore_loop;         /* repeat address written while on */
    int16_t block[28];
    int block_pos;           /* 0..28; 28 = need a new block */
    uint8_t block_flags;
    int32_t hist1, hist2;
    int16_t h[4];            /* oldest, older, old, new (gaussian taps) */
    uint32_t counter;        /* pitch phase, 12-bit fraction */
    int32_t last_out;        /* post-envelope output (for PMON) */
    /* volume sweep state (per channel) */
    int32_t vol_level[2];
    int32_t vol_counter[2];
    int on;
} Voice;

static struct {
    int active;
    uint16_t reg[0x100];
    uint8_t written[0x100];
    Voice v[NVOICE];
    uint32_t endx;
    uint32_t xfer_addr;
    int32_t main_level[2];
    int32_t main_counter[2];
    /* noise */
    int32_t noise_timer;
    int16_t noise_level;
    /* reverb */
    uint32_t rev_base;        /* mBASE, bytes */
    uint32_t rev_cur;         /* current buffer address, bytes */
    int rev_phase;
    int32_t rev_out[2];
    int16_t rev_ih[64][2];    /* 44.1 kHz reverb input history */
    int16_t rev_oh[64][2];    /* zero-stuffed 44.1 kHz reverb output */
    uint32_t rev_hp;
    /* CD audio queue */
    int16_t cd[44100 * 2];
    uint32_t cd_r, cd_w, cd_n;
    int cd_live;              /* 1 playing, 2 starved (diagnostics) */
    int cd_play;              /* jitter buffer primed and draining */
    int32_t cd_fade, cd_tail; /* fade-in / fade-out frames remaining */
    uint32_t cd_idle;         /* output frames since the last push */
    int16_t cd_last[2];
    PeSpuStats stats;
    PeSpuKonHook kon_hook;
} S;

#include "pe_spu_gauss.inc"

/* psx-spx "Reverb Buffer Resampling" 39-tap FIR, transcribed verbatim
 * (sum 0x7FFE: unity for decimation; x2 for the zero-stuffed output). */
static const int16_t k_rev_fir[39] = {
    -0x0001, 0x0000, 0x0002, 0x0000, -0x000A, 0x0000, 0x0023, 0x0000,
    -0x0067, 0x0000, 0x010A, 0x0000, -0x0268, 0x0000, 0x0534, 0x0000,
    -0x0B90, 0x0000, 0x2806, 0x4000, 0x2806, 0x0000, -0x0B90, 0x0000,
    0x0534, 0x0000, -0x0268, 0x0000, 0x010A, 0x0000, -0x0067, 0x0000,
    0x0023, 0x0000, -0x000A, 0x0000, 0x0002, 0x0000, -0x0001,
};

static const int32_t k_pos[5] = {0, 60, 115, 98, 122};
static const int32_t k_neg[5] = {0, 0, -52, -55, -60};

static uint8_t *ram(void) { return PE_SpuRam_Data(); }

static int16_t clamp16(int32_t v)
{
    return (int16_t)(v < -0x8000 ? -0x8000 : v > 0x7FFF ? 0x7FFF : v);
}

void PE_Spu_SetActive(int active) { S.active = active ? 1 : 0; }
int PE_Spu_Active(void) { return S.active; }

void PE_Spu_Reset(void)
{
    int active = S.active;
    PeSpuKonHook hook = S.kon_hook;
    memset(&S, 0, sizeof(S));
    S.active = active;
    S.kon_hook = hook;
    S.noise_level = 1;
}

void PE_Spu_SetKonHook(PeSpuKonHook hook) { S.kon_hook = hook; }

void PE_Spu_GetStats(PeSpuStats *out)
{
    if (out) *out = S.stats;
}

/* ── ADPCM ─────────────────────────────────────────────────────────── */

void PE_Spu_DecodeAdpcmBlock(const uint8_t block[16], int32_t *hist1,
                             int32_t *hist2, int16_t out[28])
{
    int shift = block[0] & 0x0F;
    int filter = (block[0] >> 4) & 0x07;
    int32_t h1 = *hist1, h2 = *hist2;

    if (shift > 12) shift = 9;           /* psx-spx: 13..15 act like 9 */
    if (filter > 4) filter = 4;          /* 5..7 are undefined; clamp */
    for (int i = 0; i < 28; i++) {
        int nib = (block[2 + i / 2] >> ((i & 1) * 4)) & 0x0F;
        int32_t s = (int32_t)(int16_t)(nib << 12) >> shift;
        s += (h1 * k_pos[filter] + h2 * k_neg[filter] + 32) >> 6;
        s = clamp16(s);
        out[i] = (int16_t)s;
        h2 = h1;
        h1 = s;
    }
    *hist1 = h1;
    *hist2 = h2;
}

/* ── envelopes ─────────────────────────────────────────────────────── */

/* One psx-spx envelope tick.  shift 0..31, step already signed
 * (increase 7-s, decrease -8+s).  Returns the new level. */
static int32_t env_tick(int32_t level, int32_t *counter, int shift,
                        int32_t step, int exp, int dec)
{
    int32_t cycles = 1 << (shift > 11 ? shift - 11 : 0);
    int32_t s = step << (shift < 11 ? 11 - shift : 0);

    if (exp && !dec && level > 0x6000) cycles *= 4;
    if (exp && dec) s = (s * level) >> 15;
    if (++*counter < cycles) return level;
    *counter = 0;
    level += s;
    if (level < 0) level = 0;
    if (level > 0x7FFF) level = 0x7FFF;
    return level;
}

void PE_Spu_EnvelopeKeyOn(PeSpuEnvelope *env, uint16_t lo, uint16_t hi)
{
    env->adsr_lo = lo;
    env->adsr_hi = hi;
    env->level = 0;
    env->counter = 0;
    env->phase = 1;
}

void PE_Spu_EnvelopeKeyOff(PeSpuEnvelope *env)
{
    if (env->phase != 0) {
        env->phase = 4;
        env->counter = 0;
    }
}

void PE_Spu_EnvelopeStep(PeSpuEnvelope *env)
{
    uint16_t lo = env->adsr_lo, hi = env->adsr_hi;

    switch (env->phase) {
    case 1: /* attack */
        env->level = env_tick(env->level, &env->counter, (lo >> 10) & 0x1F,
                              7 - ((lo >> 8) & 3), (lo >> 15) & 1, 0);
        if (env->level >= 0x7FFF) {
            env->phase = 2;
            env->counter = 0;
        }
        break;
    case 2: { /* decay: exponential decrease toward the sustain level */
        int32_t sustain = (((int32_t)(lo & 0xF)) + 1) * 0x800;
        env->level = env_tick(env->level, &env->counter, (lo >> 4) & 0xF,
                              -8, 1, 1);
        if (env->level <= sustain) {
            env->phase = 3;
            env->counter = 0;
        }
        break;
    }
    case 3: { /* sustain: runs until key off */
        int dec = (hi >> 14) & 1;
        int stepbits = (hi >> 6) & 3;
        env->level = env_tick(env->level, &env->counter, (hi >> 8) & 0x1F,
                              dec ? -8 + stepbits : 7 - stepbits,
                              (hi >> 15) & 1, dec);
        break;
    }
    case 4: /* release */
        env->level = env_tick(env->level, &env->counter, hi & 0x1F, -8,
                              (hi >> 5) & 1, 1);
        if (env->level <= 0) {
            env->level = 0;
            env->phase = 0;
        }
        break;
    default:
        env->level = 0;
        break;
    }
}

/* Volume register value -> signed linear volume, advancing a sweep. */
static int32_t volume_step(uint16_t reg, int32_t *level, int32_t *counter)
{
    if (!(reg & 0x8000)) {
        int32_t v = (int32_t)(int16_t)(uint16_t)(reg << 1);
        *level = v < 0 ? -v : v;
        return v;
    }
    {
        int exp = (reg >> 14) & 1;
        int dec = (reg >> 13) & 1;
        int neg = (reg >> 12) & 1;
        int stepbits = reg & 3;
        *level = env_tick(*level, counter, (reg >> 2) & 0x1F,
                          dec ? -8 + stepbits : 7 - stepbits, exp, dec);
        return neg ? -*level : *level;
    }
}

/* ── voices ────────────────────────────────────────────────────────── */

static void voice_decode_block(Voice *v, int n)
{
    const uint8_t *b = ram() + (v->cur_addr & (PE_SPU_RAM_SIZE - 16u));
    (void)n;
    v->block_flags = b[1];
    S.stats.blocks_decoded++;
    if ((b[0] >> 4) > 4) S.stats.blocks_bad++;
    if ((b[1] & 4) && !v->ignore_loop)
        v->loop_addr = v->cur_addr;
    PE_Spu_DecodeAdpcmBlock(b, &v->hist1, &v->hist2, v->block);
    v->block_pos = 0;
}

static void voice_end_block(Voice *v, int n)
{
    if (v->block_flags & 1) {
        S.endx |= 1u << n;
        v->cur_addr = v->loop_addr;
        if (!(v->block_flags & 2)) {
            /* loop end without repeat: mute (psx-spx) */
            v->env.phase = 0;
            v->env.level = 0;
            v->on = 0;
        }
    } else {
        v->cur_addr = (v->cur_addr + 16u) & (PE_SPU_RAM_SIZE - 1u);
    }
}

static int16_t voice_next_sample(Voice *v, int n)
{
    int16_t s;
    if (v->block_pos >= 28) {
        voice_end_block(v, n);
        voice_decode_block(v, n);
    }
    s = v->block[v->block_pos++];
    return s;
}

static void key_on(int n)
{
    Voice *v = &S.v[n];
    uint16_t *r = &S.reg[n * 8];

    v->cur_addr = ((uint32_t)r[3] * 8u) & (PE_SPU_RAM_SIZE - 1u);
    {
        const uint8_t *b = ram() + (v->cur_addr & (PE_SPU_RAM_SIZE - 16u));
        int zero = 1;
        for (int i = 0; i < 16; i++) zero &= b[i] == 0;
        if (zero) S.stats.kon_zero_block++;
        else if ((b[0] >> 4) > 4 || (b[0] & 0xF) > 12) S.stats.kon_bad_header++;
    }
    v->ignore_loop = 0;
    v->hist1 = v->hist2 = 0;
    memset(v->h, 0, sizeof(v->h));
    v->counter = 0;
    v->block_pos = 28;
    v->last_out = 0;
    v->on = 1;
    /* first block is decoded without an end-of-block transition */
    voice_decode_block(v, n);
    PE_Spu_EnvelopeKeyOn(&v->env, r[4], r[5]);
    {
        /* PE_AUDIO_VOICE_LOG=path: one line per key-on (diagnostics). */
        static FILE *vl;
        static int init;
        if (!init) {
            const char *p = getenv("PE_AUDIO_VOICE_LOG");
            init = 1;
            if (p && *p) vl = fopen(p, "w");
        }
        if (vl) {
            const uint8_t *b = ram() + (v->cur_addr & (PE_SPU_RAM_SIZE - 16u));
            const uint8_t *b2 = ram() + ((v->cur_addr + 16u) & (PE_SPU_RAM_SIZE - 16u));
            fprintf(vl, "%.3f v%02d addr=%05X pitch=%04X adsr=%04X/%04X vol=%04X/%04X "
                    "blk0=%02X%02X blk1=%02X%02X%02X%02X eon=%d\n",
                    (double)S.stats.rendered_frames / PE_SPU_RATE, n, v->cur_addr,
                    r[2], r[4], r[5], r[0], r[1], b[0], b[1], b2[0], b2[1], b2[2], b2[3],
                    (int)(((S.reg[0x198 / 2] | ((uint32_t)S.reg[0x19A / 2] << 16)) >> n) & 1u));
        }
    }
    S.endx &= ~(1u << n);
}

static void key_off(int n)
{
    PE_Spu_EnvelopeKeyOff(&S.v[n].env);
}

/* ── register interface ────────────────────────────────────────────── */

void PE_Spu_OnRegisterWrite(uint32_t offset, uint16_t value)
{
    uint32_t idx = offset / 2u;
    if (!S.active || idx >= 0x100u) return;
    S.reg[idx] = value;
    S.written[idx] = 1;
    S.stats.register_writes++;

    if (offset < 0x180u) {
        int n = (int)(offset >> 4);
        Voice *v = &S.v[n];
        switch (offset & 0xFu) {
        case 0x8: v->env.adsr_lo = value; break;
        case 0xA: v->env.adsr_hi = value; break;
        case 0xC: v->env.level = (int16_t)value; break;
        case 0xE:
            v->loop_addr = ((uint32_t)value * 8u) & (PE_SPU_RAM_SIZE - 1u);
            if (v->on && v->env.phase != 0) v->ignore_loop = 1;
            break;
        default: break;
        }
        return;
    }
    switch (offset) {
    case 0x188: case 0x18A: {
        uint32_t mask = offset == 0x188 ? value : (uint32_t)value << 16;
        mask &= 0xFFFFFFu;
        for (int n = 0; n < NVOICE; n++)
            if (mask & (1u << n)) {
                key_on(n);
                S.stats.kon_events++;
            }
        if (mask && S.kon_hook) S.kon_hook(mask);
        break;
    }
    case 0x18C: case 0x18E: {
        uint32_t mask = offset == 0x18C ? value : (uint32_t)value << 16;
        mask &= 0xFFFFFFu;
        for (int n = 0; n < NVOICE; n++)
            if (mask & (1u << n)) {
                key_off(n);
                S.stats.koff_events++;
            }
        break;
    }
    case 0x1A2:
        S.rev_base = ((uint32_t)value * 8u) & (PE_SPU_RAM_SIZE - 1u);
        S.rev_cur = S.rev_base;
        break;
    case 0x1A6:
        S.xfer_addr = ((uint32_t)value * 8u) & (PE_SPU_RAM_SIZE - 1u);
        break;
    case 0x1A8:
        ram()[S.xfer_addr] = (uint8_t)value;
        ram()[(S.xfer_addr + 1u) & (PE_SPU_RAM_SIZE - 1u)] = (uint8_t)(value >> 8);
        S.xfer_addr = (S.xfer_addr + 2u) & (PE_SPU_RAM_SIZE - 1u);
        S.stats.fifo_bytes += 2;
        break;
    default:
        break;
    }
}

int PE_Spu_OnRegisterRead(uint32_t offset, uint16_t *value)
{
    if (!S.active) return 0;
    if (offset < 0x180u && (offset & 0xFu) == 0xCu) {
        *value = (uint16_t)S.v[offset >> 4].env.level;
        return 1;
    }
    switch (offset) {
    case 0x19C: *value = (uint16_t)S.endx; return 1;
    case 0x19E: *value = (uint16_t)(S.endx >> 16); return 1;
    case 0x1B8: *value = (uint16_t)S.main_level[0]; return 1;
    case 0x1BA: *value = (uint16_t)S.main_level[1]; return 1;
    default: return 0;
    }
}

void PE_Spu_PushCdAudio(const int16_t *stereo, int frames)
{
    uint32_t cap = (uint32_t)(sizeof(S.cd) / sizeof(S.cd[0]) / 2u);
    for (int i = 0; i < frames; i++) {
        if (S.cd_n >= cap) {           /* overflow: drop the oldest frame */
            S.cd_r = (S.cd_r + 1u) % cap;
            S.cd_n--;
            S.stats.cd_overflow_frames++;
        }
        S.cd[S.cd_w * 2u] = stereo[i * 2];
        S.cd[S.cd_w * 2u + 1u] = stereo[i * 2 + 1];
        S.cd_w = (S.cd_w + 1u) % cap;
        S.cd_n++;
    }
    if (frames > 0) S.cd_idle = 0;
}

/* ── reverb (psx-spx algorithm, 22.05 kHz) ─────────────────────────── */

static uint32_t rev_addr(int32_t rel_halfwords, int32_t reg8)
{
    /* register offsets are in 8-byte units relative to the buffer ptr */
    uint32_t base = S.rev_base;
    uint32_t size = PE_SPU_RAM_SIZE - base;
    int64_t a = (int64_t)(S.rev_cur - base) + (int64_t)reg8 * 8 +
                (int64_t)rel_halfwords * 2;
    a %= (int64_t)size;
    if (a < 0) a += size;
    return base + (uint32_t)a;
}

static int32_t rev_rd(int32_t reg8, int rel)
{
    uint32_t a = rev_addr(rel, reg8) & (PE_SPU_RAM_SIZE - 2u);
    return (int16_t)(ram()[a] | (ram()[a + 1] << 8));
}

static void rev_wr(int32_t reg8, int32_t v)
{
    uint32_t a = rev_addr(0, reg8) & (PE_SPU_RAM_SIZE - 2u);
    int16_t s = clamp16(v);
    ram()[a] = (uint8_t)s;
    ram()[a + 1] = (uint8_t)((uint16_t)s >> 8);
}

#define RR(o) (S.reg[(o) / 2u])
#define RS(o) ((int32_t)(int16_t)S.reg[(o) / 2u])
#define MUL(a, b) (((int32_t)(a) * (int32_t)(b)) >> 15)

static void reverb_tick(int32_t in_l, int32_t in_r, int32_t *out_l, int32_t *out_r)
{
    int32_t lin = MUL(clamp16(in_l), RS(0x1FC));
    int32_t rin = MUL(clamp16(in_r), RS(0x1FE));
    int32_t viir = RS(0x1C4), vwall = RS(0x1CE);
    int32_t l, r;

    if (S.reg[0x1AA / 2] & 0x80) {  /* reverb master enable: write work area */
        rev_wr(RR(0x1D4), MUL(lin + MUL(rev_rd(RR(0x1E0), 0), vwall) -
                                  rev_rd(RR(0x1D4), -1), viir) +
                              rev_rd(RR(0x1D4), -1));
        rev_wr(RR(0x1D6), MUL(rin + MUL(rev_rd(RR(0x1E2), 0), vwall) -
                                  rev_rd(RR(0x1D6), -1), viir) +
                              rev_rd(RR(0x1D6), -1));
        rev_wr(RR(0x1E4), MUL(lin + MUL(rev_rd(RR(0x1F2), 0), vwall) -
                                  rev_rd(RR(0x1E4), -1), viir) +
                              rev_rd(RR(0x1E4), -1));
        rev_wr(RR(0x1E6), MUL(rin + MUL(rev_rd(RR(0x1F0), 0), vwall) -
                                  rev_rd(RR(0x1E6), -1), viir) +
                              rev_rd(RR(0x1E6), -1));
    }
    l = MUL(RS(0x1C6), rev_rd(RR(0x1D8), 0)) + MUL(RS(0x1C8), rev_rd(RR(0x1DC), 0)) +
        MUL(RS(0x1CA), rev_rd(RR(0x1E8), 0)) + MUL(RS(0x1CC), rev_rd(RR(0x1EC), 0));
    r = MUL(RS(0x1C6), rev_rd(RR(0x1DA), 0)) + MUL(RS(0x1C8), rev_rd(RR(0x1DE), 0)) +
        MUL(RS(0x1CA), rev_rd(RR(0x1EA), 0)) + MUL(RS(0x1CC), rev_rd(RR(0x1EE), 0));
    {
        uint16_t d1 = RR(0x1C0), d2 = RR(0x1C2);
        int32_t va1 = RS(0x1D0), va2 = RS(0x1D2);
        int32_t t;
        /* [mAPF - dAPF] addresses: register difference in 8-byte units */
        t = rev_rd((int32_t)RR(0x1F4) - (int32_t)d1, 0);
        l = l - MUL(va1, t);
        if (S.reg[0x1AA / 2] & 0x80) rev_wr(RR(0x1F4), l);
        l = MUL(clamp16(l), va1) + t;
        t = rev_rd((int32_t)RR(0x1F6) - (int32_t)d1, 0);
        r = r - MUL(va1, t);
        if (S.reg[0x1AA / 2] & 0x80) rev_wr(RR(0x1F6), r);
        r = MUL(clamp16(r), va1) + t;
        t = rev_rd((int32_t)RR(0x1F8) - (int32_t)d2, 0);
        l = l - MUL(va2, t);
        if (S.reg[0x1AA / 2] & 0x80) rev_wr(RR(0x1F8), l);
        l = MUL(clamp16(l), va2) + t;
        t = rev_rd((int32_t)RR(0x1FA) - (int32_t)d2, 0);
        r = r - MUL(va2, t);
        if (S.reg[0x1AA / 2] & 0x80) rev_wr(RR(0x1FA), r);
        r = MUL(clamp16(r), va2) + t;
    }
    *out_l = MUL(clamp16(l), RS(0x184));
    *out_r = MUL(clamp16(r), RS(0x186));
    S.rev_cur += 2u;
    if (S.rev_cur >= PE_SPU_RAM_SIZE) S.rev_cur = S.rev_base;
}

/* ── mixing ────────────────────────────────────────────────────────── */

static void noise_tick(void)
{
    uint16_t cnt = S.reg[0x1AA / 2];
    int step = ((cnt >> 8) & 3) + 4;
    int shift = (cnt >> 10) & 0xF;
    uint16_t lv = (uint16_t)S.noise_level;
    int parity = ((lv >> 15) ^ (lv >> 12) ^ (lv >> 11) ^ (lv >> 10) ^ 1) & 1;

    S.noise_timer -= step;
    if (S.noise_timer < 0) {
        S.noise_level = (int16_t)(uint16_t)((lv << 1) | parity);
        S.noise_timer += 0x20000 >> shift;
        if (S.noise_timer < 0) S.noise_timer += 0x20000 >> shift;
    }
}

void PE_Spu_Render(int16_t *out, int frames)
{
    uint32_t pmon = (S.reg[0x190 / 2] | ((uint32_t)S.reg[0x192 / 2] << 16)) & 0xFFFFFEu;
    uint32_t non = (S.reg[0x194 / 2] | ((uint32_t)S.reg[0x196 / 2] << 16)) & 0xFFFFFFu;
    uint32_t eon = (S.reg[0x198 / 2] | ((uint32_t)S.reg[0x19A / 2] << 16)) & 0xFFFFFFu;
    uint16_t cnt = S.reg[0x1AA / 2];
    int reverb_ok = S.written[0x1A2 / 2] && S.rev_base != 0;
    uint32_t cd_cap = (uint32_t)(sizeof(S.cd) / sizeof(S.cd[0]) / 2u);

    for (int f = 0; f < frames; f++) {
        int32_t mix[2] = {0, 0}, rin[2] = {0, 0};
        uint32_t act = 0;

        noise_tick();
        for (int n = 0; n < NVOICE; n++) {
            Voice *v = &S.v[n];
            uint16_t *r = &S.reg[n * 8];
            int32_t step, s, o;

            if (!v->on && v->env.phase == 0) {
                /* still consume volume sweeps so they settle */
                continue;
            }
            act++;
            step = r[2];
            if (step > 0x3FFF) step = 0x4000;
            if (n > 0 && (pmon & (1u << n))) {
                int32_t factor = S.v[n - 1].last_out + 0x8000;
                step = (int32_t)(((int64_t)step * factor) >> 15);
                if (step > 0x3FFF) step = 0x4000;
                if (step < 0) step = 0;
            }
            v->counter += (uint32_t)step;
            while (v->counter >= 0x1000u) {
                v->counter -= 0x1000u;
                v->h[0] = v->h[1];
                v->h[1] = v->h[2];
                v->h[2] = v->h[3];
                v->h[3] = voice_next_sample(v, n);
            }
            if (non & (1u << n)) {
                s = S.noise_level;
            } else {
                /* psx-spx 4-point gaussian: index = pitch counter bits 4..11 */
                int i = (int)((v->counter >> 4) & 0xFFu);
                s = (k_gauss[0x0FF - i] * v->h[0]) >> 15;
                s += (k_gauss[0x1FF - i] * v->h[1]) >> 15;
                s += (k_gauss[0x100 + i] * v->h[2]) >> 15;
                s += (k_gauss[0x000 + i] * v->h[3]) >> 15;
            }
            PE_Spu_EnvelopeStep(&v->env);
            o = (s * v->env.level) >> 15;
            v->last_out = o;
            {
                int32_t vl = volume_step(r[0], &v->vol_level[0], &v->vol_counter[0]);
                int32_t vr = volume_step(r[1], &v->vol_level[1], &v->vol_counter[1]);
                int32_t l = (o * vl) >> 15, rr = (o * vr) >> 15;
                mix[0] += l;
                mix[1] += rr;
                if (eon & (1u << n)) {
                    rin[0] += l;
                    rin[1] += rr;
                }
            }
            if (v->env.phase == 0) v->on = 0;
        }
        S.stats.voices_active = act;

        /* CD audio input (SPUCNT bit 0 = CD audio enable).
         * The port delivers XA sectors in bursts paced by the movie/drive
         * model rather than at a steady 75/150 Hz, so the queue is a jitter
         * buffer: it primes to CD_PREBUF frames (or plays out once sectors
         * stop arriving), a starve ramps the last frame down instead of
         * stepping to zero, and playback resumes with a fade-in. */
        S.cd_idle++;
        if (!S.cd_play && S.cd_n &&
            (S.cd_n >= CD_PREBUF || S.cd_idle > CD_PLAYOUT_IDLE)) {
            S.cd_play = 1;
            S.cd_fade = CD_FADE;
        }
        if (S.cd_play && !S.cd_n) {
            S.cd_play = 0;
            if (S.cd_idle <= CD_PLAYOUT_IDLE) {
                S.stats.cd_underruns++;
                S.cd_live = 2;
            }
            S.cd_tail = CD_FADE;
        }
        if (S.cd_live == 2 && !S.cd_play) S.stats.cd_underrun_frames++;
        if (S.cd_play || S.cd_tail) {
            int32_t cl, cr;
            S.stats.cd_frames_mixed++;
            if (S.cd_play) {
                S.cd_live = 1;
                cl = S.cd[S.cd_r * 2u];
                cr = S.cd[S.cd_r * 2u + 1u];
                S.cd_r = (S.cd_r + 1u) % cd_cap;
                S.cd_n--;
                if (S.cd_fade) {
                    int32_t g = CD_FADE - S.cd_fade + 1;
                    cl = cl * g / (CD_FADE + 1);
                    cr = cr * g / (CD_FADE + 1);
                    S.cd_fade--;
                }
                S.cd_last[0] = (int16_t)cl;
                S.cd_last[1] = (int16_t)cr;
            } else {
                cl = S.cd_last[0] * S.cd_tail / (CD_FADE + 1);
                cr = S.cd_last[1] * S.cd_tail / (CD_FADE + 1);
                S.cd_tail--;
            }
            /* The port collapses SpuInit/SsSetSerialAttr, so an unwritten
             * CD volume means "CD input on at full volume" (gap notes). */
            int cd_written = S.written[0x1B0 / 2] || S.written[0x1B2 / 2];
            if (!cd_written || (cnt & 1)) {
                int32_t vl = cd_written ? RS(0x1B0) : 0x7FFF;
                int32_t vr = cd_written ? RS(0x1B2) : 0x7FFF;
                int32_t l = (cl * vl) >> 15, rr = (cr * vr) >> 15;
                mix[0] += l;
                mix[1] += rr;
                if (cnt & 4) {
                    rin[0] += l;
                    rin[1] += rr;
                }
            }
        }

        if (reverb_ok) {
            /* psx-spx "Reverb Buffer Resampling": the 22.05 kHz unit
             * reads its input through a 39-tap half-band FIR and its output
             * is interpolated back with the same FIR (zero-stuffed, x2
             * gain).  The earlier 2-sample average + hold imaged the wet
             * signal into 11-22 kHz. */
            uint32_t hp = S.rev_hp;
            int32_t ro[2] = {0, 0};
            S.rev_ih[hp][0] = clamp16(rin[0]);
            S.rev_ih[hp][1] = clamp16(rin[1]);
            S.rev_oh[hp][0] = S.rev_oh[hp][1] = 0;
            if (++S.rev_phase >= 2) {
                int32_t il = 0, ir = 0;
                S.rev_phase = 0;
                for (int k = 0; k < 39; k++) {
                    const int16_t *x = S.rev_ih[(hp - (uint32_t)k) & 63u];
                    il += x[0] * k_rev_fir[k];
                    ir += x[1] * k_rev_fir[k];
                }
                reverb_tick(il >> 15, ir >> 15, &S.rev_out[0], &S.rev_out[1]);
                S.rev_oh[hp][0] = clamp16(S.rev_out[0]);
                S.rev_oh[hp][1] = clamp16(S.rev_out[1]);
            }
            for (int k = 0; k < 39; k++) {
                const int16_t *y = S.rev_oh[(hp - (uint32_t)k) & 63u];
                ro[0] += y[0] * k_rev_fir[k];
                ro[1] += y[1] * k_rev_fir[k];
            }
            S.rev_hp = (hp + 1u) & 63u;
            mix[0] += clamp16(ro[0] >> 14);
            mix[1] += clamp16(ro[1] >> 14);
        }

        {
            /* Main volume.  An unwritten main volume register (the port
             * collapses SpuInit) is treated as 0x3FFF, the libsnd default
             * after SsSetMVol(127,127). */
            uint16_t ml = S.written[0x180 / 2] ? S.reg[0x180 / 2] : 0x3FFF;
            uint16_t mr = S.written[0x182 / 2] ? S.reg[0x182 / 2] : 0x3FFF;
            int32_t vl = volume_step(ml, &S.main_level[0], &S.main_counter[0]);
            int32_t vr = volume_step(mr, &S.main_level[1], &S.main_counter[1]);
            int32_t l = (clamp16(mix[0]) * vl) >> 15;
            int32_t r = (clamp16(mix[1]) * vr) >> 15;
            /* SPUCNT bit 14 = unmute; honoured only once SPUCNT was
             * written with the enable bit (bit 15), see gap notes. */
            if ((cnt & 0x8000) && !(cnt & 0x4000)) l = r = 0;
            out[f * 2] = clamp16(l);
            out[f * 2 + 1] = clamp16(r);
        }
    }
    S.stats.rendered_frames += (uint64_t)frames;
}

/* ── XA-ADPCM (psx-spx "CDROM XA Audio ADPCM Compression") ────────── */

/*
 * XA -> 44.1 kHz: the CD decoder's documented 7-phase "zigzag" FIR
 * (psx-spx).  Every six 37.8 kHz samples yield seven 44.1 kHz outputs, each
 * a 29-tap sum over a 32-entry ring.  18.9 kHz XA is fed twice per sample
 * into the same 37.8 kHz path (approximation; psx-spx notes the hardware
 * spreads it with a lower-pitch zigzag).  The table's DC gain is 0x73EB..
 * 0x741D / 0x8000 (about -0.85 dB), as on hardware.  This replaced a
 * 2-point linear interpolator that left image energy at 18.9-22 kHz at
 * -30 dB (hiss).
 */
#include "pe_spu_zigzag.inc"

static struct {
    int32_t h[2][2];            /* per channel: h1, h2 */
    int16_t ring[32][2];        /* zigzag input ring (37.8 kHz) */
    uint32_t p;
    int sixstep;
    uint64_t sectors;
} X;

void PE_Spu_XaReset(void)
{
    memset(&X, 0, sizeof(X));
    S.cd_live = 0;
}

/* fmv lane: stream stop / movie skip.  Drop the queued CD input, ramp the
 * last output frame down over CD_FADE frames (no click), and reset the XA
 * decoder history.  X.sectors (diagnostic count) is kept. */
void PE_Spu_CdInputStop(void)
{
    uint64_t sectors = X.sectors;

    S.cd_r = S.cd_w = S.cd_n = 0u;
    if (S.cd_play || S.cd_tail)
        S.cd_tail = CD_FADE;
    S.cd_play = 0;
    S.cd_fade = 0;
    S.cd_idle = 0u;
    S.cd_live = 0;
    memset(&X, 0, sizeof(X));
    X.sectors = sectors;
}

uint64_t PE_Spu_XaSectors(void) { return X.sectors; }

int PE_Spu_DecodeXaSector(const uint8_t *sub_and_data, int16_t *out,
                          int *channels, int *rate)
{
    /* sub_and_data: the 8-byte subheader followed by 2324 bytes of data */
    static const int32_t K0[4] = {0, 60, 115, 98};
    static const int32_t K1[4] = {0, 0, -52, -55};
    uint8_t coding = sub_and_data[3];
    int stereo = (coding & 3) == 1;
    int bits8 = ((coding >> 4) & 3) == 1;
    const uint8_t *data = sub_and_data + 8;
    int n = 0;

    *channels = stereo ? 2 : 1;
    *rate = ((coding >> 2) & 3) == 1 ? 18900 : 37800;
    for (int g = 0; g < 18; g++) {
        const uint8_t *grp = data + g * 128;
        int units = bits8 ? 4 : 8;
        for (int u = 0; u < units; u++) {
            uint8_t hdr = grp[4 + u];
            int shift = hdr & 0xF;
            int filt = (hdr >> 4) & 3;
            int ch = stereo ? (u & 1) : 0;
            int32_t *h = X.h[ch];
            if (shift > 12) shift = 9;
            for (int j = 0; j < 28; j++) {
                int32_t s;
                if (bits8) {
                    s = (int32_t)(int16_t)(uint16_t)(grp[16 + j * 4 + u] << 8) >> shift;
                } else {
                    uint8_t b = grp[16 + j * 4 + u / 2];
                    int nib = (u & 1) ? (b >> 4) : (b & 0xF);
                    s = (int32_t)(int16_t)(uint16_t)(nib << 12) >> shift;
                }
                s += (h[0] * K0[filt] + h[1] * K1[filt] + 32) >> 6;
                s = clamp16(s);
                h[1] = h[0];
                h[0] = s;
                if (stereo) {
                    /* units alternate L/R: unit pairs (0,1),(2,3).. share j */
                    out[((u / 2) * 28 + j) * 2 + ch + (g * (units / 2) * 28) * 2] = (int16_t)s;
                } else {
                    out[g * units * 28 + u * 28 + j] = (int16_t)s;
                }
            }
        }
        n += units * 28;
    }
    return stereo ? n / 2 : n;   /* sample frames */
}

void PE_Spu_CdSector(const uint8_t *raw, uint8_t mode, int muted,
                     uint8_t filter_file, uint8_t filter_chan,
                     const uint8_t vol[4])
{
    int16_t pcm[18 * 8 * 28];
    int16_t outbuf[2 * 256];     /* flushed whenever full */
    int ch, rate, frames, k = 0;
    const uint8_t *sub = raw + 16;
    int32_t lvol[2], rvol[2];

    if (!S.active || !(mode & 0x40u)) return;         /* XA-ADPCM off */
    if (raw[15] != 2 || !(sub[2] & 0x04u) || !(sub[2] & 0x20u)) return;
    if ((mode & 0x08u) && (sub[0] != filter_file || sub[1] != filter_chan)) return;
    X.sectors++;
    {
        static FILE *xl;
        static int init;
        if (!init) {
            const char *p = getenv("PE_AUDIO_XA_LOG");
            init = 1;
            if (p && *p) xl = fopen(p, "w");
        }
        if (xl)
            fprintf(xl, "%llu %llu %u %02X %02X %02X\n",
                    (unsigned long long)X.sectors,
                    (unsigned long long)S.stats.rendered_frames, S.cd_n,
                    sub[0], sub[1], sub[3]);
    }
    frames = PE_Spu_DecodeXaSector(sub, pcm, &ch, &rate);
    if (muted) return;
    /* CD audio volume matrix (0x80 = 100%): L->L, L->R, R->R, R->L */
    lvol[0] = vol[0]; lvol[1] = vol[1]; rvol[1] = vol[2]; rvol[0] = vol[3];
    for (int i = 0; i < frames; i++) {
        int16_t l = ch == 2 ? pcm[i * 2] : pcm[i];
        int16_t r = ch == 2 ? pcm[i * 2 + 1] : pcm[i];
        for (int rep = 0; rep < (rate == 18900 ? 2 : 1); rep++) {
            X.ring[X.p & 31u][0] = l;
            X.ring[X.p & 31u][1] = r;
            X.p++;
            if (X.sixstep <= 0) X.sixstep = 6;
            if (--X.sixstep) continue;
            for (int t = 0; t < 7; t++) {
                int32_t fl = 0, fr = 0;
                for (int j = 1; j <= 29; j++) {
                    const int16_t *x = X.ring[(X.p - (uint32_t)j) & 31u];
                    fl += (x[0] * k_zigzag[t][j - 1]) / 0x8000;  /* psx-spx: /8000h */
                    fr += (x[1] * k_zigzag[t][j - 1]) / 0x8000;
                }
                fl = clamp16(fl);
                fr = clamp16(fr);
                outbuf[k * 2] = clamp16((fl * lvol[0] + fr * rvol[0]) >> 7);
                outbuf[k * 2 + 1] = clamp16((fl * lvol[1] + fr * rvol[1]) >> 7);
                if (++k == 256) { PE_Spu_PushCdAudio(outbuf, k); k = 0; }
            }
        }
    }
    if (k) PE_Spu_PushCdAudio(outbuf, k);
}

void PE_Spu_DebugSummary(void *file)
{
    FILE *f = (FILE *)file;
    static const uint16_t regs[] = {0x180, 0x182, 0x184, 0x186, 0x1A2, 0x1AA,
                                    0x1B0, 0x1B2, 0x1B4, 0x1B6};
    fprintf(f, "[AUDIO] SPU regs:");
    for (unsigned i = 0; i < sizeof(regs) / sizeof(regs[0]); i++)
        fprintf(f, " %03X=%04X%s", regs[i], S.reg[regs[i] / 2],
                S.written[regs[i] / 2] ? "" : "(unwritten)");
    fprintf(f, " konbad=%llu konzero=%llu blocks=%llu badblocks=%llu",
            (unsigned long long)S.stats.kon_bad_header,
            (unsigned long long)S.stats.kon_zero_block,
            (unsigned long long)S.stats.blocks_decoded,
            (unsigned long long)S.stats.blocks_bad);
    fprintf(f, " cd-underruns=%llu/%llu frames cd-overflow=%llu",
            (unsigned long long)S.stats.cd_underruns,
            (unsigned long long)S.stats.cd_underrun_frames,
            (unsigned long long)S.stats.cd_overflow_frames);
    fprintf(f, " EON=%06X NON=%06X PMON=%06X XA-sectors=%llu cdq=%u\n",
            (S.reg[0x198 / 2] | ((uint32_t)S.reg[0x19A / 2] << 16)),
            (S.reg[0x194 / 2] | ((uint32_t)S.reg[0x196 / 2] << 16)),
            (S.reg[0x190 / 2] | ((uint32_t)S.reg[0x192 / 2] << 16)),
            (unsigned long long)X.sectors, S.cd_n);
}

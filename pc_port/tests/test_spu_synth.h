/* Audio lane: native SPU synthesizer + audio-driver interpreter vectors.
 * Expected values are hand-computed from the psx-spx formulas (see the
 * comments beside each vector), not captured from this implementation. */
#include "pe_spu.h"
#include "pe_audio_driver.h"
#include "pe_plat/audio.h"

static void test_AUDIO_adpcm_decode(void)
{
    TEST("AUDIO_adpcm_decode");
    uint8_t b[16];
    int16_t out[28];
    int32_t h1, h2;

    /* filter 0, shift 0: nibble 7 -> 0x7000, nibble 8 -> -0x8000 */
    memset(b, 0, sizeof(b)); b[0] = 0x00; b[2] = 0x87;
    h1 = h2 = 0; PE_Spu_DecodeAdpcmBlock(b, &h1, &h2, out);
    ASSERT(out[0] == 28672 && out[1] == -32768 && out[2] == 0, "filter 0 raw nibbles");
    /* filter 0, shift 1: nibble F -> (int16)0xF000 >> 1 = -2048 */
    memset(b, 0, sizeof(b)); b[0] = 0x01; b[2] = 0x0F;
    h1 = h2 = 0; PE_Spu_DecodeAdpcmBlock(b, &h1, &h2, out);
    ASSERT(out[0] == -2048, "signed shift");
    /* shift 13 behaves like 9: 0x7000 >> 9 = 56 */
    memset(b, 0, sizeof(b)); b[0] = 0x0D; b[2] = 0x07;
    h1 = h2 = 0; PE_Spu_DecodeAdpcmBlock(b, &h1, &h2, out);
    ASSERT(out[0] == 56, "shift 13..15 act as 9");
    /* filter 1 (60/64): 16384, (16384*60)>>6 = 15360, (15360*60)>>6 = 14400 */
    memset(b, 0, sizeof(b)); b[0] = 0x10; b[2] = 0x04;
    h1 = h2 = 0; PE_Spu_DecodeAdpcmBlock(b, &h1, &h2, out);
    ASSERT(out[0] == 16384 && out[1] == 15360 && out[2] == 14400, "filter 1 recursion");
    ASSERT(h1 == out[27] && h2 == out[26], "history carries the last two samples");
    /* filter 2 (115,-52): 16384, 29440, then 39588 clamps to 32767 */
    memset(b, 0, sizeof(b)); b[0] = 0x20; b[2] = 0x04;
    h1 = h2 = 0; PE_Spu_DecodeAdpcmBlock(b, &h1, &h2, out);
    ASSERT(out[0] == 16384 && out[1] == 29440 && out[2] == 32767, "filter 2 + clamp");
    /* history from a previous block feeds the first sample: filter 1,
     * h1 = 1000 -> (1000*60)>>6 = 937 */
    memset(b, 0, sizeof(b)); b[0] = 0x10;
    h1 = 1000; h2 = 0; PE_Spu_DecodeAdpcmBlock(b, &h1, &h2, out);
    ASSERT(out[0] == 937, "cross-block history");
    PASS();
}

static void test_AUDIO_adsr(void)
{
    TEST("AUDIO_adsr");
    PeSpuEnvelope e;
    /* attack linear shift 0 step 0 (+7<<11 = 14336/sample), decay shift 0,
     * sustain level 7 (0x4000); sustain linear decrease shift 31 (slow);
     * release linear shift 0 */
    uint16_t lo = (uint16_t)((0u << 15) | (0u << 10) | (0u << 8) | (0u << 4) | 7u);
    uint16_t hi = (uint16_t)((0u << 15) | (1u << 14) | (31u << 8) | (0u << 6) | (0u << 5) | 0u);
    PE_Spu_EnvelopeKeyOn(&e, lo, hi);
    PE_Spu_EnvelopeStep(&e); ASSERT(e.level == 14336 && e.phase == 1, "attack step 1");
    PE_Spu_EnvelopeStep(&e); ASSERT(e.level == 28672 && e.phase == 1, "attack step 2");
    PE_Spu_EnvelopeStep(&e); ASSERT(e.level == 0x7FFF && e.phase == 2, "attack clamps and enters decay");
    /* decay: exp decrease, step -8<<11 scaled by level: (-16384*32767)>>15 = -16384 */
    PE_Spu_EnvelopeStep(&e); ASSERT(e.level == 16383 && e.phase == 3, "decay reaches sustain level 0x4000");
    PE_Spu_EnvelopeKeyOff(&e);
    PE_Spu_EnvelopeStep(&e); ASSERT(e.level == 0 && e.phase == 0, "linear release shift 0 ends");

    /* slow attack: shift 13 step bits 3 -> step 4 every 1<<2 = 4 samples */
    PE_Spu_EnvelopeKeyOn(&e, (uint16_t)((13u << 10) | (3u << 8)), 0);
    for (int i = 0; i < 3; i++) PE_Spu_EnvelopeStep(&e);
    ASSERT(e.level == 0, "no change before the 4th cycle");
    PE_Spu_EnvelopeStep(&e); ASSERT(e.level == 4, "+4 on the 4th cycle");
    for (int i = 0; i < 4; i++) PE_Spu_EnvelopeStep(&e);
    ASSERT(e.level == 8, "+4 per 4 cycles");

    /* exponential attack above 0x6000 is four times slower:
     * shift 11 -> 1 cycle, step 7 */
    PE_Spu_EnvelopeKeyOn(&e, (uint16_t)((1u << 15) | (11u << 10)), 0);
    e.level = 0x6001;
    for (int i = 0; i < 3; i++) PE_Spu_EnvelopeStep(&e);
    ASSERT(e.level == 0x6001, "exp attack waits 4 cycles above 0x6000");
    PE_Spu_EnvelopeStep(&e); ASSERT(e.level == 0x6008, "exp attack +7");
    PASS();
}

static void test_AUDIO_voice_render(void)
{
    TEST("AUDIO_voice_render");
    ResetTestState();
    PE_Spu_SetActive(1);
    PE_Spu_Reset();
    uint8_t *ram = PE_SpuRam_Data();
    /* block at 0x1000: filter 0 shift 0, constant nibble 4 (16384), loop end
     * without repeat (flag 1) -> plays 28 samples, sets ENDX, mutes */
    memset(ram + 0x1000, 0x44, 16); ram[0x1000] = 0x00; ram[0x1001] = 0x01;
    PE_SpuRegister_StoreU16(0x0, 0x3FFF);               /* vol L */
    PE_SpuRegister_StoreU16(0x2, 0x3FFF);               /* vol R */
    PE_SpuRegister_StoreU16(0x4, 0x1000);               /* 44100 Hz */
    PE_SpuRegister_StoreU16(0x6, 0x1000 / 8);           /* start */
    PE_SpuRegister_StoreU16(0x8, 0x000F);               /* instant attack */
    PE_SpuRegister_StoreU16(0xA, 0x0000);
    PE_SpuRegister_StoreU16(0x180, 0x3FFF);
    PE_SpuRegister_StoreU16(0x182, 0x3FFF);
    PE_SpuRegister_StoreU16(0x188, 0x0001);             /* KON voice 0 */
    ASSERT((PE_SpuRegister_LoadU16(0x19C) & 1u) == 0, "KON clears ENDX");
    int16_t out[2 * 64];
    PE_Spu_Render(out, 20);
    int nonzero = 0;
    for (int i = 0; i < 40; i++) nonzero |= out[i] != 0;
    ASSERT(nonzero, "voice produces output");
    ASSERT(PE_SpuRegister_LoadU16(0xC) != 0, "live envelope readback");
    PE_Spu_Render(out, 40);
    ASSERT(PE_SpuRegister_LoadU16(0x19C) & 1u, "loop end sets ENDX");
    PE_Spu_Render(out, 8);
    ASSERT(out[14] == 0 && out[15] == 0 && PE_SpuRegister_LoadU16(0xC) == 0,
           "end without repeat mutes");
    PE_Spu_SetActive(0);
    PE_Spu_Reset();
    PASS();
}

static void test_AUDIO_driver_interpreter(void)
{
    TEST("AUDIO_driver_interpreter");
    ResetTestState();
    static const uint32_t prog[] = {
        0x3C081F80u, /* lui   t0,0x1F80        */
        0x35081C00u, /* ori   t0,t0,0x1C00     */
        0xA5050004u, /* sh    a1,4(t0)  (SPU voice 0 pitch) */
        0x95020004u, /* lhu   v0,4(t0)         */
        0x03E00008u, /* jr    ra               */
        0x00441021u, /* addu  v0,v0,a0 (delay slot) */
    };
    for (unsigned i = 0; i < sizeof(prog) / sizeof(prog[0]); i++)
        PE_StoreU32(0x80100000u + i * 4u, prog[i]);
    int ok = 0;
    uint32_t v = PE_AudioDriver_CallGuest(0x80100000u, 5u, 0x1234u, 0, 0, &ok);
    ASSERT(ok && v == 0x1239u, "SPU store/load round trip + delay slot");
    ASSERT(PE_SpuRegister_LoadU16(4) == 0x1234u, "store reached the SPU register file");
    /* bad fetch is a fault, not an abort */
    v = PE_AudioDriver_CallGuest(0xBFC00000u, 0, 0, 0, 0, &ok);
    ASSERT(!ok, "unmapped fetch faults");
    PASS();
}

static void test_AUDIO_xa_decode(void)
{
    TEST("AUDIO_xa_decode");
    static uint8_t sec[8 + 2324];
    static int16_t pcm[18 * 8 * 28];
    int ch = 0, rate = 0, n;
    memset(sec, 0, sizeof(sec));
    sec[2] = 0x64; sec[3] = 0x00;          /* audio|form2|realtime; mono 37.8k 4-bit */
    sec[8 + 4] = 0x00;                     /* unit 0: shift 0 filter 0 */
    sec[8 + 5] = 0x0C;                     /* unit 1: shift 12 filter 0 */
    sec[8 + 16] = 0x14;                    /* word 0: unit0 nib 4, unit1 nib 1 */
    PE_Spu_XaReset();
    n = PE_Spu_DecodeXaSector(sec, pcm, &ch, &rate);
    ASSERT(n == 4032 && ch == 1 && rate == 37800, "mono 4-bit sector shape");
    ASSERT(pcm[0] == 16384 && pcm[1] == 0 && pcm[28] == 1, "unit order and shift");
    sec[3] = 0x05;                         /* stereo, 18.9 kHz */
    PE_Spu_XaReset();
    n = PE_Spu_DecodeXaSector(sec, pcm, &ch, &rate);
    ASSERT(n == 2016 && ch == 2 && rate == 18900, "stereo 4-bit sector shape");
    ASSERT(pcm[0] == 16384 && pcm[1] == 1, "units alternate L/R");
    PE_Spu_XaReset();
    PASS();
}

static void test_AUDIO_gauss_and_cd_jitter(void)
{
    TEST("AUDIO_gauss_and_cd_jitter");
    ResetTestState();
    PE_Spu_SetActive(1);
    PE_Spu_Reset();
    uint8_t *ram = PE_SpuRam_Data();
    int16_t out[2 * 512];
    /* Constant 16384 (filter 0, shift 0, nibble 4) looping on itself
     * (flags 7 = loop start + end + repeat).  psx-spx's gaussian taps sum to
     * 0x7F80/0x8000 (255/256), so the voice settles at ~16320 before the
     * volume stages (each x0x7FFE>>15 loses at most 1); plain linear
     * interpolation would give 16384. */
    memset(ram + 0x2000, 0x44, 16); ram[0x2000] = 0x00; ram[0x2001] = 0x07;
    PE_SpuRegister_StoreU16(0x0, 0x3FFF);
    PE_SpuRegister_StoreU16(0x2, 0x3FFF);
    PE_SpuRegister_StoreU16(0x4, 0x1000);
    PE_SpuRegister_StoreU16(0x6, 0x2000 / 8);
    PE_SpuRegister_StoreU16(0x8, 0x000F);
    PE_SpuRegister_StoreU16(0xA, 0x0000);
    PE_SpuRegister_StoreU16(0x180, 0x3FFF);
    PE_SpuRegister_StoreU16(0x182, 0x3FFF);
    PE_SpuRegister_StoreU16(0x188, 0x0001);
    PE_Spu_Render(out, 512);
    ASSERT(out[1000] >= 16310 && out[1000] <= 16322, "gaussian 255/256 gain");
    PE_SpuRegister_StoreU16(0x18C, 0x0001);             /* KOFF */
    PE_Spu_Reset();

    /* CD/XA jitter buffer: silent until 5880 frames are queued, then a
     * 64-frame fade-in; a starve ramps down instead of stepping to 0. */
    int16_t cd[2 * 3000];
    for (int i = 0; i < 2 * 3000; i++) cd[i] = 8000;
    PE_Spu_PushCdAudio(cd, 3000);
    PE_Spu_Render(out, 200);
    int any = 0;
    for (int i = 0; i < 400; i++) any |= out[i] != 0;
    ASSERT(!any, "CD queue primes before playing");
    PE_Spu_PushCdAudio(cd, 3000);
    PE_Spu_Render(out, 100);
    ASSERT(out[0] > 0 && out[0] < 500, "fade-in starts small");
    ASSERT(out[2 * 99] > 7900 && out[2 * 99] < 8001, "full level after fade-in");
    int maxstep = 0, last = out[2 * 99];
    for (int blk = 0; blk < 13; blk++) {       /* drain 6000 and past it */
        PE_Spu_Render(out, 500);
        for (int i = 0; i < 500; i++) {
            int d = out[2 * i] - last;
            if (d < 0) d = -d;
            if (d > maxstep) maxstep = d;
            last = out[2 * i];
        }
    }
    ASSERT(last == 0, "silent after the queue drains");
    ASSERT(maxstep < 200, "starve ramps down (no click)");
    PE_Spu_SetActive(0);
    PE_Spu_Reset();
    PASS();
}

/* MOVIE AUDIO VOLUME: the CD-input gain is exact at 100 (no multiply), halves
 * the CD contribution at 50 and silences it at 0, and survives PE_Spu_Reset. */
static void audio_cd_render(int gain, int16_t *out, int frames)
{
    int16_t cd[2 * 3000];
    PE_Spu_SetActive(1);
    PE_Spu_Reset();
    PE_Spu_SetCdGain(gain);
    PE_SpuRegister_StoreU16(0x180, 0x3FFF);
    PE_SpuRegister_StoreU16(0x182, 0x3FFF);
    for (int i = 0; i < 2 * 3000; i++) cd[i] = (int16_t)((i * 37) % 20000 - 10000);
    PE_Spu_PushCdAudio(cd, 3000);
    PE_Spu_PushCdAudio(cd, 3000);
    PE_Spu_Render(out, frames);
}
static void test_AUDIO_cd_gain(void)
{
    static int16_t retail[2 * 400], again[2 * 400], half[2 * 400], none[2 * 400];
    TEST("AUDIO_cd_gain");
    ResetTestState();
    ASSERT(PE_Spu_CdGain() == 100, "default CD gain is retail (100)");
    PE_Spu_SetCdGain(100);
    audio_cd_render(100, retail, 400);
    audio_cd_render(100, again, 400);
    ASSERT(memcmp(retail, again, sizeof retail) == 0, "gain 100 takes the unscaled path repeatably");
    audio_cd_render(50, half, 400);
    audio_cd_render(0, none, 400);
    int nonzero = 0, within = 1, silent = 1;
    for (int i = 0; i < 2 * 400; i++) {
        int want = retail[i] / 2, d = half[i] - want;
        nonzero |= retail[i] != 0;
        if (d < -2 || d > 2) within = 0;
        if (none[i] != 0) silent = 0;
    }
    ASSERT(nonzero, "the CD input reaches the mix");
    ASSERT(within, "gain 50 halves the CD contribution (within rounding)");
    ASSERT(silent, "gain 0 silences the CD input");
    PE_Spu_SetCdGain(-3);
    ASSERT(PE_Spu_CdGain() == 0, "gain clamps to 0..100");
    PE_Spu_SetCdGain(70);
    PE_Spu_Reset();
    ASSERT(PE_Spu_CdGain() == 70, "the gain is a host option and survives PE_Spu_Reset");
    PE_Spu_SetCdGain(100);
    PE_Spu_SetActive(0);
    PE_Spu_Reset();
    PASS();
}

/* MUSIC VOLUME / SFX VOLUME.  Fixture: every voice in `kon` plays one
 * looping ADPCM block (filter 0, shift 2, nibble 4 = 4096 per sample) at
 * volume 0x1000 L / 0x0800 R, far below the clamp; the key-on is classed by
 * `src`, which returns `sfx_mask`.  `reverb` adds a minimal reverb whose
 * comb taps read the same-side buffers the input writes, so the wet signal
 * follows the voice (EON set for the keyed voices). */
static uint32_t g_audio_class_mask;
static uint32_t audio_class_source(void) { return g_audio_class_mask; }
static const uint16_t k_audio_class_reverb_regs[][2] = {
    {0x1AA, 0x0080},          /* SPUCNT: reverb master enable */
    {0x1A2, 0x70000 / 8},     /* mBASE: work area 0x70000..0x7FFFF */
    {0x184, 0x4000}, {0x186, 0x4000},   /* vLOUT / vROUT */
    {0x1C4, 0x4000}, {0x1C6, 0x4000},   /* vIIR / vCOMB1 */
    {0x1D4, 0x0010}, {0x1D6, 0x0020},   /* mLSAME / mRSAME */
    {0x1D8, 0x0010}, {0x1DA, 0x0020},   /* mLCOMB1 / mRCOMB1 (= SAME) */
    {0x1FC, 0x4000}, {0x1FE, 0x4000},   /* vLIN / vRIN */
};
#define N_AUDIO_CLASS_REVERB_REGS \
    ((int)(sizeof k_audio_class_reverb_regs / sizeof k_audio_class_reverb_regs[0]))
static void audio_class_render(PeSpuSfxMaskFn src, uint32_t sfx_mask, int music, int sfx,
                               uint32_t kon, int reverb, int16_t *out, int frames)
{
    uint8_t *ram = PE_SpuRam_Data();
    PE_Spu_SetActive(1);
    PE_Spu_Reset();
    PE_Spu_SetSfxMaskSource(src);
    PE_Spu_SetVoiceGains(music, sfx);
    g_audio_class_mask = sfx_mask;
    memset(ram + 0x70000, 0, 0x10000);                     /* reverb work area */
    memset(ram + 0x2000, 0x44, 16);
    ram[0x2000] = 0x02;                                    /* filter 0, shift 2 */
    ram[0x2001] = 0x07;                                    /* loop start + repeat + end */
    for (int n = 0; n < 24; n++) {
        uint32_t r = (uint32_t)n * 16u;
        if (!(kon & (1u << n))) continue;
        PE_SpuRegister_StoreU16(r + 0x0, 0x1000);
        PE_SpuRegister_StoreU16(r + 0x2, 0x0800);
        PE_SpuRegister_StoreU16(r + 0x4, 0x1000);
        PE_SpuRegister_StoreU16(r + 0x6, 0x2000 / 8);
        PE_SpuRegister_StoreU16(r + 0x8, 0x000F);
        PE_SpuRegister_StoreU16(r + 0xA, 0x0000);
    }
    PE_SpuRegister_StoreU16(0x180, 0x3FFF);
    PE_SpuRegister_StoreU16(0x182, 0x3FFF);
    for (int i = 0; i < N_AUDIO_CLASS_REVERB_REGS; i++)
        PE_SpuRegister_StoreU16(k_audio_class_reverb_regs[i][0],
                                reverb ? k_audio_class_reverb_regs[i][1] : 0);
    PE_SpuRegister_StoreU16(0x198, reverb ? (uint16_t)kon : 0);
    PE_SpuRegister_StoreU16(0x19A, reverb ? (uint16_t)(kon >> 16) : 0);
    PE_SpuRegister_StoreU16(0x188, (uint16_t)kon);
    PE_SpuRegister_StoreU16(0x18A, (uint16_t)(kon >> 16));
    PE_Spu_Render(out, frames);
}
static int audio_class_halved(const int16_t *full, const int16_t *half, int n)
{
    int nonzero = 0;
    for (int i = 0; i < n; i++) {
        int d = half[i] - full[i] / 2;
        nonzero |= full[i] != 0;
        if (d < -2 || d > 2) return 0;
    }
    return nonzero;
}
/* mix == x + y within the main-volume rounding (both far below the clamp) */
static int audio_class_sum(const int16_t *mix, const int16_t *x, const int16_t *y, int n)
{
    for (int i = 0; i < n; i++) {
        int d = mix[i] - (x[i] + y[i]);
        if (d < -2 || d > 2) return 0;
    }
    return 1;
}
static void test_AUDIO_voice_class_gains(void)
{
    enum { F = 512, N = 2 * F };
    static int16_t ref[N], a[N], b[N], m100[N], m50[N], s100[N], s50[N];
    const uint32_t music_v = 1u << 3, sfx_v = 1u << 12;
    int silent = 1;
    TEST("AUDIO_voice_class_gains");
    ResetTestState();
    ASSERT(PE_Spu_MusicGain() == 100 && PE_Spu_SfxGain() == 100, "default voice gains are retail (100/100)");

    /* 1. classes come from the SFX mask at the key-on write and are latched */
    audio_class_render(audio_class_source, sfx_v, 100, 100, music_v | sfx_v, 0, a, 16);
    ASSERT(PE_Spu_SfxVoices() == sfx_v, "voice 12 (in the SFX mask) is SFX, voice 3 is music");
    g_audio_class_mask = 0;                       /* the SFX ends: its owner bit clears */
    PE_SpuRegister_StoreU16(0x18C, (uint16_t)sfx_v);   /* key-off: release tail */
    PE_Spu_Render(a, 16);
    ASSERT(PE_Spu_SfxVoices() == sfx_v, "the class holds through key-off and release");
    g_audio_class_mask = music_v;
    PE_SpuRegister_StoreU16(0x18A, 0);            /* empty key-on write */
    ASSERT(PE_Spu_SfxVoices() == sfx_v, "a write that keys nothing reclassifies nothing");
    PE_SpuRegister_StoreU16(0x188, (uint16_t)(music_v | sfx_v));
    ASSERT(PE_Spu_SfxVoices() == music_v, "the next key-on reclassifies each voice from the mask");
    PE_Spu_SetSfxMaskSource(NULL);
    PE_SpuRegister_StoreU16(0x188, (uint16_t)music_v);
    ASSERT(PE_Spu_SfxVoices() == 0, "no source: a keyed voice is music");
    PE_Spu_SetSfxMaskSource(audio_class_source);
    PE_SpuRegister_StoreU16(0x188, (uint16_t)music_v);
    PE_Spu_Reset();
    ASSERT(PE_Spu_SfxVoices() == 0, "latched classes are voice state: PE_Spu_Reset clears them");

    /* 2. at 100/100 the latched classes change nothing, reverb included */
    audio_class_render(NULL, 0, 100, 100, music_v | sfx_v, 1, ref, F);
    audio_class_render(audio_class_source, sfx_v, 100, 100, music_v | sfx_v, 1, a, F);
    ASSERT(PE_Spu_SfxVoices() == sfx_v, "the classed render has an SFX voice");
    ASSERT(memcmp(ref, a, sizeof ref) == 0, "gains 100/100 render bit-identically to no source");

    /* 3. each class gain scales only its own voices */
    audio_class_render(audio_class_source, sfx_v, 100, 100, music_v, 0, m100, F);
    audio_class_render(audio_class_source, sfx_v, 50, 100, music_v, 0, m50, F);
    ASSERT(audio_class_halved(m100, m50, N), "music 50 halves a music voice (within rounding)");
    audio_class_render(audio_class_source, sfx_v, 100, 50, music_v, 0, a, F);
    ASSERT(memcmp(m100, a, sizeof a) == 0, "SFX 50 leaves a music voice exact");
    audio_class_render(audio_class_source, sfx_v, 100, 100, sfx_v, 0, s100, F);
    audio_class_render(audio_class_source, sfx_v, 100, 50, sfx_v, 0, s50, F);
    ASSERT(audio_class_halved(s100, s50, N), "SFX 50 halves an SFX voice (within rounding)");
    audio_class_render(audio_class_source, sfx_v, 50, 100, sfx_v, 0, a, F);
    ASSERT(memcmp(s100, a, sizeof a) == 0, "music 50 leaves an SFX voice exact");
    audio_class_render(audio_class_source, sfx_v, 50, 100, music_v | sfx_v, 0, a, F);
    ASSERT(audio_class_sum(a, m50, s100, N), "one mix at music 50: music voice halved + SFX voice full");
    audio_class_render(audio_class_source, sfx_v, 100, 50, music_v | sfx_v, 0, a, F);
    ASSERT(audio_class_sum(a, m100, s50, N), "one mix at SFX 50: music voice full + SFX voice halved");

    /* ... dry and reverb send alike */
    audio_class_render(audio_class_source, sfx_v, 100, 100, music_v, 1, a, F);
    ASSERT(memcmp(a, m100, sizeof a) != 0, "the reverb send reaches the mix");
    audio_class_render(audio_class_source, sfx_v, 0, 100, music_v, 1, b, F);
    for (int i = 0; i < N; i++) if (b[i] != 0) silent = 0;
    ASSERT(silent, "music 0 silences a music voice, dry and reverb send");
    audio_class_render(audio_class_source, sfx_v, 100, 0, music_v, 1, b, F);
    ASSERT(memcmp(a, b, sizeof a) == 0, "SFX 0 leaves a music voice's dry and reverb send exact");

    /* 4. host options: clamped, kept across PE_Spu_Reset */
    PE_Spu_SetVoiceGains(-5, 140);
    ASSERT(PE_Spu_MusicGain() == 0 && PE_Spu_SfxGain() == 100, "gains clamp to 0..100");
    PE_Spu_SetVoiceGains(70, 40);
    PE_Spu_Reset();
    ASSERT(PE_Spu_MusicGain() == 70 && PE_Spu_SfxGain() == 40, "the gains survive PE_Spu_Reset");

    PE_Spu_SetVoiceGains(100, 100);
    PE_Spu_SetSfxMaskSource(NULL);
    for (int i = 0; i < N_AUDIO_CLASS_REVERB_REGS; i++)
        PE_SpuRegister_StoreU16(k_audio_class_reverb_regs[i][0], 0);
    PE_SpuRegister_StoreU16(0x198, 0);
    PE_SpuRegister_StoreU16(0x19A, 0);
    memset(PE_SpuRam_Data() + 0x70000, 0, 0x10000);
    PE_Spu_SetActive(0);
    PE_Spu_Reset();
    PASS();
}

/* MUSIC/SFX VOLUME on the real AKAO key-on path: a one-tick SFX.  The tick
 * writes KON inside func_80089328 (its first call) after zeroing
 * D_800BCD54, and the SFX sequencer pass later in the same tick can end the
 * SFX (func_80090F68 -> func_8008F1B0 clears its D_800BCD50 bit) before the
 * post-tick commit.  The class must come from D_800BCD50 | D_800BCD60 at
 * the KON write (PE_AudioDriver_SfxVoiceMask). */
void func_80089328(void);                                /* akao_tick_port.c */
void func_8008F1B0(pe_addr_t pe_a0, unsigned int a1);    /* generated port */
#define AUD_CTL 0x80150000u   /* two zeroed 0x68-byte sequence-control records */
static uint32_t g_audio_kon_d54;
static int g_audio_kon_calls;
static uint32_t audio_akao_source(void)
{
    g_audio_kon_calls++;
    g_audio_kon_d54 = PE_LoadU32(0x800BCD54u);
    return PE_AudioDriver_SfxVoiceMask();
}
static void test_AUDIO_one_tick_sfx_class(void)
{
    PeSpuStats s0, s1;
    TEST("AUDIO_one_tick_sfx_class");
    ResetTestState();
    PE_Spu_SetActive(1);
    PE_Spu_Reset();
    PE_Spu_SetSfxMaskSource(audio_akao_source);
    for (uint32_t a = 0; a < 2u * 0x68u; a += 4u) PE_StoreU32(AUD_CTL + a, 0u);
    for (uint32_t a = 0; a < 0x11Cu; a += 4u) PE_StoreU32(0x800BC000u + a, 0u);
    PE_StoreU32(0x8009D2C8u, AUD_CTL);     /* no music channel active */
    PE_StoreU32(0x8009D2C4u, 0u);
    PE_StoreU32(0x800BCD60u, 0u);
    PE_StoreU32(0x800BCD50u, 0x1000u);     /* SFX channel 0 owns voice 12 */
    PE_StoreU32(0x800BCD58u, 0x1000u);     /* ... with a voice update pending */
    PE_StoreU32(0x800BCD54u, 0x1000u);     /* ... and the key-on the last tick queued */
    ASSERT(PE_AudioDriver_SfxVoiceMask() == 0x1000u, "driver mask = D_800BCD50 | D_800BCD60");
    g_audio_kon_calls = 0;
    PE_Spu_GetStats(&s0);
    func_80089328();
    PE_Spu_GetStats(&s1);
    ASSERT(s1.kon_events == s0.kon_events + 1u, "func_80089328 keys voice 12 on");
    ASSERT(g_audio_kon_calls == 1 && g_audio_kon_d54 == 0u,
           "D_800BCD54 is already zero at the key-on write (cannot classify)");
    ASSERT(PE_Spu_SfxVoices() == 0x1000u, "voice 12 latched as SFX at the key-on write");
    func_8008F1B0(0x800BC000u, 0x1000u);   /* the SFX ends later in the same tick */
    pe_plat_audio_commit_registers();      /* the post-tick commit */
    ASSERT(PE_AudioDriver_SfxVoiceMask() == 0u,
           "at the commit the SFX bit is gone (a commit-time read would say music)");
    ASSERT(PE_Spu_SfxVoices() == 0x1000u, "the latched class holds through the release tail");
    PE_StoreU32(0x800BCD60u, 0x1000u);     /* parked SFX (func_8008C8B8) still own voices */
    ASSERT(PE_AudioDriver_SfxVoiceMask() == 0x1000u, "a parked SFX voice is still SFX");
    PE_StoreU32(0x800BCD60u, 0u);
    PE_SpuRegister_StoreU16(0x188, 0x1000u);   /* a later key-on of the free voice */
    ASSERT(PE_Spu_SfxVoices() == 0u, "the next key-on with no SFX owner is music");
    PE_Spu_SetSfxMaskSource(NULL);
    PE_Spu_SetActive(0);
    PE_Spu_Reset();
    PASS();
}

/* fmv lane: PE_Spu_CdInputStop (movie skip / stream end) drops the queued
 * CD input immediately and fades the last frame out over CD_FADE (64)
 * frames instead of playing ~1 s of buffered movie audio. */
static void test_AUDIO_cd_input_stop(void)
{
    TEST("AUDIO_cd_input_stop");
    ResetTestState();
    PE_Spu_SetActive(1);
    PE_Spu_Reset();
    PE_SpuRegister_StoreU16(0x180, 0x3FFF);
    PE_SpuRegister_StoreU16(0x182, 0x3FFF);
    int16_t cd[2 * 3000];
    int16_t out[2 * 512];
    for (int i = 0; i < 2 * 3000; i++) cd[i] = 8000;
    PE_Spu_PushCdAudio(cd, 3000);
    PE_Spu_PushCdAudio(cd, 3000);
    PE_Spu_Render(out, 200);                    /* past the fade-in */
    ASSERT(out[2 * 199] > 7900, "CD input playing before the stop");
    PE_Spu_CdInputStop();
    PE_Spu_Render(out, 100);
    int maxstep = 0, last = 8000;
    for (int i = 0; i < 100; i++) {
        int d = out[2 * i] - last;
        if (d < 0) d = -d;
        if (d > maxstep) maxstep = d;
        last = out[2 * i];
    }
    ASSERT(out[2 * 70] == 0 && out[2 * 99] == 0, "silent within 64 frames");
    ASSERT(maxstep < 300, "stop fades out (no click)");
    PE_Spu_Render(out, 512);
    int any = 0;
    for (int i = 0; i < 1024; i++) any |= out[i] != 0;
    ASSERT(!any, "queued audio was discarded, not replayed");
    PE_Spu_SetActive(0);
    PE_Spu_Reset();
    PASS();
}

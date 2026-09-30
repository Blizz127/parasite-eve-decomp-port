/* Audio lane: native SPU synthesizer + audio-driver interpreter vectors.
 * Expected values are hand-computed from the psx-spx formulas (see the
 * comments beside each vector), not captured from this implementation. */
#include "pe_spu.h"
#include "pe_audio_driver.h"

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
    /* filter 1 (60/64): 16384, (16384*60+32)>>6 = 15360, (15360*60+32)>>6 = 14400 */
    memset(b, 0, sizeof(b)); b[0] = 0x10; b[2] = 0x04;
    h1 = h2 = 0; PE_Spu_DecodeAdpcmBlock(b, &h1, &h2, out);
    ASSERT(out[0] == 16384 && out[1] == 15360 && out[2] == 14400, "filter 1 recursion");
    ASSERT(h1 == out[27] && h2 == out[26], "history carries the last two samples");
    /* filter 2 (115,-52): 16384, 29440, then 39588 clamps to 32767 */
    memset(b, 0, sizeof(b)); b[0] = 0x20; b[2] = 0x04;
    h1 = h2 = 0; PE_Spu_DecodeAdpcmBlock(b, &h1, &h2, out);
    ASSERT(out[0] == 16384 && out[1] == 29440 && out[2] == 32767, "filter 2 + clamp");
    /* history from a previous block feeds the first sample: filter 1,
     * h1 = 1000 -> (1000*60+32)>>6 = 938 */
    memset(b, 0, sizeof(b)); b[0] = 0x10;
    h1 = 1000; h2 = 0; PE_Spu_DecodeAdpcmBlock(b, &h1, &h2, out);
    ASSERT(out[0] == 938, "cross-block history");
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

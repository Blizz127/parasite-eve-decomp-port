/*
 * Host audio output through SDL2 (the SDL host backend; replaces host_audio.c
 * when PE_HOST_BACKEND=sdl2, and always on macOS).  Same API (host_audio.h),
 * same environment (PE_AUDIO_WAV, PE_AUDIO_LATENCY_MS) and the same feed
 * policy as the PulseAudio path in host_audio.c:
 *
 *   - HostAudio_Submit never blocks: frames go into a ring buffer, through a
 *     small adaptive resampler (cubic Hermite, ratio within +-0.5 %) that
 *     steers the ring toward a target fill, so the 60 Hz emulated audio
 *     clock, the 59.94 Hz window pacer and the device crystal never starve
 *     or overfill the stream;
 *   - the SDL audio callback drains the ring; a genuine starve fades out and
 *     re-primes to half the target before fading back in; an overflow drops
 *     the oldest audio with a crossfade;
 *   - fast-forward mutes the device with a fade; the WAV dump is never
 *     resampled, muted or volume-scaled.
 *
 * The ring/resampler and the WAV writer repeat host_audio.c's (static there);
 * docs/port/SDL_BACKEND.md proposes sharing them.  SDL is loaded through
 * HostSDL_Library (host_window_sdl.c) and only when a device is wanted:
 * HostAudio_Open(0) (headless) never touches SDL.  SDL_AUDIODRIVER selects
 * SDL's driver (e.g. "dummy" for a silent real-time sink).
 */
#include "host_audio.h"
#include "host_qol.h"
#include "pe_spu.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MASTER VOLUME and MONO AUDIO (plat_enhance.c).  Weak so a consumer without
 * the enhancement registry keeps the retail level and stereo. */
extern int pe_enh_master_volume(void) __attribute__((weak));
extern int pe_enh_mono(void) __attribute__((weak));
/* host_window_sdl.c: the shared SDL2 dlopen handle (NULL: none). */
void *HostSDL_Library(const char **loaded_from);

/* ── WAV dump (as host_audio.c) ────────────────────────────────────── */

static FILE *g_wav;
static uint64_t g_wav_frames;
static uint64_t g_wav_last_patch;

static void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}

static void wav_header(void)
{
    uint8_t h[44];
    uint64_t bytes64 = g_wav_frames * 4u;
    uint32_t bytes = bytes64 > 0xFFFFFFD0u ? 0xFFFFFFD0u : (uint32_t)bytes64;
    long pos;

    memcpy(h, "RIFF", 4); put32(h + 4, 36u + bytes);
    memcpy(h + 8, "WAVEfmt ", 8); put32(h + 16, 16);
    h[20] = 1; h[21] = 0; h[22] = 2; h[23] = 0;           /* PCM, stereo */
    put32(h + 24, PE_SPU_RATE); put32(h + 28, PE_SPU_RATE * 4u);
    h[32] = 4; h[33] = 0; h[34] = 16; h[35] = 0;
    memcpy(h + 36, "data", 4); put32(h + 40, bytes);
    pos = ftell(g_wav);
    fseek(g_wav, 0, SEEK_SET);
    fwrite(h, 1, sizeof(h), g_wav);
    if (pos > 0) fseek(g_wav, pos, SEEK_SET);
    fflush(g_wav);
}

/* ── SDL2 audio device (dlopen; ABI subset) ────────────────────────── */

#define HS_INIT_AUDIO   0x00000010u
#define HS_AUDIO_S16LSB 0x8010u

typedef void (*HsAudioCallback)(void *userdata, uint8_t *stream, int len);
typedef struct {
    int freq; uint16_t format; uint8_t channels, silence;
    uint16_t samples, padding; uint32_t size;
    HsAudioCallback callback; void *userdata;
} HsAudioSpec;
_Static_assert(sizeof(HsAudioSpec) == 16 + 2 * sizeof(void *), "SDL_AudioSpec layout");

#define RING_FRAMES 16384u          /* ~370 ms */
#define FADE_FRAMES 64u             /* ~1.5 ms click-free ramps */
#define RATE_SPAN   0.005           /* rate control: at most +-0.5 % (8.6 cents) */

static struct {
    int (*InitSubSystem)(uint32_t);
    void (*QuitSubSystem)(uint32_t);
    int (*SetHint)(const char *, const char *);
    const char *(*GetError)(void);
    const char *(*GetCurrentAudioDriver)(void);
    uint32_t (*OpenAudioDevice)(const char *, int, const HsAudioSpec *, HsAudioSpec *, int);
    void (*CloseAudioDevice)(uint32_t);
    void (*PauseAudioDevice)(uint32_t, int);
    void (*LockAudioDevice)(uint32_t);
    void (*UnlockAudioDevice)(uint32_t);
    uint32_t dev;
    int16_t ring[RING_FRAMES * 2];
    uint32_t r, w, n;
    uint32_t target;                /* ring fill the rate control aims at */
    uint32_t cap;                   /* overflow threshold */
    /* producer-side resampler */
    double pos;                     /* fractional read position in hist */
    double fill_avg;
    float hist[4][2];
    int hist_n;
    /* callback state */
    int starved;                    /* re-priming after an underrun */
    int fade_in;                    /* frames of fade-in remaining */
    int16_t last[2];                /* last frame handed to the device */
    int ff;                         /* fast-forward: device muted */
    /* stats */
    uint64_t underruns, overflow_drops, overflow_events, ff_dropped;
    uint64_t in_frames, out_frames;
} A;

static void ring_put(int16_t l, int16_t r)
{
    A.ring[A.w * 2] = l;
    A.ring[A.w * 2 + 1] = r;
    A.w = (A.w + 1u) % RING_FRAMES;
    A.n++;
}

/* Drop the oldest `k` queued frames, crossfading the join (device locked). */
static void ring_drop_oldest(uint32_t k)
{
    uint32_t f = FADE_FRAMES;
    if (k >= A.n) { A.r = A.w; A.n = 0; return; }
    if (A.n - k < f) f = A.n - k;
    for (uint32_t i = 0; i < f; i++) {
        uint32_t a = (A.r + i) % RING_FRAMES, b = (A.r + k + i) % RING_FRAMES;
        int32_t wa = (int32_t)(f - i), wb = (int32_t)(i + 1);
        for (int c = 0; c < 2; c++)
            A.ring[b * 2 + c] = (int16_t)((A.ring[a * 2 + c] * wa +
                                           A.ring[b * 2 + c] * wb) /
                                          (wa + wb));
    }
    A.r = (A.r + k) % RING_FRAMES;
    A.n -= k;
    A.overflow_drops += k;
    A.overflow_events++;
}

static int16_t sat16(float v)
{
    return (int16_t)(v > 32767.f ? 32767 : v < -32768.f ? -32768 : (int)(v + (v >= 0.f ? 0.5f : -0.5f)));
}

/* Silence ramped down from the last frame handed out, so there is no step. */
static void fill_silence(int16_t *out, uint32_t frames)
{
    for (uint32_t i = 0; i < frames; i++) {
        int32_t g = i < FADE_FRAMES ? (int32_t)(FADE_FRAMES - i) : 0;
        out[i * 2] = (int16_t)(A.last[0] * g / (int32_t)(FADE_FRAMES + 1u));
        out[i * 2 + 1] = (int16_t)(A.last[1] * g / (int32_t)(FADE_FRAMES + 1u));
    }
    A.last[0] = A.last[1] = 0;
}

/* SDL's audio thread, with the device lock held (SDL takes it around the
 * callback, and the producer takes it with SDL_LockAudioDevice). */
static void audio_callback(void *userdata, uint8_t *stream, int len)
{
    int16_t *out = (int16_t *)(void *)stream;
    uint32_t frames = len > 0 ? (uint32_t)len / 4u : 0u, k = 0;
    (void)userdata;
    if (A.ff) {                                  /* muted: nothing stays queued */
        A.r = A.w; A.n = 0;
        A.starved = 1;
        fill_silence(out, frames);
        return;
    }
    if (A.starved) {
        if (A.n < A.target / 2u) { fill_silence(out, frames); return; }   /* priming */
        A.starved = 0;
        A.fade_in = (int)FADE_FRAMES;
    }
    while (k < frames && A.n) {
        out[k * 2] = A.ring[A.r * 2];
        out[k * 2 + 1] = A.ring[A.r * 2 + 1];
        A.r = (A.r + 1u) % RING_FRAMES;
        A.n--;
        k++;
    }
    for (uint32_t i = 0; i < k && A.fade_in > 0; i++, A.fade_in--) {
        int32_t g = (int32_t)(FADE_FRAMES - (uint32_t)A.fade_in) + 1;
        out[i * 2] = (int16_t)(out[i * 2] * g / (int32_t)(FADE_FRAMES + 1u));
        out[i * 2 + 1] = (int16_t)(out[i * 2 + 1] * g / (int32_t)(FADE_FRAMES + 1u));
    }
    if (k) { A.last[0] = out[(k - 1) * 2]; A.last[1] = out[(k - 1) * 2 + 1]; }
    if (k < frames) {                            /* ran dry: fade out, re-prime */
        A.starved = 1;
        A.underruns++;
        fill_silence(out + k * 2, frames - k);
    }
}

static int sdl_audio_open(void)
{
    void *lib = HostSDL_Library(NULL);
    const char *lat = getenv("PE_AUDIO_LATENCY_MS"), *drv;
    int ms = lat ? atoi(lat) : 100;
    int dev_ms, ring_ms;
    uint32_t dev_frames, samples, ring_frames;
    HsAudioSpec want, have;

    if (!lib) {
        fprintf(stderr, "[AUDIO] SDL2 unavailable; audio output silent\n");
        return 0;
    }
#define HS_SYM(field, name) if (!(*(void **)&A.field = dlsym(lib, name))) { \
        fprintf(stderr, "[AUDIO] SDL2 missing symbol %s; audio silent\n", name); return 0; }
    HS_SYM(InitSubSystem, "SDL_InitSubSystem");   HS_SYM(QuitSubSystem, "SDL_QuitSubSystem");
    HS_SYM(SetHint, "SDL_SetHint");               HS_SYM(GetError, "SDL_GetError");
    HS_SYM(GetCurrentAudioDriver, "SDL_GetCurrentAudioDriver");
    HS_SYM(OpenAudioDevice, "SDL_OpenAudioDevice"); HS_SYM(CloseAudioDevice, "SDL_CloseAudioDevice");
    HS_SYM(PauseAudioDevice, "SDL_PauseAudioDevice");
    HS_SYM(LockAudioDevice, "SDL_LockAudioDevice"); HS_SYM(UnlockAudioDevice, "SDL_UnlockAudioDevice");
#undef HS_SYM
    if (ms < 40) ms = 40;
    if (ms > 500) ms = 500;
    dev_ms = ms * 3 / 5;             /* device share, as the PulseAudio path */
    ring_ms = ms - dev_ms;           /* producer-side ring target */
    /* The callback period: a power of two near half the device share. */
    dev_frames = (uint32_t)(PE_SPU_RATE * (uint32_t)dev_ms / 1000u);
    for (samples = 256u; samples * 2u <= dev_frames / 2u && samples < 4096u; samples *= 2u) {}
    A.SetHint("SDL_NO_SIGNAL_HANDLERS", "1");
    A.SetHint("SDL_AUDIO_DEVICE_APP_NAME", "Parasite Eve");
    A.SetHint("SDL_AUDIO_DEVICE_STREAM_NAME", "game audio");
    if (A.InitSubSystem(HS_INIT_AUDIO) != 0) {
        fprintf(stderr, "[AUDIO] SDL audio init failed (%s); audio silent\n", A.GetError());
        return 0;
    }
    memset(&want, 0, sizeof(want));
    memset(&have, 0, sizeof(have));
    want.freq = PE_SPU_RATE;
    want.format = HS_AUDIO_S16LSB;
    want.channels = 2;
    want.samples = (uint16_t)samples;
    want.callback = audio_callback;
    /* allowed_changes 0: SDL converts to whatever the hardware runs. */
    A.dev = A.OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!A.dev) {
        fprintf(stderr, "[AUDIO] SDL_OpenAudioDevice failed (%s); audio silent\n", A.GetError());
        A.QuitSubSystem(HS_INIT_AUDIO);
        return 0;
    }
    ring_frames = (uint32_t)(PE_SPU_RATE * (uint32_t)ring_ms / 1000u);
    A.target = ring_frames;
    if (A.target < 2u * (uint32_t)have.samples) A.target = 2u * (uint32_t)have.samples;
    if (A.target < 1024u) A.target = 1024u;
    A.cap = A.target * 4u;
    if (A.cap > RING_FRAMES - 1024u) A.cap = RING_FRAMES - 1024u;
    A.fill_avg = A.target;
    A.pos = 0.0;
    A.hist_n = 0;
    A.starved = 1;                   /* prime to target/2, then fade in */
    A.PauseAudioDevice(A.dev, 0);
    drv = A.GetCurrentAudioDriver();
    fprintf(stderr, "[AUDIO] SDL audio output open (driver %s, %d Hz stereo, callback %u frames, "
            "~%d ms: device %d + queue %u frames, adaptive rate +-0.5%%)\n",
            drv ? drv : "?", have.freq, (unsigned)have.samples, ms, dev_ms, (unsigned)A.target);
    return 1;
}

static void sdl_submit(const int16_t *s, int frames)
{
    double step;
    A.LockAudioDevice(A.dev);
    A.in_frames += (uint64_t)frames;
    if (A.ff) {
        /* muted: the resampler restarts cleanly on resume */
        A.ff_dropped += (uint64_t)frames;
        A.hist_n = 0;
        A.pos = 0.0;
        A.fill_avg = A.target;
        A.UnlockAudioDevice(A.dev);
        return;
    }
    /* Smoothed fill -> resampling step (>1 consumes input faster). */
    A.fill_avg += ((double)A.n - A.fill_avg) * 0.05;
    {
        double e = (A.fill_avg - (double)A.target) / (double)A.target;
        if (e > 1.0) e = 1.0;
        if (e < -1.0) e = -1.0;
        step = 1.0 + RATE_SPAN * e;
    }
    for (int i = 0; i < frames; i++) {
        memmove(A.hist[0], A.hist[1], sizeof(A.hist[0]) * 3u);
        A.hist[3][0] = (float)s[i * 2];
        A.hist[3][1] = (float)s[i * 2 + 1];
        if (A.hist_n < 4) {
            if (++A.hist_n < 4) continue;
            memcpy(A.hist[0], A.hist[1], sizeof(A.hist[0]));  /* ramp-safe start */
        }
        /* output points lie between hist[1] and hist[2] */
        while (A.pos < 1.0) {
            float t = (float)A.pos, o[2];
            for (int c = 0; c < 2; c++) {
                float y0 = A.hist[0][c], y1 = A.hist[1][c];
                float y2 = A.hist[2][c], y3 = A.hist[3][c];
                float a = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
                float b = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
                float cc = -0.5f * y0 + 0.5f * y2;
                o[c] = ((a * t + b) * t + cc) * t + y1;
            }
            if (A.n >= A.cap) ring_drop_oldest(A.n - A.target);
            ring_put(sat16(o[0]), sat16(o[1]));
            A.out_frames++;
            A.pos += step;
        }
        A.pos -= 1.0;
    }
    A.UnlockAudioDevice(A.dev);
}

static void sdl_audio_close(void)
{
    if (!A.dev) return;
    A.CloseAudioDevice(A.dev);       /* stops the callback */
    A.dev = 0;
    fprintf(stderr, "[AUDIO] output: in=%llu out=%llu underruns=%llu "
            "overflow_events=%llu overflow_dropped=%llu ff_muted=%llu\n",
            (unsigned long long)A.in_frames, (unsigned long long)A.out_frames,
            (unsigned long long)A.underruns,
            (unsigned long long)A.overflow_events,
            (unsigned long long)A.overflow_drops,
            (unsigned long long)A.ff_dropped);
    A.QuitSubSystem(HS_INIT_AUDIO);
}

/* ── public ────────────────────────────────────────────────────────── */

static int g_open, g_device;

int HostAudio_Open(int want_device)
{
    const char *wav = getenv("PE_AUDIO_WAV");
    if (g_open) return 1;
    if (wav && *wav) {
        g_wav = fopen(wav, "wb");
        if (g_wav) {
            wav_header();
            fprintf(stderr, "[AUDIO] writing WAV dump %s\n", wav);
        } else {
            fprintf(stderr, "[AUDIO] cannot open PE_AUDIO_WAV %s\n", wav);
        }
    }
    if (want_device) g_device = sdl_audio_open();
    g_open = 1;
    atexit(HostAudio_Close);
    return g_wav != NULL || g_device;
}

int HostAudio_DeviceOpen(void) { return g_device; }

int HostAudio_QueueFrames(void)
{
    int n;
    if (!g_device) return -1;
    A.LockAudioDevice(A.dev);
    n = (int)A.n;
    A.UnlockAudioDevice(A.dev);
    return n;
}

void HostAudio_SetFastForward(int on)
{
    if (g_device && A.ff != (on != 0)) {
        A.LockAudioDevice(A.dev);
        A.ff = on != 0;
        A.UnlockAudioDevice(A.dev);
    }
}

void HostAudio_Submit(const int16_t *stereo, int frames)
{
    if (frames <= 0) return;
    if (g_wav) {
        fwrite(stereo, 4, (size_t)frames, g_wav);
        g_wav_frames += (uint64_t)frames;
        if (g_wav_frames - g_wav_last_patch >= PE_SPU_RATE) {
            g_wav_last_patch = g_wav_frames;
            wav_header();
        }
    }
    if (g_device) {
        static int last = 100;     /* the level the previous submit ended at */
        int percent = pe_enh_master_volume ? pe_enh_master_volume() : 100;
        int mono = pe_enh_mono ? pe_enh_mono() : 0;
        if (percent == 100 && last == 100 && !mono) {
            sdl_submit(stereo, frames);
        } else {
            /* Fixed chunks on the stack: no allocation that could fail open. */
            int16_t chunk[2 * 256];
            for (int done = 0; done < frames; done += 256) {
                int n = frames - done < 256 ? frames - done : 256;
                if (percent == 100 && last == 100)
                    memcpy(chunk, stereo + 2 * done, sizeof(int16_t) * 2 * (size_t)n);
                else
                    HostAudio_GainRamp(chunk, stereo, done, n, frames, last, percent);
                /* MONO AUDIO: after the volume stages, on the device feed only */
                if (mono) HostQol_Downmix(chunk, chunk, n);
                sdl_submit(chunk, n);
            }
        }
        last = percent;
    }
}

void HostAudio_Close(void)
{
    if (g_wav) {
        wav_header();
        fclose(g_wav);
        g_wav = NULL;
    }
    if (g_device) sdl_audio_close();
    g_device = 0;
}

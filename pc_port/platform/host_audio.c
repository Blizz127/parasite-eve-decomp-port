/* Host audio output — see host_audio.h. */
#include "host_audio.h"
#include "pe_spu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <dlfcn.h>
#include <pthread.h>
#include <time.h>
#endif

/* ── WAV dump ──────────────────────────────────────────────────────── */

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

/* ── PulseAudio (dlopen) ───────────────────────────────────────────── */

#ifndef _WIN32
typedef struct { int format; uint32_t rate; uint8_t channels; } pa_sample_spec_t;
typedef struct { uint32_t maxlength, tlength, prebuf, minreq, fragsize; } pa_buffer_attr_t;
typedef void *(*pa_simple_new_fn)(const char *, const char *, int, const char *,
                                  const char *, const pa_sample_spec_t *,
                                  const void *, const pa_buffer_attr_t *, int *);
typedef int (*pa_simple_write_fn)(void *, const void *, size_t, int *);
typedef void (*pa_simple_free_fn)(void *);
typedef uint64_t (*pa_simple_get_latency_fn)(void *, int *);

#define RING_FRAMES 16384u          /* ~370 ms */
#define FADE_FRAMES 64u             /* ~1.5 ms click-free ramps */
#define WRITE_CHUNK 256u            /* frames per pa_simple_write */
#define RATE_SPAN   0.005           /* rate control: at most +-0.5 % (8.6 cents) */

/*
 * Output pacing.  The game renders 735 frames per emulated VBlank on a
 * 60 Hz cycle budget while the window pacer presents at 59.94 Hz, frames
 * can run long (the pacer then discards the backlog) and the device clock
 * is its own crystal.  Feeding the device raw therefore either starves it
 * (silence inserted mid-stream) or overfills it (samples dropped) — both
 * heard as crackle.  The producer side runs a small adaptive resampler
 * (cubic Hermite, ratio within +-0.5 %) steering the ring toward a target
 * fill; a genuine starve fades out and re-primes before a fade-in, and an
 * overflow drops the oldest audio with a crossfade.  Fast-forward mutes the
 * device with a fade; the WAV dump is never resampled or muted.
 */
static struct {
    void *lib;
    void *pa;
    pa_simple_write_fn write;
    pa_simple_free_fn free_;
    pa_simple_get_latency_fn latency;   /* optional */
    pthread_t thread;
    pthread_mutex_t mu;
    pthread_cond_t cv;
    int running;
    int16_t ring[RING_FRAMES * 2];
    uint32_t r, w, n;
    uint32_t target;                /* ring fill the rate control aims at */
    uint32_t cap;                   /* overflow threshold */
    /* producer-side resampler */
    double pos;                     /* fractional read position in hist */
    double fill_avg;
    float hist[4][2];
    int hist_n;
    /* writer state */
    int starved;                    /* re-priming after an underrun */
    int fade_in;                    /* frames of fade-in remaining */
    int16_t last[2];                /* last frame handed to the device */
    volatile int ff;                /* fast-forward: device muted */
    /* stats */
    uint64_t underruns, overflow_drops, overflow_events, ff_dropped;
    uint64_t in_frames, out_frames;
} P;

static void ring_put(int16_t l, int16_t r)
{
    P.ring[P.w * 2] = l;
    P.ring[P.w * 2 + 1] = r;
    P.w = (P.w + 1u) % RING_FRAMES;
    P.n++;
}

/* Drop the oldest `k` queued frames, crossfading the join (mutex held). */
static void ring_drop_oldest(uint32_t k)
{
    uint32_t f = FADE_FRAMES;
    if (k >= P.n) { P.r = P.w; P.n = 0; return; }
    if (P.n - k < f) f = P.n - k;
    for (uint32_t i = 0; i < f; i++) {
        uint32_t a = (P.r + i) % RING_FRAMES, b = (P.r + k + i) % RING_FRAMES;
        int32_t wa = (int32_t)(f - i), wb = (int32_t)(i + 1);
        for (int c = 0; c < 2; c++)
            P.ring[b * 2 + c] = (int16_t)((P.ring[a * 2 + c] * wa +
                                           P.ring[b * 2 + c] * wb) /
                                          (wa + wb));
    }
    P.r = (P.r + k) % RING_FRAMES;
    P.n -= k;
    P.overflow_drops += k;
    P.overflow_events++;
}

static int16_t sat16(float v)
{
    return (int16_t)(v > 32767.f ? 32767 : v < -32768.f ? -32768 : (int)(v + (v >= 0.f ? 0.5f : -0.5f)));
}

static void *writer(void *arg)
{
    int16_t chunk[WRITE_CHUNK * 2];
    (void)arg;
    for (;;) {
        uint32_t k = 0, need;
        int starve_now = 0;
        pthread_mutex_lock(&P.mu);
        need = P.starved ? P.target / 2u : 1u;
        if (!P.ff && P.n < need && !P.starved) {
            /* Ring empty but the device may still hold tens of ms: wait
             * for the producer while it does, starve only when it runs
             * low (without get_latency: one short wait). */
            for (int tries = 0; P.running && !P.ff && P.n < need; tries++) {
                struct timespec ts;
                uint64_t lat_us = 0;
                if (P.latency) {
                    pthread_mutex_unlock(&P.mu);
                    lat_us = P.latency(P.pa, NULL);
                    pthread_mutex_lock(&P.mu);
                    if (P.n >= need) break;
                    if (lat_us < 12000u) break;
                } else if (tries >= 1) {
                    break;
                }
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_nsec += 2 * 1000000L;
                if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
                (void)pthread_cond_timedwait(&P.cv, &P.mu, &ts);
            }
        }
        if (!P.running) { pthread_mutex_unlock(&P.mu); break; }
        if (P.ff) {
            P.r = P.w; P.n = 0;          /* nothing queued while muted */
            P.starved = 1;
            starve_now = 1;
        } else if (P.n >= need) {
            if (P.starved) { P.starved = 0; P.fade_in = (int)FADE_FRAMES; }
            while (k < WRITE_CHUNK && P.n) {
                chunk[k * 2] = P.ring[P.r * 2];
                chunk[k * 2 + 1] = P.ring[P.r * 2 + 1];
                P.r = (P.r + 1u) % RING_FRAMES;
                P.n--;
                k++;
            }
        } else {
            /* starved (or priming): keep the device fed with silence; the
             * blocking write paces this loop at real time. */
            if (!P.starved) { P.starved = 1; P.underruns++; }
            starve_now = 1;
        }
        pthread_mutex_unlock(&P.mu);
        if (k) {
            for (uint32_t i = 0; i < k && P.fade_in > 0; i++, P.fade_in--) {
                int32_t g = (int32_t)(FADE_FRAMES - (uint32_t)P.fade_in) + 1;
                chunk[i * 2] = (int16_t)(chunk[i * 2] * g / (int32_t)(FADE_FRAMES + 1u));
                chunk[i * 2 + 1] = (int16_t)(chunk[i * 2 + 1] * g / (int32_t)(FADE_FRAMES + 1u));
            }
            P.last[0] = chunk[(k - 1) * 2];
            P.last[1] = chunk[(k - 1) * 2 + 1];
        } else if (starve_now) {
            /* Silence, ramped down from the last frame so there is no step.
             * Short chunks keep the device fed without adding latency. */
            k = 128u;
            for (uint32_t i = 0; i < k; i++) {
                int32_t g = i < FADE_FRAMES ? (int32_t)(FADE_FRAMES - i) : 0;
                chunk[i * 2] = (int16_t)(P.last[0] * g / (int32_t)(FADE_FRAMES + 1u));
                chunk[i * 2 + 1] = (int16_t)(P.last[1] * g / (int32_t)(FADE_FRAMES + 1u));
            }
            P.last[0] = P.last[1] = 0;
        }
        if (k && P.write(P.pa, chunk, k * 4u, NULL) < 0) break;
    }
    return NULL;
}

static int pulse_open(void)
{
    pa_simple_new_fn pnew;
    pa_sample_spec_t spec = {3 /* PA_SAMPLE_S16LE */, PE_SPU_RATE, 2};
    pa_buffer_attr_t attr;
    const char *lat = getenv("PE_AUDIO_LATENCY_MS");
    int ms = lat ? atoi(lat) : 100;
    int dev_ms, ring_ms;
    int err = 0;

    if (ms < 40) ms = 40;
    if (ms > 500) ms = 500;
    dev_ms = ms * 3 / 5;             /* device share (PipeWire quantum room) */
    ring_ms = ms - dev_ms;           /* producer-side ring target */
    P.lib = dlopen("libpulse-simple.so.0", RTLD_NOW | RTLD_LOCAL);
    if (!P.lib) {
        fprintf(stderr, "[AUDIO] libpulse-simple.so.0 unavailable (%s); "
                "audio output silent\n", dlerror());
        return 0;
    }
    pnew = (pa_simple_new_fn)dlsym(P.lib, "pa_simple_new");
    P.write = (pa_simple_write_fn)dlsym(P.lib, "pa_simple_write");
    P.free_ = (pa_simple_free_fn)dlsym(P.lib, "pa_simple_free");
    P.latency = (pa_simple_get_latency_fn)dlsym(P.lib, "pa_simple_get_latency");
    if (!pnew || !P.write || !P.free_) {
        fprintf(stderr, "[AUDIO] libpulse-simple symbols missing; audio silent\n");
        dlclose(P.lib); P.lib = NULL;
        return 0;
    }
    attr.maxlength = (uint32_t)-1;
    attr.tlength = (uint32_t)(PE_SPU_RATE * 4u * (uint32_t)dev_ms / 1000u);
    attr.prebuf = (uint32_t)-1;
    attr.minreq = (uint32_t)-1;
    attr.fragsize = (uint32_t)-1;
    P.pa = pnew(NULL, "Parasite Eve", 1 /* PA_STREAM_PLAYBACK */, NULL,
                "game audio", &spec, NULL, &attr, &err);
    if (!P.pa) {
        fprintf(stderr, "[AUDIO] pa_simple_new failed (error %d); audio silent\n", err);
        dlclose(P.lib); P.lib = NULL;
        return 0;
    }
    P.target = (uint32_t)(PE_SPU_RATE * (uint32_t)ring_ms / 1000u);
    if (P.target < 1024u) P.target = 1024u;
    P.cap = P.target * 4u;
    if (P.cap > RING_FRAMES - 1024u) P.cap = RING_FRAMES - 1024u;
    P.fill_avg = P.target;
    P.pos = 0.0;
    P.hist_n = 0;
    P.starved = 1;                   /* prime to target/2, then fade in */
    pthread_mutex_init(&P.mu, NULL);
    pthread_cond_init(&P.cv, NULL);
    P.running = 1;
    if (pthread_create(&P.thread, NULL, writer, NULL) != 0) {
        P.running = 0;
        P.free_(P.pa); P.pa = NULL;
        return 0;
    }
    fprintf(stderr, "[AUDIO] PulseAudio/PipeWire output open (44100 Hz stereo, "
            "~%d ms: device %d + queue %d, adaptive rate +-0.5%%)\n", ms, dev_ms,
            ring_ms);
    return 1;
}

static void pulse_submit(const int16_t *s, int frames)
{
    double step;
    pthread_mutex_lock(&P.mu);
    P.in_frames += (uint64_t)frames;
    if (P.ff) {
        /* muted: the resampler restarts cleanly on resume */
        P.ff_dropped += (uint64_t)frames;
        P.hist_n = 0;
        P.pos = 0.0;
        P.fill_avg = P.target;
        pthread_cond_signal(&P.cv);
        pthread_mutex_unlock(&P.mu);
        return;
    }
    /* Smoothed fill -> resampling step (>1 consumes input faster). */
    P.fill_avg += ((double)P.n - P.fill_avg) * 0.05;
    {
        double e = (P.fill_avg - (double)P.target) / (double)P.target;
        if (e > 1.0) e = 1.0;
        if (e < -1.0) e = -1.0;
        step = 1.0 + RATE_SPAN * e;
    }
    for (int i = 0; i < frames; i++) {
        memmove(P.hist[0], P.hist[1], sizeof(P.hist[0]) * 3u);
        P.hist[3][0] = (float)s[i * 2];
        P.hist[3][1] = (float)s[i * 2 + 1];
        if (P.hist_n < 4) {
            if (++P.hist_n < 4) continue;
            memcpy(P.hist[0], P.hist[1], sizeof(P.hist[0]));  /* ramp-safe start */
        }
        /* output points lie between hist[1] and hist[2] */
        while (P.pos < 1.0) {
            float t = (float)P.pos, o[2];
            for (int c = 0; c < 2; c++) {
                float y0 = P.hist[0][c], y1 = P.hist[1][c];
                float y2 = P.hist[2][c], y3 = P.hist[3][c];
                float a = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
                float b = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
                float cc = -0.5f * y0 + 0.5f * y2;
                o[c] = ((a * t + b) * t + cc) * t + y1;
            }
            if (P.n >= P.cap) ring_drop_oldest(P.n - P.target);
            ring_put(sat16(o[0]), sat16(o[1]));
            P.out_frames++;
            P.pos += step;
        }
        P.pos -= 1.0;
    }
    pthread_cond_signal(&P.cv);
    pthread_mutex_unlock(&P.mu);
}

static void pulse_close(void)
{
    if (!P.pa) return;
    pthread_mutex_lock(&P.mu);
    P.running = 0;
    pthread_cond_signal(&P.cv);
    pthread_mutex_unlock(&P.mu);
    pthread_join(P.thread, NULL);
    fprintf(stderr, "[AUDIO] output: in=%llu out=%llu underruns=%llu "
            "overflow_events=%llu overflow_dropped=%llu ff_muted=%llu\n",
            (unsigned long long)P.in_frames, (unsigned long long)P.out_frames,
            (unsigned long long)P.underruns,
            (unsigned long long)P.overflow_events,
            (unsigned long long)P.overflow_drops,
            (unsigned long long)P.ff_dropped);
    P.free_(P.pa);
    P.pa = NULL;
}
#endif

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
#ifndef _WIN32
    if (want_device) g_device = pulse_open();
#else
    (void)want_device;
#endif
    g_open = 1;
    atexit(HostAudio_Close);
    return g_wav != NULL || g_device;
}

int HostAudio_DeviceOpen(void) { return g_device; }

int HostAudio_QueueFrames(void)
{
#ifndef _WIN32
    int n;
    if (!g_device) return -1;
    pthread_mutex_lock(&P.mu);
    n = (int)P.n;
    pthread_mutex_unlock(&P.mu);
    return n;
#else
    return -1;
#endif
}

void HostAudio_SetFastForward(int on)
{
#ifndef _WIN32
    if (g_device && P.ff != (on != 0)) {
        pthread_mutex_lock(&P.mu);
        P.ff = on != 0;
        pthread_cond_signal(&P.cv);
        pthread_mutex_unlock(&P.mu);
    }
#else
    (void)on;
#endif
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
#ifndef _WIN32
    if (g_device) pulse_submit(stereo, frames);
#endif
}

void HostAudio_Close(void)
{
    if (g_wav) {
        wav_header();
        fclose(g_wav);
        g_wav = NULL;
    }
#ifndef _WIN32
    if (g_device) pulse_close();
#endif
    g_device = 0;
}

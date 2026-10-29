/*
 * Host audio output on Windows: the host_audio.h API with a WinMM waveOut
 * device (winmm.dll ships with every Windows).  cmake/windows.cmake builds
 * this file instead of host_audio.c, whose device is PulseAudio (dlopen +
 * pthreads); the WAV dump below is byte-for-byte the same format and refresh
 * schedule as host_audio.c's, so PE_AUDIO_WAV dumps compare across OSes.
 *
 * Device feed (the same design as the Pulse backend): HostAudio_Submit never
 * blocks.  It runs a small adaptive resampler (cubic Hermite, ratio within
 * +-0.5 %) that steers a lock-protected ring toward a target fill, so the
 * 60 Hz emulated audio clock, the 59.94 Hz window pacer and the device clock
 * never starve or overfill the stream; an overflow drops the oldest audio
 * with a crossfade.  A writer thread, woken by waveOut's buffer-done event,
 * hands ring audio to the device in short buffers; while the ring is short
 * it plays a faded tail and silence and then fades back in.  Fast-forward
 * mutes the device (submitted frames are discarded); the WAV dump is never
 * resampled, muted or volume-scaled.
 *
 * Environment: PE_AUDIO_WAV, PE_AUDIO_LATENCY_MS (default 100, 40..500;
 * 60 % device buffers, 40 % ring), as in host_audio.h.
 */
#ifdef _WIN32
#include "host_audio.h"
#include "host_qol.h"
#include "pe_spu.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MASTER VOLUME and MONO AUDIO (plat_enhance.c).  Weak so a consumer without
 * the enhancement registry keeps the retail level and stereo. */
extern int pe_enh_master_volume(void) __attribute__((weak));
extern int pe_enh_mono(void) __attribute__((weak));

/* ── WAV dump (identical to host_audio.c) ─────────────────────────── */

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

/* ── WinMM waveOut device ─────────────────────────────────────────── */

#define RING_FRAMES 16384u          /* ~370 ms */
#define FADE_FRAMES 64u             /* ~1.5 ms click-free ramps */
#define SILENCE_FRAMES 128u         /* a starved buffer: short, so latency stays low */
#define MAX_BUFS 32
#define RATE_SPAN 0.005             /* rate control: at most +-0.5 % */

static struct {
    HWAVEOUT wo;
    HANDLE done_event, thread;
    CRITICAL_SECTION mu;
    volatile LONG running;
    int nbufs;
    uint32_t buf_frames;
    WAVEHDR hdr[MAX_BUFS];
    int16_t *mem[MAX_BUFS];
    int16_t ring[RING_FRAMES * 2];
    uint32_t r, w, n;
    uint32_t target;                /* ring fill the rate control aims at */
    uint32_t cap;                   /* overflow threshold */
    /* producer-side resampler */
    double pos, fill_avg;
    float hist[4][2];
    int hist_n;
    /* writer state */
    int starved, fade_in;
    int16_t last[2];
    volatile LONG ff;               /* fast-forward: device muted */
    /* stats */
    uint64_t underruns, overflow_drops, overflow_events, ff_dropped, in_frames, out_frames;
} D;

static void ring_put(int16_t l, int16_t r)
{
    D.ring[D.w * 2] = l;
    D.ring[D.w * 2 + 1] = r;
    D.w = (D.w + 1u) % RING_FRAMES;
    D.n++;
}

/* Drop the oldest `k` queued frames, crossfading the join (lock held). */
static void ring_drop_oldest(uint32_t k)
{
    uint32_t f = FADE_FRAMES;
    if (k >= D.n) { D.r = D.w; D.n = 0; return; }
    if (D.n - k < f) f = D.n - k;
    for (uint32_t i = 0; i < f; i++) {
        uint32_t a = (D.r + i) % RING_FRAMES, b = (D.r + k + i) % RING_FRAMES;
        int32_t wa = (int32_t)(f - i), wb = (int32_t)(i + 1);
        for (int c = 0; c < 2; c++)
            D.ring[b * 2 + c] = (int16_t)((D.ring[a * 2 + c] * wa + D.ring[b * 2 + c] * wb) / (wa + wb));
    }
    D.r = (D.r + k) % RING_FRAMES;
    D.n -= k;
    D.overflow_drops += k;
    D.overflow_events++;
}

static int16_t sat16(float v)
{
    return (int16_t)(v > 32767.f ? 32767 : v < -32768.f ? -32768 : (int)(v + (v >= 0.f ? 0.5f : -0.5f)));
}

/* Fill one free buffer (lock not held on entry).  `queued` is how many
 * buffers the device still holds.  Like the Pulse writer, an empty ring is a
 * starve only when the device is about to run dry: while two or more
 * buffers (>= 20 ms) are still queued the buffer stays free and the writer
 * waits for the producer, and a short ring is not sent as a tiny buffer.
 * Returns the frames written to `out`: ring audio, or (starved, device low)
 * a faded tail of the last frame followed by silence; 0 = nothing yet. */
static uint32_t fill_buffer(int16_t *out, int queued)
{
    uint32_t k = 0, need;
    int starve_now = 0, low = queued <= 1;
    EnterCriticalSection(&D.mu);
    need = D.starved ? D.target / 2u : 1u;
    if (D.ff) {
        D.r = D.w; D.n = 0;          /* nothing queued while muted */
        D.starved = 1;
        starve_now = low;
    } else if (D.n >= need && (D.starved || D.n >= D.buf_frames || low)) {
        if (D.starved) { D.starved = 0; D.fade_in = (int)FADE_FRAMES; }
        while (k < D.buf_frames && D.n) {
            out[k * 2] = D.ring[D.r * 2];
            out[k * 2 + 1] = D.ring[D.r * 2 + 1];
            D.r = (D.r + 1u) % RING_FRAMES;
            D.n--;
            k++;
        }
    } else if (low) {
        /* starved (or priming): keep the device fed with short silence */
        if (!D.starved) { D.starved = 1; D.underruns++; }
        starve_now = 1;
    }
    LeaveCriticalSection(&D.mu);
    if (k) {
        for (uint32_t i = 0; i < k && D.fade_in > 0; i++, D.fade_in--) {
            int32_t g = (int32_t)(FADE_FRAMES - (uint32_t)D.fade_in) + 1;
            out[i * 2] = (int16_t)(out[i * 2] * g / (int32_t)(FADE_FRAMES + 1u));
            out[i * 2 + 1] = (int16_t)(out[i * 2 + 1] * g / (int32_t)(FADE_FRAMES + 1u));
        }
        D.last[0] = out[(k - 1) * 2];
        D.last[1] = out[(k - 1) * 2 + 1];
    } else if (starve_now) {
        k = SILENCE_FRAMES;
        for (uint32_t i = 0; i < k; i++) {
            int32_t g = i < FADE_FRAMES ? (int32_t)(FADE_FRAMES - i) : 0;
            out[i * 2] = (int16_t)(D.last[0] * g / (int32_t)(FADE_FRAMES + 1u));
            out[i * 2 + 1] = (int16_t)(D.last[1] * g / (int32_t)(FADE_FRAMES + 1u));
        }
        D.last[0] = D.last[1] = 0;
    }
    return k;
}

static DWORD WINAPI writer(LPVOID arg)
{
    (void)arg;
    while (D.running) {
        int queued = 0;
        for (int i = 0; i < D.nbufs; i++)
            if (D.hdr[i].dwFlags & WHDR_INQUEUE) queued++;
        for (int i = 0; i < D.nbufs && D.running; i++) {
            WAVEHDR *h = &D.hdr[i];
            uint32_t k;
            if (h->dwFlags & WHDR_INQUEUE) continue;
            k = fill_buffer(D.mem[i], queued);
            if (!k) continue;            /* device still busy: wait for the producer */
            if (h->dwFlags & WHDR_PREPARED) waveOutUnprepareHeader(D.wo, h, sizeof(*h));
            memset(h, 0, sizeof(*h));
            h->lpData = (LPSTR)D.mem[i];
            h->dwBufferLength = k * 4u;
            if (waveOutPrepareHeader(D.wo, h, sizeof(*h)) != MMSYSERR_NOERROR ||
                waveOutWrite(D.wo, h, sizeof(*h)) != MMSYSERR_NOERROR) {
                fprintf(stderr, "[AUDIO] waveOutWrite failed; audio output stops\n");
                InterlockedExchange(&D.running, 0);
                break;
            }
            queued++;
        }
        /* Woken when the device finishes a buffer or the producer submits
         * (both signal done_event); the timeout re-checks shutdown and a
         * coalesced signal. */
        if (D.running) WaitForSingleObject(D.done_event, 5u);
    }
    return 0;
}

static int winmm_open(void)
{
    WAVEFORMATEX fmt;
    const char *lat = getenv("PE_AUDIO_LATENCY_MS");
    int ms = lat ? atoi(lat) : 100;
    int dev_ms, ring_ms;
    MMRESULT mr;

    if (ms < 40) ms = 40;
    if (ms > 500) ms = 500;
    dev_ms = ms * 3 / 5;             /* device buffers */
    ring_ms = ms - dev_ms;           /* producer-side ring target */
    D.buf_frames = 441u;             /* 10 ms per buffer */
    D.nbufs = dev_ms / 10;
    if (D.nbufs < 3) D.nbufs = 3;
    if (D.nbufs > MAX_BUFS) D.nbufs = MAX_BUFS;

    memset(&fmt, 0, sizeof(fmt));
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = 2;
    fmt.nSamplesPerSec = PE_SPU_RATE;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = 4;
    fmt.nAvgBytesPerSec = PE_SPU_RATE * 4u;
    D.done_event = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (!D.done_event) {
        fprintf(stderr, "[AUDIO] CreateEvent failed (%lu); audio output silent\n",
                (unsigned long)GetLastError());
        return 0;
    }
    mr = waveOutOpen(&D.wo, WAVE_MAPPER, &fmt, (DWORD_PTR)D.done_event, 0, CALLBACK_EVENT);
    if (mr != MMSYSERR_NOERROR) {
        fprintf(stderr, "[AUDIO] waveOutOpen failed (MMRESULT %u, %u output device(s)); "
                "audio output silent\n", (unsigned)mr, (unsigned)waveOutGetNumDevs());
        CloseHandle(D.done_event); D.done_event = NULL;
        D.wo = NULL;
        return 0;
    }
    for (int i = 0; i < D.nbufs; i++) {
        D.mem[i] = (int16_t *)calloc(D.buf_frames > SILENCE_FRAMES ? D.buf_frames : SILENCE_FRAMES, 4u);
        if (!D.mem[i]) {
            for (int j = 0; j < i; j++) { free(D.mem[j]); D.mem[j] = NULL; }
            waveOutClose(D.wo); D.wo = NULL;
            CloseHandle(D.done_event); D.done_event = NULL;
            fprintf(stderr, "[AUDIO] out of memory; audio output silent\n");
            return 0;
        }
    }
    D.target = (uint32_t)(PE_SPU_RATE * (uint32_t)ring_ms / 1000u);
    if (D.target < 1024u) D.target = 1024u;
    D.cap = D.target * 4u;
    if (D.cap > RING_FRAMES - 1024u) D.cap = RING_FRAMES - 1024u;
    D.fill_avg = D.target;
    D.pos = 0.0;
    D.hist_n = 0;
    D.starved = 1;                   /* prime to target/2, then fade in */
    InitializeCriticalSection(&D.mu);
    D.running = 1;
    D.thread = CreateThread(NULL, 0, writer, NULL, 0, NULL);
    if (!D.thread) {
        D.running = 0;
        waveOutReset(D.wo);
        waveOutClose(D.wo); D.wo = NULL;
        DeleteCriticalSection(&D.mu);
        CloseHandle(D.done_event); D.done_event = NULL;
        for (int i = 0; i < D.nbufs; i++) { free(D.mem[i]); D.mem[i] = NULL; }
        fprintf(stderr, "[AUDIO] CreateThread failed; audio output silent\n");
        return 0;
    }
    /* The device thread must not starve behind the game thread. */
    SetThreadPriority(D.thread, THREAD_PRIORITY_TIME_CRITICAL);
    fprintf(stderr, "[AUDIO] WinMM waveOut output open (44100 Hz stereo, "
            "~%d ms: device %d x %u frames + queue %d, adaptive rate +-0.5%%)\n",
            ms, D.nbufs, (unsigned)D.buf_frames, ring_ms);
    return 1;
}

static void winmm_submit(const int16_t *s, int frames)
{
    double step;
    EnterCriticalSection(&D.mu);
    D.in_frames += (uint64_t)frames;
    if (D.ff) {
        /* muted: the resampler restarts cleanly on resume */
        D.ff_dropped += (uint64_t)frames;
        D.hist_n = 0;
        D.pos = 0.0;
        D.fill_avg = D.target;
        LeaveCriticalSection(&D.mu);
        return;
    }
    /* Smoothed fill -> resampling step (>1 consumes input faster). */
    D.fill_avg += ((double)D.n - D.fill_avg) * 0.05;
    {
        double e = (D.fill_avg - (double)D.target) / (double)D.target;
        if (e > 1.0) e = 1.0;
        if (e < -1.0) e = -1.0;
        step = 1.0 + RATE_SPAN * e;
    }
    for (int i = 0; i < frames; i++) {
        memmove(D.hist[0], D.hist[1], sizeof(D.hist[0]) * 3u);
        D.hist[3][0] = (float)s[i * 2];
        D.hist[3][1] = (float)s[i * 2 + 1];
        if (D.hist_n < 4) {
            if (++D.hist_n < 4) continue;
            memcpy(D.hist[0], D.hist[1], sizeof(D.hist[0]));  /* ramp-safe start */
        }
        /* output points lie between hist[1] and hist[2] */
        while (D.pos < 1.0) {
            float t = (float)D.pos, o[2];
            for (int c = 0; c < 2; c++) {
                float y0 = D.hist[0][c], y1 = D.hist[1][c];
                float y2 = D.hist[2][c], y3 = D.hist[3][c];
                float a = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
                float b = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
                float cc = -0.5f * y0 + 0.5f * y2;
                o[c] = ((a * t + b) * t + cc) * t + y1;
            }
            if (D.n >= D.cap) ring_drop_oldest(D.n - D.target);
            ring_put(sat16(o[0]), sat16(o[1]));
            D.out_frames++;
            D.pos += step;
        }
        D.pos -= 1.0;
    }
    LeaveCriticalSection(&D.mu);
    SetEvent(D.done_event);          /* wake the writer: new audio is queued */
}

static void winmm_close(void)
{
    if (!D.wo) return;
    InterlockedExchange(&D.running, 0);
    SetEvent(D.done_event);
    WaitForSingleObject(D.thread, 5000);
    CloseHandle(D.thread); D.thread = NULL;
    waveOutReset(D.wo);
    for (int i = 0; i < D.nbufs; i++) {
        if (D.hdr[i].dwFlags & WHDR_PREPARED) waveOutUnprepareHeader(D.wo, &D.hdr[i], sizeof(D.hdr[i]));
        free(D.mem[i]); D.mem[i] = NULL;
    }
    waveOutClose(D.wo); D.wo = NULL;
    CloseHandle(D.done_event); D.done_event = NULL;
    DeleteCriticalSection(&D.mu);
    fprintf(stderr, "[AUDIO] output: in=%llu out=%llu underruns=%llu "
            "overflow_events=%llu overflow_dropped=%llu ff_muted=%llu\n",
            (unsigned long long)D.in_frames, (unsigned long long)D.out_frames,
            (unsigned long long)D.underruns, (unsigned long long)D.overflow_events,
            (unsigned long long)D.overflow_drops, (unsigned long long)D.ff_dropped);
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
    if (want_device) g_device = winmm_open();
    g_open = 1;
    atexit(HostAudio_Close);
    return g_wav != NULL || g_device;
}

int HostAudio_DeviceOpen(void) { return g_device; }

int HostAudio_QueueFrames(void)
{
    int n;
    if (!g_device) return -1;
    EnterCriticalSection(&D.mu);
    n = (int)D.n;
    LeaveCriticalSection(&D.mu);
    return n;
}

void HostAudio_SetFastForward(int on)
{
    if (g_device && D.ff != (on != 0)) InterlockedExchange(&D.ff, on != 0);
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
            winmm_submit(stereo, frames);
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
                winmm_submit(chunk, n);
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
    if (g_device) winmm_close();
    g_device = 0;
}
#endif /* _WIN32 */

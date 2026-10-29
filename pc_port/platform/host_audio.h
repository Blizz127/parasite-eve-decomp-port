/*
 * Host audio output (audio lane): 44100 Hz stereo int16.
 *
 * POSIX: libpulse-simple.so.0 is dlopen'ed at runtime (PipeWire's
 * pipewire-pulse provides it on Bazzite/Fedora), so the executable gains no
 * link dependency.  A writer thread drains a lock-protected ring buffer;
 * HostAudio_Submit never blocks.  The device feed is steered by a small
 * adaptive resampler (at most +-0.5 %) toward a target queue fill, so the
 * 60 Hz emulated audio clock, the 59.94 Hz window pacer and the device
 * crystal never starve or overfill the stream; a genuine starve fades out
 * and back in, an overflow (headless runs faster than real time) drops the
 * oldest audio with a crossfade.  Without the library, or on Windows,
 * output is silent.
 *
 * Environment:
 *   PE_AUDIO=0            disable audio entirely (no synth, no output)
 *   PE_AUDIO_WAV=path     also write every rendered sample to a WAV file
 *                         (works headless; the header is refreshed each
 *                         second and on exit)
 *   PE_AUDIO_LATENCY_MS   total output latency, default 100 (60 % device
 *                         buffer, 40 % queue), clamped 40..500
 */
#ifndef HOST_AUDIO_H
#define HOST_AUDIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Open outputs.  `want_device` 0 = WAV only (e.g. headless). Returns 1 if
 * any sink (device or WAV) is open. */
int  HostAudio_Open(int want_device);
void HostAudio_Submit(const int16_t *stereo, int frames);
void HostAudio_Close(void);
int  HostAudio_DeviceOpen(void);
/* Fast-forward: 1 mutes the device (queued audio fades out; submitted
 * frames are discarded), 0 resumes with a fade-in.  The WAV dump is
 * unaffected.  Safe to call every frame. */
void HostAudio_SetFastForward(int on);
/* Device queue depth in stereo frames (diagnostics, PE_FRAME_TRACE);
 * -1 when no device is open. */
int  HostAudio_QueueFrames(void);

/* MASTER VOLUME (enhancement registry) is applied to the device feed only;
 * the WAV dump stays retail.  At 100 the samples pass through untouched. */
static inline int16_t HostAudio_GainSample(int16_t sample, int percent)
{
    return (int16_t)((int32_t)sample * percent / 100);
}
/* Frames [first, first+n) of a submit of `frames` stereo frames, scaled by a
 * level that ramps linearly from `from` (frame 0) toward `to` (frame
 * `frames`), so a volume change does not click.  from == to is a constant
 * HostAudio_GainSample. */
static inline void HostAudio_GainRamp(int16_t *dst, const int16_t *src, int first, int n,
                                      int frames, int from, int to)
{
    for (int i = 0; i < n; i++) {
        int64_t f = first + i;
        int64_t level = (int64_t)from * (frames - f) + (int64_t)to * f;   /* percent x frames */
        for (int c = 0; c < 2; c++)
            dst[2 * i + c] = (int16_t)((int64_t)src[2 * (first + i) + c] * level / (100 * (int64_t)frames));
    }
}

#ifdef __cplusplus
}
#endif

#endif

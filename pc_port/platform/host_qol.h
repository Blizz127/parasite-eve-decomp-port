/* Pure helpers for the presentation and audio options (BRIGHTNESS, MONO AUDIO).
 * Header-only and side-effect free, so every backend (X11, SDL, Win32,
 * PulseAudio, SDL audio, WinMM) and the native tests share one rule.
 *
 * BRIGHTNESS: out = min(255, (in * pct + 50) / 100), integer math, so the
 * table is exact on every host.  Black stays black at every step (LUT[0] = 0),
 * which is why the letterbox bars never change.  At 100 there is no table and
 * the caller skips the pass entirely: the retail pixels are untouched.
 *
 * MONO AUDIO: M = round-half-away-from-zero((L + R) / 2), written to both
 * channels.  Off means the caller does not call this at all. */
#ifndef HOST_QOL_H
#define HOST_QOL_H

#include <stddef.h>
#include <stdint.h>

/* Fills lut for `pct` and returns 1; returns 0 (lut untouched) at the retail
 * identity 100 or any value that is not a positive percentage. */
static inline int HostQol_BrightnessLut(uint8_t lut[256], int pct)
{
    if (pct == 100 || pct <= 0) return 0;
    for (int i = 0; i < 256; i++) {
        int v = (i * pct + 50) / 100;
        lut[i] = (uint8_t)(v > 255 ? 255 : v);
    }
    return 1;
}

/* Applies lut to the first three bytes (R, G, B or B, G, R) of each pixel of
 * `n` pixels, `bpp` bytes apiece; byte 3 of a 4-byte pixel is left alone. */
static inline void HostQol_LutPixels(uint8_t *px, size_t n, int bpp, const uint8_t lut[256])
{
    for (size_t i = 0; i < n; i++, px += bpp) {
        px[0] = lut[px[0]];
        px[1] = lut[px[1]];
        px[2] = lut[px[2]];
    }
}

/* Applies lut to the rectangle (x0, y0, w, h) of an image `img_w` pixels
 * wide with `bpp` bytes per pixel. */
static inline void HostQol_LutRect(uint8_t *img, int img_w, int bpp,
                                   int x0, int y0, int w, int h, const uint8_t lut[256])
{
    for (int y = 0; y < h; y++)
        HostQol_LutPixels(img + ((size_t)(y0 + y) * (size_t)img_w + (size_t)x0) * (size_t)bpp,
                          (size_t)w, bpp, lut);
}

/* MONO AUDIO sample rule: round-half-away-from-zero of (l + r) / 2.  The sum
 * is taken in 32 bits, so the result always fits int16_t. */
static inline int16_t HostQol_MonoSample(int32_t l, int32_t r)
{
    int32_t s = l + r;
    return (int16_t)(s >= 0 ? (s + 1) / 2 : -((-s + 1) / 2));
}

/* Downmixes `frames` interleaved stereo frames from src into dst (dst may be
 * src): both channels of each frame get HostQol_MonoSample(L, R). */
static inline void HostQol_Downmix(int16_t *dst, const int16_t *src, int frames)
{
    for (int i = 0; i < frames; i++) {
        int16_t m = HostQol_MonoSample(src[2 * i], src[2 * i + 1]);
        dst[2 * i] = m;
        dst[2 * i + 1] = m;
    }
}

#endif /* HOST_QOL_H */

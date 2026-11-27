/* Host-side accessors for the two registry options that plat_enhance.c adds
 * for the presenters and the audio device feed: Graphics > BRIGHTNESS and
 * Audio > MONO AUDIO.  Both default to the retail behaviour (100 %, stereo),
 * and both are presentation/device only: they never write guest memory,
 * VRAM, the SPU mixer or the WAV dump.  The game-facing header is
 * pe_plat/enhance.h; the rest of the host options are in pe_plat/enhance_host.h.
 * See docs/port/ENHANCEMENTS.md (BRIGHTNESS, MONO AUDIO). */
#ifndef PLAT_ENHANCE_H
#define PLAT_ENHANCE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Percent gain on the presented window image: 100 (RETAIL, default), 110,
 * 120, 130, 90 or 80. */
int  pe_enh_brightness_pct(void);
void pe_enh_set_brightness_pct(int pct);

/* MONO AUDIO: 0 (stereo, default) or 1 (the device feed is downmixed to mono). */
int  pe_enh_mono(void);
void pe_enh_set_mono(int on);

#ifdef __cplusplus
}
#endif

#endif /* PLAT_ENHANCE_H */

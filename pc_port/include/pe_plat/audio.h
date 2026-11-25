/*
 * pe_plat audio interface (docs/ARCHITECTURE-PORT.md, step P0).
 *
 * Meaning-level: a 24-voice sound unit with its own sample memory, voice
 * key-on/key-off, a CD-audio (XA / CD-DA) input stream, and an output sink.
 * The mixer is pulled by emulated time (`pe_plat_audio_tick`), never by the
 * host device clock, so headless runs stay deterministic.
 *
 * Default backend: in-house pe_spu synthesizer (platform/plat_audio.c) with
 * host_audio as the device sink (platform/plat_audio_host.c).  The AKAO
 * sound driver is matched game C and stays above this interface.
 *
 * Game code must include only pe_plat headers (never pe_spu.h).
 */
#ifndef PE_PLAT_AUDIO_H
#define PE_PLAT_AUDIO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_PLAT_AUDIO_VOICES      24
#define PE_PLAT_AUDIO_SAMPLE_RAM  0x80000u   /* bytes of voice sample memory */

/* Where mixed audio goes (44.1 kHz interleaved stereo int16). */
typedef struct {
    void *ctx;
    void (*submit)(void *ctx, const int16_t *stereo, int frames);
    void (*set_fast_forward)(void *ctx, int on);   /* optional */
    void (*close)(void *ctx);                      /* optional */
    const char *name;
} PePlatAudioOutput;

typedef struct {
    uint64_t key_on_events;
    uint64_t key_off_events;
    uint64_t mixed_frames;
    uint32_t voices_active;
    uint64_t cd_underruns;
} PePlatAudioStats;

/* Lifecycle.  init activates the mixer (remembering the prior state);
 * shutdown closes the output and restores it. */
int  pe_plat_audio_init(void);
void pe_plat_audio_shutdown(void);
void pe_plat_audio_reset(void);
int  pe_plat_audio_sample_rate(void);

/* Voices: bit n of the mask = voice n (0..23). */
void pe_plat_audio_voice_key_on(uint32_t voice_mask);
void pe_plat_audio_voice_key_off(uint32_t voice_mask);

/* Sample memory upload (bytes, wraps at PE_PLAT_AUDIO_SAMPLE_RAM). */
int  pe_plat_audio_transfer_write(uint32_t address, const void *data, size_t len);
int  pe_plat_audio_transfer_read(uint32_t address, void *out, size_t len);

/* CD audio input. */
void pe_plat_audio_cd_push(const int16_t *stereo, int frames);
void pe_plat_audio_cd_stop(void);

/* Mixing and output.  `mix` renders into a caller buffer; `tick` renders
 * `frames` and hands them to the installed output (if any). */
void pe_plat_audio_mix(int16_t *stereo_out, int frames);
void pe_plat_audio_tick(int frames);
void pe_plat_audio_set_output(const PePlatAudioOutput *out);   /* NULL = none */
void pe_plat_audio_set_fast_forward(int on);
/* Host device sink (host_audio).  Defined in plat_audio_host.c, which links
 * only into executables that also carry host_audio.c. */
int  pe_plat_audio_open_host_output(int want_device);

void pe_plat_audio_stats(PePlatAudioStats *out);

/* Apply voice/register writes the game made through direct pointers into
 * the sound device's register block.  Those writes are held until the next
 * device access; callers that snapshot or compare the device state (the
 * PE_AKAO_VERIFY differential) flush them first. */
void pe_plat_audio_commit_registers(void);

#ifdef __cplusplus
}
#endif

/* _spu_setReverbAttr (pe_plat_spu_reverb.c, clean-room from black-box
 * vectors: pc_port/tools/pe_cd_leaf_vectors.py reverb). */
void pe_plat_spu_set_reverb_attr(uint32_t attr);
/* SpuSetReverbModeType (pe_plat_spu_modetype.c, clean-room from black-box
 * vectors: pc_port/tools/pe_cd_leaf_vectors.py modetype).  Its callees are
 * the existing providers func_8007DAE0/func_80085BB4 (pe_plat/state.h),
 * func_8008D140 (above) and PE_SpuReverb_ClearWorkArea (pe_spu_dma.h). */
int pe_plat_spu_set_reverb_mode_type(int mode);
/* _spu_FiDMA, the DMA4 completion handler (pe_plat_spu_fidma.c, clean-room
 * from black-box vectors: pc_port/tools/pe_cd_leaf_vectors.py fidma). */
void pe_plat_spu_fidma(void);

#define func_8008D140 pe_plat_spu_set_reverb_attr
#define func_8008CF70 pe_plat_spu_set_reverb_mode_type
#define func_8007D614 pe_plat_spu_fidma
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8008D140(uint32_t attr);
int func_8008CF70(int mode);
void func_8007D614(void);
#endif /* PE_PLAT_AUDIO_H */

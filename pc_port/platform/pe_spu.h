/*
 * Native SPU synthesizer (psx-spx semantics) for the Parasite Eve port.
 *
 * pe_spu_dma.c keeps owning the 512 KiB SPU RAM image and the 0x200-byte
 * register file that translated retail code reads and writes through
 * PE_SpuRegister_{Load,Store}U16.  When the synthesizer is active
 * (PE_Spu_SetActive(1), done by the host executable unless PE_AUDIO=0),
 * every register store is also observed here (KON/KOFF, voice state, FIFO
 * transfers) and the few hardware-live registers (ENDX, per-voice current
 * envelope, SPUSTAT) read back synthesized state.  When inactive the
 * register file keeps its historical plain-storage behavior, so the native
 * test suite is unaffected.
 *
 * Output is 44100 Hz interleaved stereo int16, rendered on demand by
 * PE_Spu_Render.  Time is emulated time: the audio driver
 * (pe_audio_driver.c) renders 44100/60 samples per emulated VBlank.
 *
 * Implemented: 24 voices, 4-bit ADPCM (5 filters, shift, loop start/end/
 * repeat flags), pitch with linear interpolation, pitch modulation, ADSR
 * (exact psx-spx rate formula), per-voice volume with sweep, main volume,
 * KON/KOFF/ENDX, noise, reverb (psx-spx algorithm at 22.05 kHz), CD-audio
 * input volume mixing, manual FIFO transfers.  Gaps are listed in
 * docs/ai_context/DAY1_FIDELITY_GAPS.md (gaussian interpolation, SPU IRQ,
 * capture buffers).
 */
#ifndef PE_SPU_H
#define PE_SPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_SPU_RATE 44100

/* Enable/disable register observation and live reads. */
void PE_Spu_SetActive(int active);
int  PE_Spu_Active(void);
void PE_Spu_Reset(void);

/* Called by pe_spu_dma.c after it stored `value` into the register file. */
void PE_Spu_OnRegisterWrite(uint32_t offset, uint16_t value);
/* Called by pe_spu_dma.c for a load; returns 1 and sets *value when the
 * register is hardware-live, else 0 (plain storage). */
int  PE_Spu_OnRegisterRead(uint32_t offset, uint16_t *value);

/* Render `frames` stereo frames (2*frames int16) into out. */
void PE_Spu_Render(int16_t *out, int frames);

/* CD audio input (XA / CD-DA), 44100 Hz stereo.  Samples are queued and
 * consumed by PE_Spu_Render, scaled by the CD input volume (0x1B0/0x1B2). */
void PE_Spu_PushCdAudio(const int16_t *stereo, int frames);

/* CD sector tap (pe_cdreg.c): every raw 2352-byte sector the drive reads.
 * With drive mode bit 0x40 (XA-ADPCM) set, form-2 audio sectors (submode
 * bits 0x04|0x20) that pass the mode-0x08 file/channel filter are decoded
 * (37.8/18.9 kHz, mono/stereo, 4/8-bit), resampled to 44.1 kHz, scaled by
 * the drive's audio volume matrix (0x80 = 100%) and queued as CD input.
 * Sector delivery to the game is not changed by this tap. */
void PE_Spu_CdSector(const uint8_t *raw, uint8_t mode, int muted,
                     uint8_t filter_file, uint8_t filter_chan,
                     const uint8_t vol[4]);
/* Decode one XA sector body (8-byte subheader + data) into interleaved
 * PCM (stereo) or mono PCM; returns sample frames (4032 mono / 2016 stereo
 * for 4-bit). Keeps ADPCM history across calls (PE_Spu_XaReset clears). */
int  PE_Spu_DecodeXaSector(const uint8_t *sub_and_data, int16_t *out,
                           int *channels, int *rate);
void PE_Spu_XaReset(void);
/* Stream stop / movie skip (fmv lane): flush the queued CD input, fade
 * the last output frame out over CD_FADE (64) frames, reset XA history. */
void PE_Spu_CdInputStop(void);
uint64_t PE_Spu_XaSectors(void);
/* Print key SPU registers (FILE *) — diagnostics. */
void PE_Spu_DebugSummary(void *file);

/* Test hooks: raw ADPCM block decode and a single-voice envelope step. */
void PE_Spu_DecodeAdpcmBlock(const uint8_t block[16], int32_t *hist1,
                             int32_t *hist2, int16_t out[28]);
typedef struct {
    int32_t level;
    int phase;           /* 0 off, 1 attack, 2 decay, 3 sustain, 4 release */
    int32_t counter;
    uint16_t adsr_lo, adsr_hi;
} PeSpuEnvelope;
void PE_Spu_EnvelopeKeyOn(PeSpuEnvelope *env, uint16_t lo, uint16_t hi);
void PE_Spu_EnvelopeKeyOff(PeSpuEnvelope *env);
void PE_Spu_EnvelopeStep(PeSpuEnvelope *env);

typedef struct {
    uint64_t kon_events;
    uint64_t koff_events;
    uint64_t register_writes;
    uint64_t fifo_bytes;
    uint64_t rendered_frames;
    uint32_t voices_active;     /* at last render */
    uint64_t kon_bad_header;    /* start block filter>4 or shift>12 */
    uint64_t kon_zero_block;    /* start block entirely zero (VAG lead-in) */
    uint64_t blocks_decoded;
    uint64_t blocks_bad;        /* decoded block with filter > 4 (garbage) */
    uint64_t cd_underruns;      /* CD/XA queue ran dry mid-stream */
    uint64_t cd_underrun_frames;
    uint64_t cd_overflow_frames;/* CD/XA frames dropped (queue full) */
    uint64_t cd_frames_mixed;   /* output frames with CD/XA input (incl. fades) */
} PeSpuStats;
void PE_Spu_GetStats(PeSpuStats *out);
/* Optional KON trace callback (voice mask, start address). */
typedef void (*PeSpuKonHook)(uint32_t mask);
void PE_Spu_SetKonHook(PeSpuKonHook hook);

#ifdef __cplusplus
}
#endif

#endif /* PE_SPU_H */

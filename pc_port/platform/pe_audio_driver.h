/*
 * Audio driver clock for the native port (audio lane, Day-1 preview).
 *
 * Retail programs root counter 2 as SetRCnt(0xF2000002, 0x44E8, 0x1000)
 * (mode 0x258: sysclk/8, reset+IRQ at target) and binds func_8008E23C to
 * its event, i.e. the AKAO sound driver tick func_8008DB7C runs at
 * 33868800 / 8 / 0x44E8 = 240 Hz.  func_8008DB7C (sequencer, voice
 * updates, command queue func_8008CA84, SPU flush) is only partly ported to
 * host C (its sequencer step func_8008E8D0 is not matched), so this module
 * runs the retail machine code of func_8008DB7C out of guest RAM with a
 * minimal R3000A integer interpreter.  SPU register accesses go to the
 * native SPU (pe_spu.h); guest RAM accesses go through pe_guest_ram.
 *
 * The clock is emulated time: PE_AudioDriver_VBlank() is called once per
 * emulated VBlank (PE_GPU_VBlankStep) and runs 4 driver ticks and
 * 44100/59.94 samples of SPU output, handed to host_audio.
 *
 * Inert until PE_AudioDriver_Enable(1) (the host executable does this
 * unless PE_AUDIO=0), so library tests keep their historical behavior.
 */
#ifndef PE_AUDIO_DRIVER_H
#define PE_AUDIO_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void PE_AudioDriver_Enable(int enable);
/* Output sink for rendered 44.1 kHz stereo (host_audio.c's HostAudio_Submit
 * in the host executable; NULL = discard). */
typedef void (*PeAudioSink)(const int16_t *stereo, int frames);
void PE_AudioDriver_SetSink(PeAudioSink sink);
int  PE_AudioDriver_Active(void);
/* One emulated VBlank. */
void PE_AudioDriver_VBlank(void);
/* Real-time pacer called at the end of every emulated VBlank (also with
 * the driver disabled).  The window registers one so emulated time -- the
 * game's VSync(n) waits, the 240 Hz AKAO clock and the 735 samples per
 * VBlank -- runs at 59.94 VBlanks per wall-clock second.  Pacing per
 * *present* instead ran the 30 fps game (2 VBlanks per present) and its
 * music at 2x.  NULL (headless, tests) = unpaced. */
typedef void (*PeVBlankPacer)(void);
void PE_AudioDriver_SetVBlankPacer(PeVBlankPacer pacer);

/* Run the guest function at `entry` in the interpreter with a0..a3;
 * returns v0.  Exposed for tests. `ok` (may be NULL) is set to 0 on a
 * fault (bad address, budget exhausted, unsupported opcode). */
uint32_t PE_AudioDriver_CallGuest(uint32_t entry, uint32_t a0, uint32_t a1,
                                  uint32_t a2, uint32_t a3, int *ok);

typedef struct {
    uint64_t ticks;
    uint64_t skipped_ticks;      /* audio event not armed / critical section */
    uint64_t faults;
    uint64_t instructions;
    uint64_t vblanks;
} PeAudioDriverStats;
void PE_AudioDriver_GetStats(PeAudioDriverStats *out);

#ifdef __cplusplus
}
#endif

#endif

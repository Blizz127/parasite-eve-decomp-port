/*
 * pe_plat timing interface (docs/ARCHITECTURE-PORT.md, step P0).
 *
 * Meaning-level: the host presentation clock (next frame deadline at the
 * NTSC 60000/1001 Hz rate), fast-forward, the emulated vertical-blank count,
 * and one one-shot hardware timeout measured in emulated CPU cycles.
 * Emulated time never comes from host wall time; only the frame deadline
 * does, so deterministic runs never sleep.
 *
 * Default backend: host_frame_pacer + pe_rcnt2 + pe_gpu's vblank counter
 * (platform/plat_timing.c).  VSync callbacks, events and critical sections
 * arrive in step P2 on this same header.
 *
 * Game code must include only pe_plat headers.
 */
#ifndef PE_PLAT_TIMING_H
#define PE_PLAT_TIMING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_PLAT_TIMING_CPU_HZ        33868800u
#define PE_PLAT_TIMING_FRAME_MILLIHZ 59940u   /* 60000/1001 Hz, rounded */

void pe_plat_timing_init(void);

/* Host clock: absolute deadline (ns, same clock as now_ns) for the next
 * presented frame.  Backlog after a stall is discarded. */
uint64_t pe_plat_timing_next_frame_deadline(uint64_t now_ns);
uint32_t pe_plat_timing_frame_rate_millihz(void);

void pe_plat_timing_set_fast_forward(int on);
int  pe_plat_timing_fast_forward(void);

/* Emulated vertical blanks since the renderer was initialised. */
uint32_t pe_plat_timing_vblank_count(void);

/* One-shot timeout: elapses once `ticks` timer ticks have passed, where a
 * tick is one CPU cycle, or eight when `cpu_div8` is set.  `ticks` 1..65535.
 * The elapsed state is sticky until the next start. */
int  pe_plat_timing_timeout_start(uint16_t ticks, int cpu_div8);
int  pe_plat_timing_timeout_elapsed(void);
void pe_plat_timing_advance_cycles(uint32_t cycles);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_TIMING_H */

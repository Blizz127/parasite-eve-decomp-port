/*
 * pe_plat timing -> host_frame_pacer, pe_rcnt2, pe_gpu vblank counter
 * (step P0 adapter).  Root-counter mode bits are the documented hardware
 * layout (psx-spx timers): bit 3 reset at target, bit 4 IRQ at target,
 * bit 9 system clock / 8, bit 11 reached target.
 */
#include "pe_plat/timing.h"

#include "host_frame_pacer.h"
#include "pe_gpu.h"
#include "pe_rcnt2.h"

static HostFramePacer plat_tm_pacer;
static int plat_tm_ff;
static int plat_tm_elapsed;

void pe_plat_timing_init(void)
{
    plat_tm_pacer.deadline_ns = 0;
    plat_tm_pacer.fraction = 0;
    plat_tm_pacer.started = 0;
    plat_tm_ff = 0;
    plat_tm_elapsed = 0;
}

uint64_t pe_plat_timing_next_frame_deadline(uint64_t now_ns)
{
    return HostFramePacer_Deadline(&plat_tm_pacer, now_ns);
}

uint32_t pe_plat_timing_frame_rate_millihz(void) { return PE_PLAT_TIMING_FRAME_MILLIHZ; }

void pe_plat_timing_set_fast_forward(int on) { plat_tm_ff = on ? 1 : 0; }
int  pe_plat_timing_fast_forward(void) { return plat_tm_ff; }

uint32_t pe_plat_timing_vblank_count(void) { return PE_GPU_VSyncQuery(); }

int pe_plat_timing_timeout_start(uint16_t ticks, int cpu_div8)
{
    if (!ticks) return 0;
    PE_Rcnt2_WriteTarget(ticks);
    PE_Rcnt2_WriteMode((uint16_t)(0x0018u | (cpu_div8 ? 0x0200u : 0u)));
    plat_tm_elapsed = 0;
    return 1;
}

int pe_plat_timing_timeout_elapsed(void)
{
    if (PE_Rcnt2_ReadMode() & 0x0800u) plat_tm_elapsed = 1;
    return plat_tm_elapsed;
}

void pe_plat_timing_advance_cycles(uint32_t cycles) { PE_Rcnt2_Advance(cycles); }

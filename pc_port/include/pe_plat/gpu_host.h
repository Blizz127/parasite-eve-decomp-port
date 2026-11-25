/* Host-model hooks for the clean-room GPU providers (DrawSync 0x80074DC0, GPU
 * DMA start 0x80076B98, PutDispEnv 0x800755F0, queue pump/wait 0x80077294).
 * The providers reproduce the original's register-level behaviour (checked
 * against black-box vectors); these hooks drive the HOST state that the
 * hardware reacts to those register writes with: the framebuffer's display
 * presentation, the DMA2 list worker, the DMA IRQ checkpoint and the
 * no-progress detection of a wait.  Implemented in platform/pe_plat_gpu_host.c;
 * the vector checkers link no-op stubs, so none of this is in the vectors. */
#ifndef PE_PLAT_GPU_HOST_H
#define PE_PLAT_GPU_HOST_H
#include <stdint.h>

/* DrawSync entry: the host framebuffer's DrawSync telemetry (one per call). */
void pe_plat_gpu_host_drawsync(int32_t mode);

/* The clock of the GPU timeout check: what its VSync(-1) reads.  The deadline word it compares against is seeded
 * by the host's seed function (func_800773D0) from the GPU model's VBlank counter (PE_GPU_VSyncQuery), so the
 * check must read that same counter.  HostFB_VSync(-1) returns a different clock (the guest's counter with a CD
 * device, else a call count) and, polled once per busy wait pass, made every check exceed the deadline: the
 * timeout path then reset the GPU and dropped the queued uploads (route gate: the field background was never
 * uploaded). */
uint32_t pe_plat_gpu_host_vsync_query(void);

/* One wait pass of the queue wait (after the timeout check): lets the host DMA2
 * transfer progress through the DMA IRQ checkpoint.  Returns 0 normally, 1 when
 * the wait can never make progress (no DMA pending and the GPU is held not-ready;
 * the provider cuts with a named boundary), -1 when a stop was requested. */
int pe_plat_gpu_host_wait_tick(void);

/* GPU linked-list DMA start: validate the first node before any register write
 * (0 = refused, named cut recorded), and run the armed list to completion node
 * by node after the CHCR write (progressive, validating every node). */
int pe_plat_gpu_host_dma_preflight(uint32_t packet);
void pe_plat_gpu_host_dma_run(void);

/* PutDispEnv: after the GP1 display commands, present the environment. */
void pe_plat_gpu_host_present_dispenv(uint32_t env);

/* ResetGraph reset path: library state that production boot always has (the register pointer words 0x80095850 (GP0),
 * 0x80095854 (GP1), 0x80095858/5C/60 (DMA2 MADR/BCR/CHCR), 0x80095870 (DPCR) and 0x80095674 (I_MASK), the mode-0 display-limit table entries) but that a unit-test RAM never seeded gets its architectural
 * default: an invalid register pointer word -> the register address, zero limit entries -> 1024 x 512.  Production values
 * are valid and untouched; the vectors never see the hook's writes (the checkers link a no-op stub). */
void pe_plat_gpu_host_reset_prepare(void);
#endif

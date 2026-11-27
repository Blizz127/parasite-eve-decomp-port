/* libgpu command-queue pump (guest 0x80076EE4) and its deadline seed (guest
 * 0x800773D0); provider platform/pe_plat2_gpuq.c, written clean-room from the
 * black-box vector sets gpu2a (docs/evidence/native-gpu2-20261126), the
 * port's host GPU/DMA model API and psx-spx.  Host-only trace/evidence
 * functions live in platform/pe_plat2_gpuq_host.c. */
#ifndef PE_PLAT_GPUQ_H
#define PE_PLAT_GPUQ_H
#include <stdint.h>
#include "pe_guest_ram.h"

/* Pump: returns the number of entries still queued ((producer - consumer) & 63),
 * or 1 at once when DMA channel 2 is busy. */
int pe_plat_gpuq_pump(void);
/* Deadline seed: the VBlank count plus 0xF0 is published to the deadline word
 * (0x80095888), the timeout poll counter (0x8009588C) is cleared; returns the deadline. */
uint32_t pe_plat_gpuq_seed_deadline(void);
#define func_80076EE4 pe_plat_gpuq_pump
#define func_800773D0 pe_plat_gpuq_seed_deadline
/* Canonical guest-name prototypes for the bindings above. */
int func_80076EE4(void);
uint32_t func_800773D0(void);

/* Internal pump entry: *retail_returned is 1 only when the pump reached its
 * normal exit, and 0 when a nested typed boundary (a worker that stopped the
 * port, a refused worker, a stuck GPU wait) did not return.  The result is the
 * pump's return value in the first case and 0 in the second.  Callers must not
 * infer the distinction from the host stop reason, which keeps only the first
 * reason requested. */
int pe_plat_gpuq_pump_ex(int *retail_returned);
#define PE_func_80076EE4_Pump pe_plat_gpuq_pump_ex
int PE_func_80076EE4_Pump(int *retail_returned);

/* Host-only observation seam (not part of the original's behaviour and not
 * visible in the vectors): when an observer is installed the pump reports
 * each step to it.  Every callback is optional.  platform/pe_plat2_gpuq_host.c
 * installs one that fills PeGpuPumpTrace (psx_compat.h). */
typedef struct PeGpuqObserver {
    void (*entered)(void);                           /* pump entry, before any register access */
    void (*mask_disabled)(uint32_t saved_mask);      /* interrupt mask cleared, previous value saved */
    void (*dma_callback_removed)(void);              /* DMACallback(2, 0) issued */
    void (*gpu_polled)(void);                        /* one GPUSTAT read */
    void (*gpu_ready)(void);                         /* GPUSTAT reports ready-for-command */
    void (*worker_begin)(pe_addr_t worker, pe_addr_t argument, uint32_t auxiliary, uint32_t consumer);
    void (*worker_end)(int returned);
    void (*consumer_advanced)(uint32_t consumer);
    void (*mask_restored)(uint32_t restored_mask);
    void (*callback_cleared)(void);                  /* work marker cleared before the callback */
    void (*callback_called)(pe_addr_t callback, int returned);
} PeGpuqObserver;
void pe_plat_gpuq_set_observer(const PeGpuqObserver *observer);
#endif

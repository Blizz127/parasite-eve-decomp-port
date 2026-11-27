/* Host-only evidence for the libgpu queue pump (platform/pe_plat2_gpuq.c): the
 * value-only trace (PeGpuPumpTrace, psx_compat.h) and the read-only names of
 * the register authorities the pump uses.  None of this participates in the
 * original's behaviour; the pump reports its steps to the observer installed
 * here, and a program that never asks for a trace (PE_Pump_TraceReset) runs the
 * pump without one.  Written from the host-model API (pe_plat/gpu_hw.h,
 * irq_hw.h) and the pump header pe_plat/gpuq.h; the extra reads below
 * (I_MASK, stored DICR, DMA2 CHCR, the DMA callback slot) are model queries made
 * only while a trace is armed. */
#include <stdint.h>
#include <string.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_plat/gpuq.h"
#include "pe_plat/gpu_hw.h"
#include "pe_plat/irq_hw.h"

#define DMA2_CALLBACK_SLOT 0x800956C8u
#define PUMP_CHCR_POINTER  0x80095860u
#define PUMP_CHCR_MMIO     0x1F8010A8u

static PeGpuPumpTrace g_trace;
static uint64_t g_order, g_entries;

static uint64_t next_order(void) { return ++g_order; }

static void on_entered(void) { g_entries++; }
static void on_mask_disabled(uint32_t saved) { g_trace.saved_mask = saved; g_trace.mask_disable_order = next_order(); }
static void on_callback_removed(void) { g_trace.callback_removal_order = next_order(); }
static void on_gpu_polled(void) { g_trace.gpustat_reads++; }
static void on_gpu_ready(void) { g_trace.gpustat_ready_order = next_order(); }
static void on_worker_begin(pe_addr_t worker, pe_addr_t argument, uint32_t auxiliary, uint32_t consumer)
{
    g_trace.worker_calls++;
    g_trace.last_worker = worker;
    g_trace.last_argument = argument;
    g_trace.last_auxiliary = auxiliary;
    g_trace.consumer_before_worker = consumer;
    g_trace.mask_at_worker = PE_IRQ_GetMask();
    g_trace.dma_callback_at_worker = PE_LoadU32(DMA2_CALLBACK_SLOT);
    g_trace.dicr_at_worker = PE_GPU_ReadStoredDICR();
    g_trace.chcr_at_worker = PE_GPU_ReadDMA2CHCR();
    g_trace.worker_returned = 0;
    g_trace.worker_call_order = next_order();
}
static void on_worker_end(int returned) { g_trace.worker_returned = returned; }
static void on_consumer_advanced(uint32_t consumer)
{
    g_trace.consumer_after_worker = consumer;
    g_trace.consumer_advance_order = next_order();
}
static void on_mask_restored(uint32_t restored) { g_trace.restored_mask = restored; g_trace.mask_restore_order = next_order(); }
static void on_callback_cleared(void) { g_trace.drawsync_clear_order = next_order(); }
static void on_callback_called(pe_addr_t callback, int returned)
{
    (void)callback; (void)returned;
    g_trace.drawsync_calls++;
    g_trace.drawsync_call_order = next_order();
}

static const PeGpuqObserver g_observer = {
    on_entered, on_mask_disabled, on_callback_removed, on_gpu_polled, on_gpu_ready, on_worker_begin,
    on_worker_end, on_consumer_advanced, on_mask_restored, on_callback_cleared, on_callback_called,
};

void PE_Pump_TraceReset(void)
{
    memset(&g_trace, 0, sizeof g_trace);
    g_order = 0;
    g_entries = 0;
    pe_plat_gpuq_set_observer(&g_observer);
}

uint64_t PE_Pump_EntryCount(void) { return g_entries; }
void PE_Pump_GetTrace(PeGpuPumpTrace *out) { if (out) *out = g_trace; }
pe_addr_t PE_Pump_Dma2ChcrPointerAddress(void) { return PUMP_CHCR_POINTER; }
uint32_t PE_Pump_Dma2ChcrMmioAddress(void) { return PUMP_CHCR_MMIO; }

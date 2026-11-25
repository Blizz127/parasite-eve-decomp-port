/*
 * Phase 6E-B53I-B2 — bounded CPU/DMA interrupt delivery surfaces.
 *
 * pe_irq.c remains the sole I_STAT/I_MASK/generation authority and pe_gpu.c
 * remains the sole DICR authority.  This module contains only the retail
 * dispatch control flow between those authorities and guest-backed callback
 * identity tables.  Zero is the only absent callback; guest identities are
 * never cast to native pointers.
 */
#ifndef PE_IRQ_DELIVERY_H
#define PE_IRQ_DELIVERY_H

#include <stdint.h>

#include "pe_irq.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PE_IRQ_SERVICE_RETURNED = 0,
    PE_IRQ_SERVICE_BOUNDARY,
    PE_IRQ_SERVICE_STALE
} PeIrqServiceResult;

typedef enum {
    PE_IRQ_EDGE_NONE = 0,
    PE_IRQ_EDGE_ASSERTED,
    PE_IRQ_EDGE_STALE
} PeIrqEdgeResult;

typedef struct {
    uint64_t edge_bridge_calls;
    uint64_t cpu_service_calls;
    uint64_t edge_assert_order;
    uint64_t cpu_ack_order;
    uint64_t cpu_callback_order;
    uint64_t dma_ack_order;
    uint64_t dma_callback_order;
    uint64_t cpu_service_entries;
    uint64_t dma_dispatch_entries;
    uint32_t last_cpu_source;
    uint32_t last_dma_channel;
    uint32_t last_cpu_handler;
    uint32_t last_dma_handler;
    uint32_t last_dma_ack_write;
    uint16_t last_cpu_ack_write;
    uint16_t dma_callback_istat;
    uint32_t dma_callback_dicr;
    uint32_t dma_callback_chcr;
    uint32_t cpu_ack_count;
    uint32_t dma_ack_count;
    uint32_t dma_diagnostic_count;
    uint32_t dma_diagnostic_dicr;
    uint32_t dma_diagnostic_madr[7];
    uint8_t cpu_ack_sources[16];
    uint8_t dma_ack_channels[32];
} PeIrqDeliveryTrace;

/* Consume at most one already-latched DICR rising edge and assert CPU source
 * 3 in the captured generation.  This never runs a CPU callback. */
PeIrqEdgeResult PE_IRQ_BridgeDICRRisingEdge(PeIrqGeneration generation);

/* Host framing (stale generation, critical section, masked status) around
 * the clean-room CPU interrupt dispatcher pe_plat_irq_dispatch. */
PeIrqServiceResult PE_IRQ_ServicePendingForGeneration(
    PeIrqGeneration generation);

/* Host entry for the clean-room DMA handler (guest 0x80074520): reports a
 * nested stop as BOUNDARY. */
PeIrqServiceResult PE_func_80074520_Dispatch(void);
/* Host observers/services for the clean-room DMA handler. */
void PE_IRQ_ObserveDicrWrite(uint32_t value);
int PE_IRQ_DmaCallbackService(uint32_t handler, int *ok);
int PE_IRQ_CpuCallbackService(uint32_t handler, int *ok);
void PE_IRQ_ObserveIstatWrite(uint16_t value);

/* Host-only, value-only evidence.  This is not callback or IRQ authority. */
void PE_IRQ_DeliveryTraceReset(void);
void PE_IRQ_GetDeliveryTrace(PeIrqDeliveryTrace *out);

#ifdef __cplusplus
}
#endif

#endif /* PE_IRQ_DELIVERY_H */

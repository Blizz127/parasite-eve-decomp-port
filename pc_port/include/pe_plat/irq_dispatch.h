/* Interrupt delivery: CPU-side service of pending interrupts and DMA completion dispatch.
 * Declaration-only header (no function bodies): the port's host model API for interrupt delivery.
 * Declarations were moved unchanged from the port's own host-model header platform/pe_irq_delivery.h
 * (commit e15e3ee7c); comments describe hardware behaviour as documented publicly (psx-spx)
 * in the project's own words.  No SDK source, tables or disassembly are reproduced here. */
#ifndef PE_PLAT_IRQ_DISPATCH_H
#define PE_PLAT_IRQ_DISPATCH_H

#include <stdint.h>

#include "pe_plat/irq_hw.h"

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

/* Host observers/services for the clean-room DMA handler (the guest entry
 * func_80074520 is bound to pe_plat_dma_handler in pe_plat/irq_callback.h; a
 * host caller that needs a returned/boundary verdict compares the port stop
 * epoch around that call). */
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

#endif /* PE_PLAT_IRQ_DISPATCH_H */

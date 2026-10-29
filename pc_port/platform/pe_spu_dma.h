/*
 * Phase 6E-B48A — minimum native SPU DMA4 event model.
 *
 * This is deliberately not a general PSX DMA emulator.  It owns the one
 * 512 KiB SPU-RAM image used by the native port and the one DMA4 transfer
 * that Parasite Eve's func_800850F4 chain may have in flight.  Initiation
 * records guest addresses only.  PE_SpuDma_Service is the controlled event
 * pump: it performs the DMA block transfer and then dispatches the retail
 * completion callback.  Initiation never calls it.
 */
#ifndef PE_SPU_DMA_H
#define PE_SPU_DMA_H

#include <stdint.h>
#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PE_SPU_RAM_SIZE       0x00080000u
#define PE_SPU_DMA_BLOCK_SIZE 0x00000040u
#define PE_SPU_DMA_MAX_REQUEST 0x0007EFF0u

#define PE_SPU_DMA_IRQ_HANDLER 0x8007D614u
#define PE_SPU_DMA_CALLBACK    0x80085098u

typedef struct {
    int pending;
    int irq_installed;
    pe_addr_t irq_handler;
    pe_addr_t source;          /* guest RAM buffer (source for writes, destination for reads) */
    uint32_t destination;      /* SPU RAM address */
    uint32_t requested_size;
    uint32_t dma_size;
    pe_addr_t callback_at_issue;
    uint64_t event_count;
    uint64_t data_order;
    uint64_t callback_order; /* callback invocation or event delivery order */
} PeSpuDmaState;

/* Host lifecycle.  Reset cancels a pending event and clears SPU RAM. */
void PE_SpuDma_Reset(void);

/* Models func_8007DD14(func_8007D614), performed by func_8007D15C. */
int PE_SpuDma_InstallIrq(pe_addr_t handler);

/* Record one mode-0 DMA4 transfer.  Returns 0 without mutation if the IRQ
 * is absent, another transfer is pending, an address is unaligned/out of
 * range, or the rounded DMA byte count cannot be represented safely.  The
 * 16-bit SPU transfer address wraps modulo the 512 KiB SPU RAM size.
 * `callback` is only recorded (callback_at_issue): retail's issue never
 * reads D_8009B434, and completion dispatches the live slot. */
int PE_SpuDma_Begin(pe_addr_t source, uint32_t destination,
                    uint32_t requested_size, pe_addr_t callback);

/* Record an SPU-to-guest transfer using the same completion owner.  A zero
 * request is not represented until its hardware DMA behavior is established. */
int PE_SpuDma_BeginRead(pe_addr_t guest_destination, uint32_t spu_source,
                        uint32_t requested_size, pe_addr_t callback);

/* Deliver at most one pending DMA interrupt. Data becomes visible in the
 * transfer destination before the live callback or zero-callback event
 * is delivered. Returns 1 if an interrupt ran, including one whose callback
 * dispatch requested a stop (an unresolved guest callback); 0 if no
 * transfer was pending. */
int PE_SpuDma_Service(void);

/* Retail func_80085174 wait adaptation.  The service call represents the
 * interrupt opportunity that exists while the retail CPU polls the flag.
 * Returns with D_8009D24C still 1 when a stop is requested; a set flag with
 * no transfer pending requests the stop itself.  Callers test
 * PE_Port_ShouldStop() before any further guest write. */
void PE_SpuDma_WaitForCompletion(void);

/* The interrupt of a transfer still pending when an issuer starts a new one.
 * Retail _spu_t (func_8007D778) never refuses an issue, and its DMA4
 * transfer ends in far less than the frame this model may keep it pending
 * for.  An issuer calls this first, before it touches the callback slot or
 * the busy flag, so the pending transfer completes as its own issuer left
 * them (data, then the live callback).  Returns 1 when nothing was pending
 * (the issue goes ahead as before) or the delivery ended without a stop.
 * Returns 0 when a transfer was pending and a stop is requested, earlier
 * (nothing is delivered then, as in the wait) or by the delivery's callback
 * dispatch: the caller then issues nothing. */
int PE_SpuDma_DeliverPending(void);

void PE_SpuDma_GetState(PeSpuDmaState *out);
/* Read-only view of the represented DMA4 MADR register.  The diagnostic
 * DMA dispatcher uses this value without becoming a second DMA4 owner. */
pe_addr_t PE_SpuDma_ReadMADR(void);
uint8_t PE_SpuRam_LoadU8(uint32_t address);
/* Host view of the 512 KiB SPU RAM image (synthesizer, pe_spu.c). */
uint8_t *PE_SpuRam_Data(void);
uint16_t PE_SpuRegister_LoadU16(uint32_t offset);
void PE_SpuRegister_StoreU16(uint32_t offset, uint16_t value);

/* libspu SpuClearReverbWorkArea (retail 0x8008D610) host backend:
 * DMA4 zero-fill of the reverb work area for mode 0..9 (pe_stream.c).
 * Returns 0, or -1 for a bad / allocated mode or a failed transfer. */
int PE_SpuReverb_ClearWorkArea(int mode);

#ifdef __cplusplus
}
#endif

#endif /* PE_SPU_DMA_H */

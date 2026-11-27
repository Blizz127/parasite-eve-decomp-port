/* SPU DMA transfer model: in-flight transfer bookkeeping and the host pump.
 * Declaration-only header (no function bodies): the port's host model API for SPU DMA (DMA channel 4).
 * Declarations were moved unchanged from the port's own host-model header platform/pe_spu_dma.h
 * (commit e15e3ee7c); comments describe hardware behaviour as documented publicly (psx-spx)
 * in the project's own words.  No SDK source, tables or disassembly are reproduced here. */
#ifndef PE_PLAT_SPU_DMA_HW_H
#define PE_PLAT_SPU_DMA_HW_H

#include <stdint.h>
#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PE_SPU_RAM_SIZE       0x00080000u
#define PE_SPU_DMA_BLOCK_SIZE 0x00000040u
/* Port-defined cap on the size of one transfer request accepted by the host model. */
#define PE_SPU_DMA_MAX_REQUEST 0x0007EFF0u

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

/* Registers the SPU DMA completion interrupt handler with the host model.
 * Only the handler address the host model recognises is accepted; returns 1
 * on success, 0 otherwise. */
int PE_SpuDma_InstallIrq(pe_addr_t handler);

/* Record one mode-0 DMA4 transfer.  Returns 0 without mutation if the IRQ
 * is absent, another transfer is pending, an address is unaligned/out of
 * range, or the rounded DMA byte count cannot be represented safely.  The
 * 16-bit SPU transfer address wraps modulo the 512 KiB SPU RAM size.
 * `callback` is only recorded (callback_at_issue): issuing a transfer never
 * reads the live callback slot, and completion dispatches whatever the slot
 * holds at that time. */
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

/* Host adaptation of the guest's wait for DMA completion.  The service call
 * represents the interrupt opportunity that exists while the guest CPU polls
 * the transfer-busy flag.  Returns with the flag still set when a stop is
 * requested; a set flag with no transfer pending requests the stop itself.
 * Callers test PE_Port_ShouldStop() before any further guest write. */
void PE_SpuDma_WaitForCompletion(void);

/* The interrupt of a transfer still pending when an issuer starts a new one.
 * The SPU transfer path never refuses an issue, and a real DMA channel 4
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

/* DMA channel 4 register file (psx-spx "DMA Channels": 0x1F8010C0 MADR, 0x1F8010C4 BCR, 0x1F8010C8 CHCR).
 * index 0 = MADR (bits 0-23), 1 = BCR, 2 = CHCR.  Reads are side-effect free; CHCR bit 24 reads back as
 * set while a transfer is pending.  A CHCR store with the start bit (bit 24) starts the transfer the
 * registers describe: block mode (bits 9-10 = 1), 16 words per block, direction from bit 0 (1 = guest
 * RAM -> SPU RAM, 0 = SPU RAM -> guest RAM), SPU RAM address = the SPU transfer address register
 * (0x1F801DA6) * 8.  The transfer is recorded exactly like PE_SpuDma_Begin / PE_SpuDma_BeginRead
 * (completion by PE_SpuDma_Service); it is not serviced inside the store, and the guest bookkeeping words
 * those calls write are the writer's own business.  A transfer the model cannot represent (handler not
 * installed, another transfer pending, misaligned source, source outside guest RAM, other block size,
 * zero-length read) is a loud stop (PE_Port_RequestStop with a stderr report), never silently dropped. */
uint32_t PE_SpuDma_Dma4Load(uint32_t index);
void PE_SpuDma_Dma4Store(uint32_t index, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_SPU_DMA_HW_H */

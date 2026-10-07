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
    pe_addr_t source;
    uint32_t destination;
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
 * A zero callback requires the enabled SPU completion event registration. */
int PE_SpuDma_Begin(pe_addr_t source, uint32_t destination,
                    uint32_t requested_size, pe_addr_t callback);

/* Deliver at most one pending DMA interrupt. Data becomes visible in SPU
 * RAM before the live callback is dispatched or its zero-callback event
 * is delivered. Returns 1 if an interrupt ran. */
int PE_SpuDma_Service(void);

/* Retail func_80085174 wait adaptation.  The service call represents the
 * interrupt opportunity that exists while the retail CPU polls the flag. */
void PE_SpuDma_WaitForCompletion(void);

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

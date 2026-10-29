#ifndef PE_MDEC_H
#define PE_MDEC_H

#include <stdint.h>

#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PE_MDEC_UPLOAD_HISTORY 4u

typedef struct {
    uint32_t command;
    pe_addr_t source;
    uint32_t word_count;
    uint32_t bcr;
    uint64_t payload_fnv1a64;
} PeMdecUpload;

typedef struct {
    uint32_t control_last_write;
    uint32_t command_last_write;
    pe_addr_t dma0_madr;
    uint32_t dma0_bcr;
    uint32_t dma0_chcr;
    uint32_t dma1_chcr;
    pe_addr_t dma1_madr;
    uint32_t dma1_bcr,output_count,completed_output_count;
    uint32_t control_write_count;
    uint32_t reset_count;
    uint32_t input_wait_count;
    uint32_t upload_count;
    uint32_t stale_input_discards;  /* commands begun with old input left */
    uint32_t stale_output_bytes;    /* old-command output queued ahead of new */
    uint32_t completed_upload_count;
    int dma0_active;
    PeMdecUpload uploads[PE_MDEC_UPLOAD_HISTORY];
} PeMdecState;

void PE_MDEC_Init(void);
void PE_MDEC_BeginReset(void);
/* Bit31 aborts decode/internal buffered data, not the separate DMA channels. */
void PE_MDEC_WriteControl(uint32_t value);
/* Native command-port writes: quant/scale and compressed decode payloads,
 * plus the documented no-operation command classes 0 and 4..7.
 * Raw/pixel output can decode complete macroblocks before the entire command
 * arrives. Output DMA waits for its next service unit (a raw monochrome
 * request block, otherwise the complete admitted output), not command end. CPU input may resume between DMA0 bursts, not during an
 * active DMA.
 * Status reads remain a boundary. */
int PE_MDEC_WriteCommandData(uint32_t value);
/* Raw port returns four FIFO bytes, in 8x8 block order for color. Empty/short
 * reads and mid-color consumer switches remain boundaries, without consuming
 * a buffered prefix or modifying the caller's output word. */
int PE_MDEC_ReadData(uint32_t *value);
/* DMA0 MADR/BCR/CHCR offsets 0/4/8. Request-mode input, block sizes 1..32,
 * incrementing or decrementing word addresses. Service consumes one raw
 * block; DPCR always gates it, CHCR bit28 can force its first block without
 * MDEC request-enable, then clears. Partial progress retains busy and can
 * enable early output; DICR bit0 optionally flags each block. Other modes
 * (including bit29 retention) remain boundaries. */
uint32_t PE_MDEC_ReadDMA0(uint32_t offset);
void PE_MDEC_WriteDMA0(uint32_t offset,uint32_t value);
/* DMA1 uses the same register offsets and request-mode block sizes. Raw
 * monochrome and pure-color streams service one block at a time with live
 * control/DPCR gates and optional DICR bit1 per-block flags. Forward color
 * words add the device's 8x8-tile-to-16x16-row displacement to forward or
 * backward MADR stepping. Every displaced address is checked before consuming
 * the service block; backward stepping can intentionally overwrite rows.
 * Queued/mixed formats and high-level output retain whole-output service.
 * Pure streams permit force-first-block bit28, still gated by DPCR and
 * cleared only when the admitted block begins. Empty/short forced output,
 * forced/backward mixed formats, mid-color CPU/helper switches and
 * active-register races remain explicit boundaries. */
uint32_t PE_MDEC_ReadDMA1(uint32_t offset);
void PE_MDEC_WriteDMA1(uint32_t offset,uint32_t value);
void PE_MDEC_ClearDmaChannels(void);
/* High-level 32-word block submission retains rounding down to whole blocks.
 * Rejects zero/over-width BA and invalid header+payload ranges before touching
 * an existing DMA; it never turns hardware's zero-BA encoding into a no-op. */
int PE_MDEC_SubmitInputTable(pe_addr_t command_block, uint32_t size_words);
void PE_MDEC_GetState(PeMdecState *out);
/* Same whole-block count validation as input; validates the word-aligned
 * destination span before servicing a pending DMA or changing DPCR. */
int PE_MDEC_SubmitOutput(pe_addr_t destination,uint32_t words);
/* Returns whether any DMA completed, not whether a partial block progressed. */
int PE_MDEC_Service(void);
int PE_MDEC_HasDecode(void);
/* Software decode of a complete run-length command; output is one macroblock
 * at a time in DMA pixel order. Hardware timing/DMA1 are separate. */
int PE_MDEC_BeginDecode(pe_addr_t command_block);
/* Admits the full readable span and consumer order before destination writes. */
int PE_MDEC_ReadPixels(pe_addr_t destination,uint32_t bytes);
uint32_t PE_MDEC_DecodedMacroblocks(void);

#ifdef __cplusplus
}
#endif

#endif

/* SPU DMA request validation ladder (clean-room, host model).
 * Decides whether the host SPU DMA model accepts one transfer request.
 * Pure function: reads guest RAM geometry only, never mutates anything.
 *
 * Sources: [H] include/pe_plat/spu_dma_hw.h; [A] platform/pe_plat_spu_dma_addrs.h;
 * [G] pe_guest_ram.h; [R] docs/evidence/native-cdx-20261126/cdgen48/REPORT.md;
 * [P] psx-spx (DMA channels, SPU); [T] the unit-test expectations listed in the .c file.
 *
 * Rules (a request is refused, with *dma_size_out untouched, when any fails):
 *  1. IRQ: the completion IRQ is installed and its handler equals PE_SPU_DMA_IRQ_HANDLER   [H, A]
 *  2. No other transfer is pending                                                         [H]
 *  3. source is 4-byte aligned (DMA4 MADR, 32-bit SPU channel words)                      [P, T]
 *  4. destination is a multiple of 8 (SPU transfer address counts 8-byte units)           [P, T]
 *  5. destination < PE_SPU_RAM_SIZE (512 KiB)                                              [H, P, T]
 *     destination + size past the end of SPU RAM is ACCEPTED: the transfer address wraps
 *     modulo SPU RAM size                                                                  [H, T]
 *  6. requested_size <= PE_SPU_DMA_MAX_REQUEST (0x7EFF0, unsigned compare)                 [H, R, T]
 *  7. read_direction with requested_size == 0 is refused (unsupported zero-length read)    [H, T]
 *     Write with requested_size == 0 is accepted with dma size 0 (port-defined: the suite
 *     expects the zero-size upload to be issued).
 *  8. dma size = ceil(requested_size / 64) * 64 (whole 64-byte blocks)                     [R, P]
 *  9. the guest buffer [canonical(source), source + dma size) lies wholly in guest RAM
 *     (the rounded size, so a buffer overrunning RAM only by padding is refused)           [H, G, T]
 *     For a zero size only the (aligned) source address must be RAM (port-defined).
 */
#ifndef PE_PLAT_SPU_DMA_CHECK_H
#define PE_PLAT_SPU_DMA_CHECK_H

#include <stdint.h>
#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Returns 1 = accept (*dma_size_out = bytes the DMA moves) or 0 = refuse (no side effects). */
int PE_SpuDmaCheck_Request(int irq_installed, pe_addr_t irq_handler, int transfer_pending,
                           pe_addr_t source, uint32_t destination, uint32_t requested_size,
                           int read_direction, uint32_t *dma_size_out);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_SPU_DMA_CHECK_H */

/*
 * SPU DMA request validation ladder, clean-room.
 *
 * Provenance: written without reading any existing validation code.  Derived only from:
 *   - include/pe_plat/spu_dma_hw.h (public contract, constants, refusal list)
 *   - platform/pe_plat_spu_dma_addrs.h (PE_SPU_DMA_IRQ_HANDLER) and platform/pe_guest_ram.h (API)
 *   - docs/evidence/native-cdx-20261126/cdgen48/REPORT.md (unsigned clamp to 0x7EFF0, block rounding)
 *   - psx-spx public documentation (DMA4 word alignment, SPU address in 8-byte units, 512 KiB SPU RAM)
 *   - suite expectations: tests/test_native.c test_B48A_address_guards_and_failure_state,
 *     zero-size/maximum upload checks, SPU_dma_read_model (zero read refused, read across SPU RAM end
 *     accepted), and the Begin calls in test_spu_dma_event.h / test_spu_dma_callbacks.h / test_spu_mode.h.
 */
#include "pe_plat/spu_dma_check.h"
#include "pe_plat/spu_dma_hw.h"
#include "pe_plat_spu_dma_addrs.h"

int PE_SpuDmaCheck_Request(int irq_installed, pe_addr_t irq_handler, int transfer_pending,
                           pe_addr_t source, uint32_t destination, uint32_t requested_size,
                           int read_direction, uint32_t *dma_size_out)
{
    uint32_t dma_size;
    pe_addr_t canonical;

    if (!irq_installed || irq_handler != PE_SPU_DMA_IRQ_HANDLER) return 0;            /* rule 1 */
    if (transfer_pending) return 0;                                                   /* rule 2 */
    if ((source & 3u) != 0u) return 0;                                                /* rule 3 */
    if ((destination & 7u) != 0u) return 0;                                           /* rule 4 */
    if (destination >= PE_SPU_RAM_SIZE) return 0;                                     /* rule 5 */
    if (requested_size > PE_SPU_DMA_MAX_REQUEST) return 0;                            /* rule 6 */
    if (read_direction && requested_size == 0u) return 0;                             /* rule 7 */

    dma_size = (requested_size + (PE_SPU_DMA_BLOCK_SIZE - 1u)) &
               ~(PE_SPU_DMA_BLOCK_SIZE - 1u);                                         /* rule 8 */

    if (dma_size == 0u) {                                                             /* rule 9 */
        canonical = PE_RamCanonical(source, 0u);
        if (!PE_AddressIsRam(canonical)) return 0;
    } else {
        canonical = PE_RamCanonical(source, dma_size);
        if (!PE_RangeIsRam(canonical, dma_size)) return 0;
    }

    if (dma_size_out) *dma_size_out = dma_size;
    return 1;
}

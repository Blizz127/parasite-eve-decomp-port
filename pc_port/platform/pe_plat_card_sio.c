/* Memory-card serial (SIO0) receive wait (guest 0x80083578), written
 * clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py entry 0x80083578), with no SDK
 * source.  Evidence: docs/evidence/native-sio-poll-20261031/. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* Guest word holding the address of the SIO0 register block (every vector
 * reads 0x1F801040 from it). */
#define SIO_BASE_PTR_WORD    0x8009B788u

/* SIO0 status register sits 4 bytes above the base (0x1F801044); 16-bit. */
#define SIO_STAT_OFFSET      0x4u

/* Status bit that ends the wait: RX FIFO not empty (bit 1). */
#define SIO_STAT_RX_READY    0x2u

/* Read the register-block pointer once, then re-read the 16-bit status
 * register until the RX-ready bit is set.  No timeout: the loop ends on
 * exactly the read the original ended on. */
void pe_plat_card_sio_rx_wait(void)
{
    uint32_t base = PE_LoadU32(SIO_BASE_PTR_WORD);
    while ((PE_LoadU16(base + SIO_STAT_OFFSET) & SIO_STAT_RX_READY) == 0) {
    }
}

/* libpress MDEC reset core (guest 0x8010C0FC) and its two overlay helpers, the
 * table-upload start 0x8010C1EC and the MDEC-idle wait 0x8010C308 (movie module
 * overlay ovl_039F), written clean-room as natives from ordered black-box
 * traces of the original code (batch cdgen40: pc_port/tools/pe_mdec_core_vectors.py,
 * checked by pc_port/tools/pe_mdec_core_check.c).  Independent re-implementation:
 * the behaviour below is exactly the guest loads, stores and calls seen in the
 * traces; hardware meanings come from psx-spx (MDEC control register bit 31
 * reset and bits 30/29 DMA request enables, MDEC(1) command port, status bit
 * 29 command busy, DMA0 MADR/BCR/CHCR, DPCR).  No SDK source and nothing from
 * any other implementation was used, and no claim is made about the original's
 * source.
 *
 * The original reaches every hardware register through a pointer word in its
 * overlay data (0x8010DB1C..0x8010DB54) that it loads before each use.  The
 * traces show those words hold the register addresses (the overlay image is
 * sha256-pinned), so the natives use the addresses directly and make none of
 * those loads: they are host-invisible constants, and the checker drops them
 * from the recorded streams.  (Reading them at run time would make the natives
 * depend on the overlay being resident and write through address 0 otherwise.)
 *
 * pe_plat_mdec_core(mode), guest 0x8010C0FC:
 *   mode 0 or 1: write 0x80000000 to the MDEC control register (reset), 0 to
 *   the DMA0 CHCR, 0 to the DMA1 CHCR, then 0x60000000 to the control register
 *   (enable both DMA requests).  Mode 1 reloads the DMA1 CHCR pointer and the
 *   control pointer and reads the DMA1 CHCR (the value is not used) before that
 *   last control write.  Mode 0 then starts two table uploads: the command
 *   block at 0x8010DA0C and the one at 0x8010DA90, 0x20 words each.
 *   Any other mode: prints through the libapi printf (format at 0x8010BCFC in
 *   overlay data) with the mode as its argument and does nothing else.
 *   The mode is compared as a whole 32-bit word.
 * pe_plat_mdec_upload_start(block, words), guest 0x8010C1EC: wait until the
 *   MDEC is not busy, then DPCR |= 0x88; DMA0 MADR = block + 4; DMA0 BCR =
 *   0x20 | ((words >> 5) << 16) (32-bit wrap); the command word stored at
 *   block goes to the MDEC command port; DMA0 CHCR = 0x01000201.
 * wait (guest 0x8010C308, internal): reload the control pointer and read the
 *   status word until bit 29 is clear.
 *
 * Host contracts (not behaviour of the original; the vectors never exercise them):
 *  - both entry points do nothing when PE_Port_ShouldStop() is non-zero on entry;
 *  - inside the wait, PE_Port_ShouldStop() ends the wait; while busy the host
 *    MDEC input is serviced (PE_MDEC_Service) once per poll; after 0x40000 polls
 *    without the bit clearing, PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY)
 *    is called and the wait is left; the upload then continues as if the bit
 *    had cleared.
 *  - status is read through PE_MDEC_StatusWord() (bit 29 only) because the MMIO
 *    value path treats MDEC status reads as a boundary. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_mdec.h"
#include "pe_bios_string.h"
#include "pe_plat/mdec_callback.h"

#define R_DMA0_MADR 0x1F801080u
#define R_DMA0_BCR  0x1F801084u
#define R_DMA0_CHCR 0x1F801088u
#define R_DMA1_CHCR 0x1F801098u
#define R_MDEC_CMD  0x1F801820u
#define R_MDEC_CTRL 0x1F801824u
#define R_DPCR      0x1F8010F0u

#define MDEC_STATUS_REG 0x1F801824u
#define STATUS_BUSY     0x20000000u
#define CTRL_RESET      0x80000000u
#define CTRL_DMA_ENABLE 0x60000000u
#define DPCR_MDEC_BITS  0x88u
#define CHCR_TO_DEVICE  0x01000201u
#define BCR_BASE        0x20u
#define TABLE_LUMA      0x8010DA0Cu
#define TABLE_CHROMA    0x8010DA90u
#define TABLE_WORDS     0x20u
#define FMT_BAD_OPTION  0x8010BCFCu
#define POLL_BUDGET     0x40000u

static void mdec_wait_idle(void)
{
    uint32_t polls = 0u;
    for (;;) {
        if ((PE_MDEC_StatusWord() & STATUS_BUSY) == 0u)
            return;
        if (PE_Port_ShouldStop() != 0)
            return;
        if (polls >= POLL_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return;
        }
        polls++;
        (void)PE_MDEC_Service();
    }
}

void pe_plat_mdec_upload_start(uint32_t block, uint32_t words)
{
    uint32_t dpcr;

    if (PE_Port_ShouldStop() != 0)
        return;
    mdec_wait_idle();
    dpcr = PE_LoadU32(R_DPCR);
    PE_StoreU32(R_DPCR, dpcr | DPCR_MDEC_BITS);
    PE_StoreU32(R_DMA0_MADR, block + 4u);
    PE_StoreU32(R_DMA0_BCR, BCR_BASE | ((words >> 5) << 16));
    PE_StoreU32(R_MDEC_CMD, PE_LoadU32(block));
    PE_StoreU32(R_DMA0_CHCR, CHCR_TO_DEVICE);
}

void pe_plat_mdec_core(int mode)
{
    if (mode != 0 && mode != 1) {
        (void)PE_Printf_Guest(FMT_BAD_OPTION, (uint32_t)mode, 0u, 0u, 0u, 0u, 0u, 0u);
        return;
    }
    if (PE_Port_ShouldStop() != 0)
        return;
    PE_StoreU32(R_MDEC_CTRL, CTRL_RESET);
    PE_StoreU32(R_DMA0_CHCR, 0u);
    PE_StoreU32(R_DMA1_CHCR, 0u);
    if (mode == 1) {
        (void)PE_LoadU32(R_DMA1_CHCR);
        PE_StoreU32(R_MDEC_CTRL, CTRL_DMA_ENABLE);
        return;
    }
    PE_StoreU32(R_MDEC_CTRL, CTRL_DMA_ENABLE);
    pe_plat_mdec_upload_start(TABLE_LUMA, TABLE_WORDS);
    pe_plat_mdec_upload_start(TABLE_CHROMA, TABLE_WORDS);
}

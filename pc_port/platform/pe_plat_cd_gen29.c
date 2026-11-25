/* libcd CD_getsector2 (guest 0x8007C044), written clean-room as a native
 * from black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen29, checked by pc_port/tools/pe_spu_caller_check.c with
 * -DCHK_HAVE_CD_GEN29).  Independent re-implementation: the behaviour below is
 * exactly the guest loads and stores seen in the vectors
 * (vectors.v1.txt sha256
 * 563d9126e3390e9a9e068014f0c4cf93e78002cf1ed015ad02f1bd18ec733b76);
 * no SDK source and nothing from any other implementation was used, and no
 * claim is made about the original's source.
 *
 * Behaviour: load the first five pointer words in the original's order, read
 * the DMA priority word, then store (in this order) 0 to the CD index register, 0x80
 * to the CD 0x1F801803 register, 0x21020843 to the 0x1F801018 word, 0x1325
 * to the 0x1F801020 word, the priority word with bit 15 set, the buffer
 * address to the DMA3 base-address word and (words | 0x10000) to the DMA3
 * block-control word.  The status pointer word is then reloaded and the
 * status byte polled until bit 6 (0x40) is set.  Finally the DMA3 control
 * pointer word is loaded, 0x11400100 is stored through it, the pointer word
 * is loaded again and the control word read back (value unused).  The result
 * is always 0.  Every pointer word is reloaded wherever the vectors show a
 * reload; none is cached across the stores.
 *
 * Orders the vectors decide (v2: the checker seeds RAM from each address's
 * first load, and pointer words are aimed at one another): the DPCR read
 * precedes the DPCR store; the DPCR store precedes the load of the MADR
 * pointer word (DPCR pointer aimed at the MADR word); the MADR store precedes
 * the load of the BCR pointer word (MADR aimed at the BCR word); the BCR store
 * precedes the first CHCR pointer-word load (BCR aimed at the CHCR word); the
 * MADR store precedes the status pointer reload (MADR aimed at the status
 * word); the status pointer is read from the reload, before the wait.  Not
 * decidable (their values cannot be aimed at plain memory): the placement of
 * the four constant stores (index, register 3, 0x1F801018, 0x1F801020) against
 * the later pointer-word loads, and of the first five pointer-word loads
 * against each other; they follow the vector load/store log order.
 *
 * Host contracts (not behaviour of the original; the vectors never exercise
 * them):
 *  - PE_Port_ShouldStop() is checked before every re-read of the status
 *    byte inside the wait; non-zero leaves the wait.
 *  - pe_plat_sio_poll_tick() is called once per wait iteration (it advances
 *    the deterministic host clock / CD device and does no guest access).
 *  - After 0x40000 re-reads without bit 6, PE_Port_RequestStop(
 *    PE_PORT_STOP_UNRESOLVED_BOUNDARY) is called and the wait is left.
 *  - After leaving the wait for any of these reasons the function continues
 *    exactly as if the bit had been seen (same remaining accesses) and
 *    returns its normal result 0.
 * All accesses through register pointer words use the pe_cdreg wrappers
 * (byte accesses for the byte registers, word accesses for the word
 * registers); only the fixed pointer words use PE_LoadU32. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "game_port.h"
#include "pe_plat/bios_file.h"

#define PTR_STATUS   0x8009B27Cu  /* CD index/status register */
#define PTR_REG3     0x8009B288u  /* CD register 3 */
#define PTR_BUSCTL   0x8009B28Cu  /* 0x1F801020 word */
#define PTR_TIMING   0x8009B2B0u  /* 0x1F801018 word */
#define PTR_DPCR     0x8009B2B4u  /* DMA priority/control word */
#define PTR_MADR     0x8009B2B8u  /* DMA3 base address */
#define PTR_BCR      0x8009B2BCu  /* DMA3 block control */
#define PTR_CHCR     0x8009B2C0u  /* DMA3 channel control */

#define STATUS_BIT        0x40u
#define POLL_BUDGET       0x40000u
#define DPCR_BIT          0x8000u
#define BCR_BIT           0x10000u
#define CHCR_START        0x11400100u

int pe_plat_cd_getsector2(uint32_t buffer, uint32_t words)
{
    uint32_t status, reg3, busctl, timing, dpcr_p, madr, bcr, chcr;
    uint32_t dpcr;
    uint32_t polls;
    uint8_t v;

    status = PE_LoadU32(PTR_STATUS);
    reg3 = PE_LoadU32(PTR_REG3);
    timing = PE_LoadU32(PTR_TIMING);
    busctl = PE_LoadU32(PTR_BUSCTL);
    dpcr_p = PE_LoadU32(PTR_DPCR);
    dpcr = PE_CdLoadU32(dpcr_p);

    PE_CdStoreU8(status, 0u);
    PE_CdStoreU8(reg3, 0x80u);
    PE_CdStoreU32(timing, 0x21020843u);
    PE_CdStoreU32(busctl, 0x1325u);
    PE_CdStoreU32(dpcr_p, dpcr | DPCR_BIT);
    madr = PE_LoadU32(PTR_MADR);     /* after the DPCR store (v2 vectors) */
    PE_CdStoreU32(madr, buffer);
    bcr = PE_LoadU32(PTR_BCR);       /* after the MADR store (v2 vectors) */
    PE_CdStoreU32(bcr, words | BCR_BIT);

    status = PE_LoadU32(PTR_STATUS);
    v = PE_CdLoadU8(status);
    for (polls = 0u; (v & STATUS_BIT) == 0u; ) {
        if (PE_Port_ShouldStop() != 0)
            break;
        if (polls >= POLL_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            break;
        }
        pe_plat_sio_poll_tick();
        polls++;
        v = PE_CdLoadU8(status);
    }

    chcr = PE_LoadU32(PTR_CHCR);
    PE_CdStoreU32(chcr, CHCR_START);
    chcr = PE_LoadU32(PTR_CHCR);
    (void)PE_CdLoadU32(chcr);
    return 0;
}

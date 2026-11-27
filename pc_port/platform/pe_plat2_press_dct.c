/* libpress DecDCT entries hosted in the movie module overlay ovl_039F: DecDCTBufSize (guest 0x8010BF94),
 * DecDCTvlcSize (0x8010C86C) and DecDCToutSync (0x8010C078, with the poll / status-read / time-out steps it ran
 * inline in the oracle), written clean-room from the black-box vector set cdgen63 (2500 cases, 600 / 700 / 1200;
 * docs/evidence/native-cdx-20261126/cdgen63/REPORT.md on branch sw-vec-cdgen62-64-20261127), its checker
 * pc_port/tools/pe_gpu2_cdgen63_check.c on pe_gpu2_check_core.h / pe_gpu2_host_stubs.h, the public headers
 * (pe_plat/press.h, mdec_hw.h, mdec_io.h, mdec_io_host.h, mdec_expand.h, psx_compat.h, pe_guest_ram.h,
 * bootstrap/game_port.h), the host MDEC model sources (platform/pe_mdec.c, pe_mmio.c) for the register-access
 * interface only, the house-style clean-room providers pe_plat2_press.c, pe_plat2_cdsync.c and pe_plat2_gpuq.c, the
 * suite test tests/test_credits_native.h that exercises the entries, and psx-spx (MDEC status register bit 24
 * = data output signed; DMA channel control register bit 24 = transfer busy).
 *
 * NOT read: src/, asm/, vendor/, game/decomp_hand/, game/boot bodies, any pe_lib*.c (including the file this one
 * replaces and its history; it was removed with `git rm` by name), pe_stream*.c, *_retail.c, the oracle sources
 * (pc_port/tools/*oracle*.py), retail_*_cases.h, and evidence that quotes original code.  Disclosed exposures:
 * grep output showed truncated lines of docs/ai_context/ACTIVE_HANDOFF.md (one of them prose about the original
 * entry's callees), configs/USA/overlays/ovl_039F.yaml and a few audit-report lines naming the replaced file; none
 * was used, the behaviour below is what the vectors show.  Derivation and the unsampled arms:
 * docs/evidence/native-cdx-20261126/cdrw-f/AUTHOR-REPORT.md.  No claim is made about the original's source.
 *
 * DecDCTBufSize(bs): returns the halfword at `bs`, zero-extended (600 / 600 cases).  One RAM read of two bytes.
 *
 * DecDCTvlcSize(size): reads the VLC output-bound word 0x8011EB8C, writes `size << 1` into it when the signed value
 *   size - 1 is positive and 0x00FFFFFF otherwise, returns the word it read (700 cases, both arms; sizes 0 / 1 and
 *   every value with bit 31 set but not 0x80000000 take the 0x00FFFFFF arm).  size 0x80000000 is outside the vectors
 *   (the original traps on the signed overflow of the subtraction); here it is a guest-trap stop before any access.
 *
 * DecDCToutSync(mode), 1200 cases: the original goes through two pointer words kept in the overlay image, one for
 * each arm, and the register address it reads is whatever the word holds (cases in which the words were aimed at
 * other register slots read those slots):
 *   mode != 0: word 0x8010DB50 (the MDEC1 status register 0x1F801824 in the image): one register read, return bit 24.
 *   mode == 0: word 0x8010DB30 (the DMA1 channel control register 0x1F801098): the word is re-read before every
 *              read of the register; bit 24 clear returns 0; bit 24 set polls again, as often as the register stays
 *              busy (runs of 1 to 1200 busy reads before an idle one are in the set, none shorter or longer).
 *   Time-out arm (not sampled, see the report): after 0x100000 polls the original prints a time-out message, resets
 *   the MDEC (control write 0x80000000), clears the DMA channels, writes 0x60000000 to the MDEC control register
 *   and returns -1; the arm needs a million register reads per case, beyond the checker's event capacity.
 *
 * Host contract (hardware wait and register seams; every item below is a host decision, not observable in the
 * vectors):
 *   - Register reads go through PE_LoadU32 on the register address, which the port's MMIO model serves for the DMA
 *     channel registers.  The MDEC status register has no read model (pe_mdec.h: status reads remain a boundary) and
 *     only bit 24 is needed, so a read of 0x1F801824 (any KUSEG / KSEG0 / KSEG1 alias) is answered from the host
 *     model's last command word (command bit 26 is the status signed-output bit 24) through PE_MDEC_GetState; the
 *     value carries only that bit.  In the cdgen63 strict build that seam performs no register access, which is why
 *     its mode != 0 cases with the genuine pointer cannot match there (see the report); the mapped build gives the
 *     seam the read.
 *   - A pointer word that does not point into the hardware window (the overlay is not loaded, or the word is zero)
 *     selects the arm's genuine register, so the entries work before the overlay image is present; the vectors never
 *     show such a word.
 *   - The device model only moves when it is serviced, and the original's hardware moves on its own, so every busy
 *     poll services the host MDEC model once (PE_MDEC_Service) as the per-poll tick.  Every poll checks
 *     PE_Port_ShouldStop (a stop returns -1 without side effects).  The 0x100000-poll bound is the stuck-wait budget
 *     and also the retail time-out point; its arm follows the description above with the host reset seams
 *     (PE_MDEC_BeginReset, PE_MDEC_ClearDmaChannels, PE_MDEC_WriteControl) and a stderr diagnostic in place of the
 *     original's message print; the sequence is unsampled by the vectors. */
#include <stdint.h>
#include <stdio.h>

#include "pe_plat/press_dct.h"
#include "pe_plat/mdec_hw.h"
#include "pe_guest_ram.h"
#include "game_port.h"

#define W_VLC_BOUND        0x8011EB8Cu
#define W_MDEC_STATUS_PTR  0x8010DB50u
#define W_DMA1_CHCR_PTR    0x8010DB30u
#define REG_MDEC_STATUS    0x1F801824u
#define REG_DMA1_CHCR      0x1F801098u
#define REG_WINDOW_MASK    0x1FFFF000u
#define REG_WINDOW         0x1F801000u
#define REG_ADDR_MASK      0x1FFFFFFFu
#define VLC_NO_BOUND       0x00FFFFFFu
#define BIT24              0x01000000u
#define POLL_LIMIT         0x100000u
#define MDEC_RESET_RELEASE 0x60000000u

uint32_t PE_DecDCTBufSize_Guest(pe_addr_t bs)
{
    return PE_LoadU16(bs);
}

int32_t PE_DecDCTvlcSize_Guest(uint32_t size)
{
    uint32_t previous;

    if (size == 0x80000000u) {          /* the original's signed overflow trap */
        PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP);
        return 0;
    }
    previous = PE_LoadU32(W_VLC_BOUND);
    PE_StoreU32(W_VLC_BOUND, (size >= 2u && size < 0x80000000u) ? size << 1 : VLC_NO_BOUND);
    return (int32_t)previous;
}

/* The register a pointer word names: the word's own address when it is inside the hardware window, the arm's genuine
 * register otherwise. */
static uint32_t target_register(pe_addr_t pointer_word, uint32_t genuine)
{
    uint32_t reg = PE_LoadU32(pointer_word);

    return (reg & REG_WINDOW_MASK) == REG_WINDOW ? reg : genuine;
}

static uint32_t read_register(uint32_t reg)
{
    if ((reg & REG_ADDR_MASK) == REG_MDEC_STATUS) {
        PeMdecState st;

        PE_MDEC_GetState(&st);
        return ((st.command_last_write >> 26) & 1u) << 24;
    }
    return PE_LoadU32(reg);
}

int32_t PE_DecDCToutSync_Guest(int32_t mode)
{
    uint32_t polls;

    if (mode != 0)
        return (int32_t)((read_register(target_register(W_MDEC_STATUS_PTR, REG_MDEC_STATUS)) >> 24) & 1u);

    for (polls = 0; polls < POLL_LIMIT; polls++) {
        if (!(read_register(target_register(W_DMA1_CHCR_PTR, REG_DMA1_CHCR)) & BIT24))
            return 0;
        if (PE_Port_ShouldStop())
            return -1;
        (void)PE_MDEC_Service();
    }
    fprintf(stderr, "[DecDCToutSync] DMA1 still busy after %u polls; time-out arm (MDEC reset)\n", (unsigned)polls);
    PE_MDEC_BeginReset();
    PE_MDEC_ClearDmaChannels();
    PE_MDEC_WriteControl(MDEC_RESET_RELEASE);
    return -1;
}

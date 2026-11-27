/* CD sector data DMA start-and-wait (retail entry 0x8007BF44), clean-room.
 *
 *   int pe_plat_cdhost_sector(uint32_t dst, uint32_t count)      (bound by pe_plat/cdhost.h)
 *
 * PROVENANCE.  Written from the black-box vector set cdgen47 `sector.txt` (own-written-load exclusion
 * generation, 300 cases; sha256 in docs/evidence/native-cdx-20261126/cdgen47/VECTORS_SHA256SUMS), the
 * public psx-spx CD-ROM controller / DMA3 / DPCR descriptions, the host-contract headers
 * pe_plat/cd_regs.h, pe_plat/framebuffer_api.h and bootstrap/game_port.h, and pe_plat/cdhost.h.
 * NOT read: the previous provider of this entry, src/, asm/, vendor/, decomp_hand/, game/boot bodies,
 * pe_lib*.c, pe_stream.c, any *_retail.c, oracle sources, retail test-vector headers.  The behaviour is
 * exactly what the vectors show; no claim is made about the original's source.
 *
 * Guest words read (each at every use, little endian; all located from the vectors):
 *   0x8009B27C  pointer to the CD index/status register   (0x1F801800 in every runtime state)
 *   0x8009B288  pointer to the CD request register        (0x1F801803)
 *   0x8009B2B0  pointer to the CD bus-timing register     (0x1F801018)
 *   0x8009B28C  pointer to the mailbox / delay register   (0x1F801020), read twice (once per store pair)
 *   0x8009B2B4  pointer to DPCR                           (0x1F8010F0)
 *   0x8009B2B8  pointer to DMA3 MADR                      (0x1F8010B0)
 *   0x8009B2BC  pointer to DMA3 BCR                       (0x1F8010B4)
 *   0x8009B2C0  pointer to DMA3 CHCR                      (0x1F8010B8), read twice (start store, busy poll)
 * Every register is reached through the pointer read from its table word (PE_CdLoad* / PE_CdStore*:
 * address-decoding accessors for table-derived addresses): the vectors move these pointers (e.g. DPCR
 * pointing at 0x1F8010F2 or into RAM), and the replayed register traffic then follows the pointer.
 *
 * Observed sequence (all 300 cases have exactly this shape; the return value is always 0):
 *   register stores, in this order:
 *     [idx]  = 0x00            index 0 (byte)
 *     [req]  = 0x80            request register: BFRD "want data" (byte)
 *     [bus]  = 0x00020943      CD bus timing (word)
 *     [mbx]  = 0x00001323      (word)
 *     [dpcr] = DPCR | 0x8000   DMA3 enable bit, DPCR read first (word)
 *     [madr] = dst             (word)
 *     [bcr]  = count | 0x10000 (word; one block of `count` words: every vector, incl. counts above 0xFFFF,
 *                               shows the argument ORed with 0x00010000, e.g. 3 -> 0x10003, 0xA8D9A8 -> 0xA9D9A8)
 *     [chcr] = 0x11000000      start, busy (word)
 *     [mbx]  = 0x00001325      (word)
 *   register reads that wait:
 *     [idx] (byte) repeated until bit 6 (DRQSTS) is set       -- seen 1..4 reads (vectors 0..3 "not ready" reads)
 *     [chcr] (word) repeated until bit 24 (busy) is clear     -- seen 1..4 reads
 *   The vectors do not interleave reads and stores (the checker compares the two orders separately);
 *   the order chosen here is the hardware-sensible one: the DRQ wait sits after the request write and before the
 *   transfer start, the busy wait after the start.
 *
 * Host contract (not in the original's observable behaviour): the CD device model only advances when told to, so
 * after every not-ready read the wait loop ticks the host clock (HostFB_StreamTick) and, for the DMA wait,
 * services DMA3 (PE_CdReg_ServiceDMA3).  Both are host-only calls the checker counts and ignores.  Each wait also
 * has a ShouldStop exit and a stuck-wait budget that requests a stop with a diagnostic instead of spinning
 * forever; the vectors never reach either (at most 4 reads per wait).  The tick also runs inside the synchronous loader
 * context (DsRead -> read poll -> status callback -> sector), where it is what lets the device model advance.  A wait
 * that still never ends there means the register pointer words are 0 (the table 0x8009B27C..0x8009B2C0 is EXE .data and
 * a fixture that reset RAM must plant it): CHCR is then guest address 0, which no device ever clears, and the
 * diagnostic names that case.  */
#include <stdint.h>
#include <stdio.h>

#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/cdhost.h"
#include "pe_plat/framebuffer_api.h"

#define CDH_SECTOR_WAIT_BUDGET 0x40000u   /* polls (each followed by one clock tick) before the wait is declared stuck */

/* Stuck-wait exit: print a diagnostic and request a stop; the caller returns 0 (no transfer completed). */
static int sector_stuck(const char *what, uint32_t polls, uint32_t reg_ptr)
{
    fprintf(stderr, "[cd-sector-dma] stuck wait on %s after %u polls (register pointer 0x%08X%s); requesting stop\n", what, polls,
            reg_ptr, reg_ptr ? "" : ": the CD/DMA pointer table 0x8009B27C..0x8009B2C0 was never planted, so the wait polls guest address 0");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}

int pe_plat_cdhost_sector(uint32_t dst, uint32_t count)
{
    uint32_t p_idx = PE_LoadU32(0x8009B27Cu);
    uint32_t p_req = PE_LoadU32(0x8009B288u);
    uint32_t p_bus = PE_LoadU32(0x8009B2B0u);
    uint32_t p_mbx = PE_LoadU32(0x8009B28Cu);
    uint32_t p_dpcr = PE_LoadU32(0x8009B2B4u);
    uint32_t dpcr, p_madr, p_bcr, p_chcr, polls;

    PE_CdStoreU8(p_idx, 0x00u);                       /* index 0 */
    PE_CdStoreU8(p_req, 0x80u);                       /* BFRD: ask the controller for the sector data */
    PE_CdStoreU32(p_bus, 0x00020943u);
    PE_CdStoreU32(p_mbx, 0x00001323u);
    dpcr = PE_CdLoadU32(p_dpcr);
    PE_CdStoreU32(p_dpcr, dpcr | 0x8000u);            /* DMA3 master enable */
    p_madr = PE_LoadU32(0x8009B2B8u);
    p_bcr = PE_LoadU32(0x8009B2BCu);
    PE_CdStoreU32(p_madr, dst);
    PE_CdStoreU32(p_bcr, count | 0x00010000u);

    /* wait for the data request (DRQSTS, bit 6 of the status register) */
    p_idx = PE_LoadU32(0x8009B27Cu);
    for (polls = 0;; polls++) {
        if (PE_CdLoadU8(p_idx) & 0x40u) break;
        if (PE_Port_ShouldStop()) return 0;
        if (polls >= CDH_SECTOR_WAIT_BUDGET) return sector_stuck("DRQSTS", polls, p_idx);
        HostFB_StreamTick();                          /* host clock tick: lets the device model raise the request */
    }

    p_chcr = PE_LoadU32(0x8009B2C0u);
    PE_CdStoreU32(p_chcr, 0x11000000u);               /* start the transfer */
    p_chcr = PE_LoadU32(0x8009B2C0u);
    for (polls = 0;; polls++) {                       /* wait for DMA3 to finish (busy bit 24) */
        if (!(PE_CdLoadU32(p_chcr) & 0x01000000u)) break;
        if (PE_Port_ShouldStop()) return 0;
        if (polls >= CDH_SECTOR_WAIT_BUDGET) return sector_stuck("DMA3 CHCR busy", polls, p_chcr);
        PE_CdReg_ServiceDMA3();                       /* host DMA3 service, then a clock tick */
        HostFB_StreamTick();
    }
    PE_CdStoreU32(PE_LoadU32(0x8009B28Cu), 0x00001325u);
    return 0;
}

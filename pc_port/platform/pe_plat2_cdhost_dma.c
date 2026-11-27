/* DMA channel programming with wait (retail entry 0x8007CEAC), clean-room.
 *
 *   void pe_plat_cdhost_dma(uint32_t channel, uint32_t address, uint32_t blocks, uint32_t words,
 *                           uint32_t control, uint32_t interrupt, uint32_t unused)   (bound by pe_plat/cdhost.h)
 *
 * PROVENANCE.  Written from the black-box vector sets cdgen47 `dma.txt` / `dma_silent.txt` (own-written-load
 * exclusion generation, 600 cases, identical files; sha256 in docs/evidence/native-cdx-20261126/cdgen47/
 * VECTORS_SHA256SUMS), the public psx-spx DMA (channel registers, DPCR, DICR) and CD-ROM controller descriptions,
 * the host-contract headers pe_plat/cd_regs.h, pe_plat/framebuffer_api.h, bootstrap/game_port.h and
 * pe_plat/cdhost.h.  NOT read: the previous provider of this entry, src/, asm/, vendor/, decomp_hand/, game/boot
 * bodies, pe_lib*.c, pe_stream.c, any *_retail.c, oracle sources, retail test-vector headers, capture logs.
 * The behaviour is exactly what the vectors show; no claim is made about the original's source.
 *
 * Parameters (the vectors' argument words 0..6; the retail routine reads 4..6 from its caller's stack, the host
 * ABI passes them as parameters).  `unused` is never read; `interrupt` is used by its low byte only.
 *
 * Guest words read (little endian, at every use; located from the vectors):
 *   0x8009B348  pointer to DICR            (0x1F8010F4)  read twice: before the byte access and before the word read
 *   0x8009B344  pointer to DPCR            (0x1F8010F0)
 *   0x8009B32C  pointer to the CD status register (0x1F801800)
 * Registers addressed directly: channel c: MADR 0x1F801080+0x10c, BCR +4, CHCR +8.
 *
 * Observed sequence (all 600 cases; channels 0..7 appear):
 *   1. chcr = CHCR read; low byte of `interrupt` loaded (no register effect);
 *      while (chcr & 0x01000000) chcr = CHCR read.            -- wait for the channel to be idle (up to 202 reads in the vectors)
 *   2. DICR byte access: p = [0x8009B348]; b = byte at p+2 (0x1F8010F6 in every runtime state; bits 16..23 of DICR =
 *      per-channel interrupt enables 0..6 and the master enable in bit 7);
 *      p = [0x8009B348]; a WORD read at p follows (its value is not used by any observed store);
 *      store byte at p+2:  b | (1<<channel)   when (interrupt & 0xFF) == 1, else  b & ~(1<<channel).
 *      (Evidence: 17 cases with low byte 1 all set the bit; every case with another low byte (0, 12..255) clears it;
 *      for channel 7 the bit is the master bit 0x80.)
 *   3. DPCR: d = word at [0x8009B344]; store d | (8 << 4*channel)   (channel enable bit of DPCR).
 *   4. stores MADR = address; BCR = (blocks << 16) | words (both operands used whole: words' upper half is ORed in);
 *   5. wait for the CD data request: s = byte at [0x8009B32C] repeated until bit 6 (0x40, DRQSTS) is set (1..10 reads);
 *   6. store CHCR = control (start), then one more CHCR read.  The routine is void (the vectors' recorded $v0 equals that
 *      last read, but the entry has no return value in the public prototype and the checker does not compare it).
 *
 * Interleaving of reads and stores is not observable in the vectors (the checker compares each order separately);
 * the order above is the hardware-sensible one: idle wait, interrupt/priority enables, address and count, request
 * wait, start.  Open point: whether the final CHCR read happens before or after the start store is not observable;
 * it is read after the store here (a read-back of the started channel's status).
 *
 * Host contract (not observable by the vectors): the device model only advances when told to, so each not-ready
 * poll ticks the host clock (HostFB_StreamTick) and, for channel 3, services the CD DMA (PE_CdReg_ServiceDMA3).
 * Each wait has a ShouldStop exit.  The CHCR wait is bounded by 0x10000 polls (the retail timeout count; the vectors
 * only reach 202); on expiry, and when the DRQ wait exceeds its budget, a diagnostic is printed and a stop is requested
 * instead of spinning.  What the retail code does after its own timeout is not shown by any vector; here the routine
 * returns the last CHCR value without starting the channel. */
#include <stdint.h>
#include <stdio.h>

#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/cdhost.h"
#include "pe_plat/framebuffer_api.h"

#define CDH_DMA_CHCR_BUSY 0x01000000u
#define CDH_DMA_CHCR_BUDGET 0x10000u
#define CDH_DMA_DRQ_BUDGET 0x40000u

/* Stuck-wait exit: the named boundary (diagnostic line, order-log entry and an unresolved-boundary stop).  The busy-channel
 * boundary keeps the name the suite looks for; a host quit already requested is never overridden (callers test it first). */
static void dma_stuck(const char *slot, const char *what, uint32_t channel, uint32_t polls)
{
    char why[96];
    snprintf(why, sizeof why, "channel %u %s wait did not finish after %u polls", (unsigned)channel, what, (unsigned)polls);
    PE_GuestHook_Refuse(slot, "pe_plat_cdhost_dma", 0x1F801088u + channel * 0x10u, 0, channel, polls, 0, 0, why);   /* fn = the channel CHCR address */
}

static void dma_tick(uint32_t channel)
{
    if (channel == 3u) PE_CdReg_ServiceDMA3();
    HostFB_StreamTick();
}

void pe_plat_cdhost_dma(uint32_t channel, uint32_t address, uint32_t blocks, uint32_t words,
                        uint32_t control, uint32_t interrupt, uint32_t unused)
{
    uint32_t base = 0x1F801080u + channel * 0x10u;
    uint32_t chcr_addr = base + 8u;
    uint32_t chcr, polls, p_dicr, p_dpcr, p_idx, dpcr;
    uint32_t enable_bit = 1u << (channel & 7u);
    uint8_t dicr_hi;
    (void)unused;

    /* 1. wait for the channel to be idle */
    chcr = PE_CdLoadU32(chcr_addr);
    interrupt &= 0xFFu;
    for (polls = 0; chcr & CDH_DMA_CHCR_BUSY; polls++) {
        if (PE_Port_ShouldStop()) return;
        if (polls >= CDH_DMA_CHCR_BUDGET) { dma_stuck("func_8007CEAC_busy", "CHCR busy", channel, polls); return; }
        dma_tick(channel);
        chcr = PE_CdLoadU32(chcr_addr);
    }

    /* 2. read DICR (byte at +2, then the word) and the DPCR pointer and value; every read precedes the stores below, so a
     *    DICR pointer aliasing the DPCR pointer word cannot change the DPCR pointer the transfer was set up with
     *    (the suite's pointer-alias test); the vectors place the DICR byte store before the DPCR store */
    p_dicr = PE_LoadU32(0x8009B348u);
    dicr_hi = PE_CdLoadU8(p_dicr + 2u);
    p_dicr = PE_LoadU32(0x8009B348u);
    (void)PE_CdLoadU32(p_dicr);
    p_dpcr = PE_LoadU32(0x8009B344u);
    dpcr = PE_CdLoadU32(p_dpcr);

    /* 3. DICR: per-channel interrupt enable (byte +2), DPCR: enable the channel */
    if (interrupt == 1u) dicr_hi = (uint8_t)(dicr_hi | enable_bit);
    else dicr_hi = (uint8_t)(dicr_hi & ~enable_bit);
    PE_CdStoreU8(p_dicr + 2u, dicr_hi);
    PE_CdStoreU32(p_dpcr, dpcr | (8u << (4u * (channel & 7u))));

    /* 4. address and block/word count */
    PE_CdStoreU32(base, address);
    PE_CdStoreU32(base + 4u, (blocks << 16) | words);

    /* 5. wait for the CD data request (DRQSTS, bit 6 of the status register) */
    p_idx = PE_LoadU32(0x8009B32Cu);
    for (polls = 0; !(PE_CdLoadU8(p_idx) & 0x40u); polls++) {
        if (PE_Port_ShouldStop()) return;
        if (polls >= CDH_DMA_DRQ_BUDGET) { dma_stuck("func_8007CEAC_drq", "DRQSTS", channel, polls); return; }
        dma_tick(channel);
    }

    /* 6. start, then read the channel status back */
    PE_CdStoreU32(chcr_addr, control);
    chcr = PE_CdLoadU32(chcr_addr);
    (void)chcr;
}

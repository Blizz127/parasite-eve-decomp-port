/* SPU data upload family (0x800850F4 upload, 0x800850C0 prepare, 0x80085174 DMA-complete wait,
 * 0x80085098 DMA-complete callback), bound by pe_plat/spu_upload.h.  Written clean-room from the black-box
 * vector sets cdgen48 (docs/evidence/native-cdx-20261126/cdgen48: upload_bound, prepare_bound, callback_bound,
 * wait; upload, upload_fault, prepare, callback as register-level cross-checks), psx-spx and the public
 * headers; no SDK source, decompiled source, tables or disassembly were read.  The behaviour is exactly what
 * the vectors show; no claim is made about the original's source.
 *
 * Guest words (little-endian guest RAM, read at every use):
 *   0x8009D24C  transfer-busy flag: 1 while an upload is in flight (set by prepare, cleared by the callback)
 *   0x8009B434  transfer callback word (owned by pe_plat_spub_set_callback, not touched here)
 * Guest address 0x80085098 is the callback the prepare step registers.
 *
 * Composition (cdgen48 REPORT, section 0): the upload is not a register programmer itself.  It calls the
 * prepare step and then the SPU RAM write adapter pe_plat_cdxspu_spu_write (platform/pe_plat2_cdx_spu.c: TSA,
 * SPUCNT, SPU_DELAY, DMA4 MADR/BCR/CHCR; the unsigned size clamp to 0x7EFF0 lives there, so the size passed
 * on is the caller's, unclamped).  The adapter's own result is not used.
 *
 * Host contracts that the vectors cannot show (all marked where applied):
 *  - Upload: a host DMA still pending from an earlier transfer completes first (PE_SpuDma_DeliverPending,
 *    documented in pe_plat/spu_dma_hw.h as the issuer's first step).  Without it that earlier transfer's
 *    completion would be delivered after this call armed the flag and the callback word, would clear them for
 *    the new transfer and release a following wait at once.  When the delivery ended in a stop nothing is
 *    issued.  This runs before the prepare step so the flag/callback words a pending transfer's own
 *    completion sees are the ones its issuer left.
 *  - Wait: nothing but the DMA interrupt of the host model clears the flag (the guest CPU has no
 *    interrupts here).  Each poll that still reads 1 gives that interrupt its opportunity
 *    (PE_SpuDma_Service) and one clock tick (pe_plat_sio_poll_tick; the port clock only moves on ticks).
 *    The loop leaves when a stop is requested.  A flag still set when no transfer is pending (the service
 *    delivered nothing) can only be a state retail spins in, so the wait stops the port with the
 *    "SPU_DMA/STALL func_80085174" diagnostic that the existing host-model tests (test_spu_dma_inflight.h)
 *    expect; a flag that survives WAIT_BUDGET polls of a running interrupt is stopped the same way. */
#include <stdint.h>
#include <stdio.h>

#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/spu_upload.h"
#include "pe_plat/spu_b.h"
#include "pe_plat/cdx_spu.h"
#include "pe_plat/spu_dma_hw.h"
#include "pe_plat/bios_file.h"

#define W_BUSY          0x8009D24Cu
#define CALLBACK_ADDR   0x80085098u
#define WAIT_BUDGET     0x40000u    /* polls of a flag that keeps reading 1 before the wait is called stuck */

void PE_Decomp_NoteStopDetail(const char *text);   /* pe_guest_decomp.h (not included: it pulls the game prototypes) */

void pe_plat_spuupload_prepare(void)
{
    PE_StoreU32(W_BUSY, 1u);
    pe_plat_spub_set_callback((int)CALLBACK_ADDR);
}

void pe_plat_spuupload_callback(void)
{
    pe_plat_spub_set_callback(0);
    PE_StoreU32(W_BUSY, 0u);
}

void pe_plat_spuupload_upload(uint32_t src, uint32_t size)
{
    if (!PE_SpuDma_DeliverPending()) return;   /* host contract: the earlier transfer completes first */
    pe_plat_spuupload_prepare();
    (void)pe_plat_cdxspu_spu_write(src, size);
}

void pe_plat_spuupload_wait(void)
{
    static const char stall[] = "SPU_DMA/STALL func_80085174: D_8009D24C=1 with no DMA4 transfer pending";
    uint32_t polls = 0u;

    while (PE_LoadU32(W_BUSY) == 1u) {
        if (PE_Port_ShouldStop()) return;
        if (polls++ >= WAIT_BUDGET) {      /* an interrupt keeps running but the flag never clears */
            PE_Decomp_NoteStopDetail("SPU upload wait: the transfer-busy flag 0x8009D24C stayed set "
                                     "after the poll budget");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return;
        }
        if (!PE_SpuDma_Service()) {        /* nothing pending: no interrupt will ever clear the flag */
            fprintf(stderr, "%s (retail spins until a DMA4 interrupt the host model has no transfer for)\n", stall);
            PE_Decomp_NoteStopDetail(stall);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return;
        }
        pe_plat_sio_poll_tick();
    }
}

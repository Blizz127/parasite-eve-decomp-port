/* libds DsRead (guest 0x80080E34): start a sector read (family cdx-dsread, batch cdgen49b).
 *
 * Written clean-room from the black-box vector set cdgen49b (docs/evidence/native-cdx-20261126/cdgen49b), the
 * cdgen49 report's contract text and psx-spx ("CDROM Controller": a location is minute, second, frame); no SDK
 * source, decompiled source, tables or disassembly were read, and no other implementation of this function was
 * opened.  The function is described by exactly the guest accesses and calls the vectors show.
 *
 * Behaviour, (pos, sectors, dest, mode):
 *  - returns 0, touching nothing, when the word 0x8009B6CC equals 1 (a read is already running) or when
 *    pe_plat_cd_error_active() is nonzero;
 *  - otherwise stores 0x200 at 0x8009B6AC, dest at 0x8009B6B0 and sectors at 0x8009B6B4;
 *  - pos == 0 means "the last position": pe_plat_ds_last_pos(0) returns where its four bytes are;
 *  - the four position bytes are copied into a frame of this function and the command sequence is started
 *    with (mode | 0x20, pointer to the frame copy, 6, 0x80080F64, -1);  the pointer argument is the copy, not pos,
 *    so the sequence reads what the copy holds;
 *  - when the sequence returns nonzero: VSync(-1)'s result goes to 0x8009B6C4; when bit 0 of the word 0x8009B6B8 is
 *    set the data-transfer callback slot is exchanged (new callback 0x80081110, previous one stored at 0x8009B6C0);
 *    then 0x8009B6CC is set to 1;
 *  - returns the command-sequence result unchanged (also when it is zero, which leaves the state untouched).
 */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_plat/cdx_dsread.h"
#include "pe_plat/hostfb_forward.h"

/* Host model: a read started with no device attached opened a loader device window (pe_plat2_cdhost_loader.c); the
 * game was ported against reads that are finished when the issue returns, so the real command chain is run to its end
 * here, after every guest access of this function.  Weak so the vector checkers (which link this file alone) skip it. */
int PE_CdHost_LoaderOpen(void) __attribute__((weak));
void PE_CdHost_LoaderDrain(void) __attribute__((weak));

#define DSR_BUSY        0x8009B6CCu  /* 1 while a read is running */
#define DSR_MODE_WORD   0x8009B6ACu  /* receives 0x200 */
#define DSR_DEST        0x8009B6B0u  /* read destination */
#define DSR_COUNT       0x8009B6B4u  /* sector count */
#define DSR_FLAGS       0x8009B6B8u  /* bit 0: install the transfer callback */
#define DSR_PREV_CB     0x8009B6C0u  /* previous transfer callback */
#define DSR_TICK        0x8009B6C4u  /* VSync count at start */
#define DSR_SEQ_CB      0x80080F64u  /* callback handed to the command sequence */
#define DSR_XFER_CB     0x80081110u  /* transfer callback installed on bit 0 */
#define DSR_SEQ_COMMAND 6
#define DSR_LOC_BYTES   4u

int pe_plat_cdxdsread_read(uint32_t pos, uint32_t sectors, uint32_t dest, uint32_t mode)
{
    pe_addr_t frame;
    uint32_t src, i;
    int result;

    if (PE_LoadU32(DSR_BUSY) == 1u || pe_plat_cd_error_active() != 0) return 0;

    if (PE_CdHost_LoaderOpen) (void)PE_CdHost_LoaderOpen();   /* host model: give the command sequence a drive (see above) */
    PE_StoreU32(DSR_MODE_WORD, 0x200u);
    PE_StoreU32(DSR_DEST, dest);
    PE_StoreU32(DSR_COUNT, sectors);

    src = pos != 0u ? pos : pe_plat_ds_last_pos(0u);
    frame = PE_GuestFrame_Enter(8u);
    for (i = 0; i < DSR_LOC_BYTES; i++) PE_StoreU8(frame + i, PE_LoadU8(src + i));

    result = pe_plat_cd_command_sequence_host((mode & 0xFFu) | 0x20u, frame, DSR_SEQ_COMMAND, DSR_SEQ_CB, 0xFFFFFFFFu);
    PE_GuestFrame_Leave(frame, 8u);

    if (result != 0) {
        PE_StoreU32(DSR_TICK, pe_plat_hostfb_vsync(-1));
        if (PE_LoadU32(DSR_FLAGS) & 1u) PE_StoreU32(DSR_PREV_CB, pe_plat_ds_data_callback(DSR_XFER_CB));
        PE_StoreU32(DSR_BUSY, 1u);
    }
    if (PE_CdHost_LoaderDrain) PE_CdHost_LoaderDrain();
    return result;
}

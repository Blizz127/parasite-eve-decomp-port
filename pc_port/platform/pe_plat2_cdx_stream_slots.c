/* The three CD callback-slot targets that the stream pump / progress handler reach through guest hook words:
 *   0x8007F7E8  libds VBlank hook, installed into slot 0x800A36A0 (called by the pump 0x8007FE24 with no arguments);
 *   0x8007F88C  libds status callback, installed into slot 0x800A36A8;
 *   0x80080778  default CD ready callback, installed into D_8009AFB8 (status, response).
 *
 * PROVENANCE: written clean-room from the black-box vector sets cdgen42c / cdgen42ci
 * (docs/evidence/native-cdx-20261126/cdgen42c, vectors in scratch: 9768 + 2432 cases, including the runtime-observed
 * tuples), the recorded seam contracts and psx-spx; no SDK source, decompiled source, tables or disassembly were read.
 * Author: sw-cdx-stream3 (in-process helper, Sonnet 5.5).  Report:
 * docs/evidence/native-cdx-20261126/cdx-stream2/SLOTS-REPORT.md.  The retail names are bound to the entries below by
 * object-like macros in pe_plat/cdx_stream.h.
 *
 * Contracts, exactly what the vectors show (all words little-endian guest RAM):
 *  - 0x8007F7E8: the lane getter pe_plat_ds_status_lane(0) is asked first; unless it answers 1 the hook returns 1.  Then
 *    word 0x800A3608 (signed): when <= 0 the hook returns that value.  The lane getter is asked a second time; unless it
 *    answers 1 the hook returns that answer.  Then index = word 0x800A3604 (signed, unchecked) selects the 24-byte record at
 *    0x800A3540 + 24 * index; when the record's first word is zero the hook returns zero, otherwise the issue entry
 *    pe_plat_cd_try_issue_cmd(record byte +4 (unsigned), record word +0xC) runs and its result is returned.  No stores.
 *  - 0x8007F88C(status, response): the status is reduced to its low byte.  Bytes 1 and 5 land in the record at 0x800A3520,
 *    byte 4 in the record at 0x800A3530: first word = 1, byte +4 = the status, then the 8 bytes at `response` (copied byte by
 *    byte) into +5..+0xC.  For status 5 the first response byte is read before the copy and, when its bit 4 is set, the
 *    command-queue flush pe_plat_ds_text_170(5, response) runs before any store.  After that the user slot word 0x800B8AB4,
 *    when nonzero, is called as slot(status, response) and its result is returned (otherwise the result is zero).
 *  - 0x80080778(status, response): the decoder runs with (low byte of status, response); when bit 4 of byte 0x8009B56C is set,
 *    word 0x8009B574 = 2 and word 0x8009B578 = 0xC; then, when slot word 0x800A36A8 is nonzero and gate word 0x8009B554 is
 *    nonzero, the slot (re-read) is called as slot(status byte, response) and its result is returned (otherwise zero).
 * Guest callbacks are dispatched only through PE_GuestHook_Call (never a bound host symbol). */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/cd.h"
#include "pe_plat/cdx_stream.h"
#include "pe_guestcode.h"
#include "game_port.h"

#define W_USER_SLOT     0x800B8AB4u
#define W_SLOT_PROG_B   0x800A36A8u
#define W_GATE          0x8009B554u
#define B_DRIVE_STAT    0x8009B56Cu
#define W_PHASE         0x8009B574u
#define W_STATE         0x8009B578u
#define W_QUEUE_INDEX   0x800A3604u
#define W_QUEUE_COUNT   0x800A3608u
#define A_QUEUE_BASE    0x800A3540u
#define A_REC_A         0x800A3520u
#define A_REC_B         0x800A3530u

/* Call a guest hook word's target with (a0, a1); returns the callee's result (zero when no call happened). */
static uint32_t cdxsl_call(const char *slot, const char *caller, uint32_t fn, uint32_t a0, uint32_t a1)
{
    int ret = 0;
    if (!PE_GuestHook_Call(slot, caller, fn, 2u, a0, a1, 0, 0, &ret)) return 0;
    return (uint32_t)ret;
}

/* ---- 0x8007F7E8: VBlank hook ----------------------------------------- */
uint32_t pe_plat_cdxstream_vblank_hook(void)
{
    int32_t count, lane;
    uint32_t rec, first;
    if (pe_plat_ds_status_lane(0) != 1) return 1u;
    count = (int32_t)PE_LoadU32(W_QUEUE_COUNT);
    if (count <= 0) return (uint32_t)count;
    lane = pe_plat_ds_status_lane(0);
    if (lane != 1) return (uint32_t)lane;
    rec = A_QUEUE_BASE + 24u * PE_LoadU32(W_QUEUE_INDEX);
    first = PE_LoadU32(rec);
    if (first == 0) return 0;
    return (uint32_t)pe_plat_cd_try_issue_cmd(PE_LoadU8(rec + 4u), PE_LoadU32(rec + 0xCu));
}

/* ---- 0x8007F88C: status callback -------------------------------------- */
uint32_t pe_plat_cdxstream_status_callback(int status, uint32_t response)
{
    uint32_t st = (uint32_t)status & 0xFFu, slot, i;
    uint32_t rec = 0;
    if (st == 5u && (PE_LoadU8(response) & 0x10u)) pe_plat_ds_text_170(5u, response);
    if (st == 1u || st == 5u) rec = A_REC_A;
    else if (st == 4u) rec = A_REC_B;
    if (rec) {
        PE_StoreU32(rec, 1u);
        PE_StoreU8(rec + 4u, (uint8_t)st);
        for (i = 0; i < 8u; i++) PE_StoreU8(rec + 5u + i, PE_LoadU8(response + i));
    }
    slot = PE_LoadU32(W_USER_SLOT);
    if (slot == 0) return 0;
    return cdxsl_call("CD_data_callback", "func_8007F88C", slot, st, response);
}

/* ---- 0x80080778: default ready callback ------------------------------- */
uint32_t pe_plat_cdxstream_ready_callback(int status, uint32_t response)
{
    uint32_t st = (uint32_t)status & 0xFFu, slot;
    pe_plat_cdxstream_decode_status((int)st, response);
    if (PE_LoadU8(B_DRIVE_STAT) & 0x10u) {
        PE_StoreU32(W_PHASE, 2u);
        PE_StoreU32(W_STATE, 0xCu);
    }
    slot = PE_LoadU32(W_SLOT_PROG_B);
    if (slot == 0) return 0;
    if (PE_LoadU32(W_GATE) == 0) return 0;
    slot = PE_LoadU32(W_SLOT_PROG_B);
    return cdxsl_call("CD_data_callback", "func_80080778", slot, st, response);
}

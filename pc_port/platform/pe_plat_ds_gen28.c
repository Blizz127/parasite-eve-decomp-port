/* libds text routine 0x80081E70 (the DS ready callback, installed through
 * 0x800824C8 by DsStartReadySystem 0x80081D74 and by 0x80082204), written as
 * an independent clean-room re-implementation from black-box vectors of the
 * original code (pc_port/tools/pe_cd_leaf_vectors.py cdgen28, vectors v2
 * (vectors.v2.txt, sha256 9d26d17a3d4a4d060f18da323476ba13e9392986a745f0cea7df1d053c83f58e;
 * also passes v1, vectors.txt, sha256
 * f073dce6e055ff359745c8b579ad729274040adeb5f65cd677f7f00013e4cdc6), checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Its behaviour is
 * exactly the guest loads, stores and callee calls seen in the vectors.
 *
 * It polls no hardware; every wait it takes part in lives in its callees.
 * The original keeps a 12-byte buffer in its own stack frame; here it is a
 * guest frame.
 *
 * Rules (a = ready-system block at 0x8009B6EC):
 *   1. If the position word a+0x00 is -1, store CdPosToInt(CdPositionPtr())
 *      into it.
 *   2. By interrupt byte:
 *      - 1: if the CD system command mode byte has bit 0x20, read three words
 *        into the frame buffer (DsDataCallback is cleared around the read and
 *        restored) and convert the buffer with CdPosToInt; when that differs
 *        from the live position word, set the flag word 0x8009B6F8 to 1 and go
 *        to rule 3 (otherwise, and when the bit is clear, continue).  Then, if
 *        the callback word a+0x08 is non-zero and the signed position word is
 *        greater than the signed last-position word a+0x04, call it with
 *        (1, result, buffer) and copy the position into the last-position
 *        word.  The position word is then incremented; continue with rule 3.
 *      - 4: call the callback word (4, result, buffer) when it is non-zero;
 *        continue with rule 3.
 *      - anything else: if result byte 0 has bit 0x10: with the mode word
 *        (0x8009B708) equal to 1, only call 0x800824C8(0); otherwise
 *        re-install the handlers 0x8009B700 / 0x8009B704 (0x800824C8 /
 *        0x800824DC), clear the pending word 0x8009B70C and call the callback
 *        word with (interrupt, result, buffer); in both cases nothing
 *        further.  Without bit 0x10: when the queue length (0x8007F778) is 0
 *        and result byte 0 (read again) has neither bit 0x20 nor bit 0x80,
 *        set the flag word to 1; continue with rule 3.
 *   3. If the flag word is 1: when the counter word 0x8009B6FC is positive
 *      (signed) or exactly -1, call text_490 and, if the counter is then
 *      positive, decrement it; otherwise (0 or any negative other than -1),
 *      when the pending word is exactly 1 re-install the handlers and issue
 *      DsCommand(9, 0, 0, -1), then (pending or not) clear the pending word and
 *      call the callback word with (5, result, buffer) when it is non-zero.
 *      Finally clear the flag word.
 */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_plat/cd.h"

#define ADDR_POS        0x8009B6ECu  /* position word (-1 when unset) */
#define ADDR_LAST_POS   0x8009B6F0u  /* last position */
#define ADDR_CALLBACK   0x8009B6F4u  /* three-argument callback word, 0 when unset */
#define ADDR_FLAG       0x8009B6F8u  /* ready flag */
#define ADDR_COUNT      0x8009B6FCu  /* signed counter */
#define ADDR_HANDLER_A  0x8009B700u  /* value passed to 0x800824C8 */
#define ADDR_HANDLER_B  0x8009B704u  /* value passed to 0x800824DC */
#define ADDR_MODE       0x8009B708u  /* mode word */
#define ADDR_PENDING    0x8009B70Cu  /* pending word */

#define INTR_DATA_READY 1u           /* interrupt byte values with their own handling */
#define INTR_DATA_END   4u
#define INTR_DONE_CODE  5u           /* first callback argument on completion */
#define RESULT_BIT_DIRECT 0x10u      /* result byte 0 bit that selects the direct path */
#define RESULT_BITS_BUSY  0xA0u      /* result byte 0 bits that suppress the flag set */
#define SYS_MODE_BIT_SECTOR 0x20u    /* system command mode bit that selects the sector check */
#define POS_UNSET       0xFFFFFFFFu
#define COUNT_UNSET     0xFFFFFFFFu  /* counter value -1: still waiting */
#define MODE_ONE        1u
#define PENDING_ONE     1u
#define SECTOR_WORDS    3
#define FRAME_BYTES     12u          /* three-word sector buffer */
#define DS_CMD_RESET    9u           /* DsCommand arguments (9, 0, 0, -1) */
#define DS_CMD_CALLBACK 0xFFFFFFFFu

static void callback_word(pe_addr_t cb, unsigned code, uint32_t result, pe_addr_t buffer)
{
    (void)PE_GuestCall("@80081E70:cb", cb, 3, (uintptr_t)code, (uintptr_t)result,
                       (uintptr_t)buffer, 0);
}

/* The second half: flag word, counter, handler re-installation. */
static void ready_tail(uint32_t result, pe_addr_t buffer)
{
    uint32_t count;
    int32_t after;
    pe_addr_t cb;

    if (PE_LoadU32(ADDR_FLAG) != 1u)
        return;
    count = PE_LoadU32(ADDR_COUNT);
    if ((int32_t)count > 0 || count == COUNT_UNSET) {
        pe_plat_ds_text_490();
        after = (int32_t)PE_LoadU32(ADDR_COUNT);
        if (after > 0)
            PE_StoreU32(ADDR_COUNT, (uint32_t)after - 1u);
    } else {
        if (PE_LoadU32(ADDR_PENDING) == PENDING_ONE) {
            (void)pe_plat_cd_ds_set_ready_callback((int)PE_LoadU32(ADDR_HANDLER_A));
            (void)pe_plat_cd_ds_set_start_callback((int)PE_LoadU32(ADDR_HANDLER_B));
            (void)func_8007EE84(DS_CMD_RESET, 0u, 0u, DS_CMD_CALLBACK);
        }
        PE_StoreU32(ADDR_PENDING, 0u);
        cb = PE_LoadU32(ADDR_CALLBACK);
        if (cb != 0u)
            callback_word(cb, INTR_DONE_CODE, result, buffer);
    }
    PE_StoreU32(ADDR_FLAG, 0u);
}

/* Interrupt 1, after the optional sector check: report a newer position. */
static void data_ready_step(uint32_t result, pe_addr_t buffer)
{
    pe_addr_t cb = PE_LoadU32(ADDR_CALLBACK);

    if (cb != 0u) {
        int32_t last = (int32_t)PE_LoadU32(ADDR_LAST_POS);
        if ((int32_t)PE_LoadU32(ADDR_POS) > last) {
            callback_word(cb, INTR_DATA_READY, result, buffer);
            PE_StoreU32(ADDR_LAST_POS, PE_LoadU32(ADDR_POS));
        }
    }
    PE_StoreU32(ADDR_POS, PE_LoadU32(ADDR_POS) + 1u);
}

void pe_plat_ds_ready_callback(unsigned char intr, uint32_t result)
{
    pe_addr_t buffer = PE_GuestFrame_Enter(FRAME_BYTES);
    pe_addr_t cb;

    if (PE_LoadU32(ADDR_POS) == POS_UNSET)
        PE_StoreU32(ADDR_POS, (uint32_t)pe_plat_cd_pos_to_int(pe_plat_cd_position_ptr()));

    if (intr == INTR_DATA_READY) {
        int proceed = 1;

        if ((pe_plat_cd_sys_command_mode() & SYS_MODE_BIT_SECTOR) != 0u) {
            pe_addr_t previous = func_800824F0(0u);
            uint32_t sector_pos;

            (void)pe_plat_cd_ds_get_sector(buffer, SECTOR_WORDS);
            (void)func_800824F0(previous);
            sector_pos = (uint32_t)pe_plat_cd_pos_to_int(buffer);
            /* the position word is read after the conversion */
            if (sector_pos != PE_LoadU32(ADDR_POS)) {
                PE_StoreU32(ADDR_FLAG, 1u);
                proceed = 0;
            }
        }
        if (proceed)
            data_ready_step(result, buffer);
        ready_tail(result, buffer);
    } else if (intr == INTR_DATA_END) {
        cb = PE_LoadU32(ADDR_CALLBACK);
        if (cb != 0u)
            callback_word(cb, INTR_DATA_END, result, buffer);
        ready_tail(result, buffer);
    } else if ((PE_LoadU8(result) & RESULT_BIT_DIRECT) != 0u) {
        if (PE_LoadU32(ADDR_MODE) == MODE_ONE) {
            (void)pe_plat_cd_ds_set_ready_callback(0);
        } else {
            (void)pe_plat_cd_ds_set_ready_callback((int)PE_LoadU32(ADDR_HANDLER_A));
            (void)pe_plat_cd_ds_set_start_callback((int)PE_LoadU32(ADDR_HANDLER_B));
            PE_StoreU32(ADDR_PENDING, 0u);
            cb = PE_LoadU32(ADDR_CALLBACK);
            if (cb != 0u)
                callback_word(cb, intr, result, buffer);
        }
    } else {
        if (pe_plat_cd_queue_len() == 0 && (PE_LoadU8(result) & RESULT_BITS_BUSY) == 0u)
            PE_StoreU32(ADDR_FLAG, 1u);
        ready_tail(result, buffer);
    }
    PE_GuestFrame_Leave(buffer, FRAME_BYTES);
}

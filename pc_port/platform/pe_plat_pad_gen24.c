/* libpad "card transfer step" (guest 0x80083014), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen24; vectors.txt sha256
 * d2bd8daa7fc9c68703314df53ad558fe6a49b4ffaa1ceb4ba7ac1bf59cd56c10, 6000
 * cases, checked by pc_port/tools/pe_spu_caller_check.c), with no SDK source.
 * An independent re-implementation: behaviour is exactly the guest loads,
 * stores and callee/slot calls seen in the vectors.
 *
 * One call runs one step of a card transfer.  A step index word is
 * post-incremented and selects an entry of a guest step table (4 bytes per
 * entry, no bound check, 32-bit address wrap) that is called with the object.
 * A negative result goes straight to the error hook.  Otherwise, unless the
 * index word is now zero, the RCNT2 timeout is started and the SIO
 * acknowledge awaited; a zero acknowledge result reports -3 to the error
 * hook.  Finally an index word above 4 is decremented.
 *
 * The function is void in the port (the original's return register is a
 * leftover that its only caller overwrites). */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_plat/pad.h"
#include "pe_plat/bios_file.h"

#define ADDR_STEP_INDEX   0x8009B768u  /* step index word (signed) */
#define ADDR_STEP_TABLE   0x8009B7A8u  /* guest function pointers, 4 bytes per index */
#define ADDR_ERROR_HOOK   0x8009B724u  /* one argument: the (negative) error code */

#define TIMEOUT_LIMIT     0x3Cu        /* argument of the RCNT2 timeout start */
#define ERR_ACK_TIMEOUT   0xFFFFFFFDu  /* -3: acknowledge wait returned zero */
#define INDEX_DECR_ABOVE  4            /* index words above this (signed) are decremented */

void pe_plat_pad_transfer_step(uint32_t obj)
{
    uint32_t idx = PE_LoadU32(ADDR_STEP_INDEX);
    uint32_t entry = PE_LoadU32(ADDR_STEP_TABLE + (idx << 2));
    uint32_t r;
    int32_t cur;

    PE_StoreU32(ADDR_STEP_INDEX, idx + 1u);
    r = PE_Card_ICall("D_8009B7A8[] (card transfer step)", entry, 1, obj, 0, 0);

    if ((int32_t)r < 0) {
        PE_Card_ICall("D_8009B724 (card error hook)", PE_LoadU32(ADDR_ERROR_HOOK), 1, r, 0, 0);
        return;
    }
    if (PE_LoadU32(ADDR_STEP_INDEX) != 0u) {
        pe_plat_pad_rc2_wait_set(TIMEOUT_LIMIT);
        if (pe_plat_card_sio_ack_wait() == 0)
            PE_Card_ICall("D_8009B724 (card error hook)", PE_LoadU32(ADDR_ERROR_HOOK), 1,
                          ERR_ACK_TIMEOUT, 0, 0);
    }
    cur = (int32_t)PE_LoadU32(ADDR_STEP_INDEX);
    if (cur > INDEX_DECR_ABOVE)
        PE_StoreU32(ADDR_STEP_INDEX, (uint32_t)cur - 1u);
}

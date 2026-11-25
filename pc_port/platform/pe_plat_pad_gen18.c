/* Memory-card object state dispatcher (guest 0x80084B78), written clean-room
 * from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen18, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the guest loads and calls seen in the vectors; the result is always 0. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"

#define OBJ_HOOK_OFF          0x14u   /* word: guest function pointer, one argument (obj) */
#define OBJ_RECORD_PTR_OFF    0x3Cu   /* word: pointer to the record's first byte */
#define OBJ_STATE_OFF         0x46u   /* byte: state value */
#define OBJ_FLAG_OFF          0xE8u   /* byte: tested for zero when the record byte is 0xF3 */

#define RECORD_BYTE_SPECIAL   0xF3u   /* record byte value that makes the flag byte matter */

#define STATE_IDLE            0x00u   /* nothing is called */
#define STATE_CMD_ONE         0x01u   /* func_80083E50 (obj, CMD_ARG_ONE) */
#define STATE_CMD_ZERO        0xFEu   /* func_80083E50 (obj, CMD_ARG_ZERO) */
#define STATE_NONE            0xFFu   /* nothing is called */

#define CMD_ARG_ZERO          0x00u   /* second argument of func_80083E50 */
#define CMD_ARG_ONE           0x01u

/* 0x80084B78 always returns 0.
 *  1. Read the word at obj+0x3C (a record pointer) and the byte it addresses.
 *  2. When that byte is exactly 0xF3, read the byte at obj+0xE8.  When it is
 *     zero, call func_80083E50 (obj, 0) and return, without reading the state.
 *  3. Otherwise read the state byte at obj+0x46:
 *      - 0x00 or 0xFF: nothing further.
 *      - 0x01: call func_80083E50 (obj, 1).
 *      - 0xFE: call func_80083E50 (obj, 0).
 *      - any other value (0x02..0xFD): read the hook word at obj+0x14.  When it
 *        is non-zero, call it through the guest dispatcher with (obj); when it
 *        is zero, call func_800835C0 (obj). */
int pe_plat_pad_card_dispatch(uint32_t obj)
{
    uint32_t record = PE_LoadU32(obj + OBJ_RECORD_PTR_OFF);
    unsigned state;

    if (PE_LoadU8(record) == RECORD_BYTE_SPECIAL &&
        PE_LoadU8(obj + OBJ_FLAG_OFF) == 0) {
        func_80083E50(obj, CMD_ARG_ZERO);
        return 0;
    }

    state = PE_LoadU8(obj + OBJ_STATE_OFF);
    if (state == STATE_IDLE || state == STATE_NONE)
        return 0;
    if (state == STATE_CMD_ONE) {
        func_80083E50(obj, CMD_ARG_ONE);
        return 0;
    }
    if (state == STATE_CMD_ZERO) {
        func_80083E50(obj, CMD_ARG_ZERO);
        return 0;
    }

    {
        uint32_t hook = PE_LoadU32(obj + OBJ_HOOK_OFF);

        if (hook != 0)
            PE_GuestCall("func_80084B78 a0->+0x14 card hook", hook, 1, obj, 0, 0, 0);
        else
            func_800835C0(obj);
    }
    return 0;
}

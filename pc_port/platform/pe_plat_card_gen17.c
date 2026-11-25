/* Memory-card record failure/retry updater (guest 0x80084EB0), written
 * clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen17, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the guest loads, stores and callback-word call seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"

#define ADDR_RESET_CALLBACK   0x8009B728u  /* word: guest function pointer, one argument (obj) */

#define OBJ_LINK_OFF          0x30u   /* word: address of a two-byte cell */
#define OBJ_TARGET_PTR_OFF    0x3Cu   /* word: pointer to a byte that is tested */
#define OBJ_STATE_OFF         0x46u   /* byte: state value */
#define OBJ_RETRY_B_OFF       0x49u   /* byte: tested for non-zero; stored 2 on a state-1 overflow */
#define OBJ_RETRY_A_OFF       0x4Au   /* byte: counter, incremented while below its limit */
#define OBJ_TICK_OFF          0x4Cu   /* word: counter incremented on every call (wraps) */
#define OBJ_CLEAR_OFF         0xE8u   /* byte: zeroed when the cell is rewritten */

#define STATE_IDLE            0x00u   /* no counter step; goes straight to the pointer test */
#define STATE_FIRST           0x01u   /* counter limit RETRY_A_LIMIT_FIRST */
#define RETRY_B_FULL_VALUE    0x02u   /* value stored to obj+0x49 when the state-1 counter is full */
#define STATE_FAILED          0xFFu   /* value stored to obj+0x46 when the state-1 counter is full */

#define RETRY_A_LIMIT_FIRST   2u      /* state 1: counter steps while byte < 2 */
#define RETRY_A_LIMIT_OTHER   4u      /* states >= 2: counter steps while byte < 4 */

#define TARGET_BYTE_SKIP      0xF3u   /* pointed-to byte value that skips the cell rewrite */
#define CELL_BYTE0_VALUE      0xFFu   /* stored at cell+0 */
#define CELL_BYTE1_VALUE      0x00u   /* stored at cell+1 */
#define CLEAR_VALUE           0x00u   /* stored at obj+0xE8 */

/* Read the pointer at obj+0x3C and the byte it addresses.  When that byte is
 * not 0xF3, read the word at obj+0x30 once for each of the two cell stores: 0xFF at
 * that address and 0x00 at the next, and zero the byte at obj+0xE8. */
static void card_target_rewrite(uint32_t obj)
{
    uint32_t target = PE_LoadU32(obj + OBJ_TARGET_PTR_OFF);

    if (PE_LoadU8(target) == TARGET_BYTE_SKIP)
        return;
    PE_StoreU8(PE_LoadU32(obj + OBJ_LINK_OFF), CELL_BYTE0_VALUE);
    PE_StoreU8(PE_LoadU32(obj + OBJ_LINK_OFF) + 1u, CELL_BYTE1_VALUE);
    PE_StoreU8(obj + OBJ_CLEAR_OFF, CLEAR_VALUE);
}

/* 0x80084EB0: read the word at obj+0x4C and store it back plus one (wrapping)
 * before anything else.  Then read the state byte at obj+0x46:
 *  - state 0: perform the pointer test / cell rewrite.
 *  - state 1: read the byte at obj+0x4A.  When it is below 2, store it plus one
 *    back to obj+0x4A.  Otherwise store 2 to obj+0x49 and then 0xFF to obj+0x46.
 *  - any other state (>= 2): read the byte at obj+0x4A.  When it is below 4,
 *    store it plus one back to obj+0x4A.  Otherwise read the byte at obj+0x49;
 *    when it is non-zero, read the callback word at 0x8009B728 and call it with
 *    (obj).  Then perform the pointer test / cell rewrite. */
void pe_plat_card_dir_fail_auto(uint32_t obj)
{
    uint32_t tick = PE_LoadU32(obj + OBJ_TICK_OFF);
    uint32_t state, retry_a;

    PE_StoreU32(obj + OBJ_TICK_OFF, tick + 1u);
    state = PE_LoadU8(obj + OBJ_STATE_OFF);

    if (state == STATE_IDLE) {
        card_target_rewrite(obj);
        return;
    }

    retry_a = PE_LoadU8(obj + OBJ_RETRY_A_OFF);
    if (state == STATE_FIRST) {
        if (retry_a < RETRY_A_LIMIT_FIRST) {
            PE_StoreU8(obj + OBJ_RETRY_A_OFF, (uint8_t)(retry_a + 1u));
        } else {
            PE_StoreU8(obj + OBJ_RETRY_B_OFF, RETRY_B_FULL_VALUE);
            PE_StoreU8(obj + OBJ_STATE_OFF, STATE_FAILED);
        }
        return;
    }

    if (retry_a < RETRY_A_LIMIT_OTHER) {
        PE_StoreU8(obj + OBJ_RETRY_A_OFF, (uint8_t)(retry_a + 1u));
        return;
    }
    if (PE_LoadU8(obj + OBJ_RETRY_B_OFF) != 0) {
        uint32_t fn = PE_LoadU32(ADDR_RESET_CALLBACK);
        PE_Card_ICall("D_8009B728 (card record reset)", fn, 1, obj, 0, 0);
    }
    card_target_rewrite(obj);
}

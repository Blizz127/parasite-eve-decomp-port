/* Two small libcard slot helpers (guest 0x80082B08 and 0x80083DF0), written
 * clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen14, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the guest loads, stores and callback-word calls seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"

#define ADDR_CARD_STATE_PTR   0x8009B784u  /* word: pointer to a two-word state block */
#define ADDR_POLL_CALLBACK    0x8009B74Cu  /* word: guest function pointer, no arguments */
#define ADDR_RESET_CALLBACK   0x8009B728u  /* word: guest function pointer, one argument */

#define STATE_WORD_FIRST_OFF  0x04u        /* first tested word: state + 0x04 */
#define STATE_WORD_SECOND_OFF 0x00u        /* second tested word: state + 0x00 */
#define STATE_WORD_FLAG       0x1u         /* a word passes when its low bit is set */

#define OBJ_GATE_BYTE_OFF     0x53u        /* byte at obj+0x53; zero selects the callback */
#define OBJ_STATE_BYTE_OFF    0x46u        /* byte at obj+0x46 */
#define OBJ_STATE_BUSY        0x02u        /* value of obj+0x46 that is left untouched */
#define OBJ_STATE_CLEARED     0xFEu        /* value stored to obj+0x46 otherwise */

/* 0x80082B08: read the state-block pointer at 0x8009B784.  Read the word at
 * state+4; if its low bit is clear, return 0.  Otherwise read the word at
 * state+0; if its low bit is clear, return 0.  Otherwise read the callback word
 * at 0x8009B74C and, when it is non-zero, call it with no arguments (the result
 * is ignored).  Return 1.  No guest stores. */
int pe_plat_card_slot_poll(void)
{
    uint32_t state = PE_LoadU32(ADDR_CARD_STATE_PTR);
    uint32_t fn;

    if (!(PE_LoadU32(state + STATE_WORD_FIRST_OFF) & STATE_WORD_FLAG))
        return 0;
    if (!(PE_LoadU32(state + STATE_WORD_SECOND_OFF) & STATE_WORD_FLAG))
        return 0;
    fn = PE_LoadU32(ADDR_POLL_CALLBACK);
    if (fn != 0)
        PE_GuestCall("@80082B08:D_8009B74C", fn, 0, 0, 0, 0, 0);
    return 1;
}

/* 0x80083DF0: read the byte at obj+0x53.  When it is zero, read the callback
 * word at 0x8009B728, call it with (obj) and return 0.  Otherwise read the byte
 * at obj+0x46: when it equals 2 return 1 with no store; for any other value
 * store 0xFE to obj+0x46 and return 0. */
int pe_plat_card_obj_reset(uint32_t obj)
{
    uint32_t fn;

    if (PE_LoadU8(obj + OBJ_GATE_BYTE_OFF) == 0) {
        fn = PE_LoadU32(ADDR_RESET_CALLBACK);
        PE_GuestCall("@80083DF0:(*host_D_8009B728)", fn, 1, obj, 0, 0, 0);
        return 0;
    }
    if (PE_LoadU8(obj + OBJ_STATE_BYTE_OFF) == OBJ_STATE_BUSY)
        return 1;
    PE_StoreU8(obj + OBJ_STATE_BYTE_OFF, OBJ_STATE_CLEARED);
    return 0;
}

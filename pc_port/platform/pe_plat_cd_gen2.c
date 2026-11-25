/* Four more small libcd / libds callers (guest 0x800808BC, 0x80081D74,
 * 0x80081DF8, 0x800819D8), written clean-room from black-box vectors of the
 * original code (pc_port/tools/pe_cd_leaf_vectors.py cdgen2, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Each function's
 * behaviour is exactly the guest loads/stores, calls and return value seen
 * in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* Guest data words read or written by the callers. */
#define ADDR_CD_CMD_CLEAR  0x8009B554u  /* cleared by 0x800808BC before its call */
#define ADDR_CD_STATE      0x8009B574u  /* 0x800808BC acts only when this is 2 */
#define ADDR_CD_SUBSTATE   0x8009B578u  /* checked by 0x800808BC when state is 2 */
#define ADDR_DS_PEND_A     0x8009B6ECu  /* set to -1 by 0x80081D74 */
#define ADDR_DS_PEND_B     0x8009B6F0u  /* cleared by 0x80081D74 */
#define ADDR_DS_PEND_C     0x8009B6F8u  /* cleared by 0x80081D74 */
#define ADDR_DS_ARG0       0x8009B6F4u  /* a0 of 0x80081D74 stored here */
#define ADDR_DS_ARG1       0x8009B6FCu  /* a1 of 0x80081D74 stored here */
#define ADDR_DS_READY_CB   0x8009B700u  /* result of 0x800824C8 */
#define ADDR_DS_START_CB   0x8009B704u  /* result of 0x800824DC */
#define ADDR_DS_STATE      0x8009B70Cu  /* 1 = ready system active */
#define ADDR_TEXT_TABLE    0x800A3CB4u  /* table searched by 0x800819D8 */

/* Guest code addresses observed in the vectors as call arguments. */
#define CB_DS_READY        0x80081E70u  /* a0 of 0x800824C8 from 0x80081D74 */
#define CB_DS_START        0x8008214Cu  /* a0 of 0x800824DC from 0x80081D74 */

#define CD_STATE_BUSY        2u    /* 0x800808BC state value that is examined */
#define CD_SUBSTATE_RESET_A  0xBu  /* substate values that reset the state */
#define CD_SUBSTATE_RESET_B  0x10u
#define CD_SUBSTATE_RESET_C  0x11u
#define CD_STATE_RESET       1u    /* state value stored on reset */
#define CD_SUBSTATE_RESET    0xBu  /* substate value stored on reset */
#define DS_STATE_ACTIVE      1u    /* state value of the ready system */
#define DS_END_COMMAND       9     /* command passed to 0x8007EE84 on end */
#define DS_END_CALLBACK      (-1)  /* callback passed to 0x8007EE84 on end */
#define TEXT_ENTRY_STRIDE    0x2Cu /* bytes per 0x800819D8 table entry */
#define TEXT_NAME_OFFSET     8u    /* name string offset in an entry */

/* 0x800808BC: clear the command word, call 0x8007B9EC, then when the state
 * word is 2 and the substate word is 0xB, 0x10 or 0x11, set the state to 1
 * and the substate to 0xB (those two stores come after the call). */
void pe_plat_cd_abort_cmd(void)
{
    PE_StoreU32(ADDR_CD_CMD_CLEAR, 0);
    func_8007B9EC();
    if (PE_LoadU32(ADDR_CD_STATE) == CD_STATE_BUSY) {
        uint32_t sub = PE_LoadU32(ADDR_CD_SUBSTATE);
        if (sub == CD_SUBSTATE_RESET_A || sub == CD_SUBSTATE_RESET_B ||
            sub == CD_SUBSTATE_RESET_C) {
            PE_StoreU32(ADDR_CD_STATE, CD_STATE_RESET);
            PE_StoreU32(ADDR_CD_SUBSTATE, CD_SUBSTATE_RESET);
        }
    }
}

/* 0x80081D74: when the state word is already 1, return 0.  Otherwise store
 * -1, 0, 0, a0, a1 into the pending words, call 0x800824C8 and 0x800824DC
 * with the two ready/start handlers (a1 passed along, ignored), storing each
 * result (the first before the second call), set the state to 1 and
 * return 1. */
int pe_plat_ds_start_ready_system(int a0, int a1)
{
    int ready, start;

    if (PE_LoadU32(ADDR_DS_STATE) == DS_STATE_ACTIVE)
        return 0;
    PE_StoreU32(ADDR_DS_PEND_A, 0xFFFFFFFFu);
    PE_StoreU32(ADDR_DS_PEND_B, 0);
    PE_StoreU32(ADDR_DS_PEND_C, 0);
    PE_StoreU32(ADDR_DS_ARG0, (uint32_t)a0);
    PE_StoreU32(ADDR_DS_ARG1, (uint32_t)a1);
    ready = func_800824C8((int)CB_DS_READY);
    PE_StoreU32(ADDR_DS_READY_CB, (uint32_t)ready);
    start = func_800824DC((int)CB_DS_START);
    PE_StoreU32(ADDR_DS_START_CB, (uint32_t)start);
    PE_StoreU32(ADDR_DS_STATE, DS_STATE_ACTIVE);
    return 1;
}

/* 0x80081DF8: when the state word is 1, restore the two stored handlers via
 * 0x800824C8 and 0x800824DC, call 0x8007EE84(9, 0, 0, -1).  The state word
 * is cleared in every case, after those calls. */
void pe_plat_ds_end_ready_system(void)
{
    if (PE_LoadU32(ADDR_DS_STATE) == DS_STATE_ACTIVE) {
        int ready = (int)PE_LoadU32(ADDR_DS_READY_CB);
        int start = (int)PE_LoadU32(ADDR_DS_START_CB);
        func_800824C8(ready);
        func_800824DC(start);
        func_8007EE84(DS_END_COMMAND, 0, 0, (uint32_t)DS_END_CALLBACK);
    }
    PE_StoreU32(ADDR_DS_STATE, 0);
}

/* 0x800819D8: walk the 0x2C-byte entries from 0x800A3CB4 until an entry whose
 * first word is 0.  An entry whose first word equals a0 and whose name at
 * +8 compares equal (0x80072A54 == 0) to a1 ends the search with its
 * 1-based index; no match returns -1. */
int pe_plat_ds_text_5c4(int a0, pe_plat_guest_addr a1)
{
    pe_plat_guest_addr entry = ADDR_TEXT_TABLE;
    int index = 1;

    while (PE_LoadU32(entry) != 0) {
        if (PE_LoadU32(entry) == (uint32_t)a0 &&
            func_80072A54(a1, entry + TEXT_NAME_OFFSET) == 0)
            return index;
        entry += TEXT_ENTRY_STRIDE;
        index++;
    }
    return -1;
}

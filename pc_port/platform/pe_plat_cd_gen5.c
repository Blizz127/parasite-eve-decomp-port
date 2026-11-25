/* Six more small libcd / libds callers (guest 0x8007FB44, 0x8007FCBC,
 * 0x80080CDC, 0x8008227C, 0x80082204, 0x8007F608), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen5, checked by pc_port/tools/pe_spu_caller_check.c), with no SDK
 * source.  Each function's behaviour is exactly the guest loads/stores,
 * calls and return value seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* Guest data words read or written by the callers. */
#define ADDR_CD_CMD_STATE   0x8009B598u  /* signed; a command is refused when > 0 */
#define ADDR_CD_CMD_READY   0x8009B554u  /* 0x8007FB44 needs this nonzero */
#define ADDR_CD_STAGE       0x8009B574u  /* 0x8007FB44 needs 1, then stores 2 */
#define ADDR_CD_ISSUE_A     0x8009B570u  /* set when a command is issued */
#define ADDR_CD_ISSUE_B     0x8009B578u  /* set when 0x8007FB44 issues */
#define ADDR_DS_LAST_POS    0x8009B6ECu  /* position stored by 0x80082204 */
#define ADDR_DS_PEND_1      0x800A3530u  /* first pending flag of 0x8007F608 */
#define ADDR_DS_PEND_2      0x800A3520u  /* second pending flag of 0x8007F608 */
#define ADDR_DS_ID_1        0x800A3534u  /* byte returned when the first is set */
#define ADDR_DS_ID_2        0x800A3524u  /* byte returned when the second is set */
#define ADDR_DS_SRC_1       0x800A3535u  /* copy source when the first is set */
#define ADDR_DS_SRC_2       0x800A3525u  /* copy source when the second is set */

/* Guest code address observed in the vectors as a call argument. */
#define CB_DS_READY         0x80081E70u  /* a0 of 0x800824C8 from 0x8008227C */

/* Guest code address observed in the vectors as the fourth argument of
 * 0x8007F0C8 from 0x80082204 (the guest address of 0x8008227C). */
#define DS_TEXT_490_DONE_CB 0x8008227Cu
/* Fifth argument of 0x8007F0C8 from 0x80082204: always 0xFFFFFFFF. */
#define DS_TEXT_490_NO_BUF  0xFFFFFFFFu

#define CD_ISSUE_A_VALUE    0x1Fu  /* value stored at ADDR_CD_ISSUE_A by 0x8007FB44 */
#define CD_ISSUE_A_SIMPLE   0x20u  /* value stored at ADDR_CD_ISSUE_A by 0x8007FCBC */
#define CD_STAGE_IDLE       1u     /* ADDR_CD_STAGE value that allows issue */
#define CD_STAGE_ISSUED     2u     /* ADDR_CD_STAGE value stored on issue */
#define CD_ISSUE_B_VALUE    0xBu   /* value stored at ADDR_CD_ISSUE_B */
#define DS_PEND_SET         1u     /* a pending flag that triggers its copy */
#define DS_TEXT_SELECT      2u     /* 0x8008227C acts only for this argument */

/* 0x8007FB44: refused (0) when the signed word at 0x8009B598 is positive,
 * when the word at 0x8009B554 is zero, or when the word at 0x8009B574 is not
 * 1.  Otherwise store 0x1F / 2 / 0xB to 0x8009B570 / 0x8009B574 / 0x8009B578
 * (before the call), then return 0x8007FCFC(cmd & 0xFF, data). */
int pe_plat_cd_try_issue_cmd(uint32_t cmd, uint32_t data)
{
    if ((int32_t)PE_LoadU32(ADDR_CD_CMD_STATE) > 0)
        return 0;
    if (PE_LoadU32(ADDR_CD_CMD_READY) == 0)
        return 0;
    if (PE_LoadU32(ADDR_CD_STAGE) != CD_STAGE_IDLE)
        return 0;
    PE_StoreU32(ADDR_CD_ISSUE_A, CD_ISSUE_A_VALUE);
    PE_StoreU32(ADDR_CD_STAGE, CD_STAGE_ISSUED);
    PE_StoreU32(ADDR_CD_ISSUE_B, CD_ISSUE_B_VALUE);
    return func_8007FCFC(cmd & 0xFFu, data);
}

/* 0x8007FCBC: refused (0) when the signed word at 0x8009B598 is positive.
 * Otherwise store 0x20 to 0x8009B570 and return
 * 0x8007FCFC(a0 & 0xFF, a1). */
int pe_plat_cd_7fcbc(int a0, int a1)
{
    if ((int32_t)PE_LoadU32(ADDR_CD_CMD_STATE) > 0)
        return 0;
    PE_StoreU32(ADDR_CD_ISSUE_A, CD_ISSUE_A_SIMPLE);
    return func_8007FCFC((uint32_t)a0 & 0xFFu, (uint32_t)a1);
}

/* 0x80080CDC: get the current-position record pointer from 0x8007FC28 and
 * copy its first four bytes to dst (dst may be unaligned).  Returns dst;
 * for a NULL dst it only makes the call and returns the call's result. */
pe_plat_guest_addr pe_plat_ds_last_pos(pe_plat_guest_addr dst)
{
    uint32_t src = func_8007FC28();
    uint32_t word;
    pe_plat_guest_addr lo = dst & ~3u;
    unsigned shift = (dst & 3u) * 8u;

    /* A NULL dst: the call is still made, but nothing is read or written
     * and the callee's result is returned instead of dst. */
    if (dst == 0)
        return src;
    word = PE_LoadU32(src);
    if (shift == 0) {
        PE_StoreU32(dst, word);
    } else {
        /* Unaligned: the original stored through both aligned words that
         * the four bytes touch, preserving their other bytes. */
        uint32_t keep_hi = PE_LoadU32(lo + 4u);
        uint32_t keep_lo = PE_LoadU32(lo);
        uint32_t mask_lo = 0xFFFFFFFFu >> (32u - shift);
        uint32_t mask_hi = 0xFFFFFFFFu << shift;

        PE_StoreU32(lo + 4u, (keep_hi & mask_hi) | (word >> (32u - shift)));
        PE_StoreU32(lo, (keep_lo & mask_lo) | (word << shift));
    }
    return dst;
}

/* 0x8008227C: for an argument of 2, call 0x800824C8 with the ready handler.
 * Any other argument does nothing.  (The original also returned the
 * callee's result for 2 and the constant 2 otherwise; the void prototype
 * drops both.) */
void pe_plat_ds_text_508(unsigned char a0)
{
    if (a0 == DS_TEXT_SELECT)
        func_800824C8((int)CB_DS_READY);
}

/* 0x80082204: call 0x800824C8(0); loc = 0x8007FC28(); store the result of
 * 0x80080C48(loc) to 0x8009B6EC; then call 0x8007FC18() (a byte), fetch a
 * fresh loc = 0x8007FC28(), call 0x8007FC44() (a byte) and finish with
 * 0x8007F0C8(mode, loc, count, DS_TEXT_490_DONE_CB, DS_TEXT_490_NO_BUF)
 * where mode is the low 8 bits of the 0x8007FC18 result and count is the low
 * 8 bits of the 0x8007FC44 result.  (The original returned the 0x8007F0C8
 * result; the void prototype drops it.)  Unconditional. */
void pe_plat_ds_text_490(void)
{
    uint32_t loc;
    int32_t pos;
    uint8_t mode;
    uint8_t count;

    func_800824C8(0);
    loc = func_8007FC28();
    pos = func_80080C48(loc);
    PE_StoreU32(ADDR_DS_LAST_POS, (uint32_t)pos);
    mode = func_8007FC18();
    loc = func_8007FC28();
    count = func_8007FC44();
    (void)func_8007F0C8(mode, loc, count, DS_TEXT_490_DONE_CB,
                        DS_TEXT_490_NO_BUF);
}

/* 0x8007F608: call 0x8007FC88(0) first.  When the word at 0x800A3530 is 1,
 * read the byte at 0x800A3534, clear the word and copy 8 bytes from
 * 0x800A3535 to a0 via 0x80080998; otherwise when the word at 0x800A3520 is
 * 1, do the same with the byte at 0x800A3524 and source 0x800A3525.  Returns
 * that byte (0..255), or 0 when neither word is 1. */
int pe_plat_ds_ready(int a0)
{
    int id;

    func_8007FC88(0);
    if (PE_LoadU32(ADDR_DS_PEND_1) == DS_PEND_SET) {
        id = (int)PE_LoadU8(ADDR_DS_ID_1);
        PE_StoreU32(ADDR_DS_PEND_1, 0);
        func_80080998((uint32_t)a0, ADDR_DS_SRC_1);
        return id;
    }
    if (PE_LoadU32(ADDR_DS_PEND_2) == DS_PEND_SET) {
        id = (int)PE_LoadU8(ADDR_DS_ID_2);
        PE_StoreU32(ADDR_DS_PEND_2, 0);
        func_80080998((uint32_t)a0, ADDR_DS_SRC_2);
        return id;
    }
    return 0;
}

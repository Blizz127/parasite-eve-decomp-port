/* libcard "MemCard_TimerCallback" (guest 0x80082B70), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen25; vectors.txt sha256
 * 6b93e36a3f219b030ffa2e57bcb9f66d37cb1dd924aad3160f2d6a5b6612cd2d, 6000
 * cases, checked by pc_port/tools/pe_spu_caller_check.c), with no SDK source.
 * An independent re-implementation: behaviour is exactly the guest loads,
 * stores and callee/slot calls seen in the vectors.
 *
 * The routine runs on the card timer tick.  It raises a flag word, then
 * advances two tick counters that saturate at 150 (signed compare): the first
 * when the "first record" word is nonzero, the second when the "last record"
 * word is zero.  If the enable word is nonzero and first <= last (signed), it
 * works on a range of 0xF0-byte records: it clears a step word, stores the
 * first record index as the current index, calls the sector routine on that
 * record and, when that returns 0, reports 0xFFFF to the error hook.  After
 * clearing a state word it then repeatedly (while current <= last, signed,
 * both words re-read each pass) calls the transfer step on the record
 * selected by the current index; the callees advance the words.  Finally it
 * stores the halfword 0x88 at offset 0xE of the block the SIO pointer word
 * points to.  The return value is always 0 (a leftover register in the
 * original, never used by the caller). */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_plat/pad.h"

#define ADDR_FIRST_REC   0x8009B774u  /* first record index (signed) */
#define ADDR_LAST_REC    0x8009B778u  /* last record index (signed) */
#define ADDR_COUNT_A     0x800A5AC0u  /* tick counter, bumped when first != 0 */
#define ADDR_COUNT_B     0x800A5AC4u  /* tick counter, bumped when last == 0 */
#define ADDR_ENABLE      0x8009B75Cu
#define ADDR_REC_BASE    0x8009B758u  /* record array; only added to here */
#define ADDR_CUR_REC     0x8009B764u  /* current record index (signed) */
#define ADDR_STEP_WORD   0x8009B768u  /* cleared before the sector call */
#define ADDR_STATE_WORD  0x8009B76Cu  /* cleared before the transfer loop */
#define ADDR_FLAG        0x8009B78Cu  /* set to 1 on every tick */
#define ADDR_SIO_PTR     0x8009B788u  /* pointer to the SIO register block */
#define ADDR_ERROR_HOOK  0x8009B724u  /* one argument */

#define REC_SIZE         0xF0u
#define COUNT_LIMIT      150          /* counters advance while below this (signed) */
#define ERR_SECTOR_ZERO  0xFFFFu      /* argument of the error hook */
#define SIO_HALF_OFFSET  0xEu
#define SIO_HALF_VALUE   0x88u

int func_80082E00(uint32_t record);   /* sector routine: existing hand port */

static void bump_counter(uint32_t addr)
{
    int32_t n = (int32_t)PE_LoadU32(addr);
    if (n < COUNT_LIMIT)
        PE_StoreU32(addr, (uint32_t)n + 1u);
}

int pe_plat_card_timer_callback(void)
{
    int32_t first, last, cur;

    PE_StoreU32(ADDR_FLAG, 1u);
    if (PE_LoadU32(ADDR_FIRST_REC) != 0u)
        bump_counter(ADDR_COUNT_A);
    if (PE_LoadU32(ADDR_LAST_REC) == 0u)
        bump_counter(ADDR_COUNT_B);

    if (PE_LoadU32(ADDR_ENABLE) == 0u)
        return 0;

    first = (int32_t)PE_LoadU32(ADDR_FIRST_REC);
    last = (int32_t)PE_LoadU32(ADDR_LAST_REC);
    if (first > last)
        return 0;

    PE_StoreU32(ADDR_STEP_WORD, 0u);
    PE_StoreU32(ADDR_CUR_REC, (uint32_t)first);
    if (func_80082E00(PE_LoadU32(ADDR_REC_BASE) + (uint32_t)first * REC_SIZE) == 0)
        PE_Card_ICall("D_8009B724 (card error hook)", PE_LoadU32(ADDR_ERROR_HOOK), 1,
                      ERR_SECTOR_ZERO, 0, 0);
    PE_StoreU32(ADDR_STATE_WORD, 0u);

    for (;;) {
        cur = (int32_t)PE_LoadU32(ADDR_CUR_REC);
        if (cur > (int32_t)PE_LoadU32(ADDR_LAST_REC))
            break;
        pe_plat_pad_transfer_step(PE_LoadU32(ADDR_REC_BASE) + (uint32_t)cur * REC_SIZE);
    }

    PE_StoreU16(PE_LoadU32(ADDR_SIO_PTR) + SIO_HALF_OFFSET, SIO_HALF_VALUE);
    return 0;
}

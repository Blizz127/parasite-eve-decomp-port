/* Memory-card card-object methods (guest 0x80083F04, 0x80083F44 and
 * 0x8008401C), written clean-room from black-box vectors of the original
 * code (pc_port/tools/pe_cd_leaf_vectors.py cdgen20, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the guest loads, stores, callback-word calls and SIO callee calls
 * seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"

#define ADDR_SLOT_3C        0x8009B73Cu  /* word: guest function pointer, one argument */
#define ADDR_SLOT_50        0x8009B750u  /* word: guest function pointer, no arguments */
#define ADDR_SLOT_54        0x8009B754u  /* word: guest function pointer, no arguments */
#define ADDR_STATE_60       0x8009B760u  /* word: tested for non-zero (command path) */
#define ADDR_STATE_64       0x8009B764u  /* word: compared with the word at 0x8009B774 */
#define ADDR_STATE_70       0x8009B770u  /* word: byte-write argument (write path) */
#define ADDR_STATE_74       0x8009B774u  /* word: compared with the word at 0x8009B764 */
#define ADDR_RESULT_9C      0x8009B79Cu  /* word: written by the write path */
#define ADDR_STATE_A4       0x8009B7A4u  /* word: written by sync, tested for non-zero elsewhere */

#define OBJ_BASE_OFF        0x0Cu   /* word: base address added to the offsets below */
#define OBJ_BYTE_OFF        0x36u   /* byte: tested for zero */

#define SYNC_EXCHANGE_ARG   0xFFFFFFFEu  /* second argument of the exchange call in sync */
#define CMD_DEFAULT_BYTE    0x42u        /* byte argument of the command write when obj+0x36 is 0 */
#define CMD_BASE_STEP       0xF0u        /* second slot call's argument offset (command) */
#define WR_BASE_STEP_A      0x1E0u       /* first slot call's argument offset (write) */
#define WR_BASE_STEP_B      0x2D0u       /* second slot call's argument offset (write) */

#define WR_FAIL_RESULT      0xFFFFFFF7u  /* returned when the write result is below 0x10 */
#define WR_LOW_LIMIT        0x10u        /* low-byte threshold of the write result */
#define WR_LOW_MASK         0x0Fu        /* low nibble selects the stored value */
#define WR_ZERO_STORE_FIRST 0x00u        /* nibble 0: store this ... */
#define WR_ZERO_STORE_NEXT  0x20u        /* ... and then this */

/* 0x80083F04: call the word at 0x8009B73C with (obj), store its result to
 * 0x8009B7A4, then return func_800830DC(obj, 0xFFFFFFFE). */
int pe_plat_card_obj_sync(uint32_t obj)
{
    uint32_t fn = PE_LoadU32(ADDR_SLOT_3C);
    int r = PE_GuestCall("@80083F04:D_8009B73C", fn, 1, obj, 0, 0, 0);

    PE_StoreU32(ADDR_STATE_A4, (uint32_t)r);
    return func_800830DC(obj, (int)SYNC_EXCHANGE_ARG);
}

/* 0x80083F44: when the words at 0x8009B764 and 0x8009B774 are equal and the
 * word at 0x8009B760 is non-zero, call the words at 0x8009B754 and
 * 0x8009B750 (each with no arguments).  Then, when the word at 0x8009B7A4 is
 * non-zero, read the base word at obj+0xC and call the word at 0x8009B73C
 * with base and with base+0xF0 (the word is re-read for each call).  Finally
 * return func_800832B4(obj, byte), where byte is the byte at obj+0x36, or
 * 0x42 when that byte is zero. */
int pe_plat_card_obj_command(uint32_t obj)
{
    uint32_t byte;
    uint32_t base, fn;

    if (PE_LoadU32(ADDR_STATE_64) == PE_LoadU32(ADDR_STATE_74)) {
        if (PE_LoadU32(ADDR_STATE_60) != 0u) {
            fn = PE_LoadU32(ADDR_SLOT_54);
            (void)PE_GuestCall("@80083F44:D_8009B754", fn, 0, 0, 0, 0, 0);
            fn = PE_LoadU32(ADDR_SLOT_50);
            (void)PE_GuestCall("@80083F44:D_8009B750", fn, 0, 0, 0, 0, 0);
        }
    }
    if (PE_LoadU32(ADDR_STATE_A4) != 0u) {
        base = PE_LoadU32(obj + OBJ_BASE_OFF);
        fn = PE_LoadU32(ADDR_SLOT_3C);
        (void)PE_GuestCall("@80083F44:(*host_D_8009B73C)", fn, 1, base, 0, 0, 0);
        base = PE_LoadU32(obj + OBJ_BASE_OFF);
        fn = PE_LoadU32(ADDR_SLOT_3C);
        (void)PE_GuestCall("@80083F44:(*host_D_8009B73C)", fn, 1, base + CMD_BASE_STEP, 0, 0, 0);
    }
    byte = PE_LoadU8(obj + OBJ_BYTE_OFF);
    if (byte == 0u)
        byte = CMD_DEFAULT_BYTE;
    return func_800832B4(obj, (int)byte);
}

/* 0x8008401C: when the word at 0x8009B7A4 is non-zero, read the base word at
 * obj+0xC and call the word at 0x8009B73C with base+0x1E0 and base+0x2D0
 * (the word is re-read for each call).  Then call func_800832B4(obj, arg),
 * where arg is the word at 0x8009B770 when the byte at obj+0x36 is zero and
 * 0 otherwise.  With result r:
 *  - r negative: return r;
 *  - low byte of r below 0x10: return 0xFFFFFFF7;
 *  - otherwise store ((r & 0xF) << 1) to 0x8009B79C (0 then 0x20 when that is
 *    0) and return 0. */
int pe_plat_card_obj_write(uint32_t obj)
{
    uint32_t base, fn, arg, low;
    int r;

    if (PE_LoadU32(ADDR_STATE_A4) != 0u) {
        base = PE_LoadU32(obj + OBJ_BASE_OFF);
        fn = PE_LoadU32(ADDR_SLOT_3C);
        (void)PE_GuestCall("@8008401C:(*host_D_8009B73C)", fn, 1, base + WR_BASE_STEP_A, 0, 0, 0);
        base = PE_LoadU32(obj + OBJ_BASE_OFF);
        fn = PE_LoadU32(ADDR_SLOT_3C);
        (void)PE_GuestCall("@8008401C:(*host_D_8009B73C)", fn, 1, base + WR_BASE_STEP_B, 0, 0, 0);
    }
    arg = 0u;
    if (PE_LoadU8(obj + OBJ_BYTE_OFF) == 0u)
        arg = PE_LoadU32(ADDR_STATE_70);
    r = func_800832B4(obj, (int)arg);
    if (r < 0)
        return r;
    low = (uint32_t)r & 0xFFu;
    if (low < WR_LOW_LIMIT)
        return (int)WR_FAIL_RESULT;
    low = (uint32_t)r & WR_LOW_MASK;
    if (low == 0u) {
        PE_StoreU32(ADDR_RESULT_9C, WR_ZERO_STORE_FIRST);
        PE_StoreU32(ADDR_RESULT_9C, WR_ZERO_STORE_NEXT);
    } else {
        PE_StoreU32(ADDR_RESULT_9C, low << 1);
    }
    return 0;
}
